/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/io/datawriter.h>
#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/io/datawriterexception.h>

namespace inviwo {

Layer::Layer(size2_t defaultDimensions, const DataFormatBase* defaultFormat, LayerType type,
             const SwizzleMask& defaultSwizzleMask)
    : Data<Layer, LayerRepresentation>()
    , StructuredGridEntity<2>()
    , layerType_(type)
    , defaultDimensions_(defaultDimensions)
    , defaultDataFormat_(defaultFormat)
    , defaultSwizzleMask_(defaultSwizzleMask) {}

Layer::Layer(std::shared_ptr<LayerRepresentation> in)
    : Data<Layer, LayerRepresentation>()
    , StructuredGridEntity<2>()
    , layerType_(in->getLayerType())
    , defaultDimensions_(in->getDimensions())
    , defaultDataFormat_(in->getDataFormat())
    , defaultSwizzleMask_(in->getSwizzleMask()) {
    addRepresentation(in);
}

Layer* Layer::clone() const { return new Layer(*this); }

LayerType Layer::getLayerType() const { return layerType_; }

void Layer::setDimensions(const size2_t& dim) {
    defaultDimensions_ = dim;
    if (lastValidRepresentation_) {
        // Resize last valid representation
        lastValidRepresentation_->setDimensions(dim);
        invalidateAllOther(lastValidRepresentation_.get());
    }
}

size2_t Layer::getDimensions() const {
    if (lastValidRepresentation_) {
        return lastValidRepresentation_->getDimensions();
    }
    return defaultDimensions_;
}

void Layer::setDataFormat(const DataFormatBase* format) { defaultDataFormat_ = format; }

const DataFormatBase* Layer::getDataFormat() const {
    if (lastValidRepresentation_) {
        return lastValidRepresentation_->getDataFormat();
    }

    return defaultDataFormat_;
}

void Layer::setSwizzleMask(const SwizzleMask& mask) {
    defaultSwizzleMask_ = mask;
    if (lastValidRepresentation_) {
        lastValidRepresentation_->setSwizzleMask(mask);
    }
}

SwizzleMask Layer::getSwizzleMask() const {
    if (lastValidRepresentation_) {
        return lastValidRepresentation_->getSwizzleMask();
    }
    return defaultSwizzleMask_;
}

void Layer::copyRepresentationsTo(Layer* targetLayer) {
    for (auto& source : representations_) {
        auto sourceRepr = source.second.get();
        if (sourceRepr->isValid()) {
            for (auto& target : targetLayer->representations_) {
                auto targetRepr = target.second.get();
                if (typeid(*sourceRepr) == typeid(*targetRepr)) {
                    if (sourceRepr->copyRepresentationsTo(targetRepr)) {
                        targetLayer->invalidateAllOther(targetRepr);
                        return;
                    }
                }
            }
        }
    }

    // Fall-back
    auto clone = std::shared_ptr<LayerRepresentation>(lastValidRepresentation_->clone());
    targetLayer->addRepresentation(clone);
    targetLayer->removeOtherRepresentations(clone.get());

    if (!lastValidRepresentation_->copyRepresentationsTo(clone.get())) {
        throw Exception("Failed to copy Layer Representation", IVW_CONTEXT);
    }
}

std::unique_ptr<std::vector<unsigned char>> Layer::getAsCodedBuffer(
    const std::string& fileExtension) const {
    if (auto writer = std::shared_ptr<DataWriterType<Layer>>(
            InviwoApplication::getPtr()
                ->getDataWriterFactory()
                ->getWriterForTypeAndExtension<Layer>(fileExtension))) {
        try {
            return writer->writeDataToBuffer(this, fileExtension);
        } catch (DataWriterException const& e) {
            LogError(e.getMessage());
        }
    } else {
        LogError("Could not find a writer for the specified file extension (\"" << fileExtension
                                                                                << "\")");
    }

    return std::unique_ptr<std::vector<unsigned char>>();
}

void Layer::updateMetaFromRepresentation(const LayerRepresentation* layerRep) {
    if (layerRep) {
        layerType_ = layerRep->getLayerType();
    }
}

template class IVW_CORE_TMPL_INST DataReaderType<Layer>;
template class IVW_CORE_TMPL_INST DataWriterType<Layer>;

}  // namespace inviwo
