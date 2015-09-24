/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include "layerglconverter.h"
#include <modules/opengl/texture/texture2d.h>

namespace inviwo {

std::shared_ptr<LayerGL> LayerRAM2GLConverter::createFrom(
    std::shared_ptr<const LayerRAM> layerRAM) const {
    auto layerGL = std::make_shared<LayerGL>(layerRAM->getDimensions(), layerRAM->getLayerType(),
                                             layerRAM->getDataFormat());
    layerGL->getTexture()->initialize(layerRAM->getData());
    return layerGL;
}

void LayerRAM2GLConverter::update(std::shared_ptr<const LayerRAM> layerSrc,
                                  std::shared_ptr<LayerGL> layerDst) const {
    if (layerSrc->getDimensions() != layerDst->getDimensions()) {
        layerDst->setDimensions(layerSrc->getDimensions());
    }

    layerDst->getTexture()->upload(layerSrc->getData());
}

std::shared_ptr<LayerRAM> LayerGL2RAMConverter::createFrom(
    std::shared_ptr<const LayerGL> layerGL) const {
    auto layerRAM =
        createLayerRAM(layerGL->getDimensions(), layerGL->getLayerType(), layerGL->getDataFormat());

    if (layerRAM) {
        layerGL->getTexture()->download(layerRAM->getData());
        return layerRAM;
    } else {
        LogError("Cannot convert format from GL to RAM:" << layerGL->getDataFormat()->getString());
    }

    return nullptr;
}

void LayerGL2RAMConverter::update(std::shared_ptr<const LayerGL> layerSrc,
                                  std::shared_ptr<LayerRAM> layerDst) const {
    if (layerSrc->getDimensions() != layerDst->getDimensions()) {
        layerDst->setDimensions(layerSrc->getDimensions());
    }
    layerSrc->getTexture()->download(layerDst->getData());
}

}  // namespace
