#ifndef MEM_MANAGER_H
#define MEM_MANAGER_H

#include <mutex>

typedef struct mem_block
{
		struct mem_block* 	next;
		struct mem_block* 	prev;
		unsigned 	free;
		size_t		size;
		uint8_t		memory[];

} MemBlock;

MemBlock* create_block(void* memory, size_t size);
bool validate_block(MemBlock* p);
void print_block(MemBlock* p);

template<typename MutexType = std::mutex>
class MemoryManager
{
	public:
		const int MINIMUM_FREE_BLOCK = 32;
		const int MINIMUM_HEAP_SIZE = 1024;

		MemBlock* linked_list;
		MemBlock* next_node;

		MutexType lock;

		size_t heap_size;


		//void* (*allocate)(size_t bytes);


		MemoryManager(void* memory, size_t size);

		MemBlock* allocate_block(MemBlock* p, size_t bytes);
		MemBlock* merge_prev(MemBlock* p);
		MemBlock* merge_next(MemBlock* p);
		void validate();
		void print();

		void* allocate(size_t bytes);
		void deallocate(void* memory);
};


#endif
