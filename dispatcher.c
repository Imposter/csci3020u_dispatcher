/**
 * CSCI3020U Operating Systems Lab 04
 * Copyright (C) 2018 Eyaz Rehman, Rishabh Patel. All Rights Reserved.
 */

#include "dispatcher.h"
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int mem_avail(resources_t *resources, int size, bool realtime) {
    int loc = -1;
    int temp_loc = -1;
    int temp_size = 0;

    // Determine where to find available memory
    int start = 0;
    int limit = MEMORY_SIZE - REALTIME_MEMORY_SIZE;
    if (realtime) {
        start += limit;
        limit += REALTIME_MEMORY_SIZE;
    }

    // Try to find the first block of memory that can fit the requested memory size
    for (int i = start; i < limit; i++) {
        if (temp_size == size) {
            loc = temp_loc;
            break;
        }

        if (resources->memory[i] == 0) {
            if (temp_size == 0) {
                temp_loc = i;
            }
            temp_size++;
        } else {
            temp_size = 0;
        }
    }
    
    // Check if we have enough space to return a block of memory
    if (temp_size == size) {
        loc = temp_loc;
    }

    return loc;
}

void mem_alloc(resources_t *resources, int loc, int size) {
    // Set memory as used
    memset(resources->memory + loc, 1, size);
}

void mem_free(resources_t *resources, int loc, int size) {
    // Set memory as unused
    memset(resources->memory + loc, 0, size);
}

bool device_avail(pid_t *devices, int size, int count) {
    int c = 0;
    for (int i = 0; i < size; i++) {
        if (devices[i] == 0) {
            c++;
        }
    }

    return c >= count;
}

void device_alloc(pid_t *devices, int size, int count, pid_t pid) {
    int c = 0;
    for (int i = 0; i < size; i++) {
        if (devices[i] == 0) {
            devices[i] = pid;
            c++;
        }
    }
}

void device_free(pid_t *devices, int size, pid_t pid) {
    for (int i = 0; i < size; i++) {
        if (devices[i] == pid) {
            devices[i] = 0;
        }
    }
}

bool dispatcher_load(dispatcher_t *dispatcher, char *file_name, char *process_name) {
    FILE *fp = fopen(file_name, "r");
    if (!fp) {
        return false;
    }

    char buffer[BUFFER_SIZE];
    int jobs = 0;

    while (fgets(buffer, BUFFER_SIZE, fp) != NULL && jobs < DISPATCHER_JOB_LIMIT) {
        int p_arrival_time = atoi(strtok(buffer, ", "));
        int p_priority = atoi(strtok(NULL, ", "));
        int p_processor_time = atoi(strtok(NULL, ", "));
        int p_memory = atoi(strtok(NULL, ", "));
        int p_printers = atoi(strtok(NULL, ", "));
        int p_scanners = atoi(strtok(NULL, ", "));
        int p_modems = atoi(strtok(NULL, ", "));
        int p_cds = atoi(strtok(NULL, ", "));

        proc_t process = { 0 };
        strncpy(process.name, process_name, BUFFER_SIZE);
        process.arrival_time = p_arrival_time;
        process.priority = p_priority;
        process.processor_time = p_processor_time;
        process.memory = p_memory;
        process.printers = p_printers;
        process.scanners = p_scanners;
        process.modems = p_modems;
        process.cds = p_cds;

        // Priority must not exceed max queue count
        if (p_priority - 1 > PRIORITY_QUEUE_COUNT) {
            return false;
        }

        if (p_priority == 0) {
            queue_push(&dispatcher->queue_real_time, process);
        } else {
            queue_push(&dispatcher->queues[p_priority - 1], process);
        }

        jobs++;
    }

    fclose(fp);

    return true;
}

bool dispatcher_queue_execute(dispatcher_t *dispatcher, resources_t *resources, int queue_index) {
    node_t **head;
    if (queue_index == -1) {
        // Real time queue
        head = &dispatcher->queue_real_time;
    } else {
        // Priority queue
        head = &dispatcher->queues[queue_index];
    }

    if (*head != NULL) {
        // Get a process
        proc_t p = queue_pop(head);

        // Check if the process is already running and is suspended
        if (p.running && p.suspended) {
            printf("Process %d was suspended, continuing execution\r\n", p.pid);

            // Signal continue
            kill(p.pid, SIGCONT);
            waitpid(p.pid, NULL, WCONTINUED);
        }

        // Attempt to create process
        pid_t pid = p.pid;
        if (pid == 0 && !p.suspended) {
            // Check if we have enough memory to execute this process
            int block;
            if ((block = mem_avail(resources, p.memory, p.priority == PRIORITY_REALTIME)) == -1) {
                // Failed, notify and push back to head
                printf("Not enough memory available to execute process %s, skipping...\r\n", p.name);
                queue_push(head, p);

                return false;
            }

            // Check if we have the devices to execute the process
            if (p.priority != PRIORITY_REALTIME) {
                // Check if we have enough printers available
                if (!device_avail(resources->printers, PRINTER_COUNT, p.printers)) {
                    // Failed, notify and push back to head
                    printf("Not enough printers available to execute process %s, skipping...\r\n", p.name);
                    queue_push(head, p);

                    return false;
                }

                // Check if we have enough scanners available
                if (!device_avail(resources->scanners, SCANNER_COUNT, p.scanners)) {
                    // Failed, notify and push back to head
                    printf("Not enough scanners available to execute process %s, skipping...\r\n", p.name);
                    queue_push(head, p);

                    return false;
                }

                // Check if we have enough modems available
                if (!device_avail(resources->modems, MODEM_COUNT, p.modems)) {
                    // Failed, notify and push back to head
                    printf("Not enough modems available to execute process %s, skipping...\r\n", p.name);
                    queue_push(head, p);

                    return false;
                }

                // Check if we have enough CDs available
                if (!device_avail(resources->cds, CD_COUNT, p.cds)) {
                    // Failed, notify and push back to head
                    printf("Not enough CD drives available to execute process %s, skipping...\r\n", p.name);
                    queue_push(head, p);

                    return false;
                }

                // Allocate devices
                device_alloc(resources->printers, PRINTER_COUNT, p.printers, p.pid);
                device_alloc(resources->scanners, SCANNER_COUNT, p.scanners, p.pid);
                device_alloc(resources->modems, MODEM_COUNT, p.modems, p.pid);
                device_alloc(resources->cds, CD_COUNT, p.cds, p.pid);
            }

            // Allocate memory
            mem_alloc(resources, block, p.memory);
            p.address = block;

            // Flush all output
            fflush(stdout);

            // Fork a process
            pid = fork();

            // Set state
            p.running = true;
            p.suspended = false;
        }

        // Execute
        if (pid == 0) {
            // Execute program, will be started in a suspended state
            int return_code = execl(p.name, "");

            // Exit process with the same exit code as the program executed
            exit(return_code);
        } else {
            // Store process id
            p.pid = pid;

            // Suspend child process
            kill(p.pid, SIGSTOP);

            // Print process information
            printf("Executing %s:\r\n", p.name);
            printf("\t- Process ID: %d\r\n", p.pid);
            printf("\t- Priority: %d\r\n", p.priority);
            printf("\t- Processor Time: %ds\r\n", p.processor_time);
            printf("\t- Memory: %dMB\r\n", p.memory);
            printf("\t- Process Address: %d\r\n", p.address);
            printf("\t- Devices:\r\n");
            printf("\t\t- Printers: %d\r\n", p.printers);
            printf("\t\t- Scanners: %d\r\n", p.scanners);
            printf("\t\t- Modems: %d\r\n", p.modems);
            printf("\t\t- CD Drives: %d\r\n", p.cds);

            // Continue executing after we've printed our lines
            kill(p.pid, SIGCONT);

            // Check process priority, if realtime, then run and wait for exit
            // otherwise run, tick, then move to low priority ONLY if there is another process
            // waiting in a queue
            if (p.priority == PRIORITY_REALTIME || (p.suspended && p.processor_time == 1)) {
                // Wait for process to finish its execution
                sleep(p.processor_time);
                kill(pid, SIGINT);
                waitpid(pid, NULL, 0);

                // Free up memory used by the processprinters
                mem_free(resources, p.address, p.memory);
                p.address = 0;

                // Free up hardware used by the process
                device_free(resources->printers, PRINTER_COUNT, pid);
                device_free(resources->scanners, SCANNER_COUNT, pid);
                device_free(resources->modems, MODEM_COUNT, pid);
                device_free(resources->cds, CD_COUNT, pid);
            } else {
                // Active process, run for a tick length (N seconds)
                sleep(PRIORITY_PROCESSOR_TIME);
                kill(pid, SIGSTOP);

                // Lower processor time
                p.processor_time -= PRIORITY_PROCESSOR_TIME;

                // Set as suspended
                p.suspended = true;

                // Requeue depending on if a process is waiting to be executed
                if (*head != NULL && (*head)->next != NULL) {
                    int next_queue_index = queue_index++;
                    if (next_queue_index == PRIORITY_QUEUE_COUNT) {
                        next_queue_index = PRIORITY_QUEUE_COUNT - 1;
                    }

                    // Requeue to a lower priority queue
                    queue_push(&dispatcher->queues[next_queue_index], p);
                } else {
                    // Requeue to same queue
                    queue_push(head, p);
                }
            }
        }
    }

    return true;
}

bool dispatcher_tick(dispatcher_t *dispatcher, resources_t *resources) {
    // As long as there is a runtime process, give it tick priority
    // otherwise, check which queue has processes to be executed from 1-3 (3 being lowest)
    if (dispatcher->queue_real_time != NULL) {
        dispatcher_queue_execute(dispatcher, resources, -1);
        return true;
    }

    for (int i = 0; i < PRIORITY_QUEUE_COUNT; i++) {
        if (dispatcher->queues[i] != NULL) {
            dispatcher_queue_execute(dispatcher, resources, i);
            return true;
        }
    }

    return false;
}