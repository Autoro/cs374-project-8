#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>

#define ALIGNMENT 16
#define GET_PAD(x) ((ALIGNMENT - 1) - ((x - 1) & (ALIGNMENT - 1)))
#define PADDED_SIZE(x) (x + GET_PAD(x))
#define PTR_OFFSET(p, offset) ((void*)((char*)p + offset))

typedef struct block block;
struct block
{
    int size;
    int in_use;
    block* next;
};

const int PADDED_BLOCK_SIZE = PADDED_SIZE(sizeof(block));
const int MEMORY_SIZE = 1024;

block* head = NULL;

void initialize_heap()
{
    head = mmap(NULL, MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    head->size = MEMORY_SIZE - PADDED_BLOCK_SIZE;
    head->in_use = 0;
    head->next = NULL;
}

block* find_free_block(int padded_size)
{
    block* block = head;
    while (block != NULL && (block->in_use || block->size < padded_size))
    {
        block = block->next;
    }

    return block;
}

void split_block(block* free_block, int padded_size)
{
    int offset = PADDED_BLOCK_SIZE + padded_size;

    block* next_block = PTR_OFFSET(free_block, offset);
    next_block->size = free_block->size - offset;
    next_block->in_use = 0;
    next_block->next = NULL;

    free_block->next = next_block;
    free_block->size = padded_size;
}

void* allocate_block(block* free_block, int padded_size)
{
    free_block->in_use = 1;

    if (free_block->size >= padded_size + PADDED_BLOCK_SIZE + ALIGNMENT)
    {
        split_block(free_block, padded_size);
    }

    return PTR_OFFSET(free_block, PADDED_BLOCK_SIZE);
}

void* myalloc(int size)
{
    if (head == NULL)
    {
        initialize_heap();
    }

    int padded_size = PADDED_SIZE(size);
    block* free_block = find_free_block(padded_size);

    if (free_block != NULL)
    {
        return allocate_block(free_block, padded_size);
    }

    return NULL;
}

void coalesce(block* current)
{
    if (current == NULL)
    {
        return;
    }

    block* next = current->next;
 
    coalesce(next);

    if (current->in_use == 0 && next != NULL && next->in_use == 0)
    {
        current->size += next->size + PADDED_BLOCK_SIZE;
        current->next = next->next;
    }
}

void myfree(void* p)
{
    block* header = (block*)p - 1;
    header->in_use = 0;

    coalesce(head);
}

void print_data()
{
    if (head == NULL)
    {
        printf("[empty]\n");
        return;
    }
    
    block* current = head;
    while (current != NULL)
    {
        if (current != head)
        {
            printf(" -> ");
        }
        
        printf("[%d,%s]", current->size, current->in_use ? "used" : "free");

        current = current->next;
    }

    printf("\n");
}

int main ()
{
    void *p, *q, *r, *s;

    p = myalloc(10); print_data();
    q = myalloc(20); print_data();
    r = myalloc(30); print_data();
    s = myalloc(40); print_data();

    myfree(q); print_data();
    myfree(p); print_data();
    myfree(s); print_data();
    myfree(r); print_data();
}
