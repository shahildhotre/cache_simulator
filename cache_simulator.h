#ifndef CACHE_SIMULATOR_H
#define CACHE_SIMULATOR_H

#include <bits/stdc++.h>
#include "cache_design.h"

typedef struct memory_config{
    unsigned long int block_size;
    unsigned long int size;
    unsigned long int assoc;
    // unsigned long int l1_block_size;
    // unsigned long int l2_size;
    // unsigned long int l2_assoc;
}memory_config;

typedef struct memory_performance_param{
    unsigned long int L1_reads = 0;
    unsigned long int L1_read_misses = 0;
    unsigned long int L1_writes = 0;
    unsigned long int L1_write_misses = 0;
    unsigned long int writebacks = 0;
    // unsigned long int L2_reads = 0;
    // unsigned long int L2_read_misses = 0;
    // unsigned long int L2_writes = 0;
    // unsigned long int L2_write_misses = 0;
    // unsigned long int writebacks_from_L2_to_MEM = 0;
    unsigned long int total_MEM_traffic = 0;
}memory_performance_param;

void cache_read_request(int address, );
void cache_read_hit();
void cache_read_miss();
void cache_write_request();
void cache_write_hit();
void cache_write_miss();



#endif