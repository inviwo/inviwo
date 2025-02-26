/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/sourcecontext.h>
#include <sstream>
#include <string_view>

namespace inviwo {

IVW_CORE_API void assertion(std::string_view message,
                            SourceContext context = std::source_location::current());

namespace util {

IVW_CORE_API void debugBreak();

}  // namespace util

}  // namespace inviwo

#if defined(IVW_DEBUG) || defined(IVW_FORCE_ASSERTIONS)

namespace inviwo::cfg {
constexpr bool assertions = true;
}

#define IVW_ASSERT(condition, message)                    \
    {                                                     \
        if (!(bool(condition))) {                         \
            std::ostringstream stream;                    \
            stream << message;                            \
            ::inviwo::assertion(std::move(stream).str()); \
        }                                                 \
    }

// Deprecated
#define ivwAssert(condition, message) IVW_ASSERT(condition, message)

#else

namespace inviwo::cfg {
constexpr bool assertions = false;
}

#define IVW_ASSERT(condition, message)
#define ivwAssert(condition, message)

#endif
