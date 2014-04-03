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
inline static void display_update(SDL_Surface *screen, int x, int y, int R, int G, int B, int flush) {
    SDL_Rect pixel;
    int color = SDL_MapRGB(screen->format, R, G, B);

    pixel.x = x * PIXELSPEROBJECT;
    pixel.y = y * PIXELSPEROBJECT;
    pixel.w = PIXELSPEROBJECT;
    pixel.h = PIXELSPEROBJECT;

    SDL_FillRect(screen, &pixel, color);
    if (flush)
        SDL_UpdateRect(screen, x*PIXELSPEROBJECT, y*PIXELSPEROBJECT, PIXELSPEROBJECT, PIXELSPEROBJECT);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
int sdl_input(SDL_Surface *screen, struct cell_cluster *cluster) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            exit(0);
            /* Catch some key presses. */
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_RETURN:
                return SDLK_RETURN;
            /* Generation view. */
            case SDLK_g:
                printf("Display generation request\n");
                draw_all(screen, cluster, DRAW_GENERATION);
                display_call = draw_local_generation;
                return SDLK_g;
            /* Energy view. */
            case SDLK_e:
                printf("Display energy request\n");
                draw_all(screen, cluster, DRAW_ENERGY);
                display_call = draw_local_energy;
                return SDLK_e;
            /* Living view. */
            case SDLK_l:
                printf("Display living request\n");
                draw_all(screen, cluster, DRAW_LIVING);
                display_call = draw_local_living;
                return SDLK_l;
            /* Genmap view. */
            case SDLK_m:
                printf("Display genmap request\n");
                draw_all(screen, cluster, DRAW_GENMAP);
                display_call = draw_local_gmap;
                return SDLK_m;
            /* Restart sim. */
            case SDLK_r:
                printf("Restart sim request\n");
                /* Do some stuff */
                cluster->sched_end = 1;
                return SDLK_r;
            case SDLK_q:
                exit(0);
            default:
                break;
            }
        default:
            break;
        }
    }
    return 0;
}

GLFWwindow *display_init(void) {
    GLFWwindow *window;

    if(!glfwInit()) {
        printf("Video init error %s\n", SDL_GetError());
        return NULL;
    }
    else if (!(window = glfwCreateWindow(X*PIXELSPEROBJECT, Y*PIXELSPEROBJECT, "sg", NULL, NULL))) {
        printf("Video mode set error %s\n", SDL_GetError());
        return NULL;
    }

    glfwSetKeyCallback(window, key_callback);
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    glewInit();

    glEnable(GL_TEXTURE_2D);
    glClearDepth(1.0);
    glDepthFunct(GL_LEQUAL);
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

void draw_all(SDL_Surface *screen, const struct cell_cluster *cluster, enum DISPLAY_TYPE type) {
    int x, y;

    for (x = 0; x < X; x++) {
        for (y = 0; y < Y; y++) {
            /* What are we going to blanket update the screen with?. */
            switch (type) {
            case DRAW_ENERGY:
                draw_local_energy(screen, cluster, x, y, 0, 0);
                break;
            case DRAW_GENERATION:
                draw_local_generation(screen, cluster, x, y, 0, 0);
                break;
            case DRAW_LIVING:
                draw_local_living(screen, cluster, x, y, 0, 0);
                break;
            case DRAW_GENMAP:
                draw_local_gmap(screen, cluster, x, y, 0 , 0);
                break;
            case DRAW_BLANK:
                display_update(screen, x, y, 0, 0, 0, 0);
                break;
            default:
                break;
            }
        }
    }
    /* Update the whole screen at once since we didnt call the draw with a
     * buffer flush for efficiency. */
    SDL_UpdateRect(screen, 0, 0, X*PIXELSPEROBJECT, Y*PIXELSPEROBJECT);
}

void draw_local_energy(SDL_Surface *screen, const struct cell_cluster *cluster, int x, int y, char neighbours, char render) {
    int i, xptr, yptr;
    struct cell_proc *proc;

    proc = cluster->cells[x][y];

    if (proc->energy < 256)
        display_update(screen, x, y, proc->energy, 0, 0, render);
    else if (proc->energy < 512)
        display_update(screen, x, y, 255, proc->energy, 0, render);
    else if (proc->energy < 768)
        display_update(screen, x, y, 255, 255, proc->energy, render);
    else
        display_update(screen, x, y, 255, 255, 255, render);

    if (neighbours) {
        for (i = LEFT; i <= DOWN; i++) {
            get_neighbour_coords(x, y, i, &xptr, &yptr);
            draw_local_energy(screen, cluster, xptr, yptr, 0, render);
        }
    }
}

void draw_local_generation(SDL_Surface *screen, const struct cell_cluster *cluster, int x, int y, char neighbours, char render) {
    int i, xptr, yptr;
    struct cell_proc *proc;

    proc = cluster->cells[x][y];

    /* Attempt to fit more colours in by stepping thru the R,G,B scale, very bad way to do this. */
    if (proc->gen < 256)
        display_update(screen, x, y, 0, proc->gen, 0, render);
    else if (proc->gen < 512)
        display_update(screen, x, y, 0, 255, proc->gen, render);
    else if (proc->gen < 768)
        display_update(screen, x, y, proc->gen, 255, 255, render);
    else
        printf("color overflow -fix me- ....%ld\n", proc->gen);

    if (neighbours) {
        for (i = LEFT; i <= DOWN; i++) {
            get_neighbour_coords(x, y, i, &xptr, &yptr);
            draw_local_generation(screen, cluster, xptr, yptr, 0, render);
        }
    }
}

void draw_local_living(SDL_Surface *screen, const struct cell_cluster *cluster, int x, int y, char neighbours, char render) {
    int i, xptr, yptr;
    struct cell_proc *ptr;

    ptr = cluster->cells[x][y];

    /* Update the current pixel. */
    if (ptr->energy > 0)
        display_update(screen, x, y, 0, 0, 255, render);
    else
        display_update(screen, x, y, 0, 0, 0, render);

    if (neighbours) {
        /* And its neighbours. */
        for (i = LEFT; i <= DOWN; i++) {
            /* Get the direction coords for the neighbour with a cellvm helper call and
             * recursivly call this function to update each neighbour. */
            get_neighbour_coords(x, y, i, &xptr, &yptr);
            draw_local_living(screen, cluster, xptr, yptr, 0, render);
        }
    }
}

void draw_local_gmap(SDL_Surface *screen, const struct cell_cluster *cluster, int x, int y, char neighbours, char render) {
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
    display_update(screen, x, y, color<<1, color<<2, color<<4, render);

    if (neighbours) {
        for (i = LEFT; i <= DOWN; i++) {
            get_neighbour_coords(x, y, i, &xptr, &yptr);
            draw_local_gmap(screen, cluster, xptr, yptr, 0, render);
        }
    }
}
