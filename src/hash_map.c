#include "hash_map.h"

#include <stdio.h>

void hash_table_init(const char *filename, BufferPool *pool, off_t n_directory_blocks) {
    init_buffer_pool(filename, pool);
    /* TODO: add code here */

    Page* p=get_page(pool,(off_t)0);
    HashMapControlBlock*ctr=(HashMapControlBlock*)p->data;
    ctr->free_block_head=-1;
    ctr->n_directory_blocks=n_directory_blocks;
    ctr->max_size=n_directory_blocks*HASH_MAP_DIR_BLOCK_SIZE-1;
    release(pool,(off_t)0);


    for(int i=1;i<=n_directory_blocks;i++){
        p=get_page(pool,(off_t)(i*PAGE_SIZE));
        HashMapDirectoryBlock*dir=(HashMapDirectoryBlock*)p->data;
        for(int j=0;j<HASH_MAP_DIR_BLOCK_SIZE;j++){
            dir->directory[j]=-1;
        }
        release(pool,(off_t)(i*PAGE_SIZE));
    }
}


void hash_table_close(BufferPool *pool) {
    close_buffer_pool(pool);
}

void hash_table_insert(BufferPool *pool, short size, off_t addr) {
    
   //haven't check if  have same addr 
    if( size>=0 && size<PAGE_SIZE){
     
     if(size==PAGE_MASK){//127
          HashMapControlBlock*ctr=(HashMapControlBlock*)get_page(pool,(off_t)0);
          if(ctr->free_block_head==-1){
             ctr->free_block_head=pool->file.length;
             HashMapBlock*mp=(HashMapBlock*)get_page(pool,pool->file.length);
             release(pool,(off_t)0);
              mp->prv=0;
              mp->next=-1;
              mp->n_items=1;
              mp->table[0]=addr;
              for(int i=1;i<HASH_MAP_BLOCK_SIZE;i++)
                mp->table[i]=-1;
            release(pool,pool->file.length-PAGE_SIZE);
          }
          else{
    HashMapBlock*mp=(HashMapBlock*)get_page(pool,ctr->free_block_head);
     off_t pre,now=ctr->free_block_head;
     pre=0;
     release(pool,pre);
     while(mp->n_items==HASH_MAP_BLOCK_SIZE&&mp->next>=0){
         pre=now;
         now=mp->next;
         mp=(HashMapBlock*)get_page(pool,now);
         release(pool,pre);
     }

     if(mp->n_items<HASH_MAP_BLOCK_SIZE){
         mp->table[mp->n_items]=addr;
         mp->n_items++;
         release(pool,now);
     }
     else{
        mp->next=pool->file.length;
        pre=now;
        now=mp->next;
        mp=(HashMapBlock*)get_page(pool,now);
        release(pool,pre);
        mp->prv=pre;
        mp->next=-1;
        mp->n_items=1;
        mp->table[0]=addr;
        for(int i=1;i<HASH_MAP_BLOCK_SIZE;i++)
        mp->table[i]=-1;
        release(pool,now);
     }
          }
     } 
     

    else{
    int  dir_id=size/HASH_MAP_DIR_BLOCK_SIZE+1;
    int  r_id=size%HASH_MAP_DIR_BLOCK_SIZE;
    HashMapDirectoryBlock*dir=(HashMapDirectoryBlock*)get_page(pool,(off_t)(dir_id*PAGE_SIZE));
    if(dir->directory[r_id]==-1){
        dir->directory[r_id]=pool->file.length;
        HashMapBlock*mp=(HashMapBlock*)get_page(pool,pool->file.length);
        release(pool,(off_t)(dir_id*PAGE_SIZE));
        mp->prv=(off_t)(dir_id*PAGE_SIZE);
        mp->next=-1;
        mp->n_items=1;
        mp->table[0]=addr;
        for(int i=1;i<HASH_MAP_BLOCK_SIZE;i++)
        mp->table[i]=-1;
        release(pool,pool->file.length-PAGE_SIZE);
    }
    else{
     HashMapBlock*mp=(HashMapBlock*)get_page(pool,dir->directory[r_id]);
     off_t pre,now=dir->directory[r_id];
     pre=dir_id*PAGE_SIZE;
     release(pool,pre);
     while(mp->n_items==HASH_MAP_BLOCK_SIZE&&mp->next>=0){
         pre=now;
         now=mp->next;
         mp=(HashMapBlock*)get_page(pool,now);
         release(pool,pre);
     }

     if(mp->n_items<HASH_MAP_BLOCK_SIZE){
         mp->table[mp->n_items]=addr;
         mp->n_items++;
         release(pool,now);
     }
     else{
        mp->next=pool->file.length;
        pre=now;
        now=mp->next;
        mp=(HashMapBlock*)get_page(pool,now);
        release(pool,pre);
        mp->prv=pre;
        mp->next=-1;
        mp->n_items=1;
        mp->table[0]=addr;
        for(int i=1;i<HASH_MAP_BLOCK_SIZE;i++)
        mp->table[i]=-1;
        release(pool,now);
     }
    }
    }
    }


    else{
        printf("hash_table_insert: size is out of range\n");
    }
}


off_t hash_table_pop_lower_bound(BufferPool *pool, short size) {

    if(size>=0&&size<PAGE_SIZE){
   
    int  dir_id=size/HASH_MAP_DIR_BLOCK_SIZE+1;
    int  r_id=size%HASH_MAP_DIR_BLOCK_SIZE;
    short tmp_size=size;
    HashMapBlock*mp;
    off_t pre,now,ans;
    HashMapDirectoryBlock*dir=(HashMapDirectoryBlock*)get_page(pool,(off_t)(dir_id*PAGE_SIZE));
    while(dir->directory[r_id]==-1&&tmp_size<PAGE_MASK){
        tmp_size++;
      r_id=tmp_size%HASH_MAP_DIR_BLOCK_SIZE;
      if(r_id==0){
          release(pool,(off_t)(dir_id*PAGE_SIZE));
          dir_id++;
          dir=(HashMapDirectoryBlock*)get_page(pool,(off_t)(dir_id*PAGE_SIZE));
      }
    }

    if(tmp_size==PAGE_MASK){
        release(pool,(off_t)(dir_id*PAGE_SIZE));
        HashMapControlBlock*ctr=(HashMapControlBlock*)get_page(pool,(off_t)0);
        if(ctr->free_block_head==-1){
             release(pool,(off_t)0);
            printf("hash_table_pop_lower_bound:no valid block\n");
            return -1;
        }
        else{
            pre=0;
            now=ctr->free_block_head;
         mp=(HashMapBlock*)get_page(pool,ctr->free_block_head);
         release(pool,pre);
        }
    }
    else{
        pre=(off_t)(dir_id*PAGE_SIZE);
        now=dir->directory[r_id];
        mp=(HashMapBlock*)get_page(pool,now);
        release(pool,pre);
    }

    ans=mp->table[mp->n_items-1];
    release(pool,now);
    hash_table_pop(pool,tmp_size,ans);
    return ans;


    }
    else{
        printf("hash_table_pop_lower_bound: size is out of range\n");
        return -1;
    }
    
}


void hash_table_pop(BufferPool *pool, short size, off_t addr) {
    if(size>=0&&size<PAGE_SIZE){
         off_t pre ,now;
         HashMapBlock*mp,*fmp;
         HashMapControlBlock*ctr;
         HashMapDirectoryBlock*dir;
        if(size==PAGE_MASK){
            pre=0;
            ctr=(HashMapControlBlock*)get_page(pool,pre);
            now=ctr->free_block_head;
            mp=(HashMapBlock*)get_page(pool,now);
        }
        else{
           pre=(off_t)(size/HASH_MAP_DIR_BLOCK_SIZE+1);
           dir=(HashMapDirectoryBlock*)get_page(pool,pre);
           now=dir->directory[size%HASH_MAP_DIR_BLOCK_SIZE];
           mp=(HashMapBlock*)get_page(pool,now);
        }

         for(int i=0;i<mp->n_items;i++){
               if(mp->table[i]==addr){
                   mp->table[i]=-1;
                   mp->n_items--;
                   
                   if(mp->n_items==0){
                    if(size==PAGE_MASK)
                     ctr->free_block_head=mp->next;
                     else
                     dir->directory[size%HASH_MAP_DIR_BLOCK_SIZE]=mp->next;

                     mp->prv=-1;
                     mp->next=-1;
                     release(pool,pre);
                     
                     //del the last block 
                    if(now==pool->file.length-PAGE_SIZE){
                        for(int j=0;j<CACHE_PAGE;j++){
                            if(pool->addrs[j]==now){
                                for(int l=0;l<PAGE_SIZE;l++)
                                   pool->pages[j].data[l]=0;
                                pool->addrs[j]=-1;
                                pool->cnt[j]=1;
                                pool->ref[i]=0;
                            }
                        }
                        pool->file.length-=PAGE_SIZE;
                    }
                    // else{
                    //   HashMapBlock*last_block=get_page(pool,pool->file.length-PAGE_SIZE);
                    //   HashMapBlock*selast_block=
                    // }

                   }
                   else{

                       for(int j=HASH_MAP_BLOCK_SIZE-1;j>i;j--){
                           if(mp->table[j]>=0){
                              mp->table[i]=mp->table[j];
                              mp->table[j]=-1;
                              break;
                           }
                       }
                    release(pool,pre);
                    release(pool,now);
                   }
                   return;
               }
           }

           while(mp->next>=0){
               release(pool,pre);
               pre=now;
               now=mp->next;
               fmp=mp;
               mp=(HashMapBlock*)get_page(pool,now);

               
             for(int i=0;i<mp->n_items;i++){
                  if(mp->table[i]==addr){
                      mp->table[i]=-1;
                      mp->n_items--;
                        

                   if(mp->n_items==0){

                    fmp->next=mp->next;

                     mp->prv=-1;
                     mp->next=-1;
                     release(pool,pre);
                     
                     //del the last block 
                    if(now==pool->file.length-PAGE_SIZE){
                        for(int j=0;j<CACHE_PAGE;j++){
                            if(pool->addrs[j]==now){
                                for(int l=0;l<PAGE_SIZE;l++)
                                   pool->pages[j].data[l]=0;
                                pool->addrs[j]=-1;
                                pool->cnt[j]=1;
                                pool->ref[i]=0;
                            }
                        }
                        pool->file.length-=PAGE_SIZE;
                    }
                    // else{
                    //   HashMapBlock*last_block=get_page(pool,pool->file.length-PAGE_SIZE);
                    //   HashMapBlock*selast_block=
                    // }

                   }
                   else{

                       for(int j=HASH_MAP_BLOCK_SIZE-1;j>i;j--){
                           if(mp->table[j]>=0){
                              mp->table[i]=mp->table[j];
                              mp->table[j]=-1;
                              break;
                           }
                       }
                    release(pool,pre);
                    release(pool,now);
                   }
                   return;
                 }
               
               }
           }
    }
    else{
         printf("hash_table_pop: size is out of range\n");
    }
}

 void print_hash_table(BufferPool *pool) {
    HashMapControlBlock *ctrl = (HashMapControlBlock*)get_page(pool, 0);
    HashMapDirectoryBlock *dir_block;
    off_t block_addr, next_addr;
    HashMapBlock *block;
    int i, j;
    printf("----------HASH TABLE----------\n");
    for (i = 0; i < ctrl->max_size; ++i) {
        dir_block = (HashMapDirectoryBlock*)get_page(pool, (i / HASH_MAP_DIR_BLOCK_SIZE + 1) * PAGE_SIZE);
        if (dir_block->directory[i % HASH_MAP_DIR_BLOCK_SIZE] != -1) {
            printf("%d:", i);
            block_addr = dir_block->directory[i % HASH_MAP_DIR_BLOCK_SIZE];
            while (block_addr != -1) {
                block = (HashMapBlock*)get_page(pool, block_addr);
                printf("  [" FORMAT_OFF_T "]", block_addr);
                printf("{");
                for (j = 0; j < block->n_items; ++j) {
                    if (j != 0) {
                        printf(", ");
                    }
                    printf(FORMAT_OFF_T, block->table[j]);
                }
                printf("}");
                next_addr = block->next;
                release(pool, block_addr);
                block_addr = next_addr;
            }
            printf("\n");
        }
        release(pool, (i / HASH_MAP_DIR_BLOCK_SIZE + 1) * PAGE_SIZE);
    }
    release(pool, 0);
    printf("------------------------------\n");
} 