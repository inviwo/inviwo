/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2026 Inviwo Foundation
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

#include <inviwo/core/util/assertion.h>
#include <modules/opengl/openglexception.h>

#include <algorithm>
#include <string_view>
#include <utility>

namespace inviwo {

std::array<std::atomic<bool>, 128> TextureUnit::textureUnits_{false};

TextureUnit::TextureUnit() : unitEnum_(0), unitNumber_(0) {
    // check which texture unit is available
    for (size_t i = 1; i < maxTextureImageUnits(); i++) {
        bool expected = false;
        if (textureUnits_[i].compare_exchange_weak(expected, true)) {
            unitNumber_ = static_cast<GLint>(i);
            unitEnum_ = GL_TEXTURE0 + unitNumber_;
            return;
        }
    }

    throw OpenGLException("Exceeding number of available texture units.");
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
    }
}

size_t TextureUnit::maxTextureImageUnits() {
    static const size_t max = []() {
        GLint numTexUnits_ = -1;
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &numTexUnits_);
        IVW_ASSERT(numTexUnits_ > 0 && numTexUnits_ < textureUnits_.size(),
                   "Unexpected number of texture units");
        return static_cast<size_t>(numTexUnits_);
    }();
    return max;
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
