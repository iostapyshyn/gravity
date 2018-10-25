#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include <pthread.h>
#include <sys/time.h>

#include "gravity.h"
#include "physics.h"

#define G 1e-4

int ms = 10;
int count = PARTICLES_MAX;

int next_m = 256;
double next_color[3] = { 0, 0, 0 };

/* flag is true when physics thread is busy making changes to the particles array */
bool running = false;
bool pause = false;

pthread_t thread_id;
pthread_mutex_t lock;

struct particle array[] =
    {
     { .x = 320, .y = 240, .vx = 0, .vy = 0,   .m = 65535, .color = { 1.0, 1.0, 0.0 }},
     { .x = 360, .y = 240, .vx = 0, .vy = 0.4, .m = 256,   .color = { 1.0, 0.0, 0.0 }},
     { .x = 250, .y = 240, .vx = 0, .vy = 0.3, .m = 128,   .color = { 0.0, 1.0, 0.5 }}, 0
    };

double *new_color() {
    next_color[0] = (rand() % 9 + 1) / 10.0;
    next_color[1] = (rand() % 9 + 1) / 10.0;
    next_color[2] = (rand() % 9 + 1) / 10.0;
    return next_color;
}

void update() {
    count = PARTICLES_MAX;
    for (int i = 0; i < PARTICLES_MAX; i++) {
        if (!array[i].m) count--;
    }
}

bool check() {
    update();
    if (count+1 > PARTICLES_MAX) {
        fputs("Out of memory\n", stderr);
        running = false;
        return false;
    } else {
        return true;
    }
}

double timedifference_msec(struct timeval t0, struct timeval t1)
{
    return (t1.tv_sec - t0.tv_sec) * 1000.0f + (t1.tv_usec - t0.tv_usec) / 1000.0f;
}

/* Returns radius of the object in pixels */
float particle_radius(struct particle *p) {
    if (p->m < 2 && p->m > -2) return 1.0f;

    return (float)(log(fabs(p->m)) / log(2) / 2);
}

/* Update velocities and positions of objects */
void physics_update() {
    for (int i = 0; i < PARTICLES_MAX; i++) {
        struct particle *p0 = &array[i];
        if (!p0->m) continue;
        for (int j = 0; j < PARTICLES_MAX; j++) {
            struct particle *p = &array[j];
            if (!p->m) continue;

            if (i == j) continue;

            double d = sqrt((p0->x-p->x) * (p0->x-p->x) + (p0->y-p->y) * (p0->y-p->y));

            pthread_mutex_lock(&lock);
            if (particle_radius(p0) + particle_radius(p) < d) {
                /* Update velocity of the object p0 */
                p0->vx += ((G * p->m) / (d * d)) * ((p->x - p0->x) / d);
                p0->vy += ((G * p->m) / (d * d)) * ((p->y - p0->y) / d);
                pthread_mutex_unlock(&lock);
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

                if (check()) {
                    int i;
                    for (i = 0; array[i].m; i++); /* Get index of empty array element */
                    array[i] = new;

                    update();
                }

                pthread_mutex_unlock(&lock);
                return;
            }
        }
        /* Update x and y coordinates of the object p0 */
        p0->x += p0->vx;
        p0->y += p0->vy;
    }
}

void *physics_run() {
    running = true;
    struct timeval t0, t1;
    gettimeofday(&t0, 0);
    double delta = 0.0;
    while (running) {
        if (!pause) {
            gettimeofday(&t1, 0);
            delta += timedifference_msec(t0, t1) / ms;
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

    update();

    if (pthread_mutex_init(&lock, NULL) != 0) {
        fprintf(stderr, "phtread_mutex_init failed\n");
        return;
    }
    if (pthread_create(&thread_id, NULL, physics_run, NULL) != 0) {
        fprintf(stderr, "phtread_create failed\n");
        return;
    }

    while (!running);
}

void physics_stop() {
    running = false;
    pthread_join(thread_id, NULL);
    pthread_mutex_destroy(&lock);
}

void particles_clear() {
    pthread_mutex_lock(&lock);

    for (int i = 0; i < PARTICLES_MAX; i++) {
        array[i].m = 0;
    }

    pthread_mutex_unlock(&lock);

    update();
}

void particle_add(double x, double y, double vx, double vy) {
    pthread_mutex_lock(&lock);

    if (check()) {
        int i;
        for (i = 0; array[i].m; i++); /* Get index of empty array element */
        array[i] = (struct particle){ x, y, vx, vy, next_m, { next_color[0], next_color[1], next_color[2] } };
    }

    pthread_mutex_unlock(&lock);

    update();

    new_color();
}

void particle_remove(double x, double y) {
    int closest = 0;
    double d0 = +INFINITY;

    pthread_mutex_lock(&lock);

    for (int i = 0; i < PARTICLES_MAX; i++) {
        struct particle *p0 = &array[i];
        if (!p0->m) continue;

        double px, py;
        particle_getcoords(*p0, &px, &py);

        double d = sqrt((px-x) * (px-x) + (py-y) * (py-y));
        if (d<d0) {
            d0 = d;
            closest = i;
        }
    }
    array[closest].m = 0;

    pthread_mutex_unlock(&lock);

    update();
}

double particle_getcoords(struct particle p, double *x, double *y) {
    double alpha = 1.0;
    if (p.x > WIDTH) {
        *x = WIDTH;
        alpha = 0.5;
    } else if (p.x < 0) {
        *x = 0;
        alpha = 0.5;
    } else *x = p.x;

    if (p.y > HEIGHT) {
        *y = HEIGHT;
        alpha = 0.5;
    } else if (p.y < 0) {
        *y = 0;
        alpha = 0.5;
    } else *y = p.y;
    return alpha;
}
