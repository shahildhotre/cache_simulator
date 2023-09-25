#ifndef CACHEDESIGN_H
#define CACHEDESIGN_H

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <cmath>

typedef struct cache_row{
    unsigned int tag = 0;
    unsigned int lru_counter = 0;
    bool valid_bit = false;
    bool dirty_bit = false;
}cache_content;

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
    void cache_action_status(unsigned int address, unsigned int *array);
};

#endif