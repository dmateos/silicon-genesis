/** @file
 * Implements a simple register based virtual machine with a 2d array of
 * processes competeing for energy and space for digital life simulation.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cellvm.h"

#ifdef DEBUG
/** Lookup table (instruction -> string) for debugging purposes.
  * Note: do not let get out of order or wrong results will be printed */
const static char *instrlookup[] = { "NOOP", "STOP", "INCR", "DNCR", "ZERO",
                                     "TURN", "CRCH", "KILL", "SHAR", "SPOR", "RDIR" };
#endif

/**
 * Reap a cell if it has no energy left.
 * @param cluster Cluster to reap cell from.
 * @param x X coord of the cell.
 * @param y Y coord of the cell.
 * @return 0 on OK, something else on fail.
 */
inline static int cell_reap(const struct cell_cluster *cluster, int x, int y) {
    struct cell_proc *current;

    current = cluster->cells[x][y];

    if (current->gen > 0 && current->energy <= 0) {
#ifdef DEBUG
        printf("Reaper:%dx%d, gen:%ld, tick:%ld\n", x, y, current->gen, cluster->tick);
#endif
        memset(current, '\0', sizeof(struct cell_proc));
        return 0;
    }
    return 1;
}

/**
 * Get the neighbour specified by direction reletive to the cell at the coords specified.
 * @param cluster Cell cluster to get cell from.
 * @param x Horizontal co-ords.
 * @param y Vertical co-ords.
 * @param direction Direction of neighbour reletive to specified cell co-ords, LEFT,RIGHT,UP,DOWN.
 */
inline static struct cell_proc *get_neighbour(const struct cell_cluster *cluster, int x, int y, int direction) {
    int xp, yp;
    if (get_neighbour_coords(x, y, direction, &xp, &yp) != -1)
        return cluster->cells[xp][yp];
    else return NULL;
}

/**
 * Core cell logic processor, process the cell at the given co-ords instructions.
 * @param cluster Cluster with the cell to compute.
 * @param x Horizontal co-ords of the cell.
 * @param y Vertical co-ords of the cell.
 * @param TODO
 * @return The cell proccessed on success, NULL on fail.
 */
static struct cell_proc *proc_cell(const struct cell_cluster *cluster, int x, int y, char *didstuff) {
    int reg0, stop, instptr, direct, tmp;
    struct cell_proc *cell, *neighb;

    reg0 = stop = instptr = 0;

    if ((cell = cluster->cells[x][y]) == NULL) {
        printf("Could not retreive cell\n");
        return NULL;
    }

    /* If the instrucions arnt null and theres enough energy. */
    if (cell->instructions && cell->energy > 0) {
        *didstuff = 1;
#ifdef DEBUG
        printf("Tick %ld, cell:%dx%d, energy:%ld, gen:%ld\n", cluster->tick, x, y, cell->energy, cell->gen);
#endif
        direct = rand() % 4;
        /* Process the cells instructions (if it has any) untill its energy has run out, it has no
         * instructions left or a STOP opcode is found. */
        while ((cell->energy > 0) && (instptr < CSIZE) && !stop) {
#ifdef DEBUG
            printf("\tiptr:0x%x, inst:%s, reg0:0x%x, dir:0x%x, energy:%ld\n", instptr, instrlookup[cell->instructions[instptr]], reg0, direct, cell->energy);
#endif
            switch (cell->instructions[instptr]) {
            case NOOP:
                break;
            case STOP:
                stop = 1;
                break;
            case INCR:
                reg0++;
                break;
            case DNCR:
                if (reg0 > 0)
                    reg0--;
                break;
            case ZERO:
                reg0 = 0;
                break;
            case TURN:
                if (reg0 < 4)
                    direct = reg0;
                break;
            case CRCH:
                if (cell->energy <= 1)
                    break;
                else if (!(neighb = get_neighbour(cluster, x, y, direct)))
                    return NULL;
                else if (neighb->gen != 0) {
                    /* EXPERIMENTAL, kill neighbour. */
                    cell->energy += 10;
                    memset(neighb, '\0', sizeof *neighb);
                }
                break;
            case KILL:
                if (cell->energy <= 1)
                    break;
                else if (!(neighb = get_neighbour(cluster, x, y, direct)))
                    return NULL;
                else if (neighb->gen != 0) {
                    /* EXPERIMENTAL, kill neighbour. */
                    memset(neighb, '\0', sizeof *neighb);
                }
                break;
            case SHAR:
                if (cell->energy <= 1)
                    break;
                else if (!(neighb = get_neighbour(cluster, x, y, direct)))
                    return NULL;
                else if (neighb->gen != 0) {
                    /* EXPERIMENTAL, share energy with neighbour. */
                    /* Give neighbour half our energy. */
                    neighb->energy += cell->energy/2;
                    cell->energy = cell->energy/2;
                }
                break;
            case SPOR:
                /* Cant SPOR if you only have one energy, you will spawn a dead child,
                 * hrm maybe we should?. */
                if (cell->energy <= 2)
                    break;
                /* invalod neighbour, should not happen. */
                else if (!(neighb = get_neighbour(cluster, x, y, direct)))
                    return NULL;
                /* Spor ok if the cell gen is zero. */
                else if (neighb->gen == 0) {
                    tmp = neighb->energy;
                    /* Copy cell's data to neighbour, increment its gen and take away an energy as a
                     * creation cost. */
                    memcpy(neighb, cell, sizeof *neighb);
                    neighb->gen++;
                    neighb->energy -= 2;
                    //neighb->energy = 10; // TEST: trying fixed child energy.
                    /* Give the child cell the energy found in cell pre spor. */
                    neighb->energy += tmp;

                    cell_mutate(cluster, x, y, MUTATIONRATE);
#ifdef DEBUG
                    printf("\tspor:true\n");
#endif
                }
                break;
            case RDIR:
                direct = rand() % 4;
                break;
            default: /* INVALID OPCODE. */
                break;
            }
            cell->energy--;
            instptr++;
        }
#ifdef DEBUG
        printf("Cell stopped: iptr:0x%x/0x%x, inst:%s, stp:%d, energy:%ld\n", instptr-1, CSIZE, instrlookup[cell->instructions[instptr-1]], stop, cell->energy);
        if (ARTIFICIAL_LIMIT > 0)
            sleep(ARTIFICIAL_LIMIT); /* Sleep so we can see results. */
#endif
    }
    return cell;
}

int cluster_init(struct cell_cluster *cluster) {
    int x,y,acount;

    memset(cluster, '\0', sizeof *cluster);
    acount = 0;
    /* Init rand just incase its not, we use it alot. */
    srand(time(NULL));

    /* Malloc some space for the cells and clear their memory. */
    for (x = 0; x < X; x++) {
        for (y = 0; y < Y; y++) {
            if (!(cluster->cells[x][y] = malloc(sizeof(struct cell_proc))))
                exit(1);

            memset(cluster->cells[x][y], '\0', sizeof(struct cell_proc));
            acount += sizeof(struct cell_proc);
        }
    }
#ifdef DEBUG
    printf("Cell alloc: %db, %dx%dx%ld\n", acount, X, Y, sizeof(struct cell_proc));
#endif
    return 0;
}

void cluster_free(struct cell_cluster *cluster) {
    int x,y,acount;
    acount = 0;

    for (x= 0; x < X; x++) {
        for (y = 0; y < Y; y++) {
            free(cluster->cells[x][y]);
            acount += sizeof(struct cell_proc);
        }
    }
#ifdef DEBUG
    printf("Cell free: %db\n", acount);
#endif
}

int cluster_reset(struct cell_cluster *cluster) {
    struct callback_stack tmp = cluster->callbacks;

    /* Destruct/restruct the object then copy back some
     * data we backed up for convenience. */
    cluster_free(cluster);
    cluster_init(cluster);
    memcpy(&cluster->callbacks, &tmp, sizeof cluster->callbacks);
    return 0;
}

int cluster_sched(struct cell_cluster *cluster) {
    struct cell_proc *current;
    int x, y;
    char didstuff;

    while (!cluster->sched_end) {
        didstuff = 0;

        /* Proc a random cell. */
        x = RANDX;
        y = RANDY;
        if (!(current = proc_cell(cluster, x, y, &didstuff))) {
#ifdef DEBUG
            printf("Cell table error: %dx%d\n", x, y);
#endif
            exit(1);
        }

        /* Do any reaping/callbacks that need doing then incremen the tick. */
        cell_reap(cluster, x, y);
        do_callbacks(&cluster->callbacks, cluster->tick, x, y, didstuff);
        cluster->tick++;
    }
    return 0;
}

int get_neighbour_coords(int x, int y, int direction, int *xp, int *yp) {
    /* Using torodial space, which means when a neighvour is requested on an edge
     * it will be wrapped to the other side of the table. */
    switch (direction) {
    case LEFT:
        if (x-1 >= 0) {
            *xp = x-1;
            *yp = y;
        } else {
            *xp = X-1;
            *yp = y;
        }
        break;
    case RIGHT:
        if (x+1 <= X-1) {
            *xp = x+1;
            *yp = y;
        } else {
            *xp = 0;
            *yp = y;
        }
        break;
    case UP:
        if (y-1 >= 0) {
            *xp = x;
            *yp = y-1;
        } else {
            *xp =  x;
            *yp = Y-1;
        }
        break;
    case DOWN:
        if (y+1 <= Y-1) {
            *xp = x;
            *yp = y+1;
        } else {
            *xp = x;
            *yp = 0;
        }
        break;
    default:
        return -1;
    }
    return 0;
}

void cell_pop(const struct cell_cluster *cluster, int x, int y, int gen, int energy, const char instructions[CSIZE]) {
    cluster->cells[x][y]->gen = gen;
    cluster->cells[x][y]->energy = energy;
    if (instructions)
        memcpy(cluster->cells[x][y]->instructions, instructions, CSIZE);
}

void cell_seed(const struct cell_cluster *cluster, int x, int y) {
    int i, imax;
    char seed[CSIZE];

    imax = rand() % CSIZE;
    for (i = 0; i < imax; i++)
        seed[i] = rand() % IEND;

    cluster->cells[x][y]->energy = 10; //rand() % 100; /* SOME VAR */
    cluster->cells[x][y]->gen = 1;
    memcpy(cluster->cells[x][y]->instructions, seed, imax);
}

void cell_mutate(const struct cell_cluster *cluster, int x, int y, unsigned long chance) {
    int i;
    for(i = 0; i < CSIZE; i++)
        if((rand() % chance) == 1)
            cluster->cells[x][y]->instructions[i] = rand() % IEND;
}
