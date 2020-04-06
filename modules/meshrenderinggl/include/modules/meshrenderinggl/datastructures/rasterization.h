/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2020 Inviwo Foundation
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

#include <modules/meshrenderinggl/meshrenderingglmoduledefine.h>
#include <modules/opengl/shader/shaderutils.h>

#include <vector>
#include <unordered_map>
#include <optional>

namespace inviwo {

/**
 * \brief A functor class for rendering geometry into a fragment list
 * Will be applied by a renderer containing an A-buffer.
 */

class IVW_MODULE_MESHRENDERINGGL_API Rasterization {
public:
    virtual ~Rasterization() {}
    virtual void rasterize(std::function<void(Shader&)> setUniforms) const = 0;
    virtual void setImageSize(const ivec2& size) const = 0;
    virtual bool usesFragmentLists() const = 0;
};

template <>
struct DataTraits<Rasterization> {
    static std::string classIdentifier() { return "org.inviwo.Rasterization"; }
    static std::string dataName() { return "Rasterization"; }
    static uvec3 colorCode() { return uvec3(80, 160, 160); }
    static Document info(const Rasterization& data) {
        std::ostringstream oss;
        oss << "Functor for rasterization.";
        Document doc;
        doc.append("p", oss.str());
        return doc;
    }
};
}  // namespace inviwo