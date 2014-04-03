/** @file
 * Sets up SDL for drawing pixels to the screen and provides wrapper functions
 * for drawing cell objects and representing their colors.
 */
#include <stdlib.h>
#include <stdio.h>

#include "sdlio.h"
#include "cellvm.h"

duptr display_call;

/**
 * Updates a pixel on the screen to the color specified.
 * @param screen SDL screen to draw to.
 * @param x Horizontal coord.
 * @param y Vertical coord.
 * @param R Red color value. (16 bit)
 * @param G Green color value.
 * @param B Blue color value.
 * @param flush Update the display now if true.
 */
inline static void display_update(int x, int y, float R, float G, float B) {
    glColor3f(R,G,B);

    int xmod = x * PIXELSPEROBJECT;
    int ymod = y * PIXELSPEROBJECT;

    glBegin(GL_QUADS);
    glVertex3f(xmod, ymod, 0);
    glVertex3f(xmod+PIXELSPEROBJECT, ymod, 0);
    glVertex3f(xmod+PIXELSPEROBJECT, ymod+PIXELSPEROBJECT, 0);
    glVertex3f(xmod, ymod+PIXELSPEROBJECT, 0);

    glEnd();
}

/*
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  switch(key) {
    case GLFW_KEY_G:
      printf("display generation request\n");
      draw_all(cluster, DRAW_GENERATION);
      display_call = draw_local_generation;
    case GLFW_KEY_E:
      printf("display energy request\n");
      draw_all(cluster, DRAW_ENERGY);
      display_call = draw_local_generation;
      break;
    case GLFW_KEY_L:
      printf("display living request\n");
      draw_all(cluster, DRAW_LIVING);
      display_call = draw_local_living;
      break;
    case GLFW_KEY_M:
      printf("display genmap request\n");
      draw_all(cluster, DRAW_GENMAP);
      display_call = draw_local_gmap;
    case GLFW_KEY_R:
      cluster->sched_end = 1;
      break;
    case GLFW_KEY_Q:
      exit(0);
  }
}
*/

GLFWwindow *display_init(void) {
    GLFWwindow *window;

    if(!glfwInit()) {
        return NULL;
    }
    else if (!(window = glfwCreateWindow(X*PIXELSPEROBJECT, Y*PIXELSPEROBJECT, "sg", NULL, NULL))) {
        return NULL;
    }

//    glfwSetKeyCallback(window, key_callback);
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    glewInit();

    glEnable(GL_TEXTURE_2D);
    glClearDepth(1.0);
    glDepthFunc(GL_LEQUAL);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, X*PIXELSPEROBJECT, Y*PIXELSPEROBJECT, 0.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClearColor(1.0, 1.0, 1.0, 0.0);
    glViewport(0, 0, X*PIXELSPEROBJECT, Y*PIXELSPEROBJECT);

    display_call = draw_local_gmap; /* Set display call to generations by default. */

#ifdef DEBUG
    printf("Display init ok: %dx%d @ %d^2\n", X*PIXELSPEROBJECT, Y*PIXELSPEROBJECT, PIXELSPEROBJECT);
#endif
    return window;
}

void display_close(void) {
  glfwTerminate();
}

void draw_all( const struct cell_cluster *cluster, enum DISPLAY_TYPE type) {
    int x, y;

    for (x = 0; x < X; x++) {
        for (y = 0; y < Y; y++) {
            /* What are we going to blanket update the screen with?. */
            switch (type) {
            case DRAW_ENERGY:
                draw_local_energy(cluster, x, y, 0, 0);
                break;
            case DRAW_GENERATION:
                draw_local_generation(cluster, x, y, 0, 0);
                break;
            case DRAW_LIVING:
                draw_local_living(cluster, x, y, 0, 0);
                break;
            case DRAW_GENMAP:
                draw_local_gmap(cluster, x, y, 0 , 0);
                break;
            case DRAW_BLANK:
                display_update(x, y, 0, 0, 0);
                break;
            default:
                break;
            }
        }
    }
}

void draw_local_energy(const struct cell_cluster *cluster, int x, int y, char neighbours, char render) {
    int i, xptr, yptr;
    struct cell_proc *proc;

    proc = cluster->cells[x][y];

    if (proc->energy < 256)
        display_update(x, y, proc->energy, 0, 0);
    else if (proc->energy < 512)
        display_update(x, y, 255, proc->energy, 0);
    else if (proc->energy < 768)
        display_update(x, y, 255, 255, proc->energy);
    else
        display_update(x, y, 255, 255, 255);

    if (neighbours) {
        for (i = LEFT; i <= DOWN; i++) {
            get_neighbour_coords(x, y, i, &xptr, &yptr);
            draw_local_energy(cluster, xptr, yptr, 0, render);
        }
    }
}

void draw_local_generation(const struct cell_cluster *cluster, int x, int y, char neighbours, char render) {
    int i, xptr, yptr;
    struct cell_proc *proc;

    proc = cluster->cells[x][y];

    /* Attempt to fit more colours in by stepping thru the R,G,B scale, very bad way to do this. */
    if (proc->gen < 256)
        display_update(x, y, 0, proc->gen, 0);
    else if (proc->gen < 512)
        display_update(x, y, 0, 255, proc->gen);
    else if (proc->gen < 768)
        display_update(x, y, proc->gen, 255, 255);
    else
        printf("color overflow -fix me- ....%ld\n", proc->gen);

    if (neighbours) {
        for (i = LEFT; i <= DOWN; i++) {
            get_neighbour_coords(x, y, i, &xptr, &yptr);
            draw_local_generation(cluster, xptr, yptr, 0, render);
        }
    }
}

void draw_local_living(const struct cell_cluster *cluster, int x, int y, char neighbours, char render) {
    int i, xptr, yptr;
    struct cell_proc *ptr;

    ptr = cluster->cells[x][y];

    /* Update the current pixel. */
    if (ptr->energy > 0)
        display_update(x, y, 0, 0, 255);
    else
        display_update(x, y, 0, 0, 0);

    if (neighbours) {
        /* And its neighbours. */
        for (i = LEFT; i <= DOWN; i++) {
            /* Get the direction coords for the neighbour with a cellvm helper call and
             * recursivly call this function to update each neighbour. */
            get_neighbour_coords(x, y, i, &xptr, &yptr);
            draw_local_living(cluster, xptr, yptr, 0, render);
        }
    }
}

void draw_local_gmap(const struct cell_cluster *cluster, int x, int y, char neighbours, char render) {
    int i, xptr, yptr, color;
    struct cell_proc *proc;

    color = 0;
    proc = cluster->cells[x][y];

    /* Simple hash alrogithem to make a unique color for the cell. */
    for (i = 0; proc->instructions[i] != STOP && i < CSIZE; i++) {
        color += proc->instructions[i];
        color += (color << 10);
        color ^= (color >> 6);
    }
    color += (color << 3);
    color += (color >> 11);
    color ^= (color << 15);

    /* the << shifts for the R,G,B channels give the output a little colour
     * rather than bland grey scale images. */
    display_update(x, y, color<<1, color<<2, color<<4);

    if (neighbours) {
        for (i = LEFT; i <= DOWN; i++) {
            get_neighbour_coords(x, y, i, &xptr, &yptr);
            draw_local_gmap(cluster, xptr, yptr, 0, render);
        }
    }
}
