/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <inviwo/core/util/memoryfilehandle.h>

#include <algorithm>

namespace inviwo {

namespace util {

MemoryFileHandle::MemoryFileHandle(size_t bufferSize) : buffer_(bufferSize, magicCode_) {
    initBuffer();
}

MemoryFileHandle::MemoryFileHandle(MemoryFileHandle&& rhs)
    : handle_{rhs.handle_}, buffer_{std::move(rhs.buffer_)} {
    rhs.handle_ = nullptr;
}

MemoryFileHandle& MemoryFileHandle::operator=(MemoryFileHandle&& rhs) {
    if (this != &rhs) {
        if (handle_) fclose(handle_);
        handle_ = rhs.handle_;
        buffer_ = std::move(rhs.buffer_);
        rhs.handle_ = nullptr;
    }
    return *this;
}

MemoryFileHandle::~MemoryFileHandle() {
    if (handle_) fclose(handle_);
}

FILE* MemoryFileHandle::getHandle() { return handle_; }

MemoryFileHandle::operator FILE*() { return handle_; }

void MemoryFileHandle::reset() {
    fflush(handle_);
    std::fill(buffer_.begin(), buffer_.end(), magicCode_);
}

void MemoryFileHandle::resize(size_t bufferSize) {
    if (buffer_.size() == bufferSize) {
        return;
    }

    // close handle first since the buffer must exist beyond the lifetime of the handle
    fclose(handle_);

    buffer_.clear();
    buffer_.resize(bufferSize, magicCode_);

    initBuffer();
}

size_t MemoryFileHandle::getBufferSize() const { return buffer_.size(); }

size_t MemoryFileHandle::getNumberOfBytesInBuffer() const {
    return static_cast<size_t>(ftell(handle_));
}

const std::vector<unsigned char>& MemoryFileHandle::getBuffer() const { return buffer_; }

std::vector<unsigned char>& MemoryFileHandle::getBuffer() { return buffer_; }

bool MemoryFileHandle::checkForOverflow() const {

    auto pos = getNumberOfBytesInBuffer();

    bool overflowDetected = false;
    // check buffer contents after current position (at least 8 byte)
    if (pos < buffer_.size()) {
        int i = 1;
        while ((i + pos < buffer_.size()) && (i <= 8) && !overflowDetected) {
            overflowDetected = ((buffer_[pos + i]) != magicCode_);
            ++i;
        }
    }
    return overflowDetected;
}

void MemoryFileHandle::initBuffer() {
#ifdef WIN32
    handle_ = fopen("NUL", "wb");
#else
    handle_ = fopen("/dev/null", "w");
#endif
    // setvbuf needs to be set first thing after the file handle has been created
    setvbuf(handle_, reinterpret_cast<char*>(buffer_.data()), _IOFBF, buffer_.size());
}

}  // namespace util

}  // namespace inviwo
