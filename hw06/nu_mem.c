#include <stdint.h>
#include <sys/mman.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "nu_mem.h"

static const int64_t CHUNK_SIZE = 65536;

// You should update these counters on memory allocation / deallocation events.
// These counters should only go up, and should provide totals for the entire
// execution of the program.
static int64_t nu_malloc_count  = 0; // How many times has malloc returned a block.
static int64_t nu_malloc_bytes  = 0; // How many bytes have been allocated total
static int64_t nu_free_count    = 0; // How many times has free recovered a block.
static int64_t nu_free_bytes    = 0; // How many bytes have been recovered total.
static int64_t nu_malloc_chunks = 0; // How many chunks have been mmapped?
static int64_t nu_free_chunks   = 0; // How many chunks have been munmapped?

typedef struct cell {
    int64_t size;
    struct cell* next;
    struct cell* prev;
} cell;

cell* head;
void nu_mem_print_stats();
void* nu_malloc(size_t usize);
void add_cell(cell* new_cell);
void remove_cell(cell* new_cell);
void nu_free(void* addr);
void coalesce();

int64_t nu_free_list_length();

int64_t nu_free_list_length()
{
    int length = 0;
    cell* curr_cell = head;
    while(curr_cell != NULL)
    {
        curr_cell = curr_cell->next;
        length++;
    }

    return length;
}

void nu_mem_print_stats()
{
    fprintf(stderr, "\n== nu_mem stats ==\n");
    fprintf(stderr, "malloc count: %ld\n", nu_malloc_count);
    fprintf(stderr, "malloc bytes: %ld\n", nu_malloc_bytes);
    fprintf(stderr, "free count: %ld\n", nu_free_count);
    fprintf(stderr, "free bytes: %ld\n", nu_free_bytes);
    fprintf(stderr, "malloc chunks: %ld\n", nu_malloc_chunks);
    fprintf(stderr, "free chunks: %ld\n", nu_free_chunks);
    fprintf(stderr, "free list length: %ld\n", nu_free_list_length());
}

void* nu_malloc(size_t usize)
{
    nu_free_list_length();
    int64_t size = (int64_t)usize;
    int64_t total_required_space = size + sizeof(cell);
    
    nu_malloc_count += 1;
    nu_malloc_bytes += total_required_space;
    
    if(total_required_space >= CHUNK_SIZE)
    {
        void* memory = mmap(0, total_required_space, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
        nu_malloc_chunks += 1;
        
        cell* occupied = (cell*)memory;
        occupied->size = size;
        
        return (void*)((int64_t)occupied + sizeof(cell));
    }
    else
    {
        cell* curr_cell = head;
        while(curr_cell != 0 && curr_cell->size < total_required_space)
        { 
            curr_cell = curr_cell->next;  
        }
        if(curr_cell)
        {
            int64_t curr_cell_real_size = curr_cell->size + sizeof(cell);
            curr_cell->size = size;
            if(curr_cell_real_size - total_required_space >= sizeof(cell))
            {
                cell* empty = (cell*)((int64_t)curr_cell + total_required_space);
                empty->size = curr_cell_real_size - total_required_space - sizeof(cell);
                empty->prev = curr_cell->prev;
                empty->next = curr_cell->next;
                if(empty->next)
                {
                    empty->next->prev = empty;
                }
                if(empty->prev)
                {
                    empty->prev->next = empty;
                }
                if(curr_cell == head)
                {
                    head = empty;
                }  
            }
            
            return (void*)((int64_t)curr_cell + sizeof(cell));
        }
        else
        {
            void* memory = mmap(0, CHUNK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
            nu_malloc_chunks += 1;
            cell* occupied = (cell*)memory;
            occupied->size = size;
            if(CHUNK_SIZE - total_required_space >= sizeof(cell))
            {
                cell* empty = (cell*)((int64_t)memory + total_required_space);
                empty->size = CHUNK_SIZE - total_required_space - sizeof(cell);
                add_cell(empty);      
            }
            return (void*)((int64_t)occupied + sizeof(cell));
        }
    }
    
    return 0;
}

void add_cell(cell* new_cell)
{
    //printf("call to add made\n");
    cell* curr_cell = head;
    if(head == NULL)
    {
        head = new_cell;
    }
    else if(new_cell < head)
    {
        new_cell->next = head;
        new_cell->prev = 0;
        new_cell->next->prev = new_cell;
        
        head = new_cell;
        nu_free_list_length(); 
    }
    else
    {
        nu_free_list_length();
        cell* curr_cell = head;
        while((int64_t)new_cell > (int64_t)curr_cell && curr_cell->next != NULL)
        {
            curr_cell = curr_cell->next;    
        }
        if(curr_cell->next == NULL && new_cell > curr_cell)
        {
            printf("fucker %i\n", new_cell->size);
            curr_cell->next = new_cell;
            new_cell->prev = curr_cell;            
        }
        else
        {
            nu_free_list_length();
            new_cell->next = curr_cell;
            new_cell->prev = curr_cell->prev;
            curr_cell->prev = new_cell;
            new_cell->prev->next = new_cell;   
        }
    }
}

void remove_cell(cell* new_cell)
{
    if(new_cell->prev)
    {
        new_cell->prev->next = new_cell->next;    
    }
    if(new_cell->next)
    {
        new_cell->next->prev = new_cell->prev;   
    }
}

void nu_free(void* addr) 
{
    cell* occupied = (cell*)((int64_t)addr - sizeof(cell));
    
    nu_free_count += 1;
    nu_free_bytes += occupied->size + sizeof(cell);
    
    if(occupied->size + sizeof(cell) >= CHUNK_SIZE)
    {
        munmap((void*)occupied, occupied->size + sizeof(cell));  
        nu_free_chunks += 1; 
    }
    else
    {
        add_cell(occupied);
        nu_free_list_length();
        coalesce();
        nu_free_list_length();
    }
}

void coalesce()
{
    cell* curr_cell = head;
    while(curr_cell != NULL /*&& curr_cell->next != NULL*/)
    {
        if(curr_cell->next)
        {
            int64_t next_addr_pred = (int64_t)curr_cell + curr_cell->size + sizeof(cell);
            if(next_addr_pred == (int64_t)curr_cell->next)
            {
                curr_cell->size += curr_cell->next->size + sizeof(cell);
                curr_cell->next = curr_cell->next->next;
                if(curr_cell->next)
                {
                    curr_cell->next->prev = curr_cell;
                }
            }
        }
        
        if(curr_cell->prev)
        {
            int64_t prev_addr_pred = (int64_t)curr_cell->prev + curr_cell->prev->size + sizeof(cell);
            if(prev_addr_pred == (int64_t)curr_cell)
            {
                curr_cell->prev->size += curr_cell->size + sizeof(cell);
                curr_cell->prev->next = curr_cell->next;
                if(curr_cell->next)
                {
                    curr_cell->next->prev = curr_cell->prev;
                }
            }
        }
        curr_cell = curr_cell->next;    
    }
}
