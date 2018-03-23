/**
 * CSCI3020U Operating Systems Lab 04
 * Copyright (C) 2018 Eyaz Rehman, Rishabh Patel. All Rights Reserved.
 */

#ifndef QUEUE_H_
#define QUEUE_H_

#include "dispatcher.h"

typedef struct queue_node_t {
    proc_t process;
    struct queue_node_t *next;
} node_t;

void queue_push(node_t **head, proc_t process);
proc_t queue_pop(node_t **head);

#endif