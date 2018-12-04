#ifndef SLIST_H_
#define SLIST_H_

#define SLIST_LENGTH_MAX  100//������󳤶�
#define SLIST_NULL  0
typedef struct snode
{
	void 		  *Data;
	unsigned char Mem_iterm;
	struct snode  *Next;
}Snode;
	
typedef struct  
{  
	Snode 		  *Head;    /*ָ��ͷ�ڵ�*/  
	Snode 		  *Tail;    /*ָ��β�ڵ�*/  
    unsigned char Size;  	/*�����С*/ 
    unsigned char MaxSize;  /*��󳤶�*/ 
}Slist; 

void slist_init(Slist *list,unsigned char maxsize);
void *slist_node_get_data(Snode *pnode);
Snode *slist_get_head(Slist *pdlist);
Snode *slist_get_tial(Slist *pdlist);
unsigned char slist_is_empty(Slist *plist); 
unsigned char slist_is_full(Slist *plist) ;
unsigned char slist_get_size(Slist *plist); 
Snode *slist_get_next(Snode *pnode);
void slist_node_remove(Slist *pdlist,Snode *pnode);
void slist_node_insert_after(Slist *pdlist,Snode *posnode,Snode *pnodein);
Snode *slist_node_make(void *pdata);
void slist_node_insert_queue(Slist *pdlist,Snode *pnodein);
Snode *slist_malloc_mem(void);
void slist_node_free(Snode *pnode);

#endif /* SLIST_H_ */
