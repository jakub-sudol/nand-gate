#ifndef LINKED_LIST_H
#define LINKED_LIST_H

typedef struct output output_t;
typedef struct list list_t;

struct list
{
    output_t* head;
    output_t* tail;
};

output_t* remove_output(output_t *, list_t *);
output_t* insert(list_t *);

#endif 