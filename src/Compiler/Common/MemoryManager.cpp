//
//  MemoryManager.cpp
//  SFSL
//
//  Created by Romain Beguet on 12.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "MemoryManager.h"
#include "Reporter.h"
#include "../../Utils/Utils.h"

namespace sfsl {

namespace common {

// ABSTRACT MEMORY MANAGER

AbstractMemoryManager::~AbstractMemoryManager() {

}

std::string AbstractMemoryManager::getInfos() const {
    return "<no info available for this Memory Manager>";
}

// MEMORY CHUNK

MemoryChunk::MemoryChunk(size_t size, MemoryChunk* parent) : _chunk(new char[size]), _chunkSize(size), _offset(0), _parent(parent) {

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

const MemoryChunk* MemoryChunk::getParent() const {
    return _parent;
}

size_t MemoryChunk::getChunkSize() const {
    return _chunkSize;
}

size_t MemoryChunk::getUsedChunkSize() const {
    return _offset;
}

// CHUNKED MEMORY MANAGER

ChunkedMemoryManager::ChunkedMemoryManager(size_t chunksSize) : _lastChunk(new MemoryChunk(chunksSize, nullptr)) {

}

ChunkedMemoryManager::~ChunkedMemoryManager() {
    for (auto ptr : _allocated) {
        ptr->~MemoryManageable();
    }
    delete _lastChunk;
}

std::string ChunkedMemoryManager::getInfos() const {
    std::string toRet = "ChunkedMemoryManager{";

    size_t chunkCount = 0, totalUsedSize = 0, totalChunkSize = 0;
    for (const MemoryChunk* cur = _lastChunk; cur != nullptr; cur = cur->getParent()) {
        ++chunkCount;
        totalUsedSize += cur->getUsedChunkSize();
        totalChunkSize += cur->getChunkSize();
    }

    toRet += utils::T_toString(chunkCount) + " chunks; ";
    toRet += utils::T_toString(_allocated.size()) + " objects; ";
    toRet += utils::T_toString(totalUsedSize) + "/";
    toRet += utils::T_toString(totalChunkSize) + " bytes}";
    return toRet;
}

MemoryManageable* ChunkedMemoryManager::alloc(size_t size) {
    MemoryManageable* ptr = reinterpret_cast<MemoryManageable*>(MemoryChunk::alloc(_lastChunk, size));
    _allocated.push_back(ptr);
    return ptr;
}



}

}
