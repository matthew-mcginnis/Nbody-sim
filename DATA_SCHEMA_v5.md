# N-Body Simulator — Data Output Schema (v5)

Two data files for the research pipeline: `energy.csv` (per-timestep physics) and
`benchmarks.csv` (per-run timing). Environment info rides along as a plain-text sidecar
(`run_meta.txt`), not a data file. Visualization is a SEPARATE program that is not part
of this pipeline and emits its own trajectory data independently.

**Changelog from v3**
- Dropped `trajectory.csv` — belongs to the separate visualizer, not the research code.
- Dropped `bodies.csv` — mass folded back in; the separate file wasn't worth it.
- `run_meta.csv` → `run_meta.txt` — environment capture is a one-time text sidecar, not
  a CSV data file.
- Unit system fixed: **G = 1** (natural units) everywhere. SI only if a real-system run
  is ever added.
- Figure-8 references removed — it was a development test, not paper content.

**Units: G = 1 (natural units), fixed for every run.**
Masses, lengths, times are in units where gravity's constant is 1, so F = m₁m₂/r². This
keeps all numbers near order-1, which is cleanest for the FP32/FP64 precision study (SI's
huge/tiny values would inject their own roundoff). `relative_drift` is dimensionless and
unaffected either way; raw energies are only interpretable *because* the unit system is
stated here. State this once in the paper's setup section.

**Float formatting:** write every float with `%.17g` (full double round-trip). `%f` gives
6 decimals, truncates, and makes conservation look like it's drifting when it isn't. In
the FP32 build, compute in FP32 but promote to double for the `%.17g` write.

---

## Glossary (know every one of these before logging it)
- **epsilon (ε), softening factor** — small constant added inside distance as √(r²+ε²) so
  force never divides by zero when bodies get close. Prevents blow-ups; physically treats
  masses as small fuzzy balls, not infinite-density points. Bigger ε = more stable, less
  accurate up close. ε = 0 = true point masses (singular if they collide).
- **total energy** — kinetic + potential. Should stay constant in reality; how much it
  doesn't is your accuracy signal.
- **relative_drift** — (total − total₀)/|total₀|. Dimensionless fractional change in total
  energy from step 0. Your headline accuracy metric. ~0 is perfect.
- **momentum (px,py,pz)** — total system momentum. Conserved to roundoff by Newton's 3rd
  law regardless of integrator, so it's a pure force-loop correctness check (not an
  accuracy check).
- **integrator** — the stepping scheme. `euler` (drifts, baseline), `euler_cromer` (bounded
  oscillation), `verlet` (best conservation).
- **dt** — timestep size. Smaller = more accurate, more steps. The dominant control on drift.
- **seed** — RNG seed for random initial conditions, so a run is reproducible from its own
  data. Deterministic ICs log −1.

---

## File 1 — `energy.csv`
One row per timestep. Whole-system quantities. Drives all conservation figures.

| Column         | Type   | Meaning                                              |
|----------------|--------|------------------------------------------------------|
| step           | int    | Iteration counter (0-indexed).                       |
| time           | double | Simulation time = step × dt (physics clock).         |
| N              | int    | Number of bodies.                                    |
| dt             | double | Timestep size.                                       |
| ic_type        | string | Initial-condition family: "plummer","uniform", etc.  |
| seed           | int    | RNG seed (−1 if IC is deterministic).                |
| kinetic        | double | Σ ½mᵢvᵢ²                                             |
| potential      | double | Σ over pairs: −mᵢmⱼ/√(r²+ε²)  (G = 1)               |
| total          | double | kinetic + potential.                                 |
| relative_drift | double | (total − total₀)/|total₀|. Computed in double always.|
| px,py,pz       | double | Total momentum components (force-loop check).        |
| integrator     | string | "euler","euler_cromer","verlet".                     |
| precision      | string | "fp32","fp64".                                        |
| epsilon        | double | Softening ε.                                          |

**Four rules that keep this data honest:**

1. **Force must match potential (check before any run).** The acceleration you integrate
   must be the exact gradient of the logged potential, or energy won't conserve even with
   a perfect integrator — and you'd blame the integrator. Consistent pair (G = 1):
   - potential per pair:  U = −mᵢmⱼ / √(r² + ε²)
   - accel on i:  **a**ᵢ = mⱼ **r** / (r² + ε²)^(3/2),  **r** = **r**ⱼ − **r**ᵢ
   Soften both magnitude AND direction (same √(r²+ε²) throughout). The momentum check
   confirms you got this right.

2. **relative_drift computed in double, even in FP32 builds.** total − total₀ in FP32 loses
   all resolution below ~1e-7 (cancellation), so you'd measure metric noise, not physics.
   Promote to double first.

3. **Log energy at synchronized full-step values.** Evaluate kinetic and potential from x
   and v at the SAME instant, end of step, same call site for all three integrators. Verlet
   trap: log kinetic from the full-step v (after the second half-kick), never the half-step
   v — otherwise Verlet shows fake drift and looks worse than Euler-Cromer (inverted result
   from a logging bug). Euler-Cromer's small bounded energy oscillation is real, not a bug.

4. **Log step 0 before the loop.** That's total₀; every relative_drift measures against it.

**Momentum check is precision-aware.** FP64 CPU sits ~1e-15; FP32 (and naive GPU) sits
~1e-6 — that's correct, not a bug. Never test momentum against a hardcoded 1e-15.

---

## File 2 — `benchmarks.csv`
One row per repetition of a run. Drives all performance figures.

| Column           | Type   | Meaning                                             |
|------------------|--------|-----------------------------------------------------|
| config           | string | Label: "cpu_baseline","gpu_naive","gpu_tiled", …    |
| N                | int    | Number of bodies.                                   |
| precision        | string | "fp32","fp64".                                       |
| integrator       | string | "euler","euler_cromer","verlet".                    |
| device           | string | "cpu","rtx5080".                                     |
| threads          | int    | CPU thread count (1 = single-threaded). Sets crossover.|
| ic_type          | string | Matches energy.csv.                                 |
| seed             | int    | Matches energy.csv.                                 |
| rep              | int    | Repetition index (0..R−1). One row per repeat.      |
| steps            | int    | Timesteps simulated.                                |
| dt               | double | Timestep size.                                      |
| epsilon          | double | Softening ε.                                         |
| wall_time_sec    | double | Wall-clock of the SIMULATION LOOP ONLY.             |
| time_per_step_ms | double | wall_time_sec / steps × 1000.                       |

**Rules:**
- **Time the compute loop only** — exclude file I/O, allocation, IC setup.
- **Repeat every config 5–10×**, one row per rep. No repeats = no error bars. Store raw
  reps; compute median + min/max band in matplotlib (lets you re-summarize without rerunning).
- **`threads` matters** — single-threaded vs OpenMP CPU gives a totally different GPU
  crossover point. Both are valid claims only if you state which. Decide and record it now.
- **GPU timing:** use CUDA events (`cudaEventRecord`/`cudaEventElapsedTime`). Host-side
  timing needs `cudaDeviceSynchronize()` before stopping the clock (async launches).
- **CPU timing:** `clock_gettime(CLOCK_MONOTONIC, …)` around the loop.
- `wall_time_sec` (real time) ≠ `energy.csv` `time` (simulation time). Never conflate.

---

## Sidecar — `run_meta.txt`
Plain text, written once per run/sweep by a wrapper script (git + `nvidia-smi` +
`/proc/cpuinfo`), so nothing is hand-typed. Not a data file — just the environment behind
the numbers. Capture: git commit, timestamp, compiler + version, opt flags, **fast_math
(true/false)**, CUDA version, driver version, CPU model, GPU model, OS.

**`-ffast-math` warning:** it reorders/relaxes float ops and can change conservation
results for reasons unrelated to your integrator — you'd measure a compiler flag and call
it physics. Don't use it for the physics builds. Record either way.

---

## Future — Barnes-Hut
Adds opening angle θ (speed/accuracy tradeoff): add a `theta` column to both files when it
lands. Not now.

---

## Which file drives which figure
| Figure                                  | Source            |
|-----------------------------------------|-------------------|
| Energy drift over time                  | energy.csv        |
| Drift vs dt                             | energy.csv        |
| Euler vs Euler-Cromer vs Verlet         | energy.csv        |
| FP32 vs FP64 drift                      | energy.csv        |
| Drift vs softening ε                    | energy.csv        |
| Momentum conservation (force check)     | energy.csv        |
| CPU vs GPU crossover (log-log)          | benchmarks.csv    |
| FP32 vs FP64 speedup                    | benchmarks.csv    |
| Naive vs tiled kernel speedup           | benchmarks.csv    |
| Performance error bars                  | benchmarks.csv (rep) |
| Reproducibility appendix                | run_meta.txt      |

---

## Build order
- `energy.csv` writer now, with the CPU code. Same sitting: verify the force/potential
  gradient pair, log momentum, compute drift in double, log step 0 first.
- `benchmarks.csv` writer + repetition/timing harness now (CPU); extend to CUDA events later.
  Decide single- vs multi-threaded CPU baseline now.
- `run_meta.txt` wrapper before the real benchmark sweep (August).
