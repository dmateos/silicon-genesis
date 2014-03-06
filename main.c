/** @file
 *  Main entry point for application, sets up the various sub systems and defines
 *  their interaction.
 */
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include "main.h"
#include "config.h"
#include "cellvm.h"
#include "sdlio.h"

/* Global cluster and screen pointers, assigned by main(). */
struct cell_cluster *cp;
SDL_Surface *sp;

/**
 * Print program usage information to console.
 */
static void print_help(void) {
#ifdef DEBUG
    printf("Debug mode enabled.\n");
#endif
    printf("Silicion Genesis %s\n", SGVER);
    printf("Commands:\n\tReturn to begin. \
           \n\te for Energy view. \
           \n\tg for Generation view. \
           \n\tl for Living cells view. \
           \n\tm for Genmap(KIND OF) view. \
           \n\tr for Restart. \
           \n\tq to Quit..\n");
}

/**
 * Called when the program gets some kind of exit signal.
 */
static void handle_exit(void) {
 /* TODO: Need to do something about this.
  * main exits and doesnt like it, reproduce by letting main exit. */
//    if (!cp)
    cluster_free(cp);
//    if (!sp)
    display_close();
//    sp = NULL;
//    cp = NULL;
}

/**
 * Do GUI and other SDL updates.
 */
static void callback_update(int x, int y, char didstuff) {
    if (didstuff)
        display_call(sp, cp, x, y, 1, 1);
}

/**
 * Wrapper for the sdl_input function that adhears to the
 * callback interface. */
static void callback_sdlinput(int x, int y, char didstuff) {
    sdl_input(sp, cp);
}

/**
 * Application entry point.
 */
int main(int argc, char *argv[]) {
    struct cell_cluster cluster;
    SDL_Surface *screen;
    int i;

    /* Test instructions, they do what they are called... */
    char randpop[CSIZE] = { SPOR, STOP };
    char popleft[CSIZE] = { ZERO, TURN, SPOR, STOP };
    char popright[CSIZE] = { INCR, TURN, SPOR, STOP };
    char popup[CSIZE] = { INCR, INCR, TURN, SPOR, STOP };
    char popdown[CSIZE] = { INCR, INCR, INCR, TURN, SPOR, STOP };
    char star[CSIZE] = { TURN, SPOR, INCR, TURN, SPOR, INCR, TURN, SPOR, INCR, TURN, SPOR, STOP };
    char rightup[CSIZE] = { INCR, TURN, SPOR, INCR, TURN, SPOR, STOP };
    char randpopeat[CSIZE] = { SPOR, RDIR, CRCH, STOP };

    /* Init display system and cluster. */
    if (cluster_init(&cluster) || !(screen = display_init()))
        exit(1);
    else
        atexit(handle_exit);

    cp = &cluster;
    sp = screen;
    srand(time(NULL));

    add_callback(&cluster.callbacks, callback_sdlinput, 100, "SDL_INPUT_COLLECT");
    add_callback(&cluster.callbacks, callback_update, 1, "SDL_GUI_UPDATE");

    /* Show title untill enter is pressed. */
    print_help();
    display_title(screen);

    do {
        draw_all(screen, &cluster, DRAW_BLANK);

        /* Populate random spots with some test cells and run the scheduler. */
//        cell_pop(&cluster, RANDX, RANDY, 1, ENERGY, randpop);
//        cell_pop(&cluster, RANDX, RANDY, 1, ENERGY, popup);
//        cell_pop(&cluster, RANDX, RANDY, 1, ENERGY, popleft);
//        cell_pop(&cluster, RANDX, RANDY, 1, ENERGY, popright);
//        cell_pop(&cluster, RANDX, RANDY, 1, ENERGY, popdown);
//        cell_pop(&cluster, RANDX, RANDY, 1, ENERGY, star);
//        cell_pop(&cluster, RANDX, RANDY, 1, ENERGY, rightup);

        /* Populate the grid with some randomly seeded cells. */
        for (i = 0; i < 300; i++)
            cell_seed(&cluster, RANDX, RANDY);

        cluster_sched(&cluster);

    } while (!cluster_reset(&cluster));

    return 0;
}
