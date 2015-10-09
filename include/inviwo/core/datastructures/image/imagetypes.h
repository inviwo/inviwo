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

#ifndef IVW_IMAGETYPES_H
#define IVW_IMAGETYPES_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/io/serialization/ivwdeserializer.h>

namespace inviwo {

enum class ImageType {
    ColorOnly = 0,
    ColorDepth = 1,
    ColorPicking = 2,
    ColorDepthPicking = 3,
    AllLayers = ColorDepthPicking
};

enum class LayerType { Color = 0, Depth = 1, Picking = 2 };

#include <warn/push>
#include <warn/ignore/unused-function>
static bool typeContainsColor(ImageType type) {
    return (type == ImageType::ColorOnly || type == ImageType::ColorDepth ||
            type == ImageType::ColorPicking || type == ImageType::ColorDepthPicking);
}

static bool typeContainsDepth(ImageType type) {
    return (type == ImageType::ColorDepth || type == ImageType::ColorDepthPicking);
}

static bool typeContainsPicking(ImageType type) {
    return (type == ImageType::ColorPicking || type == ImageType::ColorDepthPicking);
}
#include <warn/pop>
}  // namespace

#endif  // IVW_IMAGETYPES_H
