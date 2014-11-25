//
//  MemoryManager.cpp
//  SFSL
//
//  Created by Romain Beguet on 12.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "MemoryManager.h"

namespace sfsl {

namespace common {

AbstractMemoryManager::~AbstractMemoryManager() {

}

MemoryChunk::MemoryChunk(size_t size, MemoryChunk *parent) : _chunk(new char[size]), _chunkSize(size), _offset(0), _parent(parent) {

}

MemoryChunk::~MemoryChunk() {
    delete _chunk;

    if (_parent) {
        delete _parent;
    }
}

void* MemoryChunk::alloc(MemoryChunk*& chunk, size_t size) {
    if (size > chunk->_chunkSize - chunk->_offset) {
        chunk = new MemoryChunk(chunk->_chunkSize * 2, chunk);
        return alloc(chunk, size);
    } else {
        char* toRet = chunk->_chunk + chunk->_offset;
        chunk->_offset += size;
        return static_cast<void*>(toRet);
    }
}

ChunkedMemoryManager::ChunkedMemoryManager(size_t chunksSize) : _lastChunk(new MemoryChunk(chunksSize, nullptr)) {

}

MemoryManageable *ChunkedMemoryManager::alloc(size_t size) {
    MemoryManageable* ptr = reinterpret_cast<MemoryManageable*>(MemoryChunk::alloc(_lastChunk, size));
    _allocated.push_back(ptr);
    return ptr;
}

void ChunkedMemoryManager::free() {
    for (auto ptr : _allocated) {
        ptr->~MemoryManageable();
    }
    delete _lastChunk;
}

}

}
