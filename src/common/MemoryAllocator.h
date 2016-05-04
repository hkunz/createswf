/*
 * MemoryAllocator.h
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

#pragma once

#include <stddef.h>
#include <map>

namespace {
const size_t WORD_SIZE = sizeof(void*);
}

class IMemoryAllocationObject {
public:
	virtual ~IMemoryAllocationObject ()
	{
	}
	virtual size_t msize () const = 0;
};

class MemoryAllocator {
private:
	friend void* operator new (size_t nbytes, MemoryAllocator& pool);
	friend void operator delete (void* ptr, MemoryAllocator& pool);

	typedef std::map<void*, size_t> MemoryIndexMap;
	typedef MemoryIndexMap::iterator MemoryIndexMapIter;

	MemoryIndexMap _memIndexMap;
	char* _memHeap;
	size_t _size;
	size_t _sizeLeft;

	void* allocate (size_t nbytes);
	void deallocate (void* ptr);

	size_t calculateMemorySpace (size_t nbytes) const;

public:
	explicit MemoryAllocator (size_t nbytes);
	~MemoryAllocator ();

	size_t getSizeTotal () const;
	size_t getSizeLeft () const;
};

inline void* operator new (size_t nbytes, MemoryAllocator& pool)
{
	return pool.allocate(nbytes);
}

inline void operator delete (void* ptr, MemoryAllocator& pool)
{
	pool.deallocate(ptr);
}
