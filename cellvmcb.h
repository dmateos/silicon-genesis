/** @file
 *  A callback system for the cellvm, functions and times
 *  can be pushed into the callback stack and ran every X clock ticks.
 */
#ifndef _CELLVMCB_H
#define _CELLVMCB_H

/** MAX callback stack size. */
#define CB_MAX 3

/** Prototype for the callback function. */
typedef void(*callback_fptr)(int x, int y, char didstuff);

/** Holds a single callbacks information. */
struct callback_slot {
    /** How many clockticks should occour between callbacks. */
    unsigned int frequency;
    /** Function to call. */
    callback_fptr function;
};

/** Stores callbacks and metadata used for managing their add/removal. */
struct callback_stack {
    /** Array of callback structures. */
    struct callback_slot callbacks[CB_MAX];
    /** Table of active slots, value of 1 if the slot with the same
      * index number is active, 0 if its avalible. */
    char is_active[CB_MAX];
};

/**
 * Add a new callback to the specified stack.
 * @param stack Callback stack to append to.
 * @param function Function that should be called.
 * @param freq Amount of clock ticks between callbacks.
 * @param symbol Symbol name of callback function. (anything contextually meaningful)
 * @return Slot number callback was allocated, -1 on fail.
 */
int add_callback(struct callback_stack *stack, callback_fptr function, unsigned int freq, char *symbol);

/**
 * Deletes a callback from the stack.
 * @param stack Stack to delete callback from.
 * @param callback Slot number callback to remove is in.
 * @return 0 on success, 1 on fail.
 */
int del_callback(struct callback_stack *stack, int slot);

/**
 * Iterate thru the callback structure and run them.
 * @param stack Stack to run callbacks from.
 * @param reltick The current tick interval.
 * @return 0 ok ,1 fail
 */
int do_callbacks(const struct callback_stack *stack, unsigned long reltick, int x, int y, char didstuff);

#endif
