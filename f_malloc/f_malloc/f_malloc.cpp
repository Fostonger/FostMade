#include <iostream>

#define META_SIZE sizeof(class Entity)

struct Entity {
    Entity* next_entity;
    size_t size;
    int free;
};

class VirtualMemory {
public:
    void* first_element; // указатель на первый элемент в памяти
    VirtualMemory* next_memory; // указатель на сдедующую память

    VirtualMemory() {
        first_element = new char[size]; //бронируем 4 кб конструктором на хипе
    }

    void* get_memory(size_t size) {
        Entity* free_entity = (Entity*)first_element;
        Entity* last = NULL;
        while (free_entity && !free_entity->free && free_entity->size <= size ) {
            last = free_entity;
            free_entity = free_entity->next_entity;
        }
        if (!free_entity) {
            free_entity = (Entity*)((char*)last + last->size) + 1;
            last->next_entity = free_entity;
        }
        return free_entity;
    }

private:
    size_t size = 4096; // будем выделять сразу 4 килобайта, с ними танцуем
};

VirtualMemory* global_base = new VirtualMemory;

void* f_malloc(size_t size) {
    VirtualMemory* vm = global_base;
    Entity* new_memory = (Entity*)vm->get_memory(size + META_SIZE);
    new_memory->free = 0;
    new_memory->size = size;
    new_memory->next_entity = NULL;
    return new_memory + 1; // даем указатель, отступив немного для хедера
}
void f_free(void* ptr) {
    Entity* ptr_head = (Entity*)ptr - 1;
    ptr_head->free = 1;
    //начинаем дефрагментировать
    VirtualMemory* vm = global_base;
    Entity* current_entity = (Entity*)global_base->first_element; //начинаем с первого элемента
    Entity* last_entity = NULL;
    while (current_entity) {
        if (!last_entity){
            last_entity = current_entity;
            current_entity = current_entity->next_entity;
            continue; // перепрыгиваем если первый элемент
        }
        if (current_entity->free && last_entity->free) {
            last_entity->next_entity = current_entity->next_entity; // объединяем два блока
            last_entity->size += current_entity->size + META_SIZE; // если оба свободны
        }
        else if (current_entity->free) {
            // если прошлый занят, может быть что он занял немного больше чем надо.
            // тогда перемещаем поинтер туда, где кончается используемая память у прошлого
            Entity* resized_ptr = (Entity*)((char*)last_entity + META_SIZE + last_entity->size);
            resized_ptr->next_entity = current_entity->next_entity;
            resized_ptr->size = current_entity->size + (char*)current_entity - (char*)resized_ptr;
        }
        last_entity = current_entity;
        current_entity = current_entity->next_entity;
    }
}

int main() {
    int* a =(int*) f_malloc(6);
    *a = 1;
    int* b = (int*)f_malloc(4);
    *b = 2;
    f_free(a);
    int* c = (int*)f_malloc(4);
    *c = 3;
    f_free(b);
    int* d = (int*)f_malloc(4);
    *d = 4;
    
    std::cin.get();
}