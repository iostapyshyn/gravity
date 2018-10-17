#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#include <pthread.h>
#include <sys/time.h>

#include "physics.h"

#define MS 10
#define G 1e-4

int next_m = 256;
double next_color[3] = { 0, 0, 0 };

bool running = false;

bool force_collect = false;

/* flag is true when physics thread is busy making changes to the particles array */
bool flag = false;
pthread_t thread_id;

bool pause = false;

struct particles particles = {
        .array = {
                { .x = 320, .y = 240, .vx = 0, .vy = 0,   .m = 65535, .color = { 1.0, 1.0, 0.0 }},
                { .x = 360, .y = 240, .vx = 0, .vy = 0.4, .m = 256,   .color = { 1.0, 0.0, 0.0 }},
                { .x = 250, .y = 240, .vx = 0, .vy = 0.3, .m = 128,   .color = { 0.0, 1.0, 0.5 }}, 0 },
        .index = 3 };

double *new_color() {
    next_color[0] = (rand() % 9 + 1) / 10.0;
    next_color[1] = (rand() % 9 + 1) / 10.0;
    next_color[2] = (rand() % 9 + 1) / 10.0;
    return next_color;
}

/* Used for qsort */
int compare(const void *s1, const void *s2) {
    struct particle *e1 = (struct particle *)s1;
    struct particle *e2 = (struct particle *)s2;
    return e2->m - e1->m;
}

/* Sort objects array and recalculate its size if index exceeds PARTICLES_MAX/2 */
void garbage_collect() {
    if (particles.index > PARTICLES_MAX/2 || force_collect) {
        qsort(particles.array, particles.index, sizeof(struct particle), compare);
        for (particles.index = 0; particles.array[particles.index].m > 0; particles.index++);
        force_collect = false;
    }
}

double timedifference_msec(struct timeval t0, struct timeval t1)
{
    return (t1.tv_sec - t0.tv_sec) * 1000.0f + (t1.tv_usec - t0.tv_usec) / 1000.0f;
}

/* Returns radius of the object in pixels */
float particle_radius(struct particle *p) {
    if (p->m < 2 && p->m > 2) return 1.0f;

    return (float)(log(fabs(p->m)) / log(2) / 2);
}

/* Update velocities and positions of objects */
void physics_update() {
    garbage_collect();
    for (int i = 0; i < particles.index; i++) {
        struct particle *p0 = &particles.array[i];
        for (int j = 0; j < particles.index; j++) {
            struct particle *p = &particles.array[j];
            if (i == j || p->m == 0 || p0->m == 0) continue;

            double d = sqrt((p0->x-p->x) * (p0->x-p->x) + (p0->y-p->y) * (p0->y-p->y));

            flag = 1;

            if (particle_radius(p0) + particle_radius(p) < d) {
                /* Update velocity of the object p0 */
                p0->vx += ((G * p->m) / (d * d)) * ((p->x - p0->x) / d);
                p0->vy += ((G * p->m) / (d * d)) * ((p->y - p0->y) / d);
            } else {
                /* Collision, new object is created */
                struct particle new;

                new.x = (p0->m * p0->x + p->m * p->x) / (p0->m + p->m);
                new.y = (p0->m * p0->y + p->m * p->y) / (p0->m + p->m);
                new.vx = (p0->m * p0->vx + p->m * p->vx) / (p0->m + p->m);
                new.vy = (p0->m * p0->vy + p->m * p->vy) / (p0->m + p->m);
                new.m = (p0->m + p->m);
                new.color[0] = (p0->m * p0->color[0] + p->m * p->color[0]) / (p0->m + p->m);
                new.color[1] = (p0->m * p0->color[1] + p->m * p->color[1]) / (p0->m + p->m);
                new.color[2] = (p0->m * p0->color[2] + p->m * p->color[2]) / (p0->m + p->m);

                p0->m=0;
                p->m=0;

                particles.array[particles.index] = new;
                particles.index++;

                flag = 0;

                return;
            }

            flag = 0;
        }
        /* Update x and y coordinates of the object p0 */
        p0->x += p0->vx;
        p0->y += p0->vy;
    }
}

void *physics_run() {
    struct timeval t0, t1;
    gettimeofday(&t0, 0);
    double delta = 0.0;
    while (running) {
        if (!pause) {
            gettimeofday(&t1, 0);
            delta += timedifference_msec(t0, t1) / MS;
            t0 = t1;
            if (delta >= 1.0) {
                physics_update();
                delta--;
            }
        } else gettimeofday(&t0, 0);
    }
    return NULL;
}

/* Those functions are called from main thread */
void physics_start() {
    srand(time(NULL));
    new_color();

    running = true;
    pthread_create(&thread_id, NULL, physics_run, NULL);
}

void physics_stop() {
    running = false;
    pthread_join(thread_id, NULL);
}

void particles_clear() {
    while(flag);
    for (int i = 0; i < PARTICLES_MAX; i++) {
        particles.array[i].m = 0;
    }                                                
    particles.index = 0;
}

void particle_add(double x, double y, double vx, double vy) {
    while (flag);
    particles.array[particles.index] = (struct particle){ x, y, vx, vy, next_m, { next_color[0], next_color[1], next_color[2] } };
    particles.index++;
    new_color();
}

void particle_remove(double x, double y) {
    int closest = 0;
    double d0 = +INFINITY;
    while (flag);
    for (int i = 0; i < particles.index; i++) {
        struct particle *p0 = &particles.array[i];
        if (!p0->m) continue;
        
        double d = sqrt((p0->x-x) * (p0->x-x) + (p0->y-y) * (p0->y-y));
        if (d<d0) {
            d0 = d;
            closest = i;
        }
    }
    particles.array[closest].m = 0;
}
