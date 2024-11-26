/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2024 Inviwo Foundation
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

#include <inviwo/dataframe/processors/volumetodataframe.h>

#include <inviwo/core/datastructures/buffer/buffer.h>                   // for Buffer
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>       // for BufferRAMPrecision
#include <inviwo/core/datastructures/coordinatetransformer.h>           // for StructuredCoordin...
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/volume/volume.h>                   // for DataInport
#include <inviwo/core/datastructures/volume/volumeram.h>                // for VolumeRAM
#include <inviwo/core/datastructures/volume/volumeramprecision.h>       // IWYU pragma: keep
#include <inviwo/core/ports/datainport.h>                               // for DataInport
#include <inviwo/core/ports/dataoutport.h>                              // for DataOutport
#include <inviwo/core/ports/outportiterable.h>                          // for OutportIterableIm...
#include <inviwo/core/processors/processor.h>                           // for Processor
#include <inviwo/core/processors/processorinfo.h>                       // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                      // for CodeState, CodeSt...
#include <inviwo/core/processors/processortags.h>                       // for Tags
#include <inviwo/core/properties/minmaxproperty.h>                      // for IntSizeTMinMaxPro...
#include <inviwo/core/properties/optionproperty.h>                      // for OptionPropertyOption
#include <inviwo/core/util/formatdispatching.h>                         // for PrecisionValueType
#include <inviwo/core/util/formats.h>                                   // for DataFormat, DataF...
#include <inviwo/core/util/glmconvert.h>                                // for glm_convert
#include <inviwo/core/util/glmutils.h>                                  // for Matrix
#include <inviwo/core/util/glmvec.h>                                    // for size3_t, dvec3
#include <inviwo/core/util/indexmapper.h>                               // for IndexMapper, Inde...
#include <inviwo/core/util/staticstring.h>                              // for operator+
#include <inviwo/core/util/stringconversion.h>                          // for toString
#include <inviwo/dataframe/datastructures/column.h>                     // for TemplateColumn
#include <inviwo/dataframe/datastructures/dataframe.h>                  // for DataFrame

#include <cmath>          // for sqrt
#include <cstddef>        // for size_t
#include <memory>         // for shared_ptr, uniqu...
#include <optional>       // for optional
#include <sstream>        // for basic_stringbuf<>...
#include <type_traits>    // for remove_extent_t
#include <unordered_map>  // for unordered_map
#include <unordered_set>  // for unordered_set

#include <fmt/core.h>                  // for format
#include <glm/gtc/type_precision.hpp>  // for u32
#include <glm/gtc/type_ptr.hpp>        // for value_ptr
#include <glm/mat4x4.hpp>              // for operator*, mat
#include <glm/vec3.hpp>                // for vec<>::(anonymous)
#include <glm/vec4.hpp>                // for operator*, operator+

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeToDataFrame::processorInfo_{
    "org.inviwo.VolumeToDataFrame",  // Class identifier
    "Volume To DataFrame",           // Display name
    "Data Creation",                 // Category
    CodeState::Stable,               // Code state
    "CPU, DataFrame, Volume",        // Tags
};
const ProcessorInfo& VolumeToDataFrame::getProcessorInfo() const { return processorInfo_; }

VolumeToDataFrame::VolumeToDataFrame()
    : Processor()
    , inport_{"volume"}
    , outport_{"dataframe"}

    , mode_{"mode",
            "Mode",
            {{"analytics", "Analytics", Mode::Analytics},
             {"xdir", "XDir", Mode::XDir},
             {"ydir", "YDir", Mode::YDir},
             {"zdir", "ZDir", Mode::ZDir}},
            0}
    , rangeX_{"xrange", "X Range", 0, 1, 0, 1, 1, 1}
    , rangeY_{"yrange", "Y Range", 0, 1, 0, 1, 1, 1}
    , rangeZ_{"zrange", "Z Range", 0, 1, 0, 1, 1, 1} {

    addPort(inport_);
    addPort(outport_);
    addProperty(mode_);
    addProperty(rangeX_);
    addProperty(rangeY_);
    addProperty(rangeZ_);

    inport_.onChange([this]() {
        if (inport_.hasData()) {
            const auto dim = inport_.getData()->getDimensions();
            rangeX_.setRangeMax(dim.x);
            rangeY_.setRangeMax(dim.y);
            rangeZ_.setRangeMax(dim.z);
        }
    });
}

void VolumeToDataFrame::process() {
    const auto volume = inport_.getData();
    switch (mode_.get()) {
        case Mode::Analytics: {
            const auto size = (rangeX_.getEnd() - rangeX_.getStart()) *
                              (rangeY_.getEnd() - rangeY_.getStart()) *
                              (rangeZ_.getEnd() - rangeZ_.getStart());

            auto dataFrame = std::make_shared<DataFrame>(static_cast<glm::u32>(size));
            std::vector<std::vector<float>*> channelBuffer_;
            const auto numCh = volume->getDataFormat()->getComponents();
            for (size_t c = 0; c < numCh; c++) {
                auto col = dataFrame->addColumn<float>("Channel " + toString(c + 1), size);
                channelBuffer_.push_back(
                    &col->getTypedBuffer()->getEditableRAMRepresentation()->getDataContainer());
            }
            auto& magnitudes = dataFrame->addColumn<float>("Magnitude", size)
                                   ->getTypedBuffer()
                                   ->getEditableRAMRepresentation()
                                   ->getDataContainer();
            auto& indx = dataFrame->addColumn<int>("Index X", size)
                             ->getTypedBuffer()
                             ->getEditableRAMRepresentation()
                             ->getDataContainer();
            auto& indy = dataFrame->addColumn<int>("Index Y", size)
                             ->getTypedBuffer()
                             ->getEditableRAMRepresentation()
                             ->getDataContainer();
            auto& indz = dataFrame->addColumn<int>("Index Z", size)
                             ->getTypedBuffer()
                             ->getEditableRAMRepresentation()
                             ->getDataContainer();
            auto& posx = dataFrame->addColumn<float>("Position X", size)
                             ->getTypedBuffer()
                             ->getEditableRAMRepresentation()
                             ->getDataContainer();
            auto& posy = dataFrame->addColumn<float>("Position Y", size)
                             ->getTypedBuffer()
                             ->getEditableRAMRepresentation()
                             ->getDataContainer();
            auto& posz = dataFrame->addColumn<float>("Position Z", size)
                             ->getTypedBuffer()
                             ->getEditableRAMRepresentation()
                             ->getDataContainer();

            const auto indexToModel = volume->getCoordinateTransformer().getIndexToModelMatrix();

            volume->getRepresentation<VolumeRAM>()->dispatch<void>([&](auto vr) {
                const auto im = util::IndexMapper3D(vr->getDimensions());
                using ValueType = util::PrecisionValueType<decltype(vr)>;
                const auto data = vr->getDataTyped();
                auto i = 0;
                size3_t ind;
                for (ind.z = rangeZ_.getStart(); ind.z < rangeZ_.getEnd(); ind.z++) {
                    for (ind.y = rangeY_.getStart(); ind.y < rangeY_.getEnd(); ind.y++) {
                        for (ind.x = rangeX_.getStart(); ind.x < rangeX_.getEnd(); ind.x++) {
                            const auto v = util::glm_convert<dvec4>(data[im(ind)]);
                            double m = 0.0;
                            for (size_t c = 0; c < DataFormat<ValueType>::comp; c++) {
                                (*channelBuffer_[c])[i] = static_cast<float>(v[c]);
                                m += v[c] * v[c];
                            }
                            magnitudes[i] = static_cast<float>(std::sqrt(m));

                            indx[i] = static_cast<int>(ind.x);
                            indy[i] = static_cast<int>(ind.y);
                            indz[i] = static_cast<int>(ind.z);

                            const dvec3 pos{indexToModel * dvec4{ind, 1}};

                            posx[i] = static_cast<float>(pos.x);
                            posy[i] = static_cast<float>(pos.y);
                            posz[i] = static_cast<float>(pos.z);
                            ++i;
                        }
                    }
                }
            });
            outport_.setData(dataFrame);
            break;
        }
        case Mode::XDir: {
            const auto size = rangeX_.getEnd() - rangeY_.getStart();
            auto dataFrame = std::make_shared<DataFrame>(static_cast<glm::u32>(size));

            volume->getRepresentation<VolumeRAM>()->dispatch<void>([this, dataFrame,
                                                                    size](const auto vr) {
                using ValueType = util::PrecisionValueType<decltype(vr)>;
                const auto im = util::IndexMapper3D(vr->getDimensions());
                const auto data = vr->getDataTyped();
                size3_t ind;
                for (ind.z = rangeZ_.getStart(); ind.z < rangeZ_.getEnd(); ind.z++) {
                    for (ind.y = rangeY_.getStart(); ind.y < rangeY_.getEnd(); ind.y++) {
                        const std::string name = "y:" + toString(ind.y) + " z:" + toString(ind.z);
                        auto col = dataFrame->addColumn<ValueType>(name, size);
                        auto& line = col->getTypedBuffer()
                                         ->getEditableRAMRepresentation()
                                         ->getDataContainer();
                        size_t i = 0;
                        for (ind.x = rangeX_.getStart(); ind.x < rangeX_.getEnd(); ind.x++) {
                            line[i++] = data[im(ind)];
                        }
                    }
                }
            });

            outport_.setData(dataFrame);
            break;
        }
        case Mode::YDir: {
            const auto size = rangeY_.getEnd() - rangeY_.getStart();
            auto dataFrame = std::make_shared<DataFrame>(static_cast<glm::u32>(size));

            volume->getRepresentation<VolumeRAM>()->dispatch<void>(
                [this, dataFrame, size](const auto vr) {
                    using ValueType = util::PrecisionValueType<decltype(vr)>;
                    const auto im = util::IndexMapper3D(vr->getDimensions());
                    const auto data = vr->getDataTyped();
                    size3_t ind;
                    for (ind.x = rangeX_.getStart(); ind.x < rangeX_.getEnd(); ind.x++) {
                        for (ind.z = rangeZ_.getStart(); ind.z < rangeZ_.getEnd(); ind.z++) {
                            auto col = dataFrame->addColumn<ValueType>(
                                "x:" + toString(ind.x) + " z:" + toString(ind.z), size);
                            auto& line = col->getTypedBuffer()
                                             ->getEditableRAMRepresentation()
                                             ->getDataContainer();
                            size_t i = 0;
                            for (ind.y = rangeY_.getStart(); ind.y < rangeY_.getEnd(); ind.y++) {
                                line[i++] = data[im(ind)];
                            }
                        }
                    }
                });

            outport_.setData(dataFrame);
            break;
        }
        case Mode::ZDir: {
            const auto size = rangeZ_.getEnd() - rangeZ_.getStart();
            auto dataFrame = std::make_shared<DataFrame>(static_cast<glm::u32>(size));

            volume->getRepresentation<VolumeRAM>()->dispatch<void>(
                [this, dataFrame, size](const auto vr) {
                    using ValueType = util::PrecisionValueType<decltype(vr)>;
                    const auto im = util::IndexMapper3D(vr->getDimensions());
                    const auto data = vr->getDataTyped();
                    size3_t ind;
                    for (ind.x = rangeX_.getStart(); ind.x < rangeX_.getEnd(); ind.x++) {
                        for (ind.y = rangeY_.getStart(); ind.y < rangeY_.getEnd(); ind.y++) {
                            auto col = dataFrame->addColumn<ValueType>(
                                "x:" + toString(ind.x) + " y:" + toString(ind.y), size);
                            auto& line = col->getTypedBuffer()
                                             ->getEditableRAMRepresentation()
                                             ->getDataContainer();
                            size_t i = 0;
                            for (ind.z = rangeZ_.getStart(); ind.z < rangeZ_.getEnd(); ind.z++) {
                                line[i++] = data[im(ind)];
                            }
                        }
                    }
                });

            outport_.setData(dataFrame);
            break;
        }
    }
}

}  // namespace inviwo
