//
// Created by Ilya Ostapyshyn on 10/10/18.
//

#ifndef GRAVITY_PHYSICS_H
#define GRAVITY_PHYSICS_H

void physics_start();
void physics_stop();

struct particle {
    double x, y, vx, vy, m, color[3];
};

struct particles {
    struct particle array[128];
    int index;
} particles;

int flag;

float particle_radius(struct particle *p);

#endif //GRAVITY_PHYSICS_H
