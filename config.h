/** @file
 * Config file, contains defines and stuff that should be
 * global. */
#ifndef _CONFIG_H
#define _CONFIG_H

#include <sys/time.h>

/** Application build version. */
#define SGVER "alpha 0.2"

/** Defined if debug messages/features should be enabled,
 *  be warned this slows down the simulation considerably. */
#define DEBUG

#define ARTIFICIAL_LIMIT 0

/** Defined if SDL dispaly output/input collection is enabled. */
#define SDL_DISPLAY

#endif
