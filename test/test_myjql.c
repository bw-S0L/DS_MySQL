#include "myjql.h"

#include "buffer_pool.h"
#include "b_tree.h"
#include "table.h"
#include "str.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#ifdef _WIN32
typedef long long my_off_t;
#else
typedef long my_off_t;
#endif


void init();
void insert(char*a, char *s);
void del(char*s);
int myget(char*a,char*s);
size_t get_total();
size_t get_key(int k,char*s);

#define N 512


char key[N+1],value[N+1];

char random_char() {
    int op = rand() % 3;
    if (op == 0) {
        return 'a' + rand() % 26;
    } else if (op == 1) {
        return 'A' + rand() % 26;
    } else {
        return '0' + rand() % 10;
    }
}

int generate_string(int n,char*buf) {
    int len = rand() % n;
    
    int i;
    for (i = 0; i < len; ++i) {
        buf[i] = random_char();
    }
    buf[len] = 0;
    return len;
}

int test(int max_str_len, int num_op, int out)
{
   
     int flag = 0;
     int i, op, n;
     size_t key_len,value_len;
     myjql_init();
    //system("pause");
    for (i = 0; i < num_op; ++i) {
        op = rand() % 4;
       // printf("op=%d   ",op);
        if ((op == 0||op==3) && get_total() != 0) {  /* read */
            key_len=get_key(rand() % (int)get_total(),key);
            if (out) {
                printf("read  key: ");
                printf("%s  len=%lld",key,key_len);
                printf("\n");
            }
            
            value_len=myjql_get(key,key_len,value,N);
            value[value_len] = 0;
          //   printf("read continue  \n");
            if (myget(value, key)) {
                if (out) {
                    printf("OK\n");
                }
            } else {
              
                printf("get:%s\n",value);
                flag = 1;
                break;  /* for */
            }
           
        } if (op == 1 && get_total() != 0) {  /* delete */
             key_len=get_key(rand() % (int)get_total(),key);
             
            if (out) {
                printf("delete key: ");
               printf("%s  len=%lld",key,key_len);
                printf("\n");
            }
           //  printf("test=1\n");
            myjql_del(key,key_len);
           del(key);
            if (out) {
                 printf("del  end\n");
               
            }
        } else {  /* write */
           // printf("test=2\n");
            key_len = generate_string(max_str_len,key);
            value_len = generate_string(max_str_len,value);
            
            if (out) {
                 printf("write\n");
                printf("insert key: len = %d  , %s  \n ", key_len, key);
                printf("insert value:len = %d , %s \n", value_len, value);
              
            }
             insert(key,value);
              if (out) {
                 printf("write  continue\n");
               
            }
           myjql_set(key,key_len,value,value_len);
            
           
          
             if (out) {
                 printf("write  end\n\n");
               
            }
            
        }
        printf("%d   ",i);
    }

    /* validate_buffer_pool(&table.data_pool); */
    /* validate_buffer_pool(&table.fsm_pool); */
    myjql_close();
  //  system("pause");
    printf("\nflag=%d\n",flag);
  
    return flag;
}

int main()
{
    srand((unsigned int)time(NULL));

    // if (test(10, 30, 1)) {
    //     return 1;
    // }
    if (test(64, 10000, 0)) {
        system("pause");
        return 1;
    }
  
    printf("Success\n");
      //system("pause");
    return 0;
}