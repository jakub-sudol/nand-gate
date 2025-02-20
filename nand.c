#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include "linked_list.h"

typedef struct nand nand_t;
typedef struct input input_t;
typedef struct output output_t;

// list is linked list with operations:
// insert - creats new element of the list and returns pointer to it
// remove_output - removes element of the list and returns pointer to it

struct nand
{
    // inputs - table of inputs
    // outputs - list of outputs
    // default values of gate:
    // is_visited = 0, is_calculated = 0, value_of_signal = 0, max_path = 0

    input_t *inputs;
    list_t outputs;
    int number_of_connected_outputs;
    unsigned number_of_inputs;
    bool is_visited;
    bool is_calculated;
    bool value_of_signal_at_output;
    size_t max_path;
};

struct input
{
    // gate - connected output's gate
    // value - value of signal connected to input
    // default values of input:
    // if connected with gate: is_singal = false, value = NULL
    // if connected with boolean signal: gate = NULL, output = NULL

    nand_t *gate;
    output_t *output;
    bool is_signal;
    bool *value;
};

struct output
{
    // gate - connected input's gate

    nand_t *gate;
    input_t *input;
    output_t *prev_output;
    output_t *next_output;
};

ssize_t max(ssize_t a, ssize_t b) {
    if (a > b) return a;
    else return b;
}

bool min(bool a, bool b) {
    if (a < b) return a;
    else return b;
}

void turn(bool *a) {
    if (*a) *a = false;
    else *a = true;
}

// n - number of new gate inputs
// creats new nand gate
nand_t *nand_new(unsigned n) {
    nand_t *res = ( nand_t * )malloc(sizeof(nand_t));

    if (!res) {
        errno = ENOMEM;
        return NULL;
    }

    res->inputs = ( input_t * )calloc(n, sizeof(input_t));

    if (!res->inputs) {
        free(res);
        errno = ENOMEM;
        return NULL;
    }

    res->outputs.head = NULL;
    res->outputs.tail = NULL;
    res->is_calculated = false;
    res->is_visited = false;
    res->number_of_connected_outputs = 0;
    res->number_of_inputs = n;
    res->value_of_signal_at_output = 0;
    res->max_path = 0;

    return res;
}

// g_out - output's gate, g_in - input's gate 
// connects g_out's output to k'th g_in's input
// returns 0 if the operation was succesful, otherwise -1
int nand_connect_nand(nand_t *g_out, nand_t *g_in, unsigned k) {
    if (!g_out || !g_in || k >= g_in->number_of_inputs) {
        errno = EINVAL;
        return -1;
    }

    output_t *output = insert(&g_out->outputs);

    if (!output) {
        errno = ENOMEM;
        return -1;
    }

    output->gate = g_in;
    output->input = &g_in->inputs[k];

    if (g_in->inputs[k].output) {
        g_in->inputs[k].gate->number_of_connected_outputs--;
        free(remove_output(g_in->inputs[k].output,
            &g_in->inputs[k].gate->outputs));
    }

    g_in->inputs[k].is_signal = false;
    g_in->inputs[k].value = NULL;
    g_in->inputs[k].output = output;
    g_in->inputs[k].gate = g_out;
    g_out->number_of_connected_outputs++;

    return 0;
}

// s - bool, g - gate
// connects boolean signal s to k'th g input
// returns 0 if the operation was succesful, otherwise -1
int nand_connect_signal(bool const *s, nand_t *g, unsigned k) {
    if (!s || !g || k >= g->number_of_inputs) {
        errno = EINVAL;
        return -1;
    }

    if (g->inputs[k].output) {
        g->inputs[k].gate->number_of_connected_outputs--;
        free(remove_output(g->inputs[k].output, &g->inputs[k].gate->outputs));
    }

    g->inputs[k].gate = NULL;
    g->inputs[k].is_signal = true;
    g->inputs[k].output = NULL;
    g->inputs[k].value = ( bool * )s;

    return 0;
}

// g - gate
// returns gate or boolean signal connected to k'th g's input
void *nand_input(nand_t const *g, unsigned k) {
    if (g && k < g->number_of_inputs && (g->inputs[k].gate || g->inputs[k].is_signal)) {
        if (g->inputs[k].is_signal) return g->inputs[k].value;
        else return g->inputs[k].gate;
    }
    else if (!g || k >= g->number_of_inputs) {
        errno = EINVAL;
    }
    else {
        errno = 0;
    }
    return NULL;
}

// removes all input values
void clear_input(input_t *input) {
    input->gate = NULL;
    input->is_signal = false;
    input->output = NULL;
    input->value = NULL;
}

// g - gate
// removes g
void nand_delete(nand_t *g) {
    if (g) {
        for (unsigned i = 0; i < g->number_of_inputs; i++) {
            if (g->inputs[i].output) {
                g->inputs[i].gate->number_of_connected_outputs--;//nowa zmiana
                free(remove_output(g->inputs[i].output, &g->inputs[i].gate->outputs));
            }
        }

        output_t *interator = g->outputs.head;

        while (1) {
            if (!interator) break;

            clear_input(interator->input);

            if (interator->next_output) {
                interator = interator->next_output;
                free(interator->prev_output);
            }
            else {
                free(interator);
                break;
            }
        }
        free(g->inputs);
        free(g);
    }
}

// g - gate
// returns number of connected g outputs
// if the operation was not succesful returns -1
ssize_t nand_fan_out(nand_t const *g) {
    if (!g) {
        errno = EINVAL;
        return -1;
    }

    return g->number_of_connected_outputs;

}

// g - gate
// returns gate connected to k'th g output
nand_t *nand_output(nand_t const *g, ssize_t k) {
    ssize_t temp = 0;
    output_t *iterator = g->outputs.head;

    while (temp != k) {
        temp++;
        iterator = iterator->next_output;
    }

    return iterator->gate;
}

// returns the critical path and sets the value of signal at output 
// returns -1 if the operation was not succesful
ssize_t evaluate(nand_t *g) {
    if (g->is_visited) return g->max_path;

    ssize_t res = 0;
    g->value_of_signal_at_output = 1;
    g->is_visited = true;

    for (unsigned i = 0; i < g->number_of_inputs; i++) {
        if (g->inputs[i].gate) {
            if (g->inputs[i].gate->is_visited
                && g->inputs[i].gate->is_calculated) {
                res = max(res, 1 + g->inputs[i].gate->max_path);
                g->value_of_signal_at_output = min(
                    g->value_of_signal_at_output,
                    g->inputs[i].gate->value_of_signal_at_output);
            }
            else if (g->inputs[i].gate->is_visited) {
                errno = ECANCELED;
                return -1;
            }
            else {
                ssize_t temp = evaluate(g->inputs[i].gate);
                if (temp == -1) return -1;
                else {
                    g->value_of_signal_at_output = min(
                        g->value_of_signal_at_output,
                        g->inputs[i].gate->value_of_signal_at_output);
                    res = max(res, 1 + temp);
                }
            }
        }
        else if (g->inputs[i].is_signal) {
            res = max(res, 1);
            g->value_of_signal_at_output = min(
                g->value_of_signal_at_output,
                *g->inputs[i].value);
        }
        else {
            errno = ECANCELED;
            return -1;
        }
    }

    turn(&g->value_of_signal_at_output);

    g->max_path = res;
    g->is_calculated = true;

    return res;
}

// sets deafault values of g gate
void set_default_values(nand_t *g) {
    if (g && g->is_visited) {
        g->is_calculated = false;
        g->is_visited = false;
        g->max_path = 0;
        g->value_of_signal_at_output = false;

        for (size_t i = 0; i < g->number_of_inputs; i++) {
            if (g->inputs[i].gate) {
                set_default_values(g->inputs[i].gate);
            }
        }
    }
}

// returns the critical path of gates from the g array
// m - size of array
// calulates the outputs values
// returns -1 if the opearation was not succesful
ssize_t nand_evaluate(nand_t **g, bool *s, size_t m) {
    if (!m || !g || !s) {
        errno = EINVAL;
        return -1;
    }

    ssize_t res = 0;

    for (size_t i = 0; i < m; i++) {
        if (!g[i]) {
            errno = EINVAL;
            return -1;
        }

        ssize_t temp = evaluate(g[i]);

        if (temp == -1) {
            for (size_t j = 0; j <= i; j++) {
                set_default_values(g[j]);
            }
            return -1;
        }
        res = max(res, temp);
        s[i] = g[i]->value_of_signal_at_output;
    }

    for (size_t i = 0; i < m; i++) {
        set_default_values(g[i]);
    }

    return res;
}