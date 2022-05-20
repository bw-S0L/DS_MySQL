#include "str.h"

#include "table.h"

void read_string(Table *table, RID rid, StringRecord *record) {
   
    table_read(table,rid, &(record->chunk));

    record->idx=0;

}

int has_next_char(StringRecord *record) {
    
      RID rid;
      off_t addr;
      if(record->idx<get_str_chunk_size(&(record->chunk))){
            return 1;
      }
      else{
          rid=get_str_chunk_rid(&(record->chunk));
          addr=get_rid_block_addr(rid);
          if(addr==-1){
              return 0;
          }
          else{
              return 1;
          }
      }
}

char next_char(Table *table, StringRecord *record) {
     char a;
     if(record->idx<get_str_chunk_size(&(record->chunk))){
           a= *(get_str_chunk_data_ptr(&(record->chunk))+record->idx);
         (record->idx)++;
            return  a;
            
      }
      else{ 
          //make sure have next chunk
          read_string(table,get_str_chunk_rid(&(record->chunk)),record);
          record->idx=1;
          return   *(get_str_chunk_data_ptr(&(record->chunk)));
      }

}

int compare_string_record(Table *table, const StringRecord *a, const StringRecord *b) {
}

RID write_string(Table *table, const char *data, off_t size) {
       
       StringChunk chunk;
       off_t num,last_size;
       if(size==0){
         num=1;
         last_size=0;
       }
       else{
        num=(size-1)/STR_CHUNK_MAX_LEN+1;
        last_size=(size-1)%STR_CHUNK_MAX_LEN+1;
       }
       RID rid;
       char*ptr;
     //  printf("length=%lld  num=%lld\n",size,num);
       for(off_t i=num;i>0;i--){
           if(i==num){
            get_rid_block_addr(rid)=-1;
            get_rid_idx(rid)=-1;

            get_str_chunk_size(&chunk)=last_size;
           }
           else{
            get_str_chunk_size(&chunk)=STR_CHUNK_MAX_LEN;
           }

            get_str_chunk_rid(&chunk)=rid;
            ptr=get_str_chunk_data_ptr(&chunk);
            //printf("insert:\n");
            for(int j=0;j<get_str_chunk_size(&chunk);j++){
                *(ptr+j)=*(data+STR_CHUNK_MAX_LEN*(i-1)+j);
               // printf("%c",*(data+STR_CHUNK_MAX_LEN*(i-1)+j));
            }
           // printf("\n");
             
             
            rid=table_insert(table,&chunk,calc_str_chunk_size(get_str_chunk_size(&chunk)));
           //  printf("insert %lld  size=%d addr=%lld  idx=%d  allsize=%lld\n",i,get_str_chunk_size(&chunk),get_rid_block_addr(rid),get_rid_idx(rid),calc_str_chunk_size(get_str_chunk_size(&chunk)));
          //  printf("insert ok\n");
       }

       return rid;

}

void delete_string(Table *table, RID rid) {
    StringChunk chunk;
    RID next_rid;
    off_t addr;
    do{
    table_read(table,rid,&chunk);
    next_rid=get_str_chunk_rid(&chunk);
    addr=get_rid_block_addr(next_rid);
    table_delete(table,rid);
    rid=next_rid;
    }while(addr!=-1);
}

/* void print_string(Table *table, const StringRecord *record) {
    StringRecord rec = *record;
    printf("\"");
    while (has_next_char(&rec)) {
        printf("%c", next_char(table, &rec));
    }
    printf("\"");
} */

size_t load_string(Table *table, const StringRecord *record, char *dest, size_t max_size) {
    StringRecord *mp=record;
    size_t i=0;
    
    while(has_next_char(mp)){
          *(dest+i)=next_char(table,mp);
          i++;
         // printf("i=%lld char=%c  idx=%d\n",i,*(dest+i-1),mp->idx);
     }
     //system("pause");
    return i;
}

/* void chunk_printer(ItemPtr item, short item_size) {
    if (item == NULL) {
        printf("NULL");
        return;
    }
    StringChunk *chunk = (StringChunk*)item;
    short size = get_str_chunk_size(chunk), i;
    printf("StringChunk(");
    print_rid(get_str_chunk_rid(chunk));
    printf(", %d, \"", size);
    for (i = 0; i < size; i++) {
        printf("%c", get_str_chunk_data_ptr(chunk)[i]);
    }
    printf("\")");
} */