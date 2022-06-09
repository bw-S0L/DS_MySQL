#include "myjql.h"

#include "buffer_pool.h"
#include "b_tree.h"
#include "table.h"
#include "str.h"

#include <stdlib.h>

typedef struct {
    RID key;
    RID value;
} Record;

void read_record(Table *table, RID rid, Record *record) {
    table_read(table, rid, (ItemPtr)record);
}

RID write_record(Table *table, const Record *record) {
    return table_insert(table, (ItemPtr)record, sizeof(Record));
}

void delete_record(Table *table, RID rid) {
    table_delete(table, rid);
}

BufferPool bp_idx;
Table tbl_rec;
Table tbl_str;

void myjql_init() {
    b_tree_init("rec.idx", &bp_idx);
    table_init(&tbl_rec, "rec.data", "rec.fsm");
    table_init(&tbl_str, "str.data", "str.fsm");
}

void myjql_close() {
    /* validate_buffer_pool(&bp_idx);
    validate_buffer_pool(&tbl_rec.data_pool);
    validate_buffer_pool(&tbl_rec.fsm_pool);
    validate_buffer_pool(&tbl_str.data_pool);
    validate_buffer_pool(&tbl_str.fsm_pool); */
    b_tree_close(&bp_idx);
    table_close(&tbl_rec);
    table_close(&tbl_str);
}

int rid_row_row_cmp(RID a, RID b) {
    Record re_a,re_b;
    StringRecord sre_a,sre_b;
 
    read_record(&tbl_rec,a,&re_a);
    read_record(&tbl_rec,b,&re_b);
  
    read_string(&tbl_str,re_a.key,&sre_a);
   read_string(&tbl_str,re_b.key,&sre_b);
   
   return compare_string_record(&tbl_str,&sre_a,&sre_b);
}

int rid_ptr_row_cmp(void *p, size_t size, RID b) {
    Record re_b;
    StringRecord sre_b;
    size_t n,i=0;
    char* t=(char*)p;
    char*s=(char*)malloc(size+3);
    
    read_record(&tbl_rec,b,&re_b);
   
    read_string(&tbl_str,re_b.key,&sre_b);
   
    n=load_string(&tbl_str,&sre_b,s,size+2);
   
    while(i<size&&i<n){
        if(t[i]>s[i]){
         free(s);
         return 1;
        }
        else if(t[i]<s[i]){
            free(s);
        return -1;
        }
        else
        i++;
    }
     free(s);
    if(size>n)
    return 1;

    if(size<n)
    return -1;

    return 0;
} 

RID insert_handler(RID rid) {
    RID new_rid,rid_key;
    Record re,new_re;
    StringRecord sre;
    size_t n,max_size;
  
    read_record(&tbl_rec,rid,&re);
    read_string(&tbl_str,re.key,&sre);
    max_size=get_string_size(&tbl_str,&sre,re.key);

    char*s=(char*)malloc(max_size+2);
    n=load_string(&tbl_str,&sre,s,max_size+1);

    rid_key=write_string(&tbl_str,s,n);

    new_re.key=rid_key;
    get_rid_block_addr(new_re.value)=-1;  //  can't be used
    get_rid_idx(new_re.value)=-1; 

    new_rid=write_record(&tbl_rec,&new_re);

    return new_rid;
}

void delete_handler(RID rid) {
     
    Record re;
    read_record(&tbl_rec,rid,&re);
     delete_string(&tbl_str,re.key);
     delete_record(&tbl_rec,rid);
}

size_t myjql_get(const char *key, size_t key_len, char *value, size_t max_size) {
  //  printf("myjql_get\n");
    RID rid1;
    Record re;
    StringRecord sre;
    size_t ans,tmp;
    rid1=b_tree_search(&bp_idx,key,key_len,&rid_ptr_row_cmp);
    if(get_rid_block_addr(rid1)<0){ //==-1
    return -1;
    }
   
    read_record(&tbl_rec,rid1,&re);
    read_string(&tbl_str,re.value,&sre);
     ans=get_string_size(&tbl_str,&sre,re.value);
    tmp=load_string(&tbl_str,&sre,value,max_size);
   // printf("ans=%lld    tmp=%lld    max=%lld\n",ans,tmp,max_size);
    return ans;
}

void myjql_set(const char *key, size_t key_len, const char *value, size_t value_len) {
      RID rid1,rid_key,rid_value;
    Record re;
    rid1=b_tree_search(&bp_idx,key,key_len,&rid_ptr_row_cmp);
   // printf("b_search is ok\n");
    if(get_rid_block_addr(rid1)<0){ //==-1
      //don't exist
      
      rid_key=write_string(&tbl_str,key,key_len);
    
       rid_value=write_string(&tbl_str,value,value_len);
    
       re.key=rid_key;
       re.value=rid_value;
       rid1=write_record(&tbl_rec,&re);
   
       b_tree_insert(&bp_idx,rid1,&rid_row_row_cmp,&insert_handler);
    
    }
    else{
       
        b_tree_delete(&bp_idx,rid1,&rid_row_row_cmp,&insert_handler,&delete_handler);
       
        read_record(&tbl_rec,rid1,&re);
       
        delete_string(&tbl_str,re.value);
   
        rid_value=write_string(&tbl_str,value,value_len);
   
         delete_record(&tbl_rec,rid1);
   
         re.value=rid_value;
         rid1=write_record(&tbl_rec,&re);
     
         b_tree_insert(&bp_idx,rid1,&rid_row_row_cmp,&insert_handler);
     
    }
}

void myjql_del(const char *key, size_t key_len) {
   // printf("myjql_del\n");
    RID rid1;
    Record re;
    rid1=b_tree_search(&bp_idx,key,key_len,&rid_ptr_row_cmp);
    if(get_rid_block_addr(rid1)<0){ //==-1
    return ;   //don't exist
    }
    b_tree_delete(&bp_idx,rid1,&rid_row_row_cmp,&insert_handler,&delete_handler);
     read_record(&tbl_rec,rid1,&re);
    delete_string(&tbl_str,re.key);
     delete_string(&tbl_str,re.value);
      delete_record(&tbl_rec,rid1);
}

/* void myjql_analyze() {
    printf("Record Table:\n");
    analyze_table(&tbl_rec);
    printf("String Table:\n");
    analyze_table(&tbl_str);
} */