#include "cache_design.h"

unsigned int set_num;
unsigned int index_num_bits;
unsigned int set_assoc;
unsigned int tag;
unsigned int blockoffset_num_bits;
unsigned int index_bit_value;             
unsigned int tag_value;                                            
unsigned int blockoffset_value; 



cache_structure::cache_structure(int cache_size, int assoc, int block_size, int cache_level, int numberof_address_bits)
{
    //calculating number of sets, number of index bits, number of block offset bits, and tag bits 
    set_num = (cache_size)/(assoc*block_size);
    index_num_bits = log2(set_num);
    blockoffset_num_bits = log2(block_size);
    tag = numberof_address_bits-index_num_bits-blockoffset_num_bits;

    //creating cache contents table for calculated number of sets 
    cacheContent = new cache_content*[set_num];                 //using new oprator for dynamic allocation of memory

    //updating each row of cache content table based on set associativity provided
    for(int i =0; i < set_num; i++)
    {
        cacheContent[i] = new cache_content[assoc];
    }

    //updating value of lru counter in each cache content row
    for(int i = 0; i < set_num; i++)
    {
        for(int j = 0; j< assoc; j++)
        {
            cacheContent[i][j].lru_counter = j;
        }
    }
}

void cache_structure::calculate_cache_reqmt(unsigned int address, int *tag_bits_value, int *block_offset_bits_value, int *index_bits_value)
{
    //using bit manipulation to calculate binary values of tag, index, and block offset
    block_offset_bits_value = (address >> 0) & (pow(2, blockoffset_num_bits) - 1);
    index_bits_value = (address >> blockoffset_num_bits) & (pow(2, index_num_bits) - 1);
    tag_bits_value = (address >> (index_num_bits + blockoffset_num_bits)) & (pow(2, tag) - 1);
}

void cache_structure::cache_action_status(unsigned int address, unsigned int *status)
{

}