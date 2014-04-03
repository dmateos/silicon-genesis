/** @file
 * Sets up SDL for drawing pixels to the screen and provides wrapper functions
 * for drawing cell objects and representing their colors.
 */
#ifndef _SDLIO_H
#define _SDLIO_H

#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>

#include "config.h"
#include "cellvm.h"

/** Path of the start up logo to display with display_title(). */
#define STARTLOGO "logo.bmp"

#define PIXELSPEROBJECT 4

/** Function pointer for screen display functions. */
typedef void(*duptr)(const struct cell_cluster*, int x, int y, char neighbours, char render);

/** Display call function pointer, set by sdl_input dynamicly via user input
 *  to one of the various display drawing styles. */
extern duptr display_call;

/** Draw types that can be passed to the draw_all function. */
enum DISPLAY_TYPE {
    DRAW_ENERGY,
    DRAW_GENERATION,
    DRAW_LIVING,
    DRAW_GENMAP,
    DRAW_BLANK,
};

/**
 * Init the SDL display system.
 * @return SDL screen on success, NULL on fail.
 */
GLFWwindow *display_init(void);

/**
 * Shutdown SDL.
 */
void display_close(void);

/**
 * Updates the whole screen with map of the data in the cluster.
 * @param screen Screen to draw updates too.
 * @param cluster Cluster to get update information from.
 * @param type Information to collect and draw from the cluster.
 */
void draw_all(const struct cell_cluster *cluster, enum DISPLAY_TYPE type);

/**
 * Updates the pixel at the specified location with energy information from the equivlent cell in the cluster.
 * @param screen Screen to draw pixel too.
 * @param cluster Cluster to get update data from.
 * @param x X coord.
 * @param y Y coord.
 * @param neighbours Update the cells around the one specified that could have been modified if true.
 * @param render Render the updates before the function exits if true.
 */
void draw_local_energy(const struct cell_cluster *cluster, int x, int y, char neighbours, char render);

/**
 * Updates the pixel at the specified location with generation information from the equiv cell in cluster.
 * @param screen Screen to draw too.
 * @param cluster Cluster to get data from.
 * @param x X coord.
 * @param x Y coord.
 * @param neighbours Update the cells around the one specified that could have been modified if true.
 * @param render Render the updates if true.
 */
void draw_local_generation(const struct cell_cluster *cluster, int x, int y, char neighbours, char render);

/**
 * Updates the pixel at the specified location with living information from the equiv cell in cluster.
 * @param screen Screen to draw too.
 * @param cluster Cluster to get data from.
 * @param x X co-ord.
 * @param y Y co-ord.
 * @param neighbours Update the cells around the one specified that could have been modified if true.
 * @param render Render the updates now if true.
 */
void draw_local_living(const struct cell_cluster *cluster, int x, int y, char neighbours, char render);

/**
 * Updates the pixel at the specified location with genmap information from the cell specified.
 * @param screen SDL screen to draw too.
 * @param cluster Cluster containing cell.
 * @param x x co-ord of cell.
 * @param y y co-ord of cell.
 * @param neighbours If set to true, updates cells neighbours too.
 * @param render Render the updates now?
 */
void draw_local_gmap(const struct cell_cluster *cluster, int x, int y, char neighbours, char render);

#endif

