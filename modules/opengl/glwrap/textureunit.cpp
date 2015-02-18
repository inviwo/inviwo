/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include "textureunit.h"

namespace inviwo {

std::vector<bool>* TextureUnit::textureUnits_ = NULL;

TextureUnit::TextureUnit()
    : unitEnum_(0)
    , unitNumber_(0)
{
    ivwAssert(textureUnits_!=NULL, "Texture unit handler not initialized.");

    // check which texture unit is available
    for (size_t i=1; i<textureUnits_->size(); i++) {
        if (textureUnits_->at(i) == false) {
            // unit previously unused, mark as used now
            textureUnits_->at(i) = true;
            unitNumber_ = (GLint)i;
            unitEnum_ = GL_TEXTURE0 + unitNumber_;
            return;
        }
    }

    LogWarn("Exceeding number of available texture units.");
}

TextureUnit::~TextureUnit() {
    if (textureUnits_ && static_cast<int>(textureUnits_->size()) > static_cast<int>(unitNumber_)) {
        textureUnits_->at(static_cast<int>(unitNumber_)) = false;
    }
}

void TextureUnit::initialize(int numUnits) {
    if (!textureUnits_) {
        textureUnits_ = new std::vector<bool>(numUnits);
    }
}

void TextureUnit::deinitialize() {
    if (textureUnits_) {
        delete textureUnits_;
        textureUnits_ = NULL;
    }
}

} // namespace
