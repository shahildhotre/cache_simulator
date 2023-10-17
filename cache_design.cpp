// #include "cache_design.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

typedef struct cache_content{
    unsigned long tag=0 ;
    unsigned int lru_counter=0;
    bool valid_bit=0;
    bool dirty_bit=0 ;
}cache_content;

typedef  struct stream_buffer{
    bool valid_bit = 0;
    unsigned long memory_block = 0;
    int max = 0;
    unsigned int lru_counter = 0;
    
}stream_buffer;


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
unsigned int prefetches = 0;

char operation='0';
unsigned long  address=0;
unsigned  long L1tag, L2tag=0;
unsigned long L1_indexvalue, L1_indexvalue1, L1_indexvalue2, L1_offsetvalue, L1_offsetvalue1, L1_offsetvalue2, L2_indexvalue, L2_offsetvalue=0;

unsigned int blocksize ;
unsigned int L1size ;
unsigned int L1assoc;
unsigned int L2size ;
unsigned int L2assoc ;
unsigned int Prefetch_N ;
unsigned int Prefetch_M ;

uint32_t L1set;
unsigned int L1index ;
unsigned int blockoffset;
unsigned int L1tagbitscount;
unsigned int L2set;
unsigned int L2index;
unsigned int L2tagbitscount;

float L1missrate, L2missrate = 0;
bool SBhitflag;
uint32_t SBpos;
int prhit;
int prmiss;
int Blockpos;



int streambuffer_request(stream_buffer buffer[][32], unsigned long addresscheck, int cachehit)
{
    // printf("adresss---------------%x\n",address);
    // printf("=====Stream Buffer content ====\n");
    // for(unsigned int j=0; j<Prefetch_N; j++)
    // {
    //     printf("%d", buffer[j][0].lru_counter);
    //     for(unsigned int i = 0; i<Prefetch_M; i++)
    //     {
    //         printf(" %x",buffer[j][i].memory_block);
    //     }
    //     printf("\n");
    // }
    bool SBhit = 0;
    unsigned long addresslook = addresscheck>>blockoffset;
    unsigned int most_recent_stream_counter = Prefetch_N-1;
    unsigned int most_recent_stream;
    unsigned int blockpos;
    for(unsigned int k=0; k<Prefetch_N; k++)
    {
        for (unsigned int i=0; i<Prefetch_M; i++)
        {
            if(buffer[k][i].memory_block == addresslook && buffer[k][i].valid_bit == 1)
            {
                // printf("operation %c\n", operation);
                SBhit = 1;
                // if(i!=0 && i!=1)
                // {
                    
                    // printf("streambuffer_hit========== %x\n", addresscheck);
                    // printf("i value %d\n", i+1);
                    // printf("avail address %x\n", buffer[0][0].memory_block);
                    // printf("avail address %x\n", buffer[0][1].memory_block);
                    // printf("avail address %x\n", buffer[0][2].memory_block);
                    // printf("avail address %x\n", buffer[0][3].memory_block);
                    // printf("prefetcehs value %d\n", prefetches);
                // }
                
                // printf("value%d\n", buffer[k][i].lru_counter);
                if(buffer[k][i].lru_counter<= most_recent_stream_counter)
                {
                    // printf("i am here");
                    most_recent_stream_counter = buffer[k][i].lru_counter;
                    most_recent_stream= k;
                    blockpos = i;
                }        
            }
        }
    }

    if(SBhit == 1)
    {
        prefetches = prefetches + (blockpos+1);
        for (unsigned int j=0; j<Prefetch_M; j++)
        {
            buffer[most_recent_stream][j].memory_block= ++addresslook;
            buffer[most_recent_stream][j].valid_bit = 1;
            for (unsigned int i=0; i<Prefetch_N; i++)
                {
                    for(unsigned int k =0; k<Prefetch_M;k++)
                    {
                        if(buffer[i][k].lru_counter< buffer[most_recent_stream][0].lru_counter)
                        {
                            buffer[i][k].lru_counter++;
                        }
                    }
                }
            buffer[most_recent_stream][j].lru_counter = 0;
        }
    }
    

    if(SBhit == 0 && cachehit==0)
    {
        // printf("prefetch value %d\n", prefetches);
        // printf("streambuffer_misses========== %x\n", addresscheck);
        unsigned int least_recent_stream;
        for(unsigned int i=0; i<Prefetch_N; i++)
        {
            if(buffer[i][0].lru_counter==Prefetch_N-1)
            {
                least_recent_stream = i;
            }
        }
        for (unsigned int j=0; j<Prefetch_M; j++)
            {
                prefetches++;
                buffer[least_recent_stream][j].memory_block= ++addresslook;
                // printf("adding address %x\n", buffer[0][j].memory_block);
                buffer[least_recent_stream][j].valid_bit = 1;
                for (unsigned int i=0; i<Prefetch_N; i++)
                {
                    for(unsigned int k =0; k<Prefetch_M;k++)
                    {
                        if(buffer[i][k].lru_counter< buffer[least_recent_stream][0].lru_counter)
                        {
                            buffer[i][k].lru_counter++;
                        }
                    }
                }
                buffer[least_recent_stream][j].lru_counter = 0;
            }
        //  printf("prefetch value %d\n", prefetches);
    }

    return SBhit;
}

void L2cache_read_request(cache_content L2cache[][30], stream_buffer buffer[][32])
{
    // printf("i am in L2 cache to read\n");
    L2_reads++;
    bool L2hit = false;

    for (unsigned int i = 0; i<L2assoc; i++)
    {
        if(L2cache[L2_indexvalue][i].valid_bit == true && L2cache[L2_indexvalue][i].tag == L2tag)
        {
            L2_read_hit++;
            L2hit = true;
            bool sbhit = streambuffer_request(buffer, address, 1);
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
        bool sbhit = streambuffer_request(buffer, address, 0);
        if (sbhit==true)
        {
            L2_read_misses--;
        }
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

void L2cache_write_request(uint32_t **L1cache, unsigned int** L1valid, unsigned int** L1dirty,unsigned int **LRU, cache_content L2cache[][30], int evicted_block, stream_buffer buffer[][32])
{
    // printf("i am in L2 cache to write\n");
    //get details of evicted block from l1 to l2

    unsigned long long evicted_tag, evicted_address;
    unsigned long long newL2tag;
    unsigned int newL2_indexvalue;
    // unsigned int newL2_offsetvalue;

    evicted_tag = L1cache[L1_indexvalue][evicted_block];
    evicted_address =(((evicted_tag<<L1index)|L1_indexvalue)<<blockoffset)|L1_offsetvalue;

    //convert L1 values of evicted cache to L2
    newL2tag=evicted_address>>(L2index+blockoffset);
    // newL2_indexvalue=(evicted_address<<L2tagbitscount)>>(L2tagbitscount+blockoffset);
    newL2_indexvalue = (evicted_address >> blockoffset) % int(pow(2, L2index));
    
    // newL2_offsetvalue=(evicted_address<<(L2tagbitscount+L2index))>>(L2tagbitscount+L2index);

    L2_writes++;
    bool L2hit = false;

    for (unsigned int i = 0; i<L2assoc; i++)
    {
        if(L2cache[newL2_indexvalue][i].valid_bit == true && L2cache[newL2_indexvalue][i].tag == newL2tag)
        {
            L2_write_hit++;
            L2hit = true;
            bool sbhit = streambuffer_request(buffer, evicted_address, 1);
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
        bool sbhit = streambuffer_request(buffer, evicted_address, 0);
        if (sbhit==true)
        {
            L2_write_misses--;
        }
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

void L1cache_read_request(uint32_t **L1cache, unsigned int** L1valid, unsigned int** L1dirty, unsigned int **LRU , cache_content L2cache[][30], stream_buffer buffer[][32])
{
    L1_reads++;
    bool L1hit = false;

    for (unsigned int i = 0; i<L1assoc; i++)
    {
        if(L1valid[L1_indexvalue][i] == 1 && L1cache[L1_indexvalue][i] == L1tag)
        {
            L1_read_hit++;
            L1hit = true;
            if(L2size==0)
            {
                bool sbhit = streambuffer_request(buffer, address, 1);
            }
            
            for (unsigned int j = 0; j<L1assoc; j++)
            {
                if (LRU[L1_indexvalue][j] < LRU[L1_indexvalue][i])
                {
                    LRU[L1_indexvalue][j]++;
                }
            }
            LRU[L1_indexvalue][i] = 0;
            break;
        }
    }

    if(L1hit == false)
    {
        L1_read_misses++;
        // printf("address miss %lx\n", address);
        if(L2size == 0)
        {
            bool sbhit = streambuffer_request(buffer, address, 0);
            if (sbhit==true)
            {
                L1_read_misses--;
            }
        }
        unsigned int count = 0;
        int replace_cache = 0;
        for(unsigned int i=0; i<L1assoc; i++)
        {
            if(L1valid[L1_indexvalue][i]==1)
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
                if(LRU[L1_indexvalue][i]==L1assoc-1)
                {
                    replace_cache = i;
                }
            }
        }

        if(L1dirty[L1_indexvalue][replace_cache]==1)
        {
            writeback_from_L1_to_L2++;
            if(L2size != 0)
            {
                // printf("L2 write\n");
                L2cache_write_request(L1cache, L1valid, L1dirty, LRU, L2cache, replace_cache,  buffer);
            }
        }
        
        if(L2size != 0)
        {
            // printf("L2 read\n");
            L2cache_read_request(L2cache,  buffer);
        }

        L1cache[L1_indexvalue][replace_cache]=L1tag;
        L1dirty[L1_indexvalue][replace_cache]=0;
        L1valid[L1_indexvalue][replace_cache]=1;
        for (unsigned int i = 0; i<L1assoc; i++)
            {
                if (LRU[L1_indexvalue][i] < LRU[L1_indexvalue][replace_cache])
                {
                    LRU[L1_indexvalue][i]++;
                }
            }
        LRU[L1_indexvalue][replace_cache]=0;

    }
    // printf("L1 read done");
}

void L1cache_write_request(uint32_t **L1cache, unsigned int** L1valid, unsigned int** L1dirty, unsigned int **LRU , cache_content L2cache[][30], stream_buffer buffer[][32])
{
    // printf("lets start writing\n");
    L1_writes++;
    bool L1hit = false;

    for (unsigned int i = 0; i<L1assoc; i++)
    {
        // printf("checking.....\n");
        // printf("index_value %lx\n", L1_indexvalue);
        // printf("checking values %d\n", L1cache[L1_indexvalue][i].valid_bit);
        // printf("checking values %d\n", L1cache[L1_indexvalue][i].tag);
        if(L1valid[L1_indexvalue][i] == 1 && L1cache[L1_indexvalue][i] == L1tag)
        {
            // printf("its a write hit");
            L1_write_hit++;
            L1hit = true;
            if(L2size==0)
            {
                bool sbhit = streambuffer_request(buffer, address, 1);
            }
            
            for (unsigned int j = 0; j<L1assoc; j++)
            {
                if (LRU[L1_indexvalue][j] < LRU[L1_indexvalue][i])
                {
                    LRU[L1_indexvalue][j]++;
                }
            }
            LRU[L1_indexvalue][i] = 0;
            L1dirty[L1_indexvalue][i]= 1;
            break;
        }
        else{
            // printf("no cache found\n");
        }
    }

    // printf("L1hit status %d\n", L1hit);
    if(L1hit == false)
    {
        // printf("its a write miss \n");
        //if stream buffer, check in stream buffer
        L1_write_misses++;
        // printf("address miss %lx\n", address);
        if(L2size == 0)
        {
            bool sbhit = streambuffer_request(buffer, address, 0);
            if (sbhit==true)
            {
                L1_write_misses--;
            }
        }

        // int bufferPosition, blockPosition;
        // int blockAddress = address>>blockoffset;
        // std::tie(bufferPosition, blockPosition) = buffer.check_buffer_stream(blockAddress);

        unsigned int count = 0;
        int replace_cache = 0;
        for(unsigned int i=0; i<L1assoc; i++)
        {
            if(L1valid[L1_indexvalue][i]==1)
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
                if(LRU[L1_indexvalue][i]==L1assoc-1)
                {
                    replace_cache = i;
                }
            }
        }

        if(L1dirty[L1_indexvalue][replace_cache]==1)
        {
            // printf("in writeback L1");
            writeback_from_L1_to_L2++;
            if(L2size != 0)
            {
                // printf("L2 write\n");
                L2cache_write_request(L1cache, L1valid, L1dirty, LRU, L2cache, replace_cache,  buffer);
            }
        }
        
        if(L2size != 0)
        {
            // printf("L2 read\n");
            L2cache_read_request(L2cache,  buffer);
        }

        L1cache[L1_indexvalue][replace_cache]=L1tag;
        L1dirty[L1_indexvalue][replace_cache]=1;
        L1valid[L1_indexvalue][replace_cache]=1;
        for (unsigned int i = 0; i<L1assoc; i++)
            {
                if (LRU[L1_indexvalue][i] < LRU[L1_indexvalue][replace_cache])
                {
                    LRU[L1_indexvalue][i]++;
                }
            }
        LRU[L1_indexvalue][replace_cache]=0;

    }
    // printf("L1 write completed");
}


bool compareLRU(const cache_content& a, const cache_content& b)
{
    return a.lru_counter < b.lru_counter;
}


int main(int argc , char *argv[])
{   
    // printf("I am in main");

    char *trace_file;

    blocksize = atoi(argv[1]);
    L1size = atoi(argv[2]);
    L1assoc = atoi(argv[3]);
    L2size = atoi(argv[4]);
    L2assoc = atoi(argv[5]);
    Prefetch_N = atoi(argv[6]);
    Prefetch_M = atoi(argv[7]);
    trace_file = argv[8];

    printf("===== Simulator configuration =====\n");
    printf("BLOCK_SIZE:             %s\n",argv[1]);
    printf("L1_SIZE:               %s\n",argv[2]);
    printf("L1_ASSOC:              %s\n",argv[3]);
    printf("L2_SIZE:               %s\n",argv[4]);
    printf("L2_ASSOC:              %s\n",argv[5]);
    printf("PREF_N:                %s\n",argv[6]);
    printf("PREF_M:                %s\n",argv[7]);
    printf("trace_file:            %s\n\n",argv[8]);


    L1set = L1size/(blocksize*L1assoc);
    L1index = log2(L1set);
    blockoffset = log2(blocksize);
    L1tagbitscount = 32 - L1index - blockoffset;


    
    // cache_content L1cache[1000][30];
    uint32_t **L1cache= new uint32_t*[L1set];
    for (uint32_t i = 0; i < L1set; ++i) {
        L1cache[i] = new uint32_t[L1assoc];
    }

    unsigned int **L1DirtyBit = new unsigned int*[static_cast<unsigned int>(L1set)];
    for (unsigned int i = 0; i < static_cast<unsigned int>(L1set); ++i) {
        L1DirtyBit[i] = new unsigned int[static_cast<unsigned int>(L1assoc)];
    }
    unsigned int **L1ValidBit = new unsigned int*[static_cast<unsigned int>(L1set)];
    for (unsigned int i = 0; i < static_cast<unsigned int>(L1set); ++i) {
        L1ValidBit[i] = new unsigned int[static_cast<unsigned int>(L1assoc)];
    }
    
    unsigned int **LRU = new unsigned int*[static_cast<unsigned int>(L1set)];
    for (unsigned int i = 0; i < static_cast<unsigned int>(L1set); ++i) {
        LRU[i] = new unsigned int[static_cast<unsigned int>(L1assoc)];
    }

    // uint32_t *L1DirtyBit[L1set][L1assoc];
    // uint32_t *L1ValidBit[L1set][L1assoc];
    // int LRU[L1set][L1assoc];
    // printf("why??\n"); 

    for (uint32_t i = 0; i < L1set; ++i) {
        for (uint32_t j = 0; j < L1assoc; ++j) {
            L1cache[i][j] =0;
            L1DirtyBit[i][j] = 0;
            L1ValidBit[i][j] = 0;
            LRU[i][j] = j;
            
        }
    }

    // printf("woahhhhh\n");
    // for(unsigned int i = 0;i<L1set ;i++)
    // {
    //     for(unsigned int j=0;j<L1assoc;j++)
    //     {
    //         L1cache[i][j].valid_bit = false;
    //         L1cache[i][j].dirty_bit = false;
    //         L1cache[i][j].tag=0;
    //         L1cache[i][j].lru_counter=j;
    //     }
    // }

    cache_content L2cache[1000][30];
    if(L2size != 0)
    {
        L2set = L2size/(blocksize*L2assoc);
        L2index = log2(L2set);
        L2tagbitscount = 32 - L2index - blockoffset;
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
    // printf("cache created \n");

    stream_buffer buffer[100][32];
    if((Prefetch_M != 0) && (Prefetch_N != 0))
    {
        for(unsigned int j = 0; j< Prefetch_N; j++)
        {
            for(unsigned int i=0; i<Prefetch_M; i++)
            {
                buffer[j][i].valid_bit = 0;
                buffer[j][i].memory_block = 0;
                buffer[j][i].max = 0;
                buffer[j][i].lru_counter = j;

            }
        }
        
    }

    // Cache *sbuffer;

    FILE *fp;
    fp=fopen(trace_file,"r");     //open and read trace file
    if (fp == (FILE *) NULL) {
      // Exit with an error if file open failed.
      printf("Error: Unable to open file %s\n", trace_file);
    }
    fseek(fp,0,SEEK_SET);      //lof file
    // printf("trace file open\n");
    int done = 0;

    while((fscanf(fp,"%c %lx\n", &operation, &address)==2) )
    {
        
        // printf("i am in while\n");
        // printf("operation %c\n", operation);
        // printf("address %llx\n", address);
        // printf("values %d\n", L1tagbitscount);
        // printf("value %d\n", L1index);
        // printf("value %d\n", blockoffset);
        // if(feof(fp))
        //     {
        //         break;
        //     }

        
        done++;
        L1tag=address>>(L1index+blockoffset);
        // printf("file address======================= %lx\n ",address);
        // printf("operation %c\n", operation);
        // printf(" value %llx\n", L1tag);
        // printf("address %llx\n", address);
        if(L1tagbitscount + blockoffset == 32)
        {
            L1_indexvalue = 0;
        }
        else
        {
            // L1_indexvalue1=(address<<L1tagbitscount);
            // L1_indexvalue2 = (L1tagbitscount+blockoffset);
            // L1_indexvalue=L1_indexvalue1>>L1_indexvalue2;

            L1_indexvalue = (address >> blockoffset) % int(pow(2, L1index));
        }

        
        // printf("value %lx\n", L1_indexvalue);
        
        // L1_offsetvalue1=(address<<(L1tagbitscount+L1index));
        // L1_offsetvalue2= (L1tagbitscount+L1index);
        // L1_offsetvalue = L1_offsetvalue1 >> L1_offsetvalue2;
        L1_offsetvalue = address % int(pow(2, blockoffset));
        // printf("value %lx\n", L1_offsetvalue);

        // printf("L1 contents measured\n");
        if(L2size!=0)
        {
            L2tag=address>>(L2index+blockoffset);
            // L2_indexvalue=(address<<L2tagbitscount)>>(L2tagbitscount+blockoffset);
            L2_indexvalue = (address >> blockoffset) % int(pow(2, L2index));
            // L2_offsetvalue=(address<<(L2tagbitscount+L2index))>>(L2tagbitscount+L2index);
            L2_offsetvalue = address % int(pow(2, blockoffset));
        }

        if(operation=='r')
        {
            // printf("in read\n");
            L1cache_read_request(L1cache, L1ValidBit, L1DirtyBit, LRU, L2cache, buffer);
        }   
        else if(operation == 'w')
        {
            // printf("in write\n");
            L1cache_write_request(L1cache, L1ValidBit, L1DirtyBit, LRU, L2cache, buffer);
            
            
        }
    }
        
    //performance mesurement and simulator output
    L1missrate=float(((float)L1_read_misses + (float)L1_write_misses)/((float)L1_reads+(float)L1_writes));

    if(L2size!=0)
    {
        L2missrate=float(((float)L2_read_misses)/((float)L1_read_misses + (float)L1_write_misses));
        printf("L2missrate %f\n", L2missrate);
    }
    else
    {
        L2missrate=0;
    }

    if(L2size!=0)
    {
        total_MEM_traffic= L2_read_misses + L2_write_misses + writebacks_from_L2_to_MEM + prefetches;
    }
    else
    {
        total_MEM_traffic= L1_read_misses + L1_write_misses + writeback_from_L1_to_L2 + prefetches;
    }

    // simulator output

    // for (int set = 0; set < L1set; ++set) {
    //     std::vector<cache_content> setCacheContent(L1cache[set], L1cache[set] + L1assoc);
    //     std::sort(setCacheContent.begin(), setCacheContent.end(), compareLRU);

    //     // Copy the sorted cache_content back to L1cache
    //     std::copy(setCacheContent.begin(), setCacheContent.end(), L1cache[set]);
    // }


    printf("===== L1 contents =====\n");
    for(unsigned int i=0; i<L1set; i++)
    {

        printf("set %2d:", i);
        for(unsigned int j=0; j<L1assoc; j++)
        {
            for(unsigned int k=0; k<L1assoc; k++)
            {
                if(L1ValidBit[i][k] == 1 && LRU[i][k]==j)
                {
                    printf(" %x",L1cache[i][k]);
                    if(L1DirtyBit[i][k] == 1)
                    {
                        printf(" D");
                    }
                }
            }
            
        
        }
        printf("\n");
        
    }
    printf("\n");

    if(L2size !=0)
    {
        for (unsigned int set = 0; set < L2set; ++set) {
        std::vector<cache_content> setCacheContent(L2cache[set], L2cache[set] + L2assoc);
        std::sort(setCacheContent.begin(), setCacheContent.end(), compareLRU);

        // Copy the sorted cache_content back to L2cache
        std::copy(setCacheContent.begin(), setCacheContent.end(), L2cache[set]);
    }


    printf("===== L2 contents =====\n");
    for(unsigned int i=0; i<L2set; i++)
    {

        printf("set %2d:", i);
        for(unsigned int j=0; j<L2assoc; j++)
        {
            if(L2cache[i][j].valid_bit == 1)
            {
                printf(" %lx",L2cache[i][j].tag);
                if(L2cache[i][j].dirty_bit == true)
                {
                    printf(" D");
                }
            }
        
        }
        printf("\n"); 
    }
    }
    printf("\n");

    if(Prefetch_M!=0 && Prefetch_N!=0)
    {
        printf("===== Stream Buffer(s) contents ====\n");
        unsigned int count = 0;
        int loop = Prefetch_N;
        while(loop)
        {
            for(unsigned int j=0; j<Prefetch_N; j++)
            {
                // printf("%d", buffer[j][0].lru_counter);
                if(buffer[j][0].lru_counter == count)
                {
                    for(unsigned int i = 0; i<Prefetch_M; i++)
                    {
                        printf(" %lx",buffer[j][i].memory_block);
                    }
                    printf("\n");
                }     
            }
            count++;
            loop--;
        }
        
    }
    
    


    printf("===== Measurements =====\n");
    printf("a. L1 reads:                 %d\n",L1_reads);
    printf("b. L1 read misses:           %d\n",L1_read_misses);
    printf("c. L1 writes:                %d\n",L1_writes);
    printf("d. L1 write misses:          %d\n",L1_write_misses);
    printf("e. L1 miss rate:            %.4f\n",L1missrate);
    printf("f. L1 writebacks:            %d\n",writeback_from_L1_to_L2);
    if(L2size == 0)
    {
        printf("g. L1 prefetch:              %d\n",prefetches);
    }
    else{
        printf("g. L1 prefetch:              %d\n",0);    
    }
    
    printf("h. L2 reads (demand):        %d\n",L2_reads);
    printf("i. L2 read misses (demand):  %d\n",L2_read_misses);
    printf("j. L2 reads (prefetch):        %d\n",0);
    printf("k. L2 read misses (prefetch):  %d\n",0);
    printf("l. L2 writes:                %d\n",L2_writes);
    printf("m. L2 write misses:          %d\n",L2_write_misses);
    printf("n. L2 miss rate:             %.4f\n",L2missrate);
    printf("o. L2 writebacks:            %d\n",writebacks_from_L2_to_MEM);
    if(L2size!=0)
    {
        printf("p. L2 prefetches:            %d\n",prefetches);
    }
    else{
        printf("p. L2 prefetches:            %d\n",0);
    }
    
    printf("q. total memory traffic:     %d\n",total_MEM_traffic);

    
        

    // fclose(fp);
    return 0;
}

