/* SUBMIT ONLY THIS FILE */
/* NAME: ....... */
/* UCI ID: .......*/

// only include standard libraries.
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "simulator.h" // implements

double PI_VAL = 3.14159265358979323846;

// Constants for simulation
#define MAX_CYCLES 1000000  // Maximum number of cycles (10^6)
#define EPSILON 0.02        // Convergence tolerance

void simulate(double *avg_access_time,
              int avg_access_time_len,
              int procs,
              char dist) {
    
    // Run simulation for each memory module count (1 to avg_access_time_len)
    for (int mem_count = 1; mem_count <= avg_access_time_len; mem_count++) {
        // Allocate memory for simulation data structures
        int *proc_requests = (int *)malloc(procs * sizeof(int));             // Memory module requested by each processor
        int *proc_wait_times = (int *)malloc(procs * sizeof(int));           // Wait time for each processor
        int *mem_allocated = (int *)malloc(mem_count * sizeof(int));         // Which processor is using each memory module (-1 if free)
        int *preferred_mems = NULL;                                          // Preferred memory module for each processor (normal distribution)
        
        // For normal distribution, allocate and initialize preferred memory modules
        if (dist == 'n') {
            preferred_mems = (int *)malloc(procs * sizeof(int));
            for (int i = 0; i < procs; i++) {
                preferred_mems[i] = rand_uniform(mem_count);
            }
        }
        
        // Initialize data structures
        for (int i = 0; i < procs; i++) {
            proc_requests[i] = -1;  // No initial request
            proc_wait_times[i] = 0;
        }
        
        for (int i = 0; i < mem_count; i++) {
            mem_allocated[i] = -1;  // -1 indicates memory module is free
        }
        
        // Simulation variables
        int cycle = 0;
        double prev_avg_time = 0.0;
        double curr_avg_time = 0.0;
        double total_access_time = 0.0;
        int total_requests = 0;
        int priority_start = 0;  // Starting index for cyclic priority allocation
        
        // Run simulation until convergence or max cycles
        while (cycle < MAX_CYCLES) {
            // Step 1: Generate new memory requests for all processors
            for (int i = 0; i < procs; i++) {
                // Only generate a new request if the processor doesn't have a pending request
                if (proc_requests[i] == -1) {
                    if (dist == 'u') {
                        // Uniform distribution
                        proc_requests[i] = rand_uniform(mem_count);
                    } else {
                        // Normal distribution around preferred memory module
                        // Use standard deviation proportional to memory count, but with reasonable bounds
                        // int std_dev = mem_count / 8;
                        // if (std_dev < 1) std_dev = 1;
                        // if (std_dev > 64) std_dev = 64;
                        
                        proc_requests[i] = rand_normal_wrap(preferred_mems[i], 5, mem_count);
                    }
                }
            }
            
            // Step 2: Free all memory modules from previous cycle
            for (int i = 0; i < mem_count; i++) {
                if (mem_allocated[i] != -1) {
                    // Record completed access (wait time + 1 cycle for access)
                    int p = mem_allocated[i];
                    int access_time = proc_wait_times[p] + 1;
                    
                    total_access_time += access_time;
                    total_requests++;
                    
                    // Reset processor state
                    proc_requests[p] = -1;
                    proc_wait_times[p] = 0;
                    
                    // Free memory module
                    mem_allocated[i] = -1;
                }
            }
            
            // Step 3: Allocate memory modules to processors using cyclic priority
            // First, increment wait times for all waiting processors
            for (int i = 0; i < procs; i++) {
                if (proc_requests[i] != -1) {
                    proc_wait_times[i]++;
                }
            }
            
            // Create an array to track which processors have been allocated memory this cycle
            int *allocated_this_cycle = (int *)calloc(procs, sizeof(int));
            
            // Process processors in priority order
            for (int i = 0; i < procs; i++) {
                int p = (priority_start + i) % procs;
                
                // Skip if processor has no request or already allocated
                if (proc_requests[p] == -1 || allocated_this_cycle[p]) {
                    continue;
                }
                
                int requested_mem = proc_requests[p];
                
                // Check if requested memory module is free
                if (mem_allocated[requested_mem] == -1) {
                    // Allocate memory module to processor
                    mem_allocated[requested_mem] = p;
                    allocated_this_cycle[p] = 1;
                    
                    // Decrement wait time by 1 since we just incremented it
                    // and this processor is actually getting access this cycle
                    proc_wait_times[p]--;
                }
            }
            
            free(allocated_this_cycle);
            
            // Update priority start for next cycle (cyclic priority)
            priority_start = (priority_start + 1) % procs;
            
            // Calculate current average access time
            if (total_requests > 0) {
                curr_avg_time = total_access_time / total_requests;
                
                // Check for convergence after minimum cycles
                if (fabs((curr_avg_time - prev_avg_time)/curr_avg_time) < EPSILON) {
                    break;
                }
                
                prev_avg_time = curr_avg_time;
            }
            
            cycle++;
        }
        
        // Store the result for this memory module count
        avg_access_time[mem_count - 1] = curr_avg_time;
        
        // Free allocated memory
        free(proc_requests);
        free(proc_wait_times);
        free(mem_allocated);
        if (preferred_mems != NULL) {
            free(preferred_mems);
        }
    }
}

int rand_uniform(int max){
    return rand() % max;
}

int rand_normal_wrap(int mean, int dev, int max){
    static double U, V;
    static int phase = 0;
    double Z;
    if(phase == 0){
        U = (rand() + 1.) / (RAND_MAX + 2.);
        V = rand() / (RAND_MAX + 1.);
        Z = sqrt(-2 *log(U)) * sin(2 * PI_VAL * V);
    }else{
        Z = sqrt(-2 * log(U)) * cos(2 * PI_VAL * V);
    }
    phase = 1 - phase;
    double res = dev*Z + mean;

    // round result up or down depending on whether
    // it is even or odd. This compensates some bias.
    int res_int;
    // if even, round up. If odd, round down.
    if ((int)res % 2 == 0)
        res_int = (int)(res+1);
    else
        res_int = (int)(res);

    // wrap result around max
    int res_wrapped = res_int % max;
    // deal with % of a negative number in C
    if(res_wrapped < 0)
        res_wrapped += max;
    return res_wrapped;
}
