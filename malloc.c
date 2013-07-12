#include "malloc.h"

Header base;
unsigned int h = sizeof(Header);//sizeof(unsigned int) + sizeof(struct Header *);

void printList(){
	Header *traversePtr = &base;
	unsigned short numNodes = 0;
	printf("\n********** begin printList **********\n");
	printf("#%4u: %p\n", numNodes++, traversePtr);
	while(traversePtr->ptr){
		printf("#%4u: %p\n", numNodes++, traversePtr->ptr);
		traversePtr = traversePtr->ptr;
	}
	printf("********** end printList **********\n\n");
}

Header * findFollowingInList(Header* node){
	Header *traversePtr = &base;
	Header *returnPtr;
	if(base.ptr == NULL)
		returnPtr = NULL;
	while(traversePtr->ptr){
		if(traversePtr->ptr > node){
			returnPtr = traversePtr->ptr;
			break;
		}
		traversePtr = traversePtr->ptr;
	}
	return returnPtr;
}

Header * findPreviousInList(Header* node){
	Header *traversePtr = &base;
	Header *returnPtr;
	if(base.ptr == NULL)
		returnPtr = &base;
	while(traversePtr->ptr){
		if (traversePtr->ptr >= node){
			returnPtr = traversePtr;
			break;
		}
		traversePtr = traversePtr->ptr;
	}
	return returnPtr;
}

void * partitionAHole(Header *node, unsigned int nbytes){
	unsigned int n = ((nbytes-1) + (h-((nbytes-1)%h)));
	void *returnPtr = NULL;

	if(node->size > n + 2*h){
		node->size = node->size - h - n;
		returnPtr = node + h + node->size + h;
		(*(Header*)(returnPtr - h)).size = n;
	}
	else if(node->size == n + 2*h){
		node->size = h;
		returnPtr = node + 2*h;
		(*(Header*)(returnPtr - h)).size = n;
	}
	else if(node->size == n){
		/*since the whole hole is consumed, we have to delete this node*/
		if(node->ptr)
			findPreviousInList(node)->ptr = node->ptr;
		node->ptr = NULL;
		returnPtr = node + h;
	}
	return returnPtr;
}

void * malloc_simple(unsigned int nbytes) {
	unsigned int n = ((nbytes-1) + (h-((nbytes-1)%h)));
	void * returnPtr = sbrk(n+h);
	(*(Header*)(returnPtr)).size = n+h;
	(*(Header*)(returnPtr)).ptr = NULL;
	return (returnPtr+h);
}

void * malloc_first_fit(unsigned int nbytes) {
	void * returnPtr = NULL;
	Header * traversePtr;
	/*if there is nothing in the free list, directly expand the heap*/
	if(!base.ptr)
		return malloc_simple(nbytes);
	else
		traversePtr = base.ptr;
	/*look for a hole*/
	while(traversePtr){
		returnPtr = partitionAHole(traversePtr, nbytes);
		if(returnPtr)
			return returnPtr;
		else
			traversePtr = traversePtr->ptr;
	};
	/*no hole is large enough to fit in the requested size*/
	return malloc_simple(nbytes);
}

void * malloc_best_fit(unsigned int nbytes) {
	unsigned int n = ((nbytes-1) + (h-((nbytes-1)%h)));
	void *returnPtr = NULL;
	Header *traversePtr, *bestFitPtr;
	/*if there is nothing in the free list, directly expand the heap*/
	if(!base.ptr)
		return malloc_simple(n);
	else{
		traversePtr = base.ptr;
		bestFitPtr = traversePtr;
	}
	/*find the best fit hole*/
	while(traversePtr){
		if((traversePtr->size >= n) && (traversePtr->size <= bestFitPtr->size))
			bestFitPtr = traversePtr;
		traversePtr = traversePtr->ptr;
	};

	returnPtr = partitionAHole(bestFitPtr, n);
	if(returnPtr)
		return returnPtr;
	else
		/*no hole is large enough to fit in the requested size*/
		return malloc_simple(n);
}

void * malloc_worst_fit(unsigned int nbytes) {
	unsigned int n = ((nbytes-1) + (h-((nbytes-1)%h)));
	void *returnPtr = NULL;
	Header *traversePtr, *worstFitPtr;
	/*if there is nothing in the free list, directly expand the heap*/
	if(!base.ptr)
		return malloc_simple(n);
	else{
		traversePtr = base.ptr;
		worstFitPtr = traversePtr;
	}
	/*find the worst fit hole*/
	while(traversePtr){
		if((traversePtr->size >= n) && (traversePtr->size >= worstFitPtr->size))
			worstFitPtr = traversePtr;
		traversePtr = traversePtr->ptr;
	};

	returnPtr = partitionAHole(worstFitPtr, n);
	if(returnPtr)
		return returnPtr;
	else
		/*no hole is large enough to fit in the requested size*/
		return malloc_simple(n);
}

void insertToFreeList(Header* node){
	Header *traversePtr;
	if(base.ptr == NULL)
		base.ptr = node;
	else if(base.ptr > node){
		node->ptr = base.ptr;
		base.ptr = node;
		return;
	}
	else{
		traversePtr = base.ptr;
		while(traversePtr <= node){
			if(traversePtr->ptr >= node){
				node->ptr = traversePtr->ptr;
				traversePtr->ptr = node;
				break;
			}
			else
				traversePtr = traversePtr->ptr;
		}
	}
}

void mergeFreeBlocks(Header *front, Header *back){
	front->size += back->size;
	front->ptr = back->ptr;
	back->size = 0;
	back->ptr = NULL;
}

void free_simple(void *addr) {
	Header *node = (Header *)(addr - h);
	Header *nextNode = findFollowingInList(node);
	Header *previousNode = findPreviousInList(node);

	insertToFreeList(node);
	
	/*merge with the next hole if it is adjacent to the current block*/
	while(nextNode && (node+node->size >= nextNode)){
		mergeFreeBlocks(node, nextNode);
		nextNode = node->ptr;
	}

	/*merge with the previous hole if it is adjacent to the current block*/
	while(previousNode+previousNode->size > node){
		mergeFreeBlocks(previousNode, node);
		node = previousNode;
	}
}

void free_no_tail(void* addr) {
	Header *lastNode = &base;
	Header *secondLastNode;
	free_simple(addr);

	if(!base.ptr)
		return;
	while(lastNode){
		if(lastNode->ptr)
			lastNode = lastNode->ptr;
		else
			break;
	}

	if(lastNode+lastNode->size >= (Header*)sbrk(0)){
		secondLastNode = findPreviousInList(lastNode);
		sbrk(-lastNode->size);
		secondLastNode->ptr = NULL;
	}
}


