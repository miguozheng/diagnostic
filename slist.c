/*
 * slist.c
 *
 *  Created on: Nov 8, 2017
 *      Author: zheng long
 */
#include "slist.h"

/******************************************************************************
* variables prototypes
******************************************************************************/
Snode Slist_mem[SLIST_LENGTH_MAX];
unsigned char Slist_men_man[SLIST_LENGTH_MAX];

/******************************************************************************
* function prototypes
******************************************************************************/

void slist_init(Slist *list,unsigned char maxsize);
void *slist_node_get_data(Snode *pnode);
Snode *slist_get_head(Slist *pdlist);
Snode *slist_get_tial(Slist *pdlist);
unsigned char slist_is_empty(Slist *plist); 
unsigned char slist_get_size(Slist *plist); 
Snode *slist_get_next(Snode *pnode);
void slist_node_remove(Slist *pdlist,Snode *pnode);
void slist_node_insert_after(Slist *pdlist,Snode *posnode,Snode *pnodein);
Snode *slist_node_make(void *pdata);
void slist_node_insert_queue(Slist *pdlist,Snode *pnodein);
Snode *slist_malloc_mem(void);

/*�ڴ�����*/  
Snode *slist_malloc_mem(void)
{
	unsigned char i;
	Snode *pnoderet = SLIST_NULL;
	
	for(i = 0;i < SLIST_LENGTH_MAX;i++){
		if(0 == Slist_men_man[i]){
			Slist_men_man[i] = 1;
			pnoderet = &Slist_mem[i];
			pnoderet->Mem_iterm = i;
			break;
		}
	}
	
	return pnoderet;
}
/*��ȡ����ͷ*/  
Snode *slist_get_head(Slist *pdlist)
{
	Slist *tempdlist = pdlist;
	Snode *pnode = SLIST_NULL;
	
	if(SLIST_NULL != tempdlist){
		pnode = tempdlist->Head;
	}
	
	return pnode;
}
/*��ȡ����β*/  
Snode *slist_get_tial(Slist *pdlist)
{
	Slist *tempdlist = pdlist;
	Snode *pnode = SLIST_NULL;
	
	if(SLIST_NULL != tempdlist){
		pnode = tempdlist->Tail;
	}
	
	return pnode;
}
/*��ȡ������*/  
unsigned char slist_get_size(Slist *plist)  
{  
	unsigned char ret = 0xFF;

	if(SLIST_NULL != plist){
		ret = plist->Size;
	}
	
    return ret;  
}  
/*��ȡ�����Ƿ�Ϊ��*/  
unsigned char slist_is_empty(Slist *plist)  
{  
    if((slist_get_size(plist) == 0) || (slist_get_tial(plist) == slist_get_head(plist))){
    	return 1;  
    }else{  
        return 0;  
    }
} 
/*��ȡ�����Ƿ���*/  
unsigned char slist_is_full(Slist *plist)  
{  
    if(plist->Size >= plist->MaxSize){
    	return 1;  
    }else{  
        return 0;  
    }
} 

/*��ȡ�ڵ�Next*/  
Snode *slist_get_next(Snode *pnode)
{
	Snode *ret = SLIST_NULL;

	if(SLIST_NULL != pnode){
		ret = pnode->Next;
	}
	
	return ret;
}

/*��ȡ�ڵ�����*/ 
void *slist_node_get_data(Snode *pnode)
{
	void *ret = SLIST_NULL;

	if(SLIST_NULL != pnode){
		ret = pnode->Data;
	}

	return ret;
}
/*ɾ�������нڵ�*/ 
void slist_node_remove(Slist *pdlist,Snode *pnode)
{
	Slist *tempdlist = pdlist;
	Snode *pnodetemp = pnode,*pprev = SLIST_NULL;
	unsigned char i = 0;
	
	if((SLIST_NULL != tempdlist) && (SLIST_NULL != pnodetemp)){
		if(tempdlist->Size){
			pprev = slist_get_head(tempdlist);
			for(i = 0;i < tempdlist->Size;i++){
				if(pprev){
					if(pprev->Next == pnodetemp){
						pprev->Next = pnodetemp->Next;
						if(SLIST_NULL == pprev->Next){
							tempdlist->Tail = pprev;
						}
						slist_node_free(pnodetemp);
						tempdlist->Size--;
						break;
					}else{
						pprev = pprev->Next;
					}
				}else{
					break;
				}
			}
		}
	}
}

/*�ڵ�����ڵ�*/ 
void slist_node_insert_after(Slist *pdlist,Snode *posnode,Snode *pnodein)
{
	Slist *tempdlist = pdlist;
	Snode *pposnode = posnode,*pnext = SLIST_NULL,*pnodeintemp = pnodein;

	if(SLIST_NULL != tempdlist){
		pnext = pposnode->Next;
		pposnode->Next = pnodeintemp;
		pnodeintemp->Next = pnext;
		if(SLIST_NULL == pnext){
			tempdlist->Tail = pnodeintemp;
		}
		tempdlist->Size++;
	}
}
/*�������ڵ�*/ 
void slist_node_insert_queue(Slist *pdlist,Snode *pnodein)
{
	Slist *tempdlist = pdlist;
	Snode *pnodeintemp = pnodein,*nodetail = SLIST_NULL;
	
	if(slist_is_empty(tempdlist)){
		tempdlist->Tail = pnodeintemp;
		tempdlist->Head->Next = pnodeintemp;
		pdlist->Size++;
	}else{
		nodetail = slist_get_tial(tempdlist);
		if(SLIST_NULL != nodetail){
			slist_node_insert_after(tempdlist,nodetail,pnodeintemp);
		}
	}
}
/*�����ڵ�*/ 
Snode *slist_node_make(void *pdata)
{
	Snode *pnoderet = SLIST_NULL,*nodetemp = SLIST_NULL;
	
	nodetemp = slist_malloc_mem();
	if(SLIST_NULL != nodetemp){
		nodetemp->Data = pdata;
		nodetemp->Next = SLIST_NULL;
	}
	pnoderet = nodetemp;
	
	return pnoderet;
}
/*free�ڵ�*/ 
void slist_node_free(Snode *pnode)
{
	if(!pnode){
		return;
	}

	Slist_men_man[pnode->Mem_iterm] = 0;
	pnode->Data = SLIST_NULL;
	pnode->Next = SLIST_NULL;
	pnode->Mem_iterm = 0xFF;
}

/*�����ʼ��*/ 
void slist_init(Slist *_slist,unsigned char maxsize)
{
	Slist *plist = _slist;
	Snode *nodehead = SLIST_NULL;
	
	nodehead = slist_node_make(SLIST_NULL);
	if(SLIST_NULL != nodehead){
		plist->Head = nodehead;
		plist->Tail = nodehead;
		plist->Size = 0;
		plist->MaxSize = maxsize;
	}
}

