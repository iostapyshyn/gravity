#include <stdio.h>
#include <GLFW/glfw3.h>

#include "physics.h"

#define WIDTH 640
#define HEIGHT 480

GLFWwindow* window;

void error_callback(int error, const char* desc) {
    fputs(desc, stderr);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

}

void mouse_callback(GLFWwindow* window, int button, int action, int mods) {

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

    while (!glfwWindowShouldClose(window)) {
        while (flag);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (int i = 0; i < particles.index; i++) {
            struct particle *p = &particles.array[i];

            if (p->m) {
                glPointSize(particle_radius(p) * 2.0f);
                glColor3d(p->color[0], p->color[1], p->color[2]);

                glBegin(GL_POINTS);
                glVertex2d(p->x, p->y);
                glEnd();
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
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

    window = glfwCreateWindow(WIDTH, HEIGHT, "Gravity | M = ", NULL, NULL);
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
