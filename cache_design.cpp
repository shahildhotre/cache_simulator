// #include "cache_design.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <cmath>

typedef struct cache_content{
    unsigned long tag = 0;
    unsigned int lru_counter = 0;
    bool valid_bit = false;
    bool dirty_bit = false;
}cache_content;



// int L1cache_read_request(cache_content L1cache[1000][100], cache_content L2cache[1000][100]);
// int L1cache_write_request(cache_content L1cache[1000][100], cache_content L2cache[1000][100]);
// int L2cache_read_request(cache_content L2cache[1000][100]);
// int L2cache_write_request(cache_content L1cache[1000][100], cache_content L2cache[1000][100], int evicted_block);


unsigned int L1_reads = 0;
unsigned int L1_read_misses = 0;
unsigned int L1_read_hit = 0;
unsigned int L1_writes = 0;
unsigned int L1_write_misses = 0;
unsigned int L1_write_hit = 0;
unsigned int writeback_from_L1_to_L2 = 0;
unsigned int L2_reads = 0;
unsigned int L2_read_misses = 0;
unsigned int L2_read_hit = 0;
unsigned int L2_writes = 0;
unsigned int L2_write_misses = 0;
unsigned int L2_write_hit = 0;
unsigned int writebacks_from_L2_to_MEM = 0;
unsigned int total_MEM_traffic = 0;

char operation;
unsigned long long address;
unsigned  long L1tag, L2tag;
unsigned int L1_indexvalue, L1_offsetvalue, L2_indexvalue, L2_offsetvalue;

unsigned int blocksize ;
unsigned int L1size ;
unsigned int L1assoc ;
unsigned int L2size ;
unsigned int L2assoc ;
unsigned int Prefetch_N ;
unsigned int Prefetch_M ;
char *trace_file;

unsigned int L1set;
unsigned int L1index ;
unsigned int blockoffset;
unsigned int L1tagbitscount;
unsigned int L2set;
unsigned int L2index;
unsigned int L2tagbitscount;

float L1missrate, L2missrate;

void L2cache_read_request(cache_content L2cache[1000][100])
{
    L2_reads++;
    bool L2hit = false;

    for (unsigned int i = 0; i<L2assoc; i++)
    {
        if(L2cache[L2_indexvalue][i].valid_bit == true && L2cache[L2_indexvalue][i].tag == L2tag)
        {
            L2_read_hit++;
            L2hit = true;
            for (unsigned int j = 0; j<L2assoc; j++)
            {
                if (L2cache[L2_indexvalue][j].lru_counter < L2cache[L2_indexvalue][i].lru_counter)
                {
                    L2cache[L2_indexvalue][j].lru_counter++;
                }
            }
            L2cache[L2_indexvalue][i].lru_counter = 0;
            break;
        }
    }

    if(L2hit == false)
    {
        L2_read_misses++;
        unsigned int count = 0;
        int replace_cache = 0;
        for(unsigned int i=0; i<L2assoc; i++)
        {
            if(L2cache[L2_indexvalue][i].valid_bit==true)
            {
                count++;
            }
            else
            {
                replace_cache = i;
            }
        }

        if(count == L2assoc)
        {
            //check for LRU cache to find valid_cache to replace
            for (unsigned int i = 1; i<L2assoc; i++)
            {
                if(L2cache[L2_indexvalue][i].lru_counter==L2assoc-1)
                {
                    replace_cache = i;
                }
            }
        }

        if(L2cache[L2_indexvalue][replace_cache].dirty_bit==true)
        {
            writebacks_from_L2_to_MEM++;
        }


        L2cache[L2_indexvalue][replace_cache].tag=L2tag;
        L2cache[L2_indexvalue][replace_cache].dirty_bit=false;
        L2cache[L2_indexvalue][replace_cache].valid_bit=true;
        for (unsigned int i = 0; i<L2assoc; i++)
            {
                if (L2cache[L2_indexvalue][i].lru_counter < L2cache[L2_indexvalue][replace_cache].lru_counter)
                {
                    L2cache[L2_indexvalue][i].lru_counter++;
                }
            }
        L2cache[L2_indexvalue][replace_cache].lru_counter=0;

    }

}

void L2cache_write_request(cache_content L1cache[1000][100], cache_content L2cache[1000][100], int evicted_block)
{
    //get details of evicted block from l1 to l2

    unsigned long long evicted_tag, evicted_address;
    unsigned long long newL2tag;
    unsigned int newL2_indexvalue;
    // unsigned int newL2_offsetvalue;

    evicted_tag = L1cache[L1_indexvalue][evicted_block].tag;
    evicted_address =(((evicted_tag<<L1index)|L1_indexvalue)<<blockoffset)|L1_offsetvalue;

    //convert L1 values of evicted cache to L2
    newL2tag=evicted_address>>(L2index+blockoffset);
    newL2_indexvalue=(evicted_address<<L2tagbitscount)>>(L2tagbitscount+blockoffset);
    // newL2_offsetvalue=(evicted_address<<(L2tagbitscount+L2index))>>(L2tagbitscount+L2index);

    L2_writes++;
    bool L2hit = false;

    for (unsigned int i = 0; i<L2assoc; i++)
    {
        if(L2cache[newL2_indexvalue][i].valid_bit == true && L2cache[newL2_indexvalue][i].tag == newL2tag)
        {
            L2_write_hit++;
            L2hit = true;
            for (unsigned int j = 0; j<L2assoc; j++)
            {
                if (L2cache[newL2_indexvalue][j].lru_counter < L2cache[newL2_indexvalue][i].lru_counter)
                {
                    L2cache[newL2_indexvalue][j].lru_counter++;
                }
            }
            L2cache[newL2_indexvalue][i].lru_counter = 0;
            L2cache[newL2_indexvalue][i].dirty_bit = true;
            break;
        }
    }

    if(L2hit == false)
    {
        L2_write_misses++;
        unsigned int count = 0;
        int replace_cache = 0;
        for(unsigned int i=0; i<L2assoc; i++)
        {
            if(L2cache[newL2_indexvalue][i].valid_bit==true)
            {
                count++;
            }
            else
            {
                replace_cache = i;
            }
        }

        if(count == L2assoc)
        {
            //check for LRU cache to find valid_cache to replace
            for (unsigned int i = 1; i<L2assoc; i++)
            {
                if(L2cache[newL2_indexvalue][i].lru_counter==L2assoc-1)
                {
                    replace_cache = i;
                }
            }
        }

        if(L2cache[newL2_indexvalue][replace_cache].dirty_bit==true)
        {
            writebacks_from_L2_to_MEM++;
        }


        L2cache[newL2_indexvalue][replace_cache].tag=L2tag;
        L2cache[newL2_indexvalue][replace_cache].dirty_bit=true;
        L2cache[newL2_indexvalue][replace_cache].valid_bit=true;
        for (unsigned int i = 0; i<L2assoc; i++)
            {
                if (L2cache[newL2_indexvalue][i].lru_counter < L2cache[newL2_indexvalue][replace_cache].lru_counter)
                {
                    L2cache[newL2_indexvalue][i].lru_counter++;
                }
            }
        L2cache[newL2_indexvalue][replace_cache].lru_counter=0;

    }
}

void L1cache_read_request(cache_content L1cache[1000][100], cache_content L2cache[1000][100])
{
    L1_reads++;
    bool L1hit = false;

    for (unsigned int i = 0; i<L1assoc; i++)
    {
        if(L1cache[L1_indexvalue][i].valid_bit == true && L1cache[L1_indexvalue][i].tag == L1tag)
        {
            L1_read_hit++;
            L1hit = true;
            for (unsigned int j = 0; j<L1assoc; j++)
            {
                if (L1cache[L1_indexvalue][j].lru_counter < L1cache[L1_indexvalue][i].lru_counter)
                {
                    L1cache[L1_indexvalue][j].lru_counter++;
                }
            }
            L1cache[L1_indexvalue][i].lru_counter = 0;
            break;
        }
    }

    if(L1hit == false)
    {
        L1_read_misses++;
        unsigned int count = 0;
        int replace_cache = 0;
        for(unsigned int i=0; i<L1assoc; i++)
        {
            if(L1cache[L1_indexvalue][i].valid_bit==true)
            {
                count++;
            }
            else
            {
                replace_cache = i;
            }
        }

        if(count == L1assoc)
        {
            //check for LRU cache to find valid_cache to replace
            for (unsigned int i = 1; i<L1assoc; i++)
            {
                if(L1cache[L1_indexvalue][i].lru_counter==L1assoc-1)
                {
                    replace_cache = i;
                }
            }
        }

        if(L1cache[L1_indexvalue][replace_cache].dirty_bit==true)
        {
            writeback_from_L1_to_L2++;
            if(L2size != 0)
            {
                L2cache_write_request(L1cache, L2cache, replace_cache);
            }
        }
        
        if(L2size != 0)
        {
            L2cache_read_request(L2cache);
        }

        L1cache[L1_indexvalue][replace_cache].tag=L1tag;
        L1cache[L1_indexvalue][replace_cache].dirty_bit=false;
        L1cache[L1_indexvalue][replace_cache].valid_bit=true;
        for (unsigned int i = 0; i<L1assoc; i++)
            {
                if (L1cache[L1_indexvalue][i].lru_counter < L1cache[L1_indexvalue][replace_cache].lru_counter)
                {
                    L1cache[L1_indexvalue][i].lru_counter++;
                }
            }
        L1cache[L1_indexvalue][replace_cache].lru_counter=0;

    }
}

void L1cache_write_request(cache_content L1cache[1000][100], cache_content L2cache[1000][100])
{
    L1_writes++;
    bool L1hit = false;

    for (unsigned int i = 0; i<L1assoc; i++)
    {
        if(L1cache[L1_indexvalue][i].valid_bit == true && L1cache[L1_indexvalue][i].tag == L1tag)
        {
            L1_write_hit++;
            L1hit = true;
            for (unsigned int j = 0; j<L1assoc; j++)
            {
                if (L1cache[L1_indexvalue][j].lru_counter < L1cache[L1_indexvalue][i].lru_counter)
                {
                    L1cache[L1_indexvalue][j].lru_counter++;
                }
            }
            L1cache[L1_indexvalue][i].lru_counter = 0;
            L1cache[L1_indexvalue][i].dirty_bit = true;
            break;
        }
    }

    if(L1hit == false)
    {
        L1_write_misses++;
        unsigned int count = 0;
        int replace_cache = 0;
        for(unsigned int i=0; i<L1assoc; i++)
        {
            if(L1cache[L1_indexvalue][i].valid_bit==true)
            {
                count++;
            }
            else
            {
                replace_cache = i;
            }
        }

        if(count == L1assoc)
        {
            //check for LRU cache to find valid_cache to replace
            for (unsigned int i = 1; i<L1assoc; i++)
            {
                if(L1cache[L1_indexvalue][i].lru_counter==L1assoc-1)
                {
                    replace_cache = i;
                }
            }
        }

        if(L1cache[L1_indexvalue][replace_cache].dirty_bit==true)
        {
            writeback_from_L1_to_L2++;
            if(L2size != 0)
            {
                L2cache_write_request(L1cache, L2cache, replace_cache);
            }
        }
        
        if(L2size != 0)
        {
            L2cache_read_request(L2cache);
        }

        L1cache[L1_indexvalue][replace_cache].tag=L1tag;
        L1cache[L1_indexvalue][replace_cache].dirty_bit=true;
        L1cache[L1_indexvalue][replace_cache].valid_bit=true;
        for (unsigned int i = 0; i<L1assoc; i++)
            {
                if (L1cache[L1_indexvalue][i].lru_counter < L1cache[L1_indexvalue][replace_cache].lru_counter)
                {
                    L1cache[L1_indexvalue][i].lru_counter++;
                }
            }
        L1cache[L1_indexvalue][replace_cache].lru_counter=0;

    }
}




int main(int argc , char *argv[])
{   
    blocksize = atoi(argv[1]);
    L1size = atoi(argv[2]);
    L1assoc = atoi(argv[3]);
    L2size = atoi(argv[4]);
    L2assoc = atoi(argv[5]);
    Prefetch_N = atoi(argv[6]);
    Prefetch_M = atoi(argv[7]);
    trace_file = argv[8];


    L1set = L1size/(blocksize*L1assoc);
    L1index = log2(L1set);
    blockoffset = log2(blocksize);
    L1tagbitscount = 32 - L1index - blockoffset;
    L2set = L2size/(blocksize*L2assoc);
    L2index = log2(L2set);
    L2tagbitscount = 32 - L2index - blockoffset;

    cache_content L1cache[1000][100];
    for(unsigned int i = 0;i<L1set ;i++)
        {
            for(unsigned int j=0;j<L1assoc;j++)
            {
                L1cache[i][j].valid_bit = false;
                L1cache[i][j].dirty_bit = false;
                L1cache[i][j].tag=0;
                L1cache[i][j].lru_counter=j;
            }
        }

    cache_content L2cache[1000][100];
    if(L2size != 0)
    {
        for(unsigned int i = 0;i<L2set ;i++)
            {
                for(unsigned int j=0;j<L2assoc;j++)
                {
                    L2cache[i][j].valid_bit = false;
                    L2cache[i][j].dirty_bit = false;
                    L2cache[i][j].tag=0;
                    L2cache[i][j].lru_counter=j;
                }
            }
    }

    FILE *fp;
    fp=fopen(trace_file,"r");     //open and read trace file
    if (fp == (FILE *) NULL) {
      // Exit with an error if file open failed.
      printf("Error: Unable to open file %s\n", trace_file);
    }
    fseek(fp,0,SEEK_SET);      //lof file

    while(fscanf(fp,"%c %llx\n", &operation, &address)==2)
    {
        L1tag=address>>(L1index+blockoffset);
        L1_indexvalue=(address<<L1tagbitscount)>>(L1tagbitscount+blockoffset);
        L1_offsetvalue=(address<<(L1tagbitscount+L1index))>>(L1tagbitscount+L1index);


        if(L2size!=0)
        {
            L2tag=address>>(L2index+blockoffset);
            L2_indexvalue=(address<<L2tagbitscount)>>(L2tagbitscount+blockoffset);
            L2_offsetvalue=(address<<(L2tagbitscount+L2index))>>(L2tagbitscount+L2index);
        }

        if(operation=='r')
        {
            L1cache_read_request(L1cache, L2cache);
        }   
        else if(operation == 'w')
        {
            L1cache_write_request(L1cache, L2cache);
        }
    }
        
    //performance mesurement and simulator output
    L1missrate=float((L1_read_misses + L1_write_misses)/(L1_reads+L1_writes));

    if(L2size!=0)
    {
        L2missrate=float((L2_read_misses + L2_write_misses)/(L2_reads+L2_writes));
    }
    else
    {
        L2missrate=0;
    }

    if(L2size!=0)
    {
        total_MEM_traffic= L2_read_misses + L2_write_misses + writebacks_from_L2_to_MEM;
    }
    else
    {
        total_MEM_traffic= L1_read_misses + L1_write_misses + writeback_from_L1_to_L2;
    }

    //simulator output

    printf("===== Simulator configuration =====\n");
    printf("BLOCKSIZE:             %s\n",argv[1]);
    printf("L1_SIZE:               %s\n",argv[2]);
    printf("L1_ASSOC:              %s\n",argv[3]);
    printf("L2_SIZE:               %s\n",argv[4]);
    printf("L2_ASSOC:              %s\n",argv[5]);
    printf("PREF_N:                %s\n",argv[6]);
    printf("PREF_M:                %s\n",argv[7]);
    printf("trace_file:            %s\n",argv[8]);

    printf("===== L1 contents =====\n");
    for(unsigned int i=0; i<L1set; i++)
    {
        printf("set %2d:", i);
        for(unsigned int j=0; j<L1assoc; j++)
        {
            printf("%lu",L1cache[i][j].tag);
        }
        printf("\n");
        
    }

    printf("===== Measurements =====\n");
    printf("a. L1 reads:                 %d\n",L1_reads);
    printf("b. L1 read misses:           %d\n",L1_read_misses);
    printf("c. L1 writes:                %d\n",L1_writes);
    printf("d. L1 write misses:          %d\n",L1_write_misses);
    printf("e. L1 miss rate:             %f\n",L1missrate);
    printf("f. L1 writebacks:            %d\n",writeback_from_L1_to_L2);
    printf("g. L1 prefetch:              %d\n",0);
    printf("h. L2 reads (demand):        %d\n",L2_reads);
    printf("i. L2 read misses (demand):  %d\n",L2_read_misses);
    printf("j. L2 reads (demand):        %d\n",0);
    printf("k. L2 read misses (demand):  %d\n",0);
    printf("l. L2 writes:                %d\n",L2_writes);
    printf("m. L2 write misses:          %d\n",L2_write_misses);
    printf("n. L2 miss rate:             %f\n",L2missrate);
    printf("o. L2 writebacks:            %d\n",writebacks_from_L2_to_MEM);
    printf("p. L2 prefetches:            %d\n",0);
    printf("q. total memory traffic:     %d",total_MEM_traffic);

    
        

    fclose(fp);
    return 0;
}

