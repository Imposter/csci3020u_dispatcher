/**
 * CSCI3020U Operating Systems Lab 04
 * Copyright (C) 2018 Eyaz Rehman, Rishabh Patel. All Rights Reserved.
 */

#ifndef DISPATCHER_H_
#define DISPATCHER_H_

#include <stdbool.h>
#include <sys/types.h>

#define BUFFER_SIZE 256

#define MEMORY_SIZE 1024
#define REALTIME_MEMORY_SIZE 64

#define PRINTER_COUNT 2
#define SCANNER_COUNT 1
#define MODEM_COUNT 1
#define CD_COUNT 2

#define PRIORITY_QUEUE_COUNT 3
#define PRIORITY_REALTIME 0
#define PRIORITY_PROCESSOR_TIME 1 // seconds each non-realtime process should 
                                  // execute for before suspending

#define DISPATCHER_JOB_LIMIT 100

// Forward declarations
struct node_t;

typedef struct {
    char memory[MEMORY_SIZE];
    pid_t printers[PRINTER_COUNT];
    pid_t scanners[SCANNER_COUNT];
    pid_t modems[MODEM_COUNT];
    pid_t cds[CD_COUNT];
} resources_t;

typedef struct {
    // Process information
    char name[BUFFER_SIZE];
    int arrival_time;
    int priority;
    int processor_time;
    int memory;
    int printers;
    int scanners;
    int modems;
    int cds;

    // Resources used by the processes
    int address;

    // State information
    pid_t pid;
    bool running;
    bool suspended;
} proc_t;

typedef struct {
    struct node_t *queue_real_time;
    struct node_t *queues[PRIORITY_QUEUE_COUNT];
} dispatcher_t;

int mem_avail(resources_t *resources, int size, bool realtime);
void mem_alloc(resources_t *resources, int loc, int size);
void mem_free(resources_t *resources, int loc, int size);

bool device_avail(pid_t *devices, int size, int count);
void device_alloc(pid_t *devices, int size, int count, pid_t pid);
void device_free(pid_t *devices, int size, pid_t pid);

bool dispatcher_load(dispatcher_t *dispatcher, char *file_name, char *process_name);
bool dispatcher_tick(dispatcher_t *dispatcher, resources_t *resources);

#endif