/*
 * MemoryAllocator.cpp
 * Copyright (c) 2012, Harry Kunz <harry.kunz@ymail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 *    3. You must have great looks
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "common/MemoryAllocator.h"
#include "ports/System.h"
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

MemoryAllocator::MemoryAllocator (size_t nbytes) :
	_memIndexMap(), _memHeap(NULL), _size(nbytes), _sizeLeft(nbytes)
{
	_memHeap = (char*) malloc(_size);
}

MemoryAllocator::~MemoryAllocator ()
{
	free(_memHeap);
}

void* MemoryAllocator::allocate (size_t nbytes)
{
	const size_t memsize = calculateMemorySpace(nbytes);
	char* placement = _memHeap;
	_sizeLeft -= memsize;
	for (MemoryIndexMapIter iter = _memIndexMap.begin(); iter != _memIndexMap.end(); ++iter) {
		char* mem = (char*) iter->first;
		const size_t diff = size_t(mem - placement);
		if (diff >= memsize) {
			_memIndexMap[placement] = memsize;
			return placement;
		}
		placement = mem + iter->second;
	}
	if (placement + memsize <= _memHeap + _size) {
		_memIndexMap[placement] = memsize;
		return placement;
	}
	System.exit("fatal: out of pool memory!", EXIT_FAILURE);
	return NULL;
}

void MemoryAllocator::deallocate (void* ptr)
{
	IMemoryAllocationObject* object = reinterpret_cast<IMemoryAllocationObject*> (ptr);
	const size_t size = object->msize();
	if (size == 0) {
		System.exit("object must extend IMemoryAllocationObject", EXIT_FAILURE);
	}
	object->~IMemoryAllocationObject();
	MemoryIndexMapIter iter = _memIndexMap.find(ptr);
	_sizeLeft += calculateMemorySpace(iter->second);
	_memIndexMap.erase(iter);
}

size_t MemoryAllocator::getSizeTotal () const
{
	return _size;
}

size_t MemoryAllocator::getSizeLeft () const
{
	return _sizeLeft;
}

size_t MemoryAllocator::calculateMemorySpace (size_t nbytes) const
{
	const size_t alignExcess = nbytes % WORD_SIZE;
	size_t memsize = nbytes;
	if (alignExcess > 0)
		return memsize + (WORD_SIZE - alignExcess);
	return memsize;
}
