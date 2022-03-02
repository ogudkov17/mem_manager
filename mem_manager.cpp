#include <thread>
#include <mutex>
#include <iostream>
#include <cassert>
#include <memory>
#include <cstring>
#include <string>
#include <stdexcept>

#include "mem_manager.h"
/**
 * 	Выделение нового блока памяти из существующего списка
 * \return блок памяти
 * \param[in] memory адрес блока памяти
 * \maram[in] size размер блока
*/
MemBlock* create_block(void* memory, size_t size)
{
	if (!memory) 
			throw std::invalid_argument("create_block: memory NULL.");
	if (size <= sizeof(MemBlock))
			throw std::invalid_argument("create_block: size too long");

	MemBlock* p = static_cast<MemBlock*>(memory);
	p->next = NULL;
	p->prev = NULL;
	p->free = 1;
	p->size = size - sizeof(MemBlock);

	return p;
}

/**
 * 	Простая проверка блока памяти
 * 	\return true/false
 * 	\param[in] p блок данных для проверки
*/
bool validate_block(MemBlock* p)
{
	if (p->next != NULL && p->next->prev != p) return false;
	if (p->prev != NULL && p->prev->next != p) return false;
	if (p->size <= 0) return false;

	return true;
}

/**
 *	Вывод информации о блоке памяти
 * 	\param[in] p блок данных для вывода 
*/
void print_block(MemBlock* p)
{
	std::cout<<"address["<<std::hex<<p<<"] | ";
	std::cout<<"size["<<p->size<<"] | ";
	std::cout<<"free["<<p->free<<"]";
	std::cout<<std::endl;
}


/**
 *	Конструктор менеджера памяти
 *
 *	\param[in]	memory  буфер памяти
 *	\param[in] 	size 	размер буфера
*/
template<typename MutexType>
MemoryManager<MutexType>::MemoryManager(void* memory, size_t size)
{
	if (!memory) 
			throw std::invalid_argument("MemoryManager(): memory is NULL");

	if (size <= MINIMUM_HEAP_SIZE)
			throw std::invalid_argument("MemoryManager(): insuficient size");

	MemBlock* p = create_block(memory, size);

	heap_size = size;

	linked_list = p;

	next_node = NULL;

}

/**
 *	Процедура выделения блока памяти
 *	
 *	\return блок памяти
 *	\param[in/out] p Блок памяти
 *	\param[in]	bytes Количество выделяемых байт
*/
template<typename MutexType>
MemBlock* MemoryManager<MutexType>::allocate_block(MemBlock* p, size_t bytes)
{
	if (!p) throw std::invalid_argument("allocate_block(): MemBlock is NULL");
	if (bytes <= 0) 
			throw std::invalid_argument("allocate_block(): size must be greater than 0");

	size_t remaining = p->size - bytes;

	if (remaining >= sizeof(MemBlock) + MINIMUM_FREE_BLOCK)
	{
		MemBlock* pblock = create_block(&p->memory[bytes], remaining);

		pblock->next = p->next;
		pblock->prev = p;

		if (pblock->next)
			pblock->next->prev = pblock;

		p->next = pblock;
		p->size = bytes;
	}

	p->free = 0;
	std::memset(p->memory, 0, p->size);

	return p;
}


/**
 * Метод слияния освобождаемого блока с предыдущим 
 * \return адрес общего блока
 * \param[in] p	адрес освобождаемого блока
*/
template<typename MutexType>
MemBlock* MemoryManager<MutexType>::merge_prev(MemBlock* p)
{
	if (!p) throw std::invalid_argument("allocate_block(): MemBlock is NULL");

	p->prev->next = p->next;
	p->prev->size += sizeof(MemBlock) + p->size;

	if (p->next)
		p->next->prev = p->prev;

	return p->prev;
}

/**
 * Метод слияния освобождаемого блока со следующим 
 * \return адрес общего блока
 * \param[in] p	адрес освобождаемого блока
*/
template<typename MutexType>
MemBlock* MemoryManager<MutexType>::merge_next(MemBlock* p)
{
	if (!p) throw std::invalid_argument("allocate_block(): MemBlock is NULL");

	p->size += sizeof(MemBlock) + p->next->size;

	p->next = p->next->next;

	if (p->next)
		p->next->prev = p;

	return p;
}

/**
 * Проверка списка блоков памяти
*/
template<typename MutexType>
void MemoryManager<MutexType>::validate()
{
		std::lock_guard<MutexType> guard(lock);
		MemBlock* p = linked_list;
		size_t counter = 0;

		while(p)
		{
			bool is_valid = validate_block(p);
			if (is_valid == false) throw std::invalid_argument("validate(): invalid block p");
			
			counter += p->size + sizeof(MemBlock);
			p = p->next;
		}

		if (counter != heap_size) throw std::invalid_argument("validate(): counter != heap size");
}

/**
 * Вывод списка блоков памяти
*/
template<typename MutexType>
void MemoryManager<MutexType>::print()
{
	std::lock_guard<MutexType> guard(lock);
	
	MemBlock* p = linked_list;
	int i = 0;
	while(p) 
	{
		std::cout<<"block["<<i++<<"] | ";
		print_block(p);
		p = p->next;
	}

}

/**
 * Выделение блока памяти
 * \return адрес блока памяти
 * \param[in] bytes размер блока
*/
template<typename MutexType>
void* MemoryManager<MutexType>::allocate(size_t bytes)
{
	std::lock_guard<MutexType> guard(lock);

	if (bytes <= 0) throw std::invalid_argument("allocate(): size must be > 0");

	MemBlock* p = linked_list;

	if (!p) throw std::invalid_argument("allocate(): list is null");

	while(p)
	{
		if (p->free && p->size >= bytes)
		{
			allocate_block(p, bytes);
			return p->memory;
		}

		p = p->next;
	}

	return NULL;
}

/**
 * Освобождение блока памяти
 * \param[in] адрес блока памяти
*/
template<typename MutexType>
void MemoryManager<MutexType>::deallocate(void* memory)
{
	std::lock_guard<MutexType> guard(lock);
	
	if (!linked_list) throw std::invalid_argument("deallocate(): list is null");

	if (memory == NULL)
	{
		return;
	}

	if (reinterpret_cast<uintptr_t>(memory) < reinterpret_cast<uintptr_t>(linked_list) ||
			reinterpret_cast<uintptr_t>(memory) >= reinterpret_cast<uintptr_t>(linked_list) +
				static_cast<uintptr_t>(heap_size))
	{
		throw std::invalid_argument("deallocate(): memory deallocate error");
	}


	MemBlock* p = static_cast<MemBlock*>(memory) - 1;

	if (p->free)
	{
		std::cerr<<"Error: memory already free"<<std::endl;
		return;
	}

	p->free = 1;

	if (p->prev && p->prev->free)
	{
		if (next_node == p)
			next_node = p->next;

		p = merge_prev(p);
	}

	
	if (p->next && p->next->free)
	{
		if (next_node == p->next)
			next_node = p->next->next;

		merge_next(p);
	}

}

/**
 *	Стандартный мьютекс
 *	Можно добавить класс мьютекса для тестов
*/
template class MemoryManager<std::mutex>;
