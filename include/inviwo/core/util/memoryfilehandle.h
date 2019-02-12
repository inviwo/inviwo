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

#ifndef IVW_MEMORYFILEHANDLE_H
#define IVW_MEMORYFILEHANDLE_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

#include <cstdio>

namespace inviwo {

namespace util {

/**
 * \class MemoryFileHandle
 * \brief RAII class for providing a FILE stream handle to a buffer in memory.
 *      This class will open a file handle to /dev/null and use a dedicated buffer for
 *      buffering. As long as less bytes than getBufferSize() bytes are written/read, its
 *      status is well defined. However, after writing more than buffer size bytes, the
 *      buffer contents will be flushed, i.e. are no longer accessible.
 *
 *      To detect potential overflows, the entire buffer is initialized with the value 0xaf.
 *      If the number of bytes in the buffer is less than the buffer size, the next 8 bytes
 *      are checked whether they contain 0xaf. If not, then there was a potential buffer
 *      overflow.
 */
class IVW_CORE_API MemoryFileHandle {
public:
    MemoryFileHandle() = delete;
    explicit MemoryFileHandle(size_t bufferSize);

    MemoryFileHandle(const MemoryFileHandle&) = delete;
    MemoryFileHandle& operator=(const MemoryFileHandle&) = delete;

    MemoryFileHandle(MemoryFileHandle&& rhs);
    MemoryFileHandle& operator=(MemoryFileHandle&& rhs);

    ~MemoryFileHandle();

    FILE* getHandle();
    operator FILE*();

    void reset();

    void resize(size_t bufferSize);
    size_t getBufferSize() const;

    /**
     * \brief returns the number of bytes currently stored in the buffer.
     * If the number of bytes written using the file handle exceeds the buffer size,
     * the result will be the number of bytes modulo buffer size.
     *
     * @return number of bytes
     */
    size_t getNumberOfBytesInBuffer() const;

    std::vector<unsigned char>& getBuffer();
    const std::vector<unsigned char>& getBuffer() const;

    bool checkForOverflow() const;

private:
    void initBuffer();
    const unsigned char magicCode_ = 0xaf;
    FILE* handle_;
    std::vector<unsigned char> buffer_;
};

}  // namespace util

}  // namespace inviwo

#endif  // IVW_MEMORYFILEHANDLE_H
