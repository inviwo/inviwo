/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2023 Inviwo Foundation
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

#include <modules/opengl/texture/textureunit.h>

#include <inviwo/core/util/assertion.h>      // for ivwAssert
#include <inviwo/core/util/sourcecontext.h>  // for IVW_CONTEXT
#include <modules/opengl/openglexception.h>  // for OpenGLException
#include <modules/opengl/texture/samplerobject.h>

#include <algorithm>    // for max
#include <string_view>  // for string_view
#include <utility>      // for swap, move

namespace inviwo {

std::vector<bool> TextureUnit::textureUnits_{};
std::vector<GLint> TextureUnit::samplerObjects_{};

TextureUnit::TextureUnit() : unitEnum_(0), unitNumber_(0) {
    ivwAssert(!textureUnits_.empty(), "Texture unit handler not initialized.");

    // check which texture unit is available
    for (size_t i = 1; i < textureUnits_.size(); i++) {
        if (textureUnits_[i] == false) {
            // unit previously unused, mark as used now
            textureUnits_[i] = true;
            unitNumber_ = (GLint)i;
            unitEnum_ = GL_TEXTURE0 + unitNumber_;
            return;
        }
    }

    throw OpenGLException("Exceeding number of available texture units.", IVW_CONTEXT);
}

TextureUnit::TextureUnit(TextureUnit&& rhs) noexcept
    : unitEnum_(rhs.unitEnum_), unitNumber_(rhs.unitNumber_) {
    rhs.unitEnum_ = 0;
    rhs.unitNumber_ = 0;
}

TextureUnit& TextureUnit::operator=(TextureUnit&& that) noexcept {
    if (this != &that) {
        if (textureUnits_.size() > static_cast<size_t>(unitNumber_)) {
            textureUnits_[static_cast<size_t>(unitNumber_)] = false;
        }
        unitEnum_ = 0;
        unitNumber_ = 0;
        std::swap(unitEnum_, that.unitEnum_);
        std::swap(unitNumber_, that.unitNumber_);
    }
    return *this;
}

TextureUnit::~TextureUnit() {
    if (textureUnits_.size() > static_cast<size_t>(unitNumber_)) {
        textureUnits_[static_cast<size_t>(unitNumber_)] = false;
        if (samplerObjects_[static_cast<size_t>(unitNumber_)] != 0) {
            glBindSampler(unitNumber_, 0);
            samplerObjects_[static_cast<size_t>(unitNumber_)] = 0;
        }
    }
}

void TextureUnit::initialize(int numUnits) {
    textureUnits_.resize(numUnits, false);
    samplerObjects_.resize(numUnits, 0);
}

void TextureUnit::deinitialize() {
    textureUnits_.clear();
    samplerObjects_.clear();
}

void TextureUnit::bindSampler(const SamplerObject& so) {
    glBindSampler(unitNumber_, so.getID());
    samplerObjects_[static_cast<size_t>(unitNumber_)] = so.getID();
}

TextureUnitContainer::TextureUnitContainer(size_t i) : units_{} {
    units_.reserve(std::max(size_t{8}, i));
    units_.resize(i);
}

void TextureUnitContainer::push_back(TextureUnit&& unit) { units_.push_back(std::move(unit)); }

TextureUnit& TextureUnitContainer::emplace_back() { return units_.emplace_back(); }

TextureUnit& TextureUnitContainer::operator[](size_t i) { return units_[i]; }
const TextureUnit& TextureUnitContainer::operator[](size_t i) const { return units_[i]; }
size_t TextureUnitContainer::size() const { return units_.size(); }
void TextureUnitContainer::clear() { units_.clear(); }

}  // namespace inviwo
