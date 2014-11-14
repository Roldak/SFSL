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

MemoryChunk::MemoryChunk(size_t size) : _chunk(new char[size]), _chunkSize(size), _offset(0), _parent(nullptr) {

}

MemoryChunk::~MemoryChunk() {
    delete _chunk;

    if (_parent) {
        delete _parent;
    }
}

void* MemoryChunk::alloc(MemoryChunk*& chunk, size_t size) {
    if (size > chunk->_chunkSize - chunk->_offset) {

        chunk = new MemoryChunk(chunk->_chunkSize * 2);
        return alloc(chunk, size);

    } else {

        char* toRet = chunk->_chunk + chunk->_offset;
        chunk->_offset += size;
        return static_cast<void*>(toRet);

    }
}

ChunkedMemoryManager::ChunkedMemoryManager(size_t chunksSize) : _lastChunk(new MemoryChunk(chunksSize)) {

}

void* ChunkedMemoryManager::alloc(size_t size) {
    return MemoryChunk::alloc(_lastChunk, size);
}

void ChunkedMemoryManager::manage(MemoryManageable *ptr) {
    _allocated.push_back(ptr);
}

void ChunkedMemoryManager::free() {
    for (auto ptr : _allocated) {
        ptr->~MemoryManageable();
    }
    delete _lastChunk;
}

}

}
