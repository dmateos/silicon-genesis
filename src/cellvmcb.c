/** @file
 *  A callback system for the cellvm, functions and times
 *  can be pushed into the callback stack and ran every X clock ticks.
 */
#include <stdio.h>
#include "cellvmcb.h"
#include "config.h"

/**
 * Get the next free callback slot avalible in the callback_stack.
 * @param stack Stack to get slot from.
 * @return Slot number free on success, -1 on fail.
 */
static int get_slot(const struct callback_stack *stack) {
    int i, ret;

    for (ret = -1, i = 0; i < CB_MAX; i++) {
        if (!stack->is_active[i]) {
            ret = i;
            break;
        }
    }
    return ret;
}

int add_callback(struct callback_stack *stack, callback_fptr function, unsigned int freq, char *symbol) {
    int slot;
    struct callback_slot *ptr;

    /* Get the next free slot, bail if there is none. */
    if ((slot = get_slot(stack)) == -1)
        return -1;

    ptr = &stack->callbacks[slot];
    ptr->function = function;
    ptr->frequency = freq;

    /* Set the entry for this slot in the active table to true. */
    stack->is_active[slot] = 1;

#ifdef DEBUG
    printf("Callback slot %d/%d assigned, freq:1/%d, address:%p symbol:%s\n", slot, CB_MAX, freq, function, symbol);
#endif

    return slot;
}

int del_callback(struct callback_stack *stack, int slot) {
    /* If the slots not active this request is bullshit so dont do anything
     * if it is we set it to not active, deleting the structure it self is redundent as
     * all callbacks are checked against the active list first anyway. */
    if (stack->is_active[slot]) {
        stack->is_active[slot] = 0;
#ifdef DEBUG
        printf("Callback slot %d removed, address:%p\n", slot, stack->callbacks[slot].function);
#endif
        return 0;
    } else
        return 1;
}

int do_callbacks(const struct callback_stack *stack, unsigned long reltick, int x, int y, char didstuff) {
    int i;

    for (i = 0; i < CB_MAX; i++)
        /* If the callback slot is active and it is the right tick % freqnecy. */
        if (stack->is_active[i] && (!(reltick % stack->callbacks[i].frequency)))
            stack->callbacks[i].function(x,y, didstuff);

    return 0;
}
