/**
 * CSCI3020U Operating Systems Lab 04
 * Copyright (C) 2018 Eyaz Rehman, Rishabh Patel. All Rights Reserved.
 */

#include "dispatcher.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define DISPATCHER_FILE "dispatchlist"
#define PROCESS_NAME "process"

int main(int argc, char **argv) {
    // Initialize
    dispatcher_t dispatcher = { 0 };
    resources_t resources = { 0 };

    // Load processes
    if (!dispatcher_load(&dispatcher, DISPATCHER_FILE, PROCESS_NAME)) {
        printf("Unable to load " DISPATCHER_FILE "!\r\n");
        return 1;
    }

    // Run dispatcher
    while (dispatcher_tick(&dispatcher, &resources)) {
        // Do nothing
    }

    printf("Dispatcher has finished executing all processes\r\n");

    return 0;
}