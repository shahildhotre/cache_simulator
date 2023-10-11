#include "cache_design.h"

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
unsigned long address;
unsigned  long L1tag, L2tag;
unsigned int L1_indexvalue, L1_offsetvalue, L2_indexvalue, L2_offsetvalue;

int blocksize ;
int L1size ;
int L1assoc ;
int L2size ;
int L2assoc ;
int Prefetch_N ;
int Prefetch_M ;

int L1set;
int L1index ;
int blockoffset;
int L1tagbitscount;
int L2set;
int L2index;
int L2tagbitscount;

unsigned float L1missrate, L2missrate;


int main(int argc , char *argv[])
{   
    blocksize = atoi(argv[1]);
    L1size = atoi(argv[2]);
    L1assoc = atoi(argv[3]);
    L2size = atoi(argv[4]);
    L2assoc = atoi(argv[5]);
    Prefetch_N = atoi(argv[6]);
    Prefetch_M = atoi(argv[7]);

    L1set = L1size/(blocksize*L1assoc);
    L1index = log2(L1set);
    blockoffset = log2(blocksize);
    L1tagbitscount = 32 - L1index - blockoffset;
    L2set = L2size/(blocksize*L2assoc);
    L2index = log2(L2set);
    L2tagbitscount = 32 - L2index - blockoffset;

    cache_content L1cache[1000][100];
    for(int i = 0;i<L1set ;i++)
        {
            for(int j=0;j<L1assoc;j++)
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
        for(int i = 0;i<L2set ;i++)
            {
                for(int j=0;j<L2assoc;j++)
                {
                    L2cache[i][j].valid_bit = false;
                    L2cache[i][j].dirty_bit = false;
                    L2cache[i][j].tag=0;
                    L2cache[i][j].lru_counter=j;
                }
            }
    }

    FILE *fp;
    fp=fopen(argv[8],"r");     //open and read trace file
    fseek(fp,0,SEEK_SET);      //lof file

    while(1)
    {
        if(feof(fp))          //end of file
        {
            break;
        }
        else
        {
            fscanf(fp,"%c %llx\n", &operation, &address);     //scan file and store read/write opdress
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
    
    
        

    fclose(fp);
    return 0;
}

int L1cache_read_request(cache_content L1cache[1000][100], cache_content L2cache[1000][100])
{
    L1_reads++;
    bool L1hit = false;

    for (int i = 0; i<L1assoc; i++)
    {
        if(L1cache[L1_indexvalue][i].valid_bit == true && L1cache[L1_indexvalue][i].tag == L1tag)
        {
            L1_read_hit++;
            L1hit = true;
            for (int j = 0; j<L1assoc; j++)
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
        int count = 0;
        int replace_cache;
        for(int i=0; i<L1assoc; i++)
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
            for (int i = 1; i<L1assoc; i++)
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
        for (int i = 0; i<L1assoc; i++)
            {
                if (L1cache[L1_indexvalue][i].lru_counter < L1cache[L1_indexvalue][replace_cache].lru_counter)
                {
                    L1cache[L1_indexvalue][i].lru_counter++;
                }
            }
        L1cache[L1_indexvalue][replace_cache].lru_counter=0;

    }
}

int L1cache_write_request(cache_content L1cache[1000][100], cache_content L2cache[1000][100])
{
    L1_writes++;
    bool L1hit = false;

    for (int i = 0; i<L1assoc; i++)
    {
        if(L1cache[L1_indexvalue][i].valid_bit == true && L1cache[L1_indexvalue][i].tag == L1tag)
        {
            L1_write_hit++;
            L1hit = true;
            for (int j = 0; j<L1assoc; j++)
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
        int count = 0;
        int replace_cache;
        for(int i=0; i<L1assoc; i++)
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
            for (int i = 1; i<L1assoc; i++)
            {
                if(L1cache[L1_indexvalue][i].lru_counter==L1assoc-1)
                {
                    replace_cache = i;
                }
            }
        }

        if(L1cache[L1_indexvalue][replace_cache].dirty_bit==true)
        {
            writeback_from_L1_to_L2;
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
        for (int i = 0; i<L1assoc; i++)
            {
                if (L1cache[L1_indexvalue][i].lru_counter < L1cache[L1_indexvalue][replace_cache].lru_counter)
                {
                    L1cache[L1_indexvalue][i].lru_counter++;
                }
            }
        L1cache[L1_indexvalue][replace_cache].lru_counter=0;

    }
}

int L2cache_read_request(cache_content L2cache[1000][100])
{
    L2_reads++;
    bool L2hit = false;

    for (int i = 0; i<L2assoc; i++)
    {
        if(L2cache[L2_indexvalue][i].valid_bit == true && L2cache[L2_indexvalue][i].tag == L2tag)
        {
            L2_read_hit++;
            L2hit = true;
            for (int j = 0; j<L2assoc; j++)
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
        int count = 0;
        int replace_cache;
        for(int i=0; i<L2assoc; i++)
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
            for (int i = 1; i<L2assoc; i++)
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
        for (int i = 0; i<L2assoc; i++)
            {
                if (L2cache[L2_indexvalue][i].lru_counter < L2cache[L2_indexvalue][replace_cache].lru_counter)
                {
                    L2cache[L2_indexvalue][i].lru_counter++;
                }
            }
        L2cache[L2_indexvalue][replace_cache].lru_counter=0;

    }

}

int L2cache_write_request(cache_content L1cache[1000][100], cache_content L2cache[1000][100], int evicted_block)
{
    //get details of evicted block from l1 to l2

    unsigned long long evicted_tag, evicted_address;
    unsigned long long newL2tag;
    unsigned int newL2_indexvalue, newL2_offsetvalue;

    evicted_tag = L1cache[L1_indexvalue][evicted_block].tag;
    evicted_address =(((evicted_tag<<L1index)|L1_indexvalue)<<blockoffset)|L1_offsetvalue;

    //convert L1 values of evicted cache to L2
    newL2tag=evicted_address>>(L2index+blockoffset);
    newL2_indexvalue=(evicted_address<<L2tagbitscount)>>(L2tagbitscount+blockoffset);
    newL2_offsetvalue=(evicted_address<<(L2tagbitscount+L2index))>>(L2tagbitscount+L2index);

    L2_writes++;
    bool L2hit = false;

    for (int i = 0; i<L2assoc; i++)
    {
        if(L2cache[newL2_indexvalue][i].valid_bit == true && L2cache[newL2_indexvalue][i].tag == newL2tag)
        {
            L2_write_hit++;
            L2hit = true;
            for (int j = 0; j<L2assoc; j++)
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
        int count = 0;
        int replace_cache;
        for(int i=0; i<L2assoc; i++)
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
            for (int i = 1; i<L2assoc; i++)
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
        for (int i = 0; i<L2assoc; i++)
            {
                if (L2cache[newL2_indexvalue][i].lru_counter < L2cache[newL2_indexvalue][replace_cache].lru_counter)
                {
                    L2cache[newL2_indexvalue][i].lru_counter++;
                }
            }
        L2cache[newL2_indexvalue][replace_cache].lru_counter=0;

    }
}