/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

static void* place(void* bp, size_t asize);
static void* find_fit(size_t asize);
static int getClass(size_t size);
static void delNode(char* bp);
static void addNode(char* bp);
static void* coalesce(void *bp);
static void *extend_heap(size_t words);
static void heapChecker();
static void segListChecker();

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your student information in the following struct.
 ********************************************************/
student_t student = {
    /* POVIS ID */
    "kdwon",
    /* Your full name */
    "Dongwon Kim",
    /* Your email address */
    "kdwon@postech.ac.kr",
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define WSIZE 4 // word size
#define DSIZE 8 // double word size
#define CHUNKSIZE (1<<12) // extend heap by this amount
#define CLASSMAX 30 // number of size calsses

#define MAX(x, y) ((x > y)? x : y )

#define PACK(size, alloc) ((size) | (alloc)) // generate header and footer data

#define GET(p) (*(unsigned *)(p))
#define PUT(p, val) (*(unsigned *)(p) = val)
#define PUT_PTR(p, val) (*(unsigned *)(p) = (unsigned )(val))//put ptr or val

#define GET_SIZE(p) (GET(p) & ~0x7) //get size bits in header and footer
#define GET_ALLOC(p) (GET(p) & 0x1) //get alloc bit in header and footer

#define HDPR(bp) ((char *)bp - WSIZE) // get pointer to header of bp
#define FTPR(bp) ((char *)bp + GET_SIZE(HDPR(bp)) - DSIZE) // get pointer to footer of bp

#define NEXT_BLKP(bp) ((char *)bp + GET_SIZE((char *)bp - WSIZE)) // get pointer of next block
#define PREV_BLKP(bp) ((char *)bp - GET_SIZE((char *)bp - DSIZE)) // get pointer of previous block
#define NEXT_HEAPPTR(bp) ((char *)bp + WSIZE)
#define PREV_HEAPPTR(bp) ((char *)bp)
#define NEXT_HEAPLIST(bp) (*((char **)NEXT_HEAPPTR(bp)))
#define PREV_HEAPLIST(bp) (*((char **)bp))

static char* heap_listp; // pointer that points to starting point of heap
static char* segList[CLASSMAX]; // pointer array wich contains each size class of free list

static int nth_free;
static int nth_alloc;
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    // printf("init\n");
    int i;
    for(i = 0; i < CLASSMAX; i++){
        segList[i] = NULL;
    }
    nth_free = 0;
    nth_alloc = 0;
    heap_listp = NULL;
    if( (heap_listp = mem_sbrk(4 * WSIZE)) == (char *)-1)
        return -1;
    PUT(heap_listp, 0); //padding to keep double word allignment rule
    // initialize first empty free list
    PUT(heap_listp + WSIZE, PACK(DSIZE, 1));
    PUT(heap_listp + WSIZE*2, PACK(DSIZE, 1)); //prologue
    PUT(heap_listp + WSIZE*3, PACK(0, 1)); //epilogue

    heap_listp += 4 * WSIZE;
    if(extend_heap(CHUNKSIZE/(DSIZE)) == NULL) //extend heap with block of chunkbye size
        return -1;
    
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize; //adjusted size
    size_t extendsize; //size to extend heap when there's no more space in current heap
    char *bp;

    // printf("a %d\n", nth_alloc);

    if(size == 0){
        return NULL;
    }
    //align size rto mainrtain alignment rule
    if(size < DSIZE)
        asize = 2 * DSIZE;
    else
        asize = ALIGN(size + DSIZE);
    //if theres free block which fit to size
    if((bp = find_fit(asize)) != NULL){
       bp = place(bp, asize);
       return bp; 
    }
    //when theres no free block that is bigger than size
    extendsize = MAX(asize, CHUNKSIZE);
    if((bp = extend_heap(extendsize / WSIZE)) == NULL)
        return NULL;
    bp = place(bp, asize);   // place new block

    // heapChecker();
    // segListChecker();
    return bp;
    
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDPR(ptr));
    size_t alloc = GET_ALLOC(HDPR(ptr));

    // printf("f %d\n", nth_free);
    //return without any operation if ptr is the pointer of already freed block
    if(ptr == NULL)
        return;
    if(!alloc)
        return;
    //modify alloc bit to 0
    PUT(HDPR(ptr), PACK(size, 0));
    PUT(FTPR(ptr), PACK(size, 0));
    //add node after coalesce
    addNode(ptr);
    coalesce(ptr);
    // printf("f finish\n");
    return;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;   //ptr that rthis function return
    void *nextptr = NULL;
    void *prevptr = NULL;
    size_t oldSize;     //size of bp 
    size_t newSize;     //size of newly allocated bvlock 
    size_t copySize;    //size that memcpy copy
    size_t preNextSize = 0; // sum of size of prev block and nnext block
    size_t preSize = 0;
    size_t nextSize = 0;
    size_t remainSize;
    size_t preRemainSize;
    //if size is 0 : equivalent to free
    if(size == 0){
        mm_free(ptr);
        return NULL;
    }
    // if ptr is NULL, equivalent to malloc
    if(oldptr == NULL)
        return mm_malloc(size);
    // return NULL if ptr is freed block
    if(GET_ALLOC(HDPR(ptr)) != 1)
        return NULL;
    // get size of old block and align new size
    oldSize = GET_SIZE(HDPR(ptr));
    //align size
    if(size <= DSIZE)
        newSize = 2 * DSIZE;
    else
        newSize = ALIGN(size + DSIZE);

    copySize = oldSize - DSIZE;

    if(oldSize > newSize){ // when original block is bigger than requested size
        // modify size of ptr
        PUT(HDPR(ptr), PACK(newSize, 1));
        PUT(FTPR(ptr), PACK(newSize, 1));
        remainSize = oldSize - newSize;
        if(remainSize >= 2 * DSIZE){    // when block size is bigger than 16 byte
            // add size of remainder
            nextptr = NEXT_BLKP(oldptr);
            PUT(HDPR(nextptr), PACK(remainSize, 0));
            PUT(FTPR(nextptr), PACK(remainSize, 0));
            addNode(nextptr); // add new free block to list
            coalesce(nextptr);
        }else{ // when size of remain block is 2 * WSIZE == cant insert to free list
            nextptr = NEXT_BLKP(oldptr);
            PUT(HDPR(nextptr), PACK(DSIZE, 0));
            PUT(FTPR(nextptr), PACK(DSIZE, 0));
            if(GET_ALLOC(HDPR(NEXT_BLKP(nextptr))) == 0){   //when theres free node at next next blcok 
                nextSize = GET_SIZE(HDPR(NEXT_BLKP(nextSize)));
                if(nextSize >= 2 * DSIZE)   //if block size is smaller than 2 * DSIZE, than we can tdelete it from free list
                    delNode(NEXT_BLKP(nextptr));
                PUT(HDPR(nextptr), PACK(nextSize + DSIZE, 0));
                PUT(FTPR(nextptr), PACK(nextSize + DSIZE, 0)); //put new size, sum of next block and next next block
                addNode(nextptr);
                coalesce(nextptr);      //add node and coalesce
            } 
        }
        newptr = ptr;
    }else if(oldSize == newSize){
        newptr = ptr;   //return same pointer
    }else{
        if(GET_ALLOC(HDPR(PREV_BLKP(oldptr))) == 0){  
            preSize = GET_SIZE(HDPR(PREV_BLKP(oldptr)));
            preNextSize += preSize;
        }
        if(GET_ALLOC(HDPR(NEXT_BLKP(oldptr))) == 0){
            nextSize = GET_SIZE(HDPR(NEXT_BLKP(oldptr)));
            preNextSize += nextSize;
        }   //add size of prev block and next block
        remainSize = newSize - oldSize;
        if(preNextSize >= remainSize){      //when reamin size is smaller than sum of  next block size and prev block size
            if(nextSize > remainSize){
                delNode(NEXT_BLKP(oldptr)); //delete next node
                PUT(HDPR(oldptr), PACK(newSize, 1));
                PUT(FTPR(oldptr), PACK(newSize, 1)); //put new size to oldptr
                //add sliced next node
                if(nextSize - remainSize >= 2 * DSIZE){
                    PUT(HDPR(NEXT_BLKP(oldptr)), PACK(nextSize - remainSize, 0));
                    PUT(FTPR(NEXT_BLKP(oldptr)), PACK(nextSize - remainSize, 0));
                    addNode(NEXT_BLKP(oldptr));
                    coalesce(NEXT_BLKP(oldptr));
                }else if(nextSize - remainSize == DSIZE){   // is block size is smae as DSIZE, than we cant delete it from free list
                    nextptr = NEXT_BLKP(oldptr);
                    PUT(HDPR(nextptr), PACK(DSIZE, 0));
                    PUT(FTPR(nextptr), PACK(DSIZE, 0));
                    if(GET_ALLOC(HDPR(NEXT_BLKP(nextptr))) == 0){ // when theres free block at next next block
                        nextSize = GET_SIZE(HDPR(NEXT_BLKP(nextptr)));
                        if(nextSize >= 2 * DSIZE)   // dont delete node froim list 
                            delNode(NEXT_BLKP(nextptr));
                        PUT(HDPR(nextptr), PACK(nextSize + DSIZE, 0));
                        PUT(FTPR(nextptr), PACK(nextSize + DSIZE, 0));
                        addNode(nextptr);
                        coalesce(nextptr);
                    } 
                }
                newptr = oldptr;
            }else if(nextSize == remainSize){   //size of next vblock is exactly smae as remain block size
                //delte next node and put new size to oldptr
                if(GET_SIZE(HDPR(NEXT_BLKP(oldptr))) >= 2 * DSIZE )
                    delNode(NEXT_BLKP(oldptr));
                PUT(HDPR(oldptr), PACK(newSize, 1));
                PUT(FTPR(oldptr), PACK(newSize, 1));
                newptr = oldptr;
            }else{      // size of next block is smaller than remain size
                //delete next node & prev node from seg list
                if(preSize >= 2 * DSIZE)
                    delNode(PREV_BLKP(oldptr));
                if(nextSize >= 2 * DSIZE)
                    delNode(NEXT_BLKP(oldptr));
                //put new size to oldptr
                preRemainSize = remainSize - nextSize;
                newptr = (char *)oldptr - preRemainSize;
                PUT(HDPR(newptr), PACK(newSize, 1));
                PUT(FTPR(newptr), PACK(newSize, 1));
                
                if(preSize - preRemainSize >= 2 * DSIZE){
                    // put new size to new prev block
                    PUT((char *)newptr - DSIZE, PACK(preSize - preRemainSize, 0));
                    PUT(HDPR(PREV_BLKP(newptr)), PACK(preSize - preRemainSize, 0));
                    // add new prev block to free list
                    addNode(PREV_BLKP(newptr));
                    coalesce(PREV_BLKP(newptr));
                }else if (preSize - preRemainSize == DSIZE){    //when size is DSIZE, it doesnt exist in list
                    PUT((char *)newptr - DSIZE, PACK(DSIZE, 0));
                    PUT(HDPR(PREV_BLKP(newptr)), PACK(DSIZE, 0));
                    prevptr = PREV_BLKP(newptr);
                    if(GET_ALLOC(HDPR(PREV_BLKP(prevptr))) == 0){   //coaleces with nextnext block
                        if(GET_SIZE(HDPR(PREV_BLKP(prevptr))) > 2 * DSIZE) // if block size is smaller tthan DSIZE
                            delNode(PREV_BLKP(prevptr));
                        PUT(HDPR(PREV_BLKP(prevptr)), PACK(GET_SIZE(HDPR(PREV_BLKP(prevptr))) + DSIZE, 0));
                        PUT(FTPR(PREV_BLKP(prevptr)), PACK(GET_SIZE(HDPR(PREV_BLKP(prevptr))) + DSIZE, 0));
                        addNode(nextptr);
                        coalesce(nextptr);
                    } 
                }
                //memcpy() original data
                memcpy(newptr, oldptr, copySize);
            }
        }else{
            // printf("case 4\n");
            //allocate new block and copy
            newptr = mm_malloc(newSize);
            memcpy(newptr, oldptr, copySize);
            //modify alloc bit to 0
            PUT(HDPR(oldptr), PACK(oldSize, 0));
            PUT(FTPR(oldptr), PACK(oldSize, 0));
            //add node after coalesce
            addNode(ptr);
            coalesce(ptr);
        }
    }
    // heapChecker();
    return newptr;
}

static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    // printf("extend_heap\n");

    size = (words % 2) ? (words + 1) * WSIZE : (words) * WSIZE;
    
    // to maintain allignment
    if((bp = mem_sbrk(size)) == (void *)-1)
        return NULL;

    PUT(HDPR(bp), PACK(size, 0)); // header of block : rewrite at location of previous epilogue
    PUT(FTPR(bp), PACK(size, 0)); //footer of block
    PUT(HDPR(NEXT_BLKP(bp)), PACK(0, 1)); // new epilogue
    addNode(bp);
    return coalesce(bp);
}

static void* coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTPR(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDPR(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDPR(bp));
    int preReTag = 1;
    int nextReTag = 1;
    //if block size is smaller than DSIZE, dont delete it because it doesnt exisr in free lsit
    if((GET_SIZE(HDPR(PREV_BLKP(bp))) <= DSIZE) && (prev_alloc == 0))
        preReTag = 0;
    if((GET_SIZE(HDPR(NEXT_BLKP(bp))) <= DSIZE) && (next_alloc == 0))
        nextReTag = 0;
    if(prev_alloc && next_alloc){
        return bp;
    }// case 1
    else if(prev_alloc && !next_alloc){
        size += GET_SIZE(HDPR(NEXT_BLKP(bp)));
        delNode(bp);
        if(nextReTag)
            delNode(NEXT_BLKP(bp));
        PUT(HDPR(bp), PACK(size, 0));
        PUT(FTPR(bp), PACK(size, 0));
    }// case 2
    else if(!prev_alloc && next_alloc){
        size += GET_SIZE(HDPR(PREV_BLKP(bp)));
        delNode(bp);
        if(preReTag)
            delNode(PREV_BLKP(bp));
        PUT(HDPR(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTPR(bp), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }// case 3
    else{
        size += GET_SIZE(HDPR(NEXT_BLKP(bp))) + GET_SIZE(FTPR(PREV_BLKP(bp)));
        //delete prev and next free blocks from segList
        delNode(bp);
        if(nextReTag)
            delNode(NEXT_BLKP(bp));
        if(preReTag)
            delNode(PREV_BLKP(bp));
        PUT(HDPR(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTPR(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }// case 4
    addNode(bp);
    return bp;
}

static void addNode(char* bp)
{
    size_t size = GET_SIZE(HDPR(bp));
    int class = getClass(size);
    void* classPtr = segList[class];
    void* temp = classPtr;
    void* insertPtr = NULL;
    //search for appropriate place in segList
    if(classPtr != NULL){
        while(size > GET_SIZE(HDPR(temp))){
            insertPtr = temp;
            temp = NEXT_HEAPLIST(temp);
            if(temp == NULL)
                break;
        }
    }
    //add to segList : node that new block will be stored is placed between node that pointed by inserPtr and temp
    if(temp == NULL){
        if(insertPtr == NULL){ // case 1
            PUT_PTR(NEXT_HEAPPTR(bp), NULL);
            PUT_PTR(PREV_HEAPPTR(bp), NULL);
            segList[class] = bp;
        }else{  //case 2
            PUT_PTR(PREV_HEAPPTR(bp), insertPtr);
            PUT_PTR(NEXT_HEAPPTR(bp), NULL);
            PUT_PTR(NEXT_HEAPPTR(insertPtr), bp);
        }
    }else{
        if(insertPtr == NULL){  //case 3
            PUT_PTR(NEXT_HEAPPTR(bp), temp);
            PUT_PTR(PREV_HEAPPTR(bp), NULL);
            PUT_PTR(PREV_HEAPPTR(temp), bp);
            segList[class] = bp;
        }else{  //case 4
            PUT_PTR(NEXT_HEAPPTR(insertPtr), bp);
            PUT_PTR(PREV_HEAPPTR(temp), bp);
            PUT_PTR(PREV_HEAPPTR(bp), insertPtr);
            PUT_PTR(NEXT_HEAPPTR(bp), temp);
        }
    }
    return;
}

static void delNode(char* bp)
{
    size_t size = GET_SIZE(HDPR(bp));
    int class;
    void* nextptr;
    void* prevptr;

    if(size < 2 * DSIZE) //theres no node whose size is smalleer than DSIZE
        return;

    class = getClass(size);
    nextptr = NEXT_HEAPLIST(bp);
    prevptr = PREV_HEAPLIST(bp);

    if(nextptr == NULL){
        if(prevptr == NULL){    // case 1
            segList[class] = NULL;
        }else{  //case 2
            PUT_PTR(NEXT_HEAPPTR(prevptr), NULL);
        }
    }else{  //case 3
        if(prevptr == NULL){
            PUT_PTR(PREV_HEAPPTR(nextptr), NULL);
            segList[class] = nextptr;
        }else{  //case 4
            PUT_PTR(NEXT_HEAPPTR(prevptr), nextptr);
            PUT_PTR(PREV_HEAPPTR(nextptr), prevptr);
        }
    }
    return;
}

static int getClass(size_t size) // return biggest k that satissfy 2^k < size <2^k +1
{
    int i = 0;
    int x = 1;
    for(i = 0; i < CLASSMAX; i++){
        if(size <= x)
            break;
        else
            x = x * 2;
    }

    return i - 1;
}

static void* find_fit(size_t asize)
{
    int class = getClass(asize);
    void* bp = NULL;
    void* prebp = NULL;
    int i;
    //search free lsit
    for(i = class; i < CLASSMAX; i ++){ // search for another size class if theres no matched size
        for(bp = segList[i]; bp != NULL; bp = NEXT_HEAPLIST(bp)){
            prebp = PREV_HEAPLIST(bp);
            if(GET_SIZE(HDPR(bp)) >= asize) // when thers a freed block which is bigger than asize
                break;
        }
        if(bp != NULL)
            break;
    } 
    return (void *)bp;
}
static void* place(void* bp, size_t asize) // allocate freed block
{
    size_t size = GET_SIZE(HDPR(bp));
    size_t rBlockSize = size - asize;
    void* returnbp;

    delNode(bp);

    if(rBlockSize <= (2 * DSIZE)){ // change allocation bit
        PUT(HDPR(bp), PACK(size, 1));
        PUT(FTPR(bp), PACK(size, 1));
        returnbp = bp;
    }else if (asize >= 120) {
        // Split block
        PUT(HDPR(bp), PACK(rBlockSize, 0));
        PUT(FTPR(bp), PACK(rBlockSize, 0));
        PUT(HDPR(NEXT_BLKP(bp)), PACK(asize, 1));
        PUT(FTPR(NEXT_BLKP(bp)), PACK(asize, 1));
        addNode(bp);
        returnbp = NEXT_BLKP(bp);
    }else{  //change  allocation bit and size than split
        PUT(HDPR(bp), PACK(asize, 1));
        PUT(FTPR(bp), PACK(asize, 1));
        PUT(HDPR(NEXT_BLKP(bp)), PACK(rBlockSize, 0));
        PUT(FTPR(NEXT_BLKP(bp)), PACK(rBlockSize, 0));
        addNode(NEXT_BLKP(bp));
        returnbp = bp;
    }
    return returnbp;
}

static void heapChecker(){ //print every block in current heap
    void* heapPtr;
    nth_alloc++;
    printf("\n\n");
    printf("%dth allocation\n", nth_alloc);
    for(heapPtr = heap_listp ; GET_SIZE(HDPR(heapPtr)) != 0; heapPtr = NEXT_BLKP(heapPtr)){ //from start to end of heap
            printf("header> size : %4d, alloc: %d\n", GET_SIZE(HDPR(heapPtr)), GET_ALLOC(HDPR(heapPtr)));
            printf("footer> size : %4d, alloc: %d\n", GET_SIZE(FTPR(heapPtr)), GET_ALLOC(FTPR(heapPtr)));
    }
    return;
}
static void segListChecker(){ //print every seglist element
    void* segPtr;
    int i;
    nth_free++;
    printf("\n\n%dth free", nth_free);
    for(i = 0; i < CLASSMAX; i ++){
        if(segList[i] == NULL){ //if size class is empty, dont print anything
        }else{
            printf("\n\nsegList[%2d]", i);
            for(segPtr = segList[i]; segPtr != NULL; segPtr = NEXT_HEAPLIST(segPtr)){ // print every node in size class
                printf("-> [size: %4d, alloc : %d ]", GET_SIZE(HDPR(segPtr)), GET_ALLOC(HDPR(segPtr)));
                if(NEXT_HEAPLIST(segPtr) != NULL)
                    printf("%d:", (unsigned)NEXT_HEAPLIST(segPtr));
                if((unsigned)segPtr == (unsigned)NEXT_HEAPLIST(segPtr)){
                    printf("loop"); // print loop if node formed loop
                    return;
                }
            }
        }
    }
    return;
}



