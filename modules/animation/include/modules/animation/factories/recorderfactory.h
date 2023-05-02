/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023 Inviwo Foundation
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

#include <modules/animation/animationmoduledefine.h>

#include <inviwo/core/util/glmvec.h>

#include <string>
#include <memory>

namespace inviwo {

class BoolCompositeProperty;
class LayerRAM;

namespace animation {

class IVW_MODULE_ANIMATION_API Recorder {
public:
    Recorder() = default;
    Recorder(const Recorder&) = delete;
    Recorder& operator=(const Recorder&) = delete;
    Recorder(Recorder&&) = delete;
    Recorder& operator=(Recorder&&) = delete;
    virtual ~Recorder() = default;

    virtual void record(const LayerRAM& layer) = 0;
};

struct IVW_MODULE_ANIMATION_API RecorderOptions {
    size2_t dimensions = size2_t{512, 512};
    int frameRate = 25;
    int expectedNumberOfFrames = 1000;
    std::string sourceName = "";
};

class IVW_MODULE_ANIMATION_API RecorderFactory {
public:
    RecorderFactory() = default;
    RecorderFactory(const RecorderFactory&) = delete;
    RecorderFactory& operator=(const RecorderFactory&) = delete;
    RecorderFactory(RecorderFactory&&) = delete;
    RecorderFactory& operator=(RecorderFactory&&) = delete;
    virtual ~RecorderFactory() = default;

    virtual const std::string& getClassIdentifier() const = 0;
    virtual BoolCompositeProperty* options() = 0;
    virtual std::unique_ptr<Recorder> create(const RecorderOptions& opts) = 0;
};
}  // namespace animation
}  // namespace inviwo
