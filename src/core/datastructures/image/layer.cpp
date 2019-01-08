/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2018 Inviwo Foundation
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
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/io/datawriter.h>
#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/io/datawriterexception.h>

namespace inviwo {

Layer::Layer(size2_t dimensions, const DataFormatBase* format, LayerType type,
             const SwizzleMask& swizzleMask)
    : Data<Layer, LayerRepresentation>(format)
    , StructuredGridEntity<2>(dimensions)
    , layerType_(type)
    , swizzleMask_(swizzleMask) {}

Layer::Layer(std::shared_ptr<LayerRepresentation> in)
    : Data<Layer, LayerRepresentation>(in->getDataFormat())
    , StructuredGridEntity<2>(in->getDimensions())
    , layerType_(in->getLayerType())
    , swizzleMask_(in->getSwizzleMask()) {
    addRepresentation(in);
}

Layer* Layer::clone() const { return new Layer(*this); }

size2_t Layer::getDimensions() const {
    if (hasRepresentations() && lastValidRepresentation_) {
        size2_t dim = lastValidRepresentation_->getDimensions();
        if (dim != size2_t(0)) return dim;
    }
    return StructuredGridEntity<2>::getDimensions();
}

void Layer::setDimensions(const size2_t& dim) {
    StructuredGridEntity<2>::setDimensions(dim);

    if (lastValidRepresentation_) {
        // Resize last valid representation
        removeOtherRepresentations(lastValidRepresentation_.get());
        lastValidRepresentation_->setDimensions(dim);
    }
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
    targetLayer->StructuredGridEntity<2>::setDimensions(clone->getDimensions());

    if (!lastValidRepresentation_->copyRepresentationsTo(clone.get())) {
        throw Exception("Failed to copy Layer Representation", IvwContext);
    }
}

LayerType Layer::getLayerType() const { return layerType_; }

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

void Layer::setSwizzleMask(const SwizzleMask& mask) {
    if ((layerType_ == LayerType::Color) && this->hasRepresentations()) {
        // update swizzle mask of all representations
        for (auto rep : representations_) {
            rep.second->setSwizzleMask(mask);
        }
    }
    swizzleMask_ = mask;
}

SwizzleMask Layer::getSwizzleMask() const {
    if (this->hasRepresentations() && lastValidRepresentation_) {
        return lastValidRepresentation_->getSwizzleMask();
    }
    return swizzleMask_;
}

std::shared_ptr<LayerRepresentation> Layer::createDefaultRepresentation() const {
    return createLayerRAM(getDimensions(), getLayerType(), getDataFormat(), getSwizzleMask());
}

void Layer::updateMetaFromRepresentation(const LayerRepresentation* layerRep) {
    if (layerRep) {
        StructuredGridEntity<2>::setDimensions(layerRep->getDimensions());
        layerType_ = layerRep->getLayerType();
        swizzleMask_ = layerRep->getSwizzleMask();
    }
}

template class IVW_CORE_TMPL_INST DataReaderType<Layer>;
template class IVW_CORE_TMPL_INST DataWriterType<Layer>;

}  // namespace inviwo
