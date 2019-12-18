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

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/ports/inport.h>
#include <inviwo/core/properties/property.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shadersegment.h>
#include <modules/opengl/texture/textureunit.h>

#include <vector>
#include <string>
#include <tuple>

namespace inviwo {

class IVW_MODULE_BASEGL_API RaycasterComponent {
public:
    using Type = typename ShaderSegment::Type;

    struct Segment {
        static const Type include;
        static const Type uniform;
        static const Type main;
        static const Type pre;
        static const Type loop;
        static const Type post;

        std::string snippet;
        Type type = loop;
        size_t priority = 1000;
    };

    virtual ~RaycasterComponent() = default;

    virtual std::string getName() const = 0;
    virtual void setUniforms(Shader& shader, TextureUnitContainer& cont) const;
    virtual void setDefines(Shader& shader) const;
    virtual std::vector<std::tuple<Inport*, std::string>> getInports() { return {}; }
    virtual std::vector<Property*> getProperties() { return {}; }

    virtual std::vector<Segment> getSegments() const { return {}; }
};

}  // namespace inviwo
