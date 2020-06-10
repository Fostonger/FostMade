#include <iostream>

#define META_SIZE sizeof(struct block_meta)

struct block_meta {
    size_t size;
    struct block_meta* next_block;
    int free;
};

block_meta* global_base = NULL;

block_meta* find_free_block(block_meta** last, size_t size) {
    block_meta* current = global_base;
    while (current && !(current->free && current->size >= size)) {
        *last = current;
        current = current->next_block;
    }
    return current;
};

block_meta* get_more_memory(block_meta* last, size_t size) {
    block_meta* new_memory = (block_meta*)new char[size + META_SIZE];
    if (last) {
        last->next_block = new_memory;
    }
    new_memory->next_block = NULL;
    new_memory->size = size;
    new_memory->free = 0;
    return new_memory;
};

void* f_malloc(size_t size) {
    if (size <= 0)
        return NULL;
    struct block_meta* block;
    block_meta* last = global_base;
    if (!global_base) {
        block = get_more_memory(NULL, size);
        if (!block)
            return NULL;
        global_base = block;
    }
    else {
        block = find_free_block(&last, size);
        if (!block) {
            block = get_more_memory(last, size);
        }
        block->free = 0;
    }
    return block + 1;
}

block_meta* get_block_pointer(void* ptr) {
    return (block_meta*)ptr - 1;
}

void* f_free(void* ptr) {
    if (!ptr)
        return NULL;
    block_meta* block_ptr = get_block_pointer(ptr);
    block_ptr->free = 1;
    return NULL;
}

void test(){
    int* a = (int*)f_malloc(sizeof(int));
    *a = 1;
    std::cout << "a adress is " << a << " value is " << *a << "\n";
           
    int* b = (int*)f_malloc(sizeof(int));
    *b = 2;
    std::cout << "b adress is " << b << " value is " << *b << "\n";
       
    int* c = (int*)f_malloc(sizeof(int));
    *c = 3;
    std::cout << "c adress is " << c << " value is " << *c << "\n";
       
    f_free(a);
    f_free(b);
    f_free(c);
            
}

int main()
{
    test();
    std::cin.get();
}
