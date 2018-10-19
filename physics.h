#ifndef GRAVITY_PHYSICS_H
#define GRAVITY_PHYSICS_H

#include <stdbool.h>

#define PARTICLES_MAX 64

void physics_start();
void physics_stop();

struct particle {
    double x, y, vx, vy, m, color[3];
};

struct particles {
    struct particle array[PARTICLES_MAX];
    int index;
} particles;

bool flag;
bool pause;
bool force_collect;

int ms;

int next_m;
double next_color[3];

void particles_clear();
float particle_radius(struct particle *p);

void particle_add(double x, double y, double vx, double vy);
void particle_remove(double x, double y);

double particle_getcoords(struct particle p, double *x, double *y);

#endif //GRAVITY_PHYSICS_H
