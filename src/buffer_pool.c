#include "buffer_pool.h"
#include "file_io.h"

#include <stdio.h>
#include <stdlib.h>

void init_buffer_pool(const char *filename, BufferPool *pool){

   

    open_file(&(pool->file), filename);

    for (int i = 0; i < CACHE_PAGE; i++)
    {
        for (int j = 0; j < PAGE_SIZE; j++)
            pool->pages[i].data[j] = 0;

        pool->addrs[i] = (off_t)-1;

        pool->cnt[i] = (size_t)1;

        pool->ref[i] = (size_t)0;


    }
}

void close_buffer_pool(BufferPool *pool){
    for (int i = 0; i < CACHE_PAGE; i++){
        if(pool->addrs[i]>=0)
        write_page(&(pool->pages[i]),&(pool->file),pool->addrs[i]);
    }

    close_file(&(pool->file));

  
    pool=NULL;
}

Page *get_page(BufferPool *pool, off_t addr){
     
     if(pool->file.length<addr){
         printf("get page: pool->file.length<addr\n");
         return NULL;
     }
     int k=-1;
       //hit
      for(int i=0;i<CACHE_PAGE;i++){
          if(pool->addrs[i]==addr){
              k=i;
              pool->ref[i]=1;
              pool->cnt[i]=1;
          }
          else{
               pool->cnt[i]++;
          }
      }

    if(k>=0&&k<CACHE_PAGE)
    return &(pool->pages[k]);


    //miss
     
    //not full 
      for(int i=0;i<CACHE_PAGE;i++){
          if(pool->addrs[i]==-1){
          k=i;
          break;
          }
      }
    
    //full
     size_t tmp=0;
    if(k==-1){
      for(int i=0;i<CACHE_PAGE;i++){
        if(pool->ref[i]==0){
            if(pool->cnt[i]>tmp){
                tmp=pool->cnt[i];
                k=i;
            }
        }
      }

      if(k>=0&&k<CACHE_PAGE)
      write_page(&(pool->pages[k]),&(pool->file),pool->addrs[k]);
    }
    

    //read

    if(k>=0&&k<CACHE_PAGE){
      pool->cnt[k]=1;
      pool->ref[k]=1;

      
    if(pool->file.length==addr){
        //add page to  file
        //init page with 0
        for (int j = 0; j < PAGE_SIZE; j++)
            pool->pages[k].data[j] = 0;
        pool->addrs[k]=addr;

       write_page(&(pool->pages[k]),&(pool->file),addr);
    }
    else{
      read_page(&(pool->pages[k]),&(pool->file),addr);
      pool->addrs[k]=addr;
    }
      return  &(pool->pages[k]);
      }

      //more than CACHE_PAGE(8) use pages
    else{
          printf("All buffer pool is used and not compare\n");
          // system("pause");
           return  NULL;
      }
     
}

void release(BufferPool *pool, off_t addr){
    int i=0;
     for(;i<CACHE_PAGE;i++)
         if(pool->addrs[i]==addr)
         break;
    if(i<CACHE_PAGE){
       pool->ref[i]=0;
    }
    else{
       
        printf("addr is not in buffer_pool\n");
    }
}

int num_valid_pool(BufferPool *pool){
   int sum=0;
    for(int i=0;i<CACHE_PAGE;i++){
        if(pool->ref[i]==0)
        sum++;
    }
    return sum;
}
/* void print_buffer_pool(BufferPool *pool) {
} */

/* void validate_buffer_pool(BufferPool *pool) {
} */
