/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#include <modules/oit/oitmoduledefine.h>

#include <inviwo/core/util/fmtutils.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/dispatcher.h>

#include <fmt/format.h>

namespace inviwo {
class Rasterization;
class Shader;

enum class UseFragmentList { Yes, No };

constexpr std::string_view enumToStr(UseFragmentList val) {
    switch (val) {
        case UseFragmentList::Yes:
            return "Yes";
        case UseFragmentList::No:
            return "No";
    }
    throw Exception(IVW_CONTEXT_CUSTOM("enumToStr"), "Invalid UseFragmentList found: {}",
                    static_cast<int>(val));
}

class RasterizationRendererBase {
public:
    virtual void configureShader(Shader&) const = 0;
    virtual void setUniforms(Shader&, UseFragmentList useFragmentList,
                             std::string_view rasterizerId) const = 0;
    virtual DispatcherHandle<void()> addInitializeShaderCallback(
        std::function<void()> callback) = 0;
};

}  // namespace inviwo

#ifndef DOXYGEN_SHOULD_SKIP_THIS
template <>
struct fmt::formatter<inviwo::UseFragmentList> : inviwo::FlagFormatter<inviwo::UseFragmentList> {};
#endif
