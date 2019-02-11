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

#ifndef IVW_TEMPFILEHANDLE_H
#define IVW_TEMPFILEHANDLE_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

#include <cstdio>
#include <string>

namespace inviwo {

namespace util {

/**
 * \class TempFileHandle
 * \brief RAII interface for providing a file handle and file name to a temporary file
 */
class IVW_CORE_API TempFileHandle {
public:
    explicit TempFileHandle(const std::string& prefix = "", const std::string& suffix = "");

    TempFileHandle(const TempFileHandle&) = delete;
    TempFileHandle& operator=(const TempFileHandle&) = delete;

    TempFileHandle(TempFileHandle&& rhs);
    TempFileHandle& operator=(TempFileHandle&& rhs);

    ~TempFileHandle();

    const std::string& getFileName() const;

    FILE* getHandle();
    operator FILE*();

private:
    void cleanup();

    FILE* handle_;
    std::string filename_;
};

}  // namespace util

}  // namespace inviwo

#endif  // IVW_TEMPFILEHANDLE_H
