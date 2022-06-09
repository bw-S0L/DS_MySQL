#include "b_tree.h"
#include "buffer_pool.h"

#include <stdio.h>

void b_tree_init(const char *filename, BufferPool *pool) {
    init_buffer_pool(filename, pool);
    /* TODO: add code here */
    if(pool->file.length==0){
    BCtrlBlock*ctr=(BCtrlBlock*)get_page(pool,0);
    BNode*node=(BNode*)get_page(pool,PAGE_SIZE);
      //INIT root addr=128
    ctr->root_node=PAGE_SIZE;
    ctr->free_node_head=-1;
    ctr->free_node_tail=-1;
    ctr->free_node_num=0;

    node->n=0;
    node->next=-1;
    node->parent=-1;
    node->leaf=1;
  
    release(pool,0);
    release(pool,PAGE_SIZE);
    }
}

void b_tree_close(BufferPool *pool) {
    close_buffer_pool(pool);
}

RID b_tree_search(BufferPool *pool, void *key, size_t size, b_tree_ptr_row_cmp_t cmp) {
    // RID tm;
    // get_rid_block_addr(tm)=21672;
    // if(cmp(key,size,tm)==0){
    //      printf("\n\n");
    //     BNode*node=(BNode*)get_page(pool,1920);
    //     if(node!=NULL){
    //          printf("1920\n");
    //     for(int i=0;i<node->n;i++)
    //     printf("  %lld  ",get_rid_block_addr(node->row_ptr[i]));
    //     if(!node->leaf){
    //         printf("child  ");
    //         for(int i=0;i<=node->n;i++)
    //     printf("  %lld  ",node->child[i]);
    //  printf("\n");
    //     }
    //     printf("pa=%lld\n",node->parent);
    //     printf("\n");
        
    //     }
    //     release(pool,1920);

    //     system("pause");
    // }
    // don't del ctr , this addr=0
    RID ans;
     off_t pre,now;
    BCtrlBlock*ctr=(BCtrlBlock*)get_page(pool,0);
    pre=now=ctr->root_node;
    release(pool,0);
    BNode*node=(BNode*)get_page(pool,now);
    
        // if(cmp(key,size,tm)==0){
        //  printf("\nwzh  root=%lld\n",now);
        // }
    //INIT root should individually consider
    if(node->n==0){
          get_rid_block_addr(ans)=-1;
          get_rid_idx(ans)=0;
          release(pool,pre);
          return ans;
    }

    while(1){
        int i=lower_bound_pr(node->row_ptr,node->row_ptr+node->n,key,size,cmp)-node->row_ptr;
       
    //     if(cmp(key,size,tm)==0){
    //      printf("\n\n");
    //     printf("now %lld  i= %d   node->n=%lld  value=%lld",now,i,node->n,get_rid_block_addr(node->row_ptr[0]));

    //     printf("\n\n");
      
    // }
        //hit
        if(i!=node->n&&cmp(key,size,node->row_ptr[i])==0){
            //leaf find
            if(node->leaf){
            ans=node->row_ptr[i];
             release(pool,pre);
            return ans;
            }
            //non leaf
            else{

            now=node->child[i+1];
            release(pool,pre);
            node=(BNode*)get_page(pool,now);
            pre=now;
            continue;
            }

        }    
        //not exist
        if(node->leaf){
          get_rid_block_addr(ans)=-1;
          get_rid_idx(ans)=0;
           release(pool,pre);
          return ans;
        }
        //continuing
        now=node->child[i];
        release(pool,pre);
        node=(BNode*)get_page(pool,now);
        pre=now;
    }


}

RID b_tree_insert(BufferPool *pool, RID rid, b_tree_row_row_cmp_t cmp, b_tree_insert_nonleaf_handler_t insert_handler) {
     
    //this value will not be used
    RID nothing;
     get_rid_block_addr(nothing)=-1;
     get_rid_idx(nothing)=0;
    //
    int tag_split=0,handler=0;
    
     off_t pre,now;
    BCtrlBlock*ctr=(BCtrlBlock*)get_page(pool,0);
    pre=now=ctr->root_node;
    release(pool,0);
    BNode*node=(BNode*)get_page(pool,now);
    
    while(1){
        int i=lower_bound_rr(node->row_ptr,node->row_ptr+node->n,rid,cmp)-node->row_ptr;
       // printf("insert i=%d addr=%lld\n",i,now);
         if(i!=node->n&&cmp(rid,node->row_ptr[i])==0){
             //non leaf
            if(!node->leaf){
               now=node->child[i+1];
            release(pool,pre);
            node=(BNode*)get_page(pool,now);
            pre=now;
            continue;
            }
            else{
                printf("b_tree_insert:wrong this rid exists\n");
            }
        } 

        if(node->leaf){
          if(node->n==2*DEGREE-1){
               //  printf("leaf split \n");
               
                //split the leaf
                 handler=1;

                off_t y_addr=get_new_BNode(pool);
                if(y_addr==-1)
                  y_addr=pool->file.length;
                 BNode*y=(BNode*)get_page(pool,y_addr);
                
                 y->n=node->n=DEGREE;
                 y->parent=node->parent;
                 y->leaf=node->leaf;
                
                tag_split=1;
                //   printf("i=%d\n",i);
                // system("pause");
                
                if(i<DEGREE){
                    for(int j=DEGREE-1;j<2*DEGREE-1;j++){
                        y->row_ptr[j-DEGREE+1]=node->row_ptr[j];
                    }
                    for(int j=DEGREE-1;j>i;j--){
                      node->row_ptr[j]=node->row_ptr[j-1];
                    }
                    node->row_ptr[i]=rid;
                }
                else{
                    for(int j=DEGREE,count=0,tag=0;count<DEGREE;count++){
                        if(i!=j||tag){
                           y->row_ptr[count]=node->row_ptr[j];
                           j++;
                        }
                        else{
                            y->row_ptr[count]=rid;
                            tag=1;
                        }
              
                    }
                }
            
                //
                // non leaf   begin
                
                off_t p_addr;
                BNode*p;

                rid=y->row_ptr[0];
                
                while(tag_split){
                p_addr=node->parent;
                if(p_addr>=0){//!=-1
                   
                     p=(BNode*)get_page(pool,p_addr);
                     
                     i=lower_bound_rr(p->row_ptr,p->row_ptr+p->n,rid,cmp)-p->row_ptr;
                     if(p->n==2*DEGREE-1){
                    //     printf(" split contine \n");
                       
                    //     printf("i=%d\n",i);
                     //  system("pause");
                         tag_split=1;

                        off_t tmp_addr=get_new_BNode(pool);
                        if(tmp_addr==-1)
                           tmp_addr=pool->file.length;
                        BNode*tmp=(BNode*)get_page(pool,tmp_addr);
                        
                        p->n=DEGREE-1;
                         tmp->n=DEGREE;
                         tmp->parent=p->parent;
                         tmp->leaf=p->leaf;
                          
                        if(i<DEGREE){
                          
                            for(int j=DEGREE-1;j<2*DEGREE-1;j++){
                                tmp->row_ptr[j-DEGREE+1]=p->row_ptr[j];
                            }
                            for(int j=DEGREE-1;j>i;j--){
                            p->row_ptr[j]=p->row_ptr[j-1];
                            }
                            if(handler){
                             p->row_ptr[i]=insert_handler(rid);
                             handler=0;
                            }
                            else
                            p->row_ptr[i]=rid;
                        

                        }
                        else{
                           
                            for(int j=DEGREE,count=0,tag=0;count<DEGREE;count++){
                                if(i!=j||tag){
                                tmp->row_ptr[count]=p->row_ptr[j];
                                j++;
                                }
                                else{
                                     if(handler){
                                     tmp->row_ptr[count]=insert_handler(rid);
                                     handler=0;
                                     }
                                  else
                                   tmp->row_ptr[count]=rid;
                                   tag=1;
                                }
                            }
                        
                        }

                         rid=p->row_ptr[DEGREE-1];
                        release(pool,now);
                        release(pool,y_addr);
                        //ptr
                        off_t ch[2*DEGREE+1];

                        for(int j=0,count=0,tag=0;count<2*DEGREE+1;count++){
                            if((i+1)!=j||tag){
                                ch[count]=p->child[j];
                                j++;
                            }
                            else{
                            ch[count]=y_addr;
                            tag=1;
                            }
                           
                          
                        }

                     
                         
                         

                        for(int j=0;j<DEGREE;j++){
                           
                           p->child[j]=ch[j];
                           node=(BNode*)get_page(pool,p->child[j]);
                           node->parent=p_addr;
                           release(pool,p->child[j]);
                        }

                         for(int j=0;j<=DEGREE;j++){
                        tmp->child[j]=ch[j+DEGREE];
                         node=(BNode*)get_page(pool,tmp->child[j]);
                           node->parent=tmp_addr;
                           release(pool,tmp->child[j]);
                         }

                       
                        

                                            
                        
                        now=p_addr;
                        y_addr=tmp_addr;
                        node=p;
                        y=tmp;
                       
                     }
                     else{
                       //   printf(" split end \n");
                       //  system("pause");
                         tag_split=0;
                          for(int j=p->n;j>i;j--)
                            p->row_ptr[j]=p->row_ptr[j-1];
                         if(handler){
                             p->row_ptr[i]=insert_handler(rid);
                             handler=0;
                            }
                        else
                         p->row_ptr[i]=rid;

                            for(int j=p->n+1;j>i+1;j--)
                            p->child[j]=p->child[j-1];
                         p->child[i+1]=y_addr;
 
                         p->n++;
                         release(pool,now);
                         release(pool,y_addr);
                         release(pool,p_addr);

                     }
                }
                else{
                   //  printf("root split \n");
                   //  system("pause");
                    p_addr=get_new_BNode(pool);
                    if(p_addr==-1)
                    p_addr=pool->file.length;
                    p=(BNode*)get_page(pool,p_addr);

                    p->n=1;
                    p->next=-1;
                    p->parent=-1;
                    p->child[0]=now;
                    p->child[1]=y_addr;
                     if(handler){
                             p->row_ptr[0]=insert_handler(rid);
                             handler=0;
                     }
                    else
                    p->row_ptr[0]=rid;
                    p->leaf=0;

                    y->parent=p_addr;
                    node->parent=p_addr;
                    release(pool,now);
                    release(pool,y_addr);
                    
                    ctr=(BCtrlBlock*)get_page(pool,0);
                    ctr->root_node=p_addr;
                    release(pool,0);

                    release(pool,p_addr);

                    tag_split=0;
                }
            }
            //end

          }
          else{
              for(int j=node->n;j>i;j--)
                 node->row_ptr[j]=node->row_ptr[j-1];
              node->row_ptr[i]=rid;
              node->n++;
               release(pool,pre);
          }

           return nothing;
        }
        //continuing
        now=node->child[i];
        release(pool,pre);
        node=(BNode*)get_page(pool,now);
        pre=now;

    }

}

void b_tree_delete(BufferPool *pool, RID rid, b_tree_row_row_cmp_t cmp, b_tree_insert_nonleaf_handler_t insert_handler, b_tree_delete_nonleaf_handler_t delete_handler) {
    

    int tag_merge=0;

    off_t pre,now;
    BCtrlBlock*ctr=(BCtrlBlock*)get_page(pool,0);
    pre=now=ctr->root_node;
    release(pool,0);
   
    BNode*node=(BNode*)get_page(pool,now);
     // printf("\nwzh  root=%lld  size=%lld\n\n",now,node->n);
      while(1){
        int i=lower_bound_rr(node->row_ptr,node->row_ptr+node->n,rid,cmp)-node->row_ptr;
         //   printf("finding i=%d  addr=%lld\n",i,now);
         //hit
         if(i!=node->n&&cmp(rid,node->row_ptr[i])==0){
             //leaf find
           if(node->leaf){


                  for(int j=i;j<node->n-1;j++)
                      node->row_ptr[j]=node->row_ptr[j+1];
                  node->n--;
                 //root is leaf    root should not be deleted
                 if(node->parent<0){   //-1
                     release(pool,pre);
                     return ;
                 }

                 //deal leaf
                if(node->n<DEGREE-1){   //before ==
                
                   //redistribute
                    off_t sib_addr,p_addr,son_addr;
                    BNode*sib,*p,*son;
                    int index;

                    p_addr=node->parent;
                    p=(BNode*)get_page(pool,p_addr);

                    for(int j=0;j<=p->n;j++){
                        if(p->child[j]==now){
                            index=j;
                            break;
                        }
                    }

                    if(index>0){
                        sib_addr=p->child[index-1];
                        sib=(BNode*)get_page(pool,sib_addr);
                        if(sib->n>DEGREE-1){
                           for(int j=node->n;j>0;j--){
                               node->row_ptr[j]=node->row_ptr[j-1];
                           }
                           node->row_ptr[0]=sib->row_ptr[sib->n-1];
                           node->n++;
                          //  printf("\n ti1=%lld\n",get_rid_block_addr(node->row_ptr[0]));
                           sib->n--;
                           delete_handler( p->row_ptr[index-1]);
                           p->row_ptr[index-1]=insert_handler(node->row_ptr[0]);
                            release(pool,now);
                            release(pool,sib_addr);
                            release(pool,p_addr);
                          //   printf("1\n");
                           return ;
                        }
                        else{
                            release(pool,sib_addr);
                        }
                    }

                    if(index<p->n){
                        sib_addr=p->child[index+1];
                        sib=(BNode*)get_page(pool,sib_addr);
                        if(sib->n>DEGREE-1){
                             node->row_ptr[node->n]=sib->row_ptr[0];
                             node->n++;
                            for(int j=0;j<sib->n-1;j++){
                               sib->row_ptr[j]=sib->row_ptr[j+1];
                           }
                           sib->n--;
                         //  printf("\n ti2=%lld\n",get_rid_block_addr(sib->row_ptr[0]));
                         delete_handler(p->row_ptr[index]);
                            p->row_ptr[index]=insert_handler(sib->row_ptr[0]);
                            release(pool,now);
                            release(pool,sib_addr);
                            release(pool,p_addr);
                            //  printf("2\n");
                            //  printf("now=%lld  sib=%lld   p=%lld\n",now,sib_addr,p_addr);
                           return ;
                        }

                    }
                    
                    release(pool,sib_addr);
                   
                   //merge
                   tag_merge=1;
                
                //   printf(" merge leaf \n");
                 //   printf("i=%d\n",i);
                   if(index>0){
                        sib_addr=p->child[index-1];
                        sib=(BNode*)get_page(pool,sib_addr);
                        for(int j=0;j<node->n;j++){
                            sib->row_ptr[sib->n+j]=node->row_ptr[j];
                        }
                        
                        sib->n+=node->n;
                        node->n=0;
                        release_new_BNode(pool,now);
                        release(pool,sib_addr);
                        rid=p->row_ptr[index-1];
                   }
                   else{
                        sib_addr=p->child[index+1];
                        sib=(BNode*)get_page(pool,sib_addr);
                        for(int j=0;j<sib->n;j++){
                            node->row_ptr[node->n+j]=sib->row_ptr[j];
                        }
                        node->n+=sib->n;
                        sib->n=0;
                        release_new_BNode(pool,sib_addr);
                        release(pool,now);
                        rid=p->row_ptr[index];
                   }
                 
                    now=p_addr;
                    node=p;
                    //


                   // non leaf begin
                   while(tag_merge){
                       i=lower_bound_rr(node->row_ptr,node->row_ptr+node->n,rid,cmp)-node->row_ptr;
                        
                      //  printf(" non leaf \n");
                      //   printf("i=%d\n",i);
                      delete_handler(node->row_ptr[i]);
                        for(int j=i;j<node->n-1;j++){
                            node->row_ptr[j]=node->row_ptr[j+1];
                            node->child[j+1]=node->child[j+2];
                        }

                        node->n--;
                       if(node->parent>=0){
                            if(node->n<DEGREE-1){
                                //redistribute
                                p_addr=node->parent;
                                p=(BNode*)get_page(pool,p_addr);

                                for(int j=0;j<=p->n;j++){
                                    if(p->child[j]==now){
                                        index=j;
                                        break;
                                    }
                                }

                                if(index>0){
                                    sib_addr=p->child[index-1];
                                    sib=(BNode*)get_page(pool,sib_addr);
                                    if(sib->n>DEGREE-1){
                                    for(int j=node->n;j>0;j--){
                                        node->row_ptr[j]=node->row_ptr[j-1];
                                    }
                                    node->row_ptr[0]=p->row_ptr[index-1];

                                    for(int j=node->n+1;j>0;j--){
                                        node->child[j]=node->child[j-1];
                                    }
                                    node->child[0]=sib->child[sib->n];
                                                        
                                    p->row_ptr[index-1]=sib->row_ptr[sib->n-1];
                                   // printf("\n tidai1=%lld\n",get_rid_block_addr(p->row_ptr[index-1]));
                                    node->n++;
                                    sib->n--;
                                    
                                     son_addr=node->child[0];
                                     son=(BNode*)get_page(pool,son_addr);
                                     son->parent=now;

                                     release(pool,son_addr);
                                        release(pool,now);
                                        release(pool,sib_addr);
                                        release(pool,p_addr);
                                    return ;
                                    }
                                      else{
                                      release(pool,sib_addr);
                                     }
                                }

                                if(index<p->n){
                                    sib_addr=p->child[index+1];
                                    sib=(BNode*)get_page(pool,sib_addr);
                                    if(sib->n>DEGREE-1){
                                        node->row_ptr[node->n]=p->row_ptr[index];
                                        node->child[node->n+1]=sib->child[0];
                                        p->row_ptr[index]=sib->row_ptr[0];

                                        for(int j=0;j<sib->n-1;j++){
                                        sib->row_ptr[j]=sib->row_ptr[j+1];
                                        }
                                        for(int j=0;j<sib->n;j++){
                                            sib->child[j]=sib->child[j+1];
                                        }
                                  //  printf("\n tidai2=%lld\n",get_rid_block_addr( p->row_ptr[index]));
                                    node->n++;
                                    sib->n--;

                                      son_addr=node->child[node->n];
                                     son=(BNode*)get_page(pool,son_addr);
                                     son->parent=now;

                                         release(pool,son_addr);
                                        release(pool,now);
                                        release(pool,sib_addr);
                                        release(pool,p_addr);
                                    return ;
                                    }
                                }
                                
                                release(pool,sib_addr);
                              
                                //merge
                                tag_merge=1;

                                 if(index>0){
                                    sib_addr=p->child[index-1];
                                    sib=(BNode*)get_page(pool,sib_addr);

                                    sib->row_ptr[sib->n]=p->row_ptr[index-1];
                                    for(int j=0;j<node->n;j++){
                                        sib->row_ptr[sib->n+j+1]=node->row_ptr[j];
                                    }
                                    
                                   
                                    for(int j=0;j<node->n+1;j++){
                                        sib->child[sib->n+1+j]=node->child[j];
                                    }
                                    
                                    for(int j=0;j<=node->n;j++){
                                        son_addr=node->child[j];
                                        son=(BNode*)get_page(pool,son_addr);
                                        son->parent=sib_addr;
                                         release(pool,son_addr);
                                    }

                                    sib->n+=node->n+1;
                                    node->n=0;
                                    release_new_BNode(pool,now);
                                    release(pool,sib_addr);
                                    rid=p->row_ptr[index-1];
                                }
                                else{
                                    sib_addr=p->child[index+1];
                                    sib=(BNode*)get_page(pool,sib_addr);

                                    node->row_ptr[node->n]=p->row_ptr[index];
                                    for(int j=0;j<sib->n;j++){
                                        node->row_ptr[node->n+j+1]=sib->row_ptr[j];
                                    }

                                    for(int j=0;j<sib->n+1;j++){
                                        node->child[node->n+1+j]=sib->child[j];
                                    }
                                    for(int j=0;j<=sib->n;j++){
                                        son_addr=sib->child[j];
                                        son=(BNode*)get_page(pool,son_addr);
                                        son->parent=now;
                                         release(pool,son_addr);
                                    }

                                    node->n+=sib->n+1;
                                    sib->n=0;
                                    release_new_BNode(pool,sib_addr);
                                    release(pool,now);
                                    rid=p->row_ptr[index];
                                }

                                 now=p_addr;
                                 node=p;

                            }
                            else{
                                tag_merge=0;
                                
                                 release(pool,now);
                            }
                       }
                       else{
                           tag_merge=0;
                           if(node->n==0){  //change the root
                             ctr=(BCtrlBlock*)get_page(pool,0);
                             ctr->root_node=node->child[0];
                             pre=node->child[0];
                             release(pool,0);
                             release_new_BNode(pool,now);
                          
                             node=(BNode*)get_page(pool,pre);
                            node->parent=-1;
                            release(pool,pre);
                           }
                           else{
                           release(pool,now);
                           }
                       }
                   }
                   
                   //end

                }
                else{
                
                release(pool,pre);
                }

                return ;
            }

            //non leaf
            else{
            now=node->child[i+1];
            release(pool,pre);
            node=(BNode*)get_page(pool,now);
            pre=now;
            continue;
            }
        } 
        //not exist
        if(node->leaf){
      
            printf("\nb_tree_delete: value don't exist\n");
           system("pause");
         return;
        }
        //continuing
        now=node->child[i];
        release(pool,pre);
        node=(BNode*)get_page(pool,now);
        pre=now;


      }

}



/*************************************************/

RID* lower_bound_pr(RID* first, RID* last,void*ptr,size_t size,b_tree_ptr_row_cmp_t cmp){
    int l = 0, r = last - first, ans = r,mid;
    while (l <= r) {
        mid = (l + r) / 2;
        if ( cmp(ptr,size,*(first+mid))>0) {
            l = mid + 1;
        }
        else {
            ans = mid;
            r = mid - 1;
        }
    }
    return first + ans;
}
RID* lower_bound_rr(RID* first, RID* last,RID rid,b_tree_row_row_cmp_t cmp){
     int l = 0, r = last - first, ans = r,mid;
    while (l <= r) {
        mid = (l + r) / 2;
        if ( cmp(rid,*(first+mid))>0) {
            l = mid + 1;
        }
        else {
            ans = mid;
            r = mid - 1;
        }
    }
    return first + ans;
}

off_t get_new_BNode(BufferPool*pool){
      off_t ans;
      BCtrlBlock*ctr=(BCtrlBlock*)get_page(pool,0);
      if(ctr->free_node_num){
           ans=ctr->free_node_head;
           BNode*node=(BNode*)get_page(pool,ans);
           ctr->free_node_head=node->next;
           node->next=-1;
           release(pool,ans);
           ctr->free_node_num--;
           if(ctr->free_node_num==0)
           ctr->free_node_tail=-1;
      }
      else{
         ans= -1;
      }
      release(pool,0);
      return ans;
}
void  release_new_BNode(BufferPool*pool,off_t addr){
     release(pool,addr);
    if(addr==pool->file.length-PAGE_SIZE){
        for(int j=0;j<CACHE_PAGE;j++){
            if(pool->addrs[j]==addr){
             
                pool->addrs[j]=-1;
                pool->cnt[j]=1;
                pool->ref[j]=0;
                break;
            }
        }
        pool->file.length-=PAGE_SIZE;
        return ;
    }

    BCtrlBlock*ctr=(BCtrlBlock*)get_page(pool,0);
    BNode*node;
    if(ctr->free_node_num){
       ctr->free_node_num++;
        node=(BNode*)get_page(pool,ctr->free_node_tail);
        node->next=addr;
        release(pool,ctr->free_node_tail);
        ctr->free_node_tail=addr;
    }
    else{
        ctr->free_node_head=addr;
        ctr->free_node_tail=addr;
        ctr->free_node_num=1;
    }
     release(pool,0);
   
   node=(BNode*)get_page(pool,addr);
   node->next=-1;
   release(pool,addr);
}