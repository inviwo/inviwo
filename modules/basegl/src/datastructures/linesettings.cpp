/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2022 Inviwo Foundation
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

#include <modules/basegl/datastructures/linesettings.h>

#include <modules/basegl/datastructures/linesettingsinterface.h>  // for LineSettingsInterface
#include <modules/basegl/datastructures/stipplingsettings.h>      // for StipplingSettings

namespace inviwo {
class StipplingSettingsInterface;

LineSettings::LineSettings(const LineSettingsInterface* other)
    : lineWidth(other->getWidth())
    , antialiasing(other->getAntialiasingWidth())
    , miterLimit(other->getMiterLimit())
    , roundCaps(other->getRoundCaps())
    , pseudoLighting(other->getPseudoLighting())
    , roundDepthProfile(other->getRoundDepthProfile())
    , stippling(&other->getStippling()) {}

float LineSettings::getWidth() const { return lineWidth; }

float LineSettings::getAntialiasingWidth() const { return antialiasing; }

float LineSettings::getMiterLimit() const { return miterLimit; }

bool LineSettings::getRoundCaps() const { return roundCaps; }

bool LineSettings::getPseudoLighting() const { return pseudoLighting; }

bool LineSettings::getRoundDepthProfile() const { return roundDepthProfile; }

const StipplingSettingsInterface& LineSettings::getStippling() const { return stippling; }

}  // namespace inviwo
