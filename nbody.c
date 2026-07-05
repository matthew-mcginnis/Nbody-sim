#include <stdio.h>
#include <math.h>
#include <string.h>
#define N 3
#define G 6.674e-11
    typedef struct {
        double x, y, z, vx, vy, vz, mass;
    } particle;


int main()
{
    int n = 0;
    printf("How many Bodies?");
    scanf("%i", &n);
    particle bod[n];
    memset(bod, 0, sizeof(bod));
    for(int i = 0; i < n; i++){

    printf("Particle %d: (x y z vx vy vz mass)\n", i+1);
    scanf(" %lf %lf %lf %lf %lf %lf %lf", &bod[i].x, &bod[i].y, &bod[i].z, &bod[i].vx, &bod[i].vy, &bod[i].vz, &bod[i].mass);
       }

    double ax[n];
    double ay[n];
    double az[n];

    double dt = 0.1;
    int steps = 100;
    for(int p = 0; p < steps; p++){
        

    memset(ax, 0, sizeof(ax));
    memset(ay, 0, sizeof(ay));
    memset(az, 0, sizeof(az));


    for(int i = 0; i < n; i++){
        for(int j = i+1; j < n; j++ ){
    double dx = bod[j].x - bod[i].x;
    double dy = bod[j].y - bod[i].y;
    double dz = bod[j].z - bod[i].z;
    double dist = sqrt(dx*dx + dy*dy + dz*dz + 0.001*0.001); //0.001 is the softening factor, keeps objects creating numerical singularities

    double force = G * (bod[i].mass * bod[j].mass) / (dist*dist);

    double fx = force * (dx / dist);
    double fy = force * (dy / dist); //force calculator
    double fz = force * (dz / dist);

    ax[i] += fx / bod[i].mass;
    ax[j] -= fx / bod[j].mass;
    ay[i] += fy / bod[i].mass;
    ay[j] -= fy / bod[j].mass;
    az[i] += fz / bod[i].mass;
    az[j] -= fz / bod[j].mass;
        }
    }
    for(int i = 0; i < n; i++){
    bod[i].vx += ax[i] * dt;
    bod[i].vy += ay[i] * dt;
    bod[i].vz += az[i] * dt;
    bod[i].x += bod[i].vx * dt;
    bod[i].y += bod[i].vy * dt;
    bod[i].z += bod[i].vz * dt;
    }
    for(int i = 0; i < n; i++){
        printf("%i velocity: %lf %lf %lf\n", i+1, bod[i].vx, bod[i].vy, bod[i].vz);
        printf("%i position: %lf %lf %lf\n", i+1, bod[i].x, bod[i].y, bod[i].z);
        
    }
}

    return 0;

}


    

    
    

