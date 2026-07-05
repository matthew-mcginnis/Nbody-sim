#include <stdio.h>
#include <math.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <time.h>

#define G 1
    typedef struct {
        double x, y, z, vx, vy, vz, mass;
    } particle;




double frand(double lo, double hi) {
return lo + (hi - lo) * ((double)rand() / RAND_MAX); 
}

int main()
{

    srand(time(NULL));
    int n;
    int answer;
    double dt = 0.01;
    int cx = 800; int cy = 800;
    double angle = 0;
    double cam = 30; //camera distance; Changable
    double pitch = 0; //tilt
    int drag = 0; //mouse movement
    int pause = 0;

    printf("1 = figure-8, 2 = random\n");
    scanf("%d", &answer);
    int scale;
    if (answer == 1){
        scale = 60;
    }
    else {
        scale = 20;
    }
    if (answer == 2){

    printf("How many Bodies\n");
    scanf("%d", &n);
    }
    else {
        n = 3;
    }
    particle bod[n];

        if(answer ==2){

        memset(bod, 0, sizeof(bod));

            for(int i = 0; i < n; i++){
                bod[i].x = frand(-10, 10);
                bod[i].y = frand(-10, 10);
                bod[i].z = frand(-10, 10);
                bod[i].vx = frand(-.3, 0.3);
                bod[i].vy = frand(-.3, 0.3);
                bod[i].vz = frand(-.3, 0.3);
                bod[i].mass = frand(0.1, 2.5);
                
        }
    }
    else{
    
    memset(bod, 0, sizeof(bod));

        bod[0].x = -0.97000436;
        bod[1].x =  0.97000436;
        bod[2].x = 0;
        bod[0].y =  0.24308753;
        bod[1].y = -0.24308753;
        bod[2].y = 0;
        bod[0].z = 0;
        bod[1].z = 0;
        bod[2].z = 0;
        bod[0].vx = 0.466203685;
        bod[1].vx = 0.466203685;
        bod[2].vx = -0.93240737;
        bod[0].vy = 0.43236573;
        bod[1].vy = 0.43236573;
        bod[2].vy = -0.86473146;
        bod[0].vz = 0;
        bod[1].vz = 0;
        bod[2].vz = 0;
        bod[0].mass = 1.0;
        bod[1].mass = 1.0;
        bod[2].mass = 1.0;
    }

    
    int r = 1;
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window1 = SDL_CreateWindow("N_Body", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1600, 1600, SDL_WINDOW_SHOWN);
    SDL_Renderer *rend1 = SDL_CreateRenderer(window1, -1, SDL_RENDERER_ACCELERATED);

    while(r == 1){
        SDL_Event e; 
        while (SDL_PollEvent(&e)) {
            if(e.type == SDL_QUIT){
                r = 0;
            }
            if(e.type == SDL_MOUSEBUTTONDOWN) drag = 1;
            if(e.type == SDL_MOUSEBUTTONUP) drag = 0;
            if(e.type == SDL_MOUSEMOTION && drag){
                angle += e.motion.xrel * 0.01; //spin
                pitch += e.motion.yrel * 0.01; //tilt
            }
            if(e.type == SDL_MOUSEWHEEL){
                cam -= e.wheel.y;
            }
            if(e.type==SDL_KEYDOWN && e.key.keysym.sym==SDLK_SPACE){
                pause = !pause;
            }
        }
        

    double ax[n];
    double ay[n];
    double az[n];

    
        
    memset(ax, 0, sizeof(ax));
    memset(ay, 0, sizeof(ay)); //sets arrays to 0
    memset(az, 0, sizeof(az));

if(!pause){
    for(int i = 0; i < n; i++){
        for(int j = i+1; j < n; j++ ){
    double dx = bod[j].x - bod[i].x;
    double dy = bod[j].y - bod[i].y;
    double dz = bod[j].z - bod[i].z;
    double dist = sqrt(dx*dx + dy*dy + dz*dz + 0.2*0.2); //0.2 is the softening factor, keeps objects creating numerical singularities

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
}
SDL_SetRenderDrawColor(rend1, 0, 0, 0, 255);
    SDL_RenderClear(rend1);

    if (drag == 0) {
        angle += 0.0025;
    }
    
    for(int i = 0; i < n; i++){
    double x = bod[i].x, y = bod[i].y, z =bod[i].z;
    double rx = x*cos(angle) + z*sin(angle); //rotated x using linear algebra; Rotational matrices
    double rz = -x*sin(angle) + z*cos(angle); //how far away
    double ry = y*cos(pitch) - rz*sin(pitch); //adds pitch to y
    double rz2 = y*sin(pitch) + rz*cos(pitch); //adds pitch to rz; rx stays unchanged by pitch
    double depth = cam - rz2; //depth of object; how far
    if(depth <= 0) continue;
    double factor = cam / depth; //perspective ratio
    int px = cx + (int)(rx * scale * factor);
    int py = cy - (int)(ry * scale * factor);

   //int px = cx + bod[i].x * scale;
   //int py = cy - bod[i].y * scale;


    double s = (float)(6 * factor);
    if(s < 1)  s = 1;
    SDL_Rect rect = { (int)(px -s/2), (int)(py - s/2), s, s};
    SDL_SetRenderDrawColor(rend1, 255, 255, 255, 255);
    SDL_RenderFillRect(rend1, &rect);
    }    

    SDL_RenderPresent(rend1);
    SDL_Delay(8);
 
    
    }
    
    SDL_DestroyRenderer(rend1);
    SDL_DestroyWindow(window1);
    SDL_Quit();
    return 0;
}
