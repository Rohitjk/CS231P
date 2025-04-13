/* SUBMIT ONLY THIS FILE */
/* NAME: Rohit Jayakrishnan, Joel Anil John, Yanding Zhang*/
/* UCI ID: jayakrr1, janiljoh, yandinz*/

// only include standard libraries.
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "simulator.h" // implements

double PI_VAL = 3.14159265358979323846;

// Constants for simulation
#define MAX_CYCLES 1000000  // Maximum number of cycles (10^6)
#define EPSILON 0.02        // Convergence tolerance

void simulate(double *avg_access_time, int avg_access_time_len, int procs, char dist) {
    for (int mems = 1; mems <= avg_access_time_len; ++mems) {
        int *request = malloc(sizeof(int) * procs);          // current request of each processor
        int *access_count = malloc(sizeof(int) * procs);     // total number of accesses per processor
        int *preferred_module = malloc(sizeof(int) * procs); // only used for normal distribution
        int *wait_time = malloc(sizeof(int) * procs);        // tracks how long each processor waits for access
        double T_c_S = 0.0, T_c_prev = 0.0;
        int stable = 0, cycle = 0, priority_start_idx = 0;

        // Initialize: each processor is assigned its first request
        for (int i = 0; i < procs; ++i) {
            preferred_module[i] = (dist == 'n') ? rand_uniform(mems) : 0;
            request[i] = (dist == 'u') ? rand_uniform(mems)
                                       : rand_normal_wrap(preferred_module[i], 5, mems);
            access_count[i] = 0;
            wait_time[i] = 0;
        }
        // Simulation loop
        while (cycle < MAX_CYCLES) {
            int *module_used = calloc(mems, sizeof(int));
            int first_denied = -1; // for circular priority relabeling
            // Check which modules are used and update access counts
            for (int i = 0; i < procs; ++i) {
                int idx = (priority_start_idx + i) % procs;
                int req = request[idx];
                // Check if the requested module is available
                if (module_used[req] == 0) {
                    module_used[req] = 1;
                    access_count[idx]++;
                    wait_time[idx] = 0;
                    request[idx] = (dist == 'u') ? rand_uniform(mems)
                                                : rand_normal_wrap(preferred_module[idx], 5, mems);
                } else { // Module is busy
                    wait_time[idx]++;
                    if (first_denied == -1) {
                        first_denied = idx;
                    }
                }
            }
            free(module_used);
            // Calculate the average access time for each processor
            double T_c_S_sum = 0.0;
            int active_procs = 0;
            for (int i = 0; i < procs; ++i) {
                // Only consider processors that have made at least one access
                if (access_count[i] > 0) {
                    double avg_T_pi = ((double)wait_time[i] + access_count[i]) / access_count[i];
                    T_c_S_sum += avg_T_pi;
                    active_procs++;
                }
            }
            // Calculate the average access time for the system
            if (active_procs == procs) {
                T_c_S = T_c_S_sum / procs;
                if (cycle > 0 && fabs(1.0 - T_c_prev / T_c_S) < EPSILON) {
                    stable = 1;
                    break;
                }
                T_c_prev = T_c_S;
            }

            cycle++;
            // Use circular relabeling: first processor denied goes first next round
            if (first_denied != -1) {
                priority_start_idx = first_denied;
            } else {
                priority_start_idx = (priority_start_idx + 1) % procs;
            }
        }
        // If the simulation did not converge, set T_c_S to -1.0
        if (!stable) {
            T_c_S = -1.0;
        }

        avg_access_time[mems - 1] = T_c_S;
        // Free allocated memory
        free(request);
        free(access_count);
        free(preferred_module);
        free(wait_time);
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
