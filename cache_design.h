#ifndef CACHEDESIGN_H
#define CACHEDESIGN_H

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <cmath>

typedef struct cache_content{
    unsigned long tag = 0;
    unsigned int lru_counter = 0;
    bool valid_bit = false;
    bool dirty_bit = false;
}cache_content;

typedef struct memory_config{
    unsigned long int block_size;
    unsigned long int size;
    unsigned long int assoc;
    unsigned long int l2_size;
    unsigned long int l2_assoc;
}memory_config;

typedef struct stream_buffer_config{
    unsigned int N;   //no.of stream buffers
    unsigned int M;   //no.of memory blocks in each stream buffer
}

class cache_structure
{
    public:
    cache_content **cacheContent;
    unsigned int set_num;
    unsigned int index_num_bits;             //1, 2, 3,4
    unsigned int set_assoc;
    unsigned int tag;                          //5, 6, 7, 8
    unsigned int blockoffset_num_bits;          //1, 2,3, 4

    cache_structure(int cache_size, int assoc, int block_size, int cache_level, int number_address_bits);
    void calculate_cache_reqmt(unsigned int address, int *tag_bits_value, int *block_offset_bits_value, int *index_bits_value);
    void cache_action_status(unsigned int address, unsigned int *status);
};

int cache_read_request(cache_content L1cache, cache_content L2cache);
void cache_read_hit();
void cache_read_miss();
void cache_write_request(cache_content L1cache, cache_content L2cache);
void cache_write_hit();
void cache_write_miss();

#endif