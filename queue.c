/**
 * CSCI3020U Operating Systems Lab 04
 * Copyright (C) 2018 Eyaz Rehman, Rishabh Patel. All Rights Reserved.
 */

#include "queue.h"
#include <stdlib.h>

void queue_push(node_t **head, proc_t process) {
    // Create a new node
    node_t *new_node = (node_t *)malloc(sizeof(node_t));
    new_node->process = process;
    new_node->next = NULL;

    // Check if we have a head, otherwise set the head as the new node
    node_t *temp = (*head);
    if (temp == NULL) {
        *head = new_node;
        return;
    }

    // Find the end of the head
    while (temp->next != NULL) {
        temp = temp->next;
    }

    // Set the last node as the new node
    temp->next = new_node;
}

proc_t queue_pop(node_t **head) {
    node_t *temp = (*head)->next;

    proc_t returned = (*head)->process;
    free(*head);
    *head = temp;

    return returned;
}