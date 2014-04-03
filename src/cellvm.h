/** @file
 * Implements a simple register based virtual machine with a 2d array of
 * processes competeing for energy and space for digital life simulation.
 */
#ifndef _CELLVM_H
#define _CELLVM_H

#include <math.h>

#include "config.h"
#include "cellvmcb.h"

/********** TWEAKABLE **************/
/** Size of cell instruction cache. */
#define CSIZE 16
/** Horizontal cell cluster resolution. */
#define X 200
/** Vertical cell cluster resolution. */
#define Y 200

#define MUTATIONRATE pow(2, 16)

/** Default cell energy. PROB TEMP. */
#define ENERGY 100
/** 1/X chance a weaker cell will win a cell_vs. */
#define LUCKYCHANCE 1000

/******* END TWEAKBLE*************/

/** Virtual machine instruction table, never use the literal values as
  * they should beable to change without fucking with heaps of code. */
enum INSTRUCTIONS {
    /** Null instruction, does nothing. */
    NOOP = 0x00,
    /** Stop instruction, stops the current program and returns the vm. */
    STOP = 0x01,
    /** Increment instruction, increments the proccess register by 1. */
    INCR = 0x02,
    /** Deincrement instruction, deincrements the proccess register by 1. */
    DNCR = 0x03,
    /** Zero instruction, sets the proccess register to 0. */
    ZERO = 0x04,
    /** Turn instruction, sets cell direction for CRCH and SPOR to the value in the proc reg. */
    TURN = 0x05,
    /** Crunch instruction, attempts to eat the energy of a process pointed to by direction. */
    CRCH = 0x06,
    /** Kill instruction, kills a cell pointed to by direction out of cold blood. */
    KILL = 0x07,
    /** Share instruction, Share half the cells energy with neighbour pointed to by direction. */
    SHAR = 0x08,
    /** Spor instruction, attempts to spore a copy proc in the space pointed to by direction. */
    SPOR = 0x9,
    /** Reset the direction register to a random dir. */
    RDIR = 0xa,

    /** Signifies the end of the instruction list, please keep it at the end. */
    IEND
};

/** Possible values for the direction register. */
enum DIRECTIONS {
    /** Value for left/west. */
    LEFT,
    /** Value for right/east. */
    RIGHT,
    /** Value for up/north. */
    UP,
    /** Value for down/south. */
    DOWN
};

/** Random number between 0 and X. */
#define RANDX (rand() % X)
/** Random number between 0 and Y. */
#define RANDY (rand() % Y)

/** Invovidual cell proccess. */
struct cell_proc {
    /** Generation level of the cell. */
    unsigned long gen;
    /** Cells energy level which deturmins how many instructions it can exec. */
    unsigned long energy;
    /** Instruction cache. Holds instructions to be executed by the VM. */
    char instructions[CSIZE];
};

/**TODO*/
struct cluster_stats {
    /**TODO*/
    unsigned long energy_death;
    /**TODO*/
    unsigned long spor_copies;
    /**TODO*/
    unsigned long vs_lucky;
};

/** Holds a cluster of computeable cells. */
struct cell_cluster {
    /** 2d 'table' array of cells. */
    struct cell_proc *cells[X][Y];
    /** VM ticks. Incremented each time a cell finishes executeing. */
    unsigned long tick;
    /**TODO*/
    struct cluster_stats stats;
    /** Callback subsystem structure, keeps track of callbacks registered to this VM. */
    struct callback_stack callbacks;
    /** Tells the virtual machine when to stop proccessing cells. */
    char sched_end;
};

/**
 * Init a cell_cluster structure, must be done before use to allocate
 * space for the cell array.
 * @param cluster Cluster to init.
 * @return 0 on ok, 1 on fail.
 */
int cluster_init(struct cell_cluster *cluster);

/**
 * Free a cell_cluster structure init'd with cluster_init.
 * @param cluster Cluster structure to deallocate.
 */
void cluster_free(struct cell_cluster *cluster);

/**
 * Reset a cell_cluster to its default state before it was ran
 * preserving various data such as callback assignments.
 * @param cluster Cluster structure to reset.
 * @return 0 ok, 1 fail.
 */
int cluster_reset(struct cell_cluster *cluster);

/**
 * Scheduler, hands processes over to proc_cell for processing.
 * @param cluster Cluster containing cell processes to schedule.
 * @return 0
 */
int cluster_sched(struct cell_cluster *cluster);

/**
 * Get the coordenents of the neighbour reletive to cell at x,y.
 * @param x x coord of the cell to get neighbour from.
 * @param y y coord of the cell.
 * @param direction Direction to look for the neighbour.
 * @param xp Pointer to store the x location of neighbour.
 * @param yp Pointer to store the y location of neighbour.
 * @return 0 on ok, 1 on fail.
 */
int get_neighbour_coords(int x, int y, int direction, int *xp, int *yp);

/**
 * Populate the cell at the co-ords specified with the cell attributes also specified.
 * @param cluster Cluster containing the cell to modify.
 * @param x Horizontal co-ords.
 * @param y Vertical co-ords.
 * @param gen Cell generation.
 * @param energy Cell energy level.
 * @param instructions Cell instruction set.
 */
void cell_pop(const struct cell_cluster *cluster, int x, int y, int gen, int energy, const char instructions[CSIZE]);

/**
 * Seed the cell at the coo-ords speicfied with random cell attributes.
 * @param cluster Cluster with the cell.
 * @param x x coord of the cell.
 * @param y y coord of the cell.
 */
void cell_seed(const struct cell_cluster *cluster, int x, int y);

void cell_mutate(const struct cell_cluster *cluster, int x, int y, unsigned long chance);

#endif

