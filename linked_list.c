#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include "nand.h"

typedef struct input input_t;
typedef struct output output_t;
typedef struct list list_t;

struct output
{
    nand_t *gate;
    input_t *input;
    output_t *prev_output;
    output_t *next_output;
};

struct list
{
    output_t *head;
    output_t *tail;
};

bool is_empty(list_t l) {
    return l.head == NULL;
}

output_t *insert(list_t *l) {
    output_t *res = malloc(sizeof(output_t));

    if (!res) return res;

    if (is_empty(*l)) {
        l->head = res;
        l->tail = res;
        res->next_output = NULL;
        res->prev_output = NULL;
    }
    else {
        l->tail->next_output = res;
        res->next_output = NULL;
        res->prev_output = l->tail;
        l->tail = res;
    }

    return res;
}

output_t *remove_output(output_t *o, list_t *l) {
    if (o) {
        if (o == l->head && o == l->tail) {
            l->head = NULL;
            l->tail = NULL;
        }
        else if (o == l->head) {
            o->next_output->prev_output = NULL;
            l->head = o->next_output;
        }
        else if (o == l->tail) {
            o->prev_output->next_output = NULL;
            l->tail = o->prev_output;
        }
        else {
            o->next_output->prev_output = o->prev_output;
            o->prev_output->next_output = o->next_output;
        }

        return o;
    }

    return NULL;
}
