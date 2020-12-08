#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>
#include <pthread.h>

#include "physics.h"
#include "gravity.h"

char title[50];

GLFWwindow* window;

struct {
    bool is_dragging;
    double xp, yp;
} mouse = { false };

char *newtitle() {
    char speed[10] = "*PAUSED*";
    if (!pause) sprintf(speed, "%dms", ms);
    sprintf(title, "Gravity | M = %d [%d/%d] %s", next_m, count, PARTICLES_MAX, speed);
    return title;
}

void error_callback(int error, const char* desc) {
    fputs(desc, stderr);
}

/*
  Z: decrease mass of the next object
  X: increase mass of the next object
  C: clear the objects array
  P: pause
  D: remove object closest to the mouse cursor
  A: slow down
  S: speed up
  ESC: quit
*/
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) { // Close window
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    } else if (key == GLFW_KEY_Z && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
        if (next_m == 1) {
            next_m = -1;
        } else {
            if (next_m > 0) next_m /= 2;
            if (next_m < 0) next_m *= 2;
        }
    } else if (key == GLFW_KEY_X && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
        if (next_m == -1) {
            next_m = 1;
        } else {
            if (next_m > 0) next_m *= 2;
            if (next_m < 0) next_m /= 2;
        }
    } else if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        particles_clear();
    } else if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        pause = !pause;
    } else if (key == GLFW_KEY_D && action == GLFW_PRESS) {
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        particle_remove(x, y);
    } else if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        if (ms > 1)
            ms--;
    } else if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        if (ms < 25)
            ms++;
    }
}

void mouse_callback(GLFWwindow* window, int button, int action, int mods) {
    const int vk = 70; // Velocity to mouse vector coefficient

    double x, y;
    glfwGetCursorPos(window, &x, &y);

    if (action == GLFW_PRESS) {
        mouse.is_dragging = true;
        mouse.xp = x;
        mouse.yp = y;
    } else {
        mouse.is_dragging = false;
        particle_add((mouse.xp+x)/2, (mouse.yp+y)/2, (mouse.xp - x) / vk, (mouse.yp - y) / vk);
    }
}

void loop() {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0, WIDTH, HEIGHT, 0, -1, 1);

    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    physics_start();

    while (!glfwWindowShouldClose(window) && running) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (pthread_mutex_trylock(&lock) == 0) {
            for (int i = 0; i < PARTICLES_MAX; i++) {
                struct particle *p = &array[i];
                if (!p->m) continue;

                glPointSize(particle_radius(p) * 2.0f);

                double x, y;
                double alpha = particle_getcoords(*p, &x, &y);

                glColor4d(p->color[0], p->color[1], p->color[2], alpha);

                glBegin(GL_POINTS);
                glVertex2d(x, y);
                glEnd();
            }
            pthread_mutex_unlock(&lock);
        }

        if (mouse.is_dragging) {
            double x, y;
            glfwGetCursorPos(window, &x, &y);

            glBegin(GL_LINES);
            glColor3dv(next_color);

            glVertex2d(mouse.xp, mouse.yp);
            glVertex2d(x, y);

            glEnd();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();

        glfwSetWindowTitle(window, newtitle());
    }

    physics_stop();
}

int main() {
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        return -1;

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_callback);

    const GLFWvidmode* vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    glfwSetWindowPos(window, (vidmode->width - WIDTH) / 2, (vidmode->height - HEIGHT) / 2);

    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);
    glfwShowWindow(window);

    loop();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
