#include <stdio.h>
#include <math.h>

    typedef struct {
        double x, y, z, vx, vy, vz, mass;
    } particle;

    void apply_gravity(particle *a, particle *b, double dt);
int main()
{
    particle p1, p2;
    
    printf("Particle 1: (x y z vx vy vz mass)\n");
    scanf(" %lf %lf %lf %lf %lf %lf %lf", &p1.x, &p1.y, &p1.z, &p1.vx, &p1.vy, &p1.vz, &p1.mass);

    printf("Particle 2: (x y z vx vy vz mass)\n");
    scanf(" %lf %lf %lf %lf %lf %lf %lf", &p2.x, &p2.y, &p2.z, &p2.vx, &p2.vy, &p2.vz, &p2.mass);

    double dt = 0.1;
    int steps = 100;
    for(int i = 0; i < steps; i++){

    
        apply_gravity(&p1, &p2, dt);
        printf("P1 velocity: %lf %lf %lf\n", p1.vx, p1.vy, p1.vz);
        printf("P2 velocity: %lf %lf %lf\n", p2.vx, p2.vy, p2.vz);
        printf("P1 position: %lf %lf %lf\n", p1.x, p1.y, p1.z);
        printf("P2 position: %lf %lf %lf\n", p2.x, p2.y, p2.z);
    }
    return 0;

}

void apply_gravity(particle *a, particle *b, double dt){
    double dx = b->x - a ->x;
    double dy = b->y - a ->y;
    double dz = b->z - a ->z;
    double dist = sqrt(dx*dx + dy*dy + dz*dz +0.001); //0.001 is the softening factor, keeps objects from colliding

    double G = 6.674e-11;
    double force = G * (a->mass * b->mass) / (dist*dist);

    double fx = force * (dx / dist);
    double fy = force * (dy / dist);
    double fz = force * (dz / dist);

    a->vx += (fx / a->mass) * dt;
    a->vy += (fy / a->mass) * dt;
    a->vz += (fz / a->mass) * dt;
    b->vx -= (fx / b->mass) * dt;
    b->vy -= (fy / b->mass) * dt;
    b->vz -= (fz / b->mass) * dt;
    a->x += a->vx * dt;
    a->y += a->vy * dt;
    a->z += a->vz * dt;
    b->x += b->vx * dt;
    b->y += b->vy * dt;
    b->z += b->vz * dt;
    
}

