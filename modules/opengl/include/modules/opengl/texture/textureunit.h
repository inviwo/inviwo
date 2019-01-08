/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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

#ifndef IVW_TEXTUREUNIT_H
#define IVW_TEXTUREUNIT_H

#include <modules/opengl/openglmoduledefine.h>
#include <modules/opengl/inviwoopengl.h>

#include <vector>

namespace inviwo {

class IVW_MODULE_OPENGL_API TextureUnit {
public:
    TextureUnit();
    ~TextureUnit();

    TextureUnit(TextureUnit& rhs) = delete;
    TextureUnit& operator=(const TextureUnit& that) = delete;

    TextureUnit(TextureUnit&& rhs);
    TextureUnit& operator=(TextureUnit&& that);

    // These are called from OpenGLCapabilities retrieveStaticInfo/destructor.
    static void initialize(int numUnits);
    static void deinitialize();

    inline GLint getEnum() const { return unitEnum_; }
    inline GLint getUnitNumber() const { return unitNumber_; }

    inline void activate() const { glActiveTexture(unitEnum_); }
    inline static void setZeroUnit() { glActiveTexture(GL_TEXTURE0); }

private:
    static std::vector<bool> textureUnits_;

    GLint unitEnum_;
    GLint unitNumber_;
};

class IVW_MODULE_OPENGL_API TextureUnitContainer {
public:
    TextureUnitContainer(size_t i = 0);
    TextureUnitContainer(const TextureUnitContainer&) = delete;
    TextureUnitContainer& operator=(const TextureUnitContainer&) = delete;
    TextureUnitContainer(TextureUnitContainer&& rhs);
    TextureUnitContainer& operator=(TextureUnitContainer&& that);
    ~TextureUnitContainer() = default;

    void push_back(TextureUnit&& unit);

    TextureUnit& operator[](size_t i);
    size_t size() const;

private:
    std::vector<TextureUnit> units_;
};

}  // namespace inviwo

#endif  // IVW_TEXTUREUNIT_H
