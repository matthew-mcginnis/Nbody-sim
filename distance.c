#include <stdio.h>
#include <math.h>

    typedef struct {
        double x, y, z;
    } particle;

int main()
{
    
    particle p1, p2;
    double distance;
    printf("Particle 1: (x y z)\n");
    scanf(" %lf %lf %lf", &p1.x, &p1.y, &p1.z);

    printf("Particle 2: (x y z)\n");
    scanf(" %lf %lf %lf", &p2.x, &p2.y, &p2.z);

    distance = sqrt(pow(p2.x-p1.x, 2) + pow(p2.y-p1.y, 2) +pow(p2.z-p1.z, 2));
    printf("Distance %lf\n", distance);

    return 0;
}

