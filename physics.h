#ifndef GRAVITY_PHYSICS_H
#define GRAVITY_PHYSICS_H

#include <stdbool.h>
#include <pthread.h>

#define PARTICLES_MAX 120

void physics_start();
void physics_stop();

extern struct particle {
    double x, y, vx, vy, m, color[3];
} array[PARTICLES_MAX];

extern bool pause;
extern bool running;

extern int ms;
extern int count;

extern int next_m;
extern double next_color[3];

extern pthread_mutex_t lock;

void particles_clear();
float particle_radius(struct particle *p);

void particle_add(double x, double y, double vx, double vy);
void particle_remove(double x, double y);

double particle_getcoords(struct particle p, double *x, double *y);

#endif //GRAVITY_PHYSICS_H
