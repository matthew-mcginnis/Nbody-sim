# N-Body Simulator — Data Output Schema (v8, final)

Two data files for the research pipeline: `energy.csv` (per-timestep physics) and
`benchmarks.csv` (per-run timing). Environment info rides along as a plain-text sidecar
(`run_meta.txt`), not a data file. Visualization is a SEPARATE program that is not part
of this pipeline and emits its own trajectory data independently.

**Changelog from v7 (patch — three holes closed, two warnings added)**
- **Benchmarks rule #1 added (Hole 1, the big one):** benchmark runs execute with energy
  computation DISABLED entirely; energy runs are separate executions of the same
  configuration. The potential sum is itself an O(N²) pass — leaving it on during timed
  runs ~doubles wall_time (corrupting Q1/Q2), taxes FP32 runs disproportionately if the
  energy pass is FP64 (biasing Q2's ratio), and shifts Q1's crossover by an unfindable
  constant if CPU/GPU builds differ. The two-file design already supports this; now the
  schema states it.
- **Sweep directory convention added (Hole 2):** each sweep's CSVs + run_meta.txt live in
  one timestamped directory; run_meta records the git commit; rows are never merged
  across directories without carrying a `sweep_id`. (Chosen over a per-row run_id column
  — no schema change needed.)
- **IC sanity check added (Hole 3):** initial conditions must have |total₀| well away
  from zero (bound systems are safe by default; KE ≈ |PE| ICs explode relative_drift).
  Check at generation time.
- **Warning A (rule 5 corollary):** scale the energy-sampling interval k with step count
  (target ~10⁴ logged rows per run), final row force-logged regardless — the deep FP32
  dt-sweep rungs otherwise produce multi-million-row files and become I/O-bound.
- **Warning B (Q4 sweep device scope):** decide when picking T and N whether the Q4 sweep
  runs on both devices or GPU-only. Deep FP32 rungs on single-threaded CPU at large N may
  take days; Q4's claim is about consumer Blackwell — CPU curves are optional context.

**Changelog from v6**
- **CRITICAL — `device` added to energy.csv and to the Q4 join key.** The v6 join tuple
  could not distinguish a CPU run from a GPU run of the same configuration. "Same physics,
  same drift" is false in exactly Q4's regime: CPU and GPU sum forces in different orders,
  floating-point addition is not associative, and near FP32's ~1e-7 roundoff floor the
  drift trajectories genuinely diverge. Q4's GPU points must use GPU energy data. Join
  tuple is now (N, dt, precision, integrator, epsilon, seed, ic_type, **device**).
- **Determinism note amended:** same seed → identical energy trajectory **per device**
  only. Cross-device trajectories differ at the roundoff level by design.
- **"Four rules" → "Five rules"** (header said four, listed five). *(v8 note: now six —
  IC sanity check added as rule 6.)*
- **T divisibility rule added to dt-sweep:** pick dt₀ so T/dt₀ is an integer; geometric
  halving then keeps every step count an exact integer and every run lands on exactly T.
  Final-row guarantee becomes belt-and-suspenders instead of load-bearing.
- **Warmup rep rule added to benchmarks:** run one untimed GPU warmup before rep 0 (or
  discard rep 0 in analysis — pick one and state it; this schema picks untimed warmup).
- **Scope consequence noted (no schema change):** with tiling cut there is exactly one
  GPU implementation, so Q1's crossover and Q2's penalty are properties of the naive
  kernel. Framing obligation ("crossover between these two implementations; a sharper
  kernel would move it") lives in the paper's Discussion, not this file.

**Changelog from v5 (Q4 support — fixed-budget precision inversion)**
- **Final-row guarantee (new rule, energy.csv):** every run integrates to the same fixed
  physical end-time T, and the LAST logged row of every run is exactly the run's final
  state. Q4's "drift-at-T" is read as *the last row of the run* — never by matching
  `time == T`, which fails for floating-point time across different dt values.
- **Q4 join key (new section):** spelled out explicitly — a benchmarks.csv row joins to
  its energy.csv run on the shared tuple (N, dt, precision, integrator, epsilon, seed,
  ic_type); wall_time from benchmarks.csv, drift-at-T (last row) from energy.csv.
- **Determinism requirement made explicit:** energy.csv has no `rep` column BECAUSE the
  physics must be deterministic — same seed → same bodies → identical energy trajectory.
  IC generation must be seeded deterministically or the single energy run won't match
  its timed repetitions. Do not add a rep dimension to energy.csv.
- **Tiling removed (was cut from scope, schema still referenced it):** dropped
  "gpu_tiled" from config examples; dropped "Naive vs tiled kernel speedup" from the
  figure table.
- **Q4 figure added to figure table:** accuracy-vs-cost curves (drift-at-T vs wall_time),
  one curve per precision, dt swept; the curve crossing is the Q4 boundary. Requires
  BOTH files joined.
- **dt-sweep design note added (Q4):** geometric sweep (halve dt each step), extendable —
  keep halving until FP32's drift plateaus at its roundoff floor, or the crossing can't
  be located. Do not hardcode the dt list.

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
| device         | string | "cpu","rtx5080". Which hardware produced this run.   |
| epsilon        | double | Softening ε.                                          |

**Six rules that keep this data honest:**

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

5. **Fixed end-time T + final-row guarantee (Q4).** Every run in a Q4 sweep integrates to
   the SAME fixed physical end-time T (steps = T/dt, so step count varies with dt — that
   is the point). The last logged row of every run must be exactly the run's final state.
   Q4's "drift-at-T" is read as *the last row of the run* — never by matching `time == T`,
   which fails in floating point when dt doesn't divide T exactly. If you sample energy
   output (every k-th step), still force-log the final step unconditionally.
   **Sampling corollary (deep-sweep file size):** scale k with the step count — target
   ~10⁴ logged rows per run regardless of dt. The deepest FP32 rungs run millions of
   steps; logging every step there makes the most important Q4 runs I/O-bound and
   disk-filling. Final row force-logged regardless of k.

6. **IC sanity check: |total₀| must be well away from zero.** relative_drift divides by
   |total₀|; an IC tuned near KE ≈ |PE| makes the denominator tiny and the metric
   explodes into fake "drift." Bound systems (Plummer, cold collapse) have solidly
   negative total energy and are safe by default — but check |total₀| at IC generation
   time and reject near-zero configurations.

**Momentum check is precision-aware.** FP64 CPU sits ~1e-15; FP32 (and naive GPU) sits
~1e-6 — that's correct, not a bug. Never test momentum against a hardcoded 1e-15.

**Why energy.csv has no `rep` column (determinism requirement — PER DEVICE).** The physics
must be deterministic **per device**: same seed → same bodies → identical energy trajectory
every run *on the same hardware*. So energy is logged ONCE per (configuration, device)
while benchmarks.csv times the same configuration 5–10×. Cross-device trajectories differ
at the roundoff level by design (different summation order, non-associative float addition)
— that is why `device` is a column and part of the Q4 join key, not noise to average away.
This only holds if IC generation is seeded deterministically — verify that before relying
on the single energy run to represent its timed repetitions. Do not add a rep dimension
to energy.csv.

---

## File 2 — `benchmarks.csv`
One row per repetition of a run. Drives all performance figures.

| Column           | Type   | Meaning                                             |
|------------------|--------|-----------------------------------------------------|
| config           | string | Label: "cpu_baseline","gpu_naive", …                |
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
- **RULE #1 — Energy computation is DISABLED during benchmark runs.** Computing
  kinetic + potential is not I/O — the potential sum is itself an O(N²) pass, as
  expensive as the force loop. Energy-on timed runs measure ~2× the real simulation
  cost (corrupts Q1's crossover and Q2's penalty); an FP64-only energy pass taxes FP32
  runs disproportionately (biases Q2's headline ratio); CPU/GPU inconsistency shifts the
  crossover by a constant you'll never find. Benchmark runs: energy off. Energy runs:
  separate executions of the same configuration (the Q4 join matches them by tag tuple —
  this is exactly why the two files exist).
- **Time the compute loop only** — exclude file I/O, allocation, IC setup.
- **Repeat every config 5–10×**, one row per rep. No repeats = no error bars. Store raw
  reps; compute median + min/max band in matplotlib (lets you re-summarize without rerunning).
- **`threads` matters** — single-threaded vs OpenMP CPU gives a totally different GPU
  crossover point. Both are valid claims only if you state which. Decide and record it now.
- **GPU warmup: run ONE untimed warmup iteration before rep 0.** The first GPU execution
  eats one-time costs (context creation, clock ramp-up) that would contaminate rep 0.
  This schema's convention: untimed warmup, then all logged reps are clean. (The
  alternative — logging rep 0 and discarding it in analysis — is valid but NOT what this
  pipeline does; state the choice in the paper's methodology, a reviewer will ask.)
- **GPU timing:** use CUDA events (`cudaEventRecord`/`cudaEventElapsedTime`). Host-side
  timing needs `cudaDeviceSynchronize()` before stopping the clock (async launches).
- **CPU timing:** `clock_gettime(CLOCK_MONOTONIC, …)` around the loop.
- `wall_time_sec` (real time) ≠ `energy.csv` `time` (simulation time). Never conflate.

---

## Q4 join — how the two files combine (the only cross-file computation)
Q4 (fixed-budget precision inversion) is the one question that needs BOTH files joined.
The join is explicit — do not infer it:

- **Join key:** a benchmarks.csv row matches its energy.csv run on the shared tuple
  **(N, dt, precision, integrator, epsilon, seed, ic_type, device)**. `device` is
  mandatory in the key: CPU and GPU runs of the same configuration have genuinely
  different drift trajectories near the FP32 roundoff floor (non-associative summation,
  different order) — Q4's GPU points must join to GPU energy data.
- **From benchmarks.csv:** `wall_time_sec` (aggregate the 5–10 reps to a median in
  matplotlib; keep min/max for the error band).
- **From energy.csv:** drift-at-T = `relative_drift` of the **last row** of the matched
  run (guaranteed final state by rule 5).
- **Plot:** |drift-at-T| vs median wall_time, one point per dt value, one curve per
  precision. The FP32/FP64 curve crossing IS the Q4 boundary.

**dt-sweep design (required for the crossing to exist in the data):** sweep dt
geometrically (halve each step), 6–8 values per precision to start, and keep the sweep
EXTENDABLE — continue halving until FP32's drift stops improving (its ~1e-7 roundoff
floor). Both regimes — the truncation-dominated slope and the roundoff plateau — must be
visible for at least FP32, or the crossing cannot be located. Do not hardcode the dt list.
**Pick dt₀ (the coarsest dt) so that T/dt₀ is an integer.** Geometric halving then keeps
steps an exact integer for the entire sweep and every run lands on exactly T — the
final-row guarantee becomes belt-and-suspenders instead of load-bearing.
All runs in the sweep share the same T (rule 5), same N, same integrator, same ε, same
seed/ic_type — dt, precision, and device are the ONLY variables, and each (dt, precision,
device) triple joins to its own energy run.
**Device scope — decide when picking T and N, not later:** the schema permits the sweep on
both devices, but the deepest FP32 rungs on a single-threaded CPU at large N may take days
per run. Q4's claim is about consumer Blackwell; CPU curves are optional context. Choose
GPU-only or both consciously, or the sweep design will decide for you by being unrunnable.

---

## Sidecar — `run_meta.txt`
Plain text, written once per run/sweep by a wrapper script (git + `nvidia-smi` +
`/proc/cpuinfo`), so nothing is hand-typed. Not a data file — just the environment behind
the numbers. Capture: git commit, timestamp, compiler + version, opt flags, **fast_math
(true/false)**, CUDA version, driver version, CPU model, GPU model, OS.

**Sweep directory convention (what links a CSV row to its environment):** the wrapper puts
each sweep's CSVs and its run_meta.txt together in ONE timestamped directory; run_meta
records the git commit. Rows are NEVER merged across directories without adding a
`sweep_id` column carrying the source directory's identity. Without this, a September
re-sweep with different flags concatenated onto August data has no column saying which
environment produced which row — and the tag-column design actively encourages
concatenation.

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
| **Q4: accuracy-vs-cost curves** (drift-at-T vs wall_time, one curve per precision; crossing = boundary) | **both files, joined** |
| Performance error bars                  | benchmarks.csv (rep) |
| Reproducibility appendix                | run_meta.txt      |

---

## Build order
- `energy.csv` writer now, with the CPU code. Same sitting: verify the force/potential
  gradient pair, log momentum, compute drift in double, log step 0 first, implement
  the final-row guarantee (rule 5) with scaled sampling, and add the |total₀| IC check
  (rule 6).
- **Build the energy-off switch now** (compile flag or runtime flag): benchmark runs must
  run with energy computation disabled entirely (benchmarks rule #1). Same binary
  configuration, two modes.
- `benchmarks.csv` writer + repetition/timing harness now (CPU); extend to CUDA events later.
  Decide single- vs multi-threaded CPU baseline now.
- Verify IC generation is deterministic per seed (required for the no-rep energy.csv model).
- `run_meta.txt` wrapper before the real benchmark sweep (August) — wrapper creates the
  timestamped sweep directory and drops CSVs + run_meta together (Hole 2 convention).
- Pick T for the Q4 sweep when the sweep is designed — same T for every run in it, T/dt₀
  an exact integer, and decide the sweep's device scope (GPU-only vs both) at the same time.
