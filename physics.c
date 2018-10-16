#include <math.h>
#include <pthread.h>
#include <sys/time.h>

#include "physics.h"

#define MS 10
#define G 1e-4

int running = 0;
int flag = 0;
pthread_t thread_id;

struct particles particles = {
        .array = {
                { .x = 320, .y = 240, .vx = 0, .vy = 0,   .m = 65535, .color = { 1.0, 1.0, 0.0 }},
                { .x = 360, .y = 240, .vx = 0, .vy = 0.4, .m = 256,   .color = { 1.0, 0.0, 0.0 }},
                { .x = 250, .y = 240, .vx = 0, .vy = 0.3, .m = 128,   .color = { 0.0, 1.0, 0.5 }}, 0 },
        .index = 3 };

double timedifference_msec(struct timeval t0, struct timeval t1)
{
    return (t1.tv_sec - t0.tv_sec) * 1000.0f + (t1.tv_usec - t0.tv_usec) / 1000.0f;
}

float particle_radius(struct particle *p) {
    if (p->m < 2) return 1.0f;

    return (float)(log(p->m) / log(2) / 2);
}

void physics_update() {
    for (int i = 0; i < particles.index; i++) {
        struct particle *p0 = &particles.array[i];
        for (int j = 0; j < particles.index; j++) {
            struct particle *p = &particles.array[j];
            if (i == j || !p->m || !p0->m) continue;

            double d = sqrt((p0->x-p->x) * (p0->x-p->x) + (p0->y-p->y) * (p0->y-p->y));

            flag = 1;

            if (particle_radius(p0) + particle_radius(p) < d) {
                p0->vx += ((G * p->m) / (d * d)) * ((p->x - p0->x) / d);
                p0->vy += ((G * p->m) / (d * d)) * ((p->y - p0->y) / d);

                p0->x += p0->vx;
                p0->y += p0->vy;
            } else {
                struct particle *new = &particles.array[particles.index++];

                new->x = (p0->m * p0->x + p->m * p->x) / (p0->m + p->m);
                new->y = (p0->m * p0->y + p->m * p->y) / (p0->m + p->m);
                new->vx = (p0->m * p0->vx + p->m * p->vx) / (p0->m + p->m);
                new->vy = (p0->m * p0->vy + p->m * p->vy) / (p0->m + p->m);
                new->m = (p0->m + p->m);
                new->color[0] = (p0->m * p0->color[0] + p->m * p->color[0]) / (p0->m + p->m);
                new->color[1] = (p0->m * p0->color[1] + p->m * p->color[1]) / (p0->m + p->m);
                new->color[2] = (p0->m * p0->color[2] + p->m * p->color[2]) / (p0->m + p->m);

                p0->m=0;
                p->m=0;
            }

            flag = 0;
        }
    }
}

void *physics_run() {
    struct timeval t0, t1;
    gettimeofday(&t0, 0);
    double delta = 0.0;
    while (running) {
        gettimeofday(&t1, 0);
        delta += timedifference_msec(t0, t1) / MS;
        t0 = t1;
        if (delta >= 1.0) {
            physics_update();
            delta--;
        }
    }
    return NULL;
}

void physics_start() {

    running = 1;
    pthread_create(&thread_id, NULL, physics_run, NULL);
}

void physics_stop() {
    running = 0;
    pthread_join(thread_id, NULL);
}
