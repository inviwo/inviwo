/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2018 Inviwo Foundation
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

#include <modules/basegl/processors/imageprocessing/imagemask.h>

#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/image/imageram.h>

namespace inviwo {

const ProcessorInfo ImageMask::processorInfo_{
    "org.inviwo.ImageMask",  // Class identifier
    "Image Mask",            // Display name
    "Image Operation",         // Category
    CodeState::Experimental,  // Code state
    Tags::CPU,                // Tags
};
const ProcessorInfo ImageMask::getProcessorInfo() const { return processorInfo_; }

ImageMask::ImageMask()
    : Processor()
    , imageInport_("image_inport", true)
    , imageAnnotationInport_("image_annotation_inport", true)
    , imageOutport_("image_outport", false)
    , enableMasking_("enableMasking", "Enable Masking", true)
    , fillColor_("fillColor", "Fill Color", vec4{0.0}, vec4{0.0}, vec4{65535.0}, vec4{1e-3})
    , idx_("idx", "Index", 0, 0, 10000000000, 1, InvalidationLevel::Valid)
    , addIdx_("addIdx", "Add Index")
    , removeIdx_("removeIdx", "Remove Index")
    , clearIdxList_("clearIdxList", "Clear Index List")
    , idxList_("idxList", "Index List")
    , idxTableFile_("idxTableFile", "Idx File (1 index per row)") {

    addPort(imageInport_);
    addPort(imageAnnotationInport_);
    addPort(imageOutport_);

    addProperty(enableMasking_);
    addProperty(fillColor_);
    addProperty(idx_);

    addProperty(addIdx_);
    addIdx_.onChange([this]() {
        bool contains{false};
        for (const auto& idx : idxList_.getValues()) {
            if (idx == idx_.get()) {
                contains = true;
                break;
            }
        }

        if (!contains) {
            const auto str = std::to_string(idx_);
            idxList_.addOption(str, str, idx_);
        }
    });

    addProperty(removeIdx_);
    removeIdx_.onChange([this]() {
        idxList_.removeOption(idxList_.getSelectedIndex());
    });

    addProperty(clearIdxList_);
    clearIdxList_.onChange([this]() {
        idxList_.clearOptions();
        idxList_.propertyModified();
    });

    idxList_.setCurrentStateAsDefault();
    addProperty(idxList_);

    idxTableFile_.onChange([this]() {
        // read index file line by line, each line contains 1 index
        std::ifstream file_stream(idxTableFile_);
        if (file_stream.is_open()) {
            idxList_.clearOptions();
            size_t idx;
            while (file_stream >> idx) {
                const auto str = std::to_string(idx);
                idxList_.addOption(str, str, idx);
            }
        }
    });
    addProperty(idxTableFile_);
}

void ImageMask::process() {
    if (!enableMasking_) {
        imageOutport_.setData(imageInport_.getData());
        return;
    }

    if (imageAnnotationInport_.getData()->getDimensions() != imageInport_.getData()->getDimensions()) {
        LogWarn("image dimensions do not match, no masking performed!");
        imageOutport_.setData(imageInport_.getData());
        return;
    }

    const auto imgAnnoRAM =
        imageAnnotationInport_.getData()->getColorLayer()->getRepresentation<LayerRAM>();
    auto imgOut = imageInport_.getData()->clone();
    auto imgOutRAM = imgOut->getColorLayer()->getEditableRepresentation<LayerRAM>();
    const auto imageSize = imgAnnoRAM->getDimensions();

#pragma omp parallel for
    for (long y = 0; y < imageSize.y; ++y) {
        for (size_t x = 0; x < imageSize.x; ++x) {
            const size2_t pos{x, y};
            const auto annoValue = static_cast<uint32_t>(imgAnnoRAM->getAsDouble(pos));

            bool contains{false};
            for (const auto& refIdx : idxList_.getValues()) {
                if (annoValue == refIdx) {
                    contains = true;
                    break;
                }
            }

            if (!contains) {
                imgOutRAM->setFromDVec4(pos, fillColor_.get());
            }
        }
    }

    imageOutport_.setData(imgOut);
}

}  // namespace inviwo
