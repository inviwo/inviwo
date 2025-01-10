/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2025 Inviwo Foundation
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

#include <modules/base/processors/heightfieldmapper.h>

#include <inviwo/core/algorithm/markdown.h>                             // for operator""_help
#include <inviwo/core/datastructures/image/image.h>                     // for Image
#include <inviwo/core/datastructures/image/layer.h>                     // for Layer
#include <inviwo/core/datastructures/image/layerram.h>                  // for LayerRAM
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/ports/imageport.h>                                // for ImageInport, Imag...
#include <inviwo/core/processors/processor.h>                           // for Processor
#include <inviwo/core/processors/processorinfo.h>                       // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                      // for CodeState, CodeSt...
#include <inviwo/core/processors/processortags.h>                       // for Tags, Tags::CPU
#include <inviwo/core/properties/constraintbehavior.h>                  // for ConstraintBehavior
#include <inviwo/core/properties/minmaxproperty.h>                      // for FloatMinMaxProperty
#include <inviwo/core/properties/optionproperty.h>                      // for OptionPropertyOption
#include <inviwo/core/properties/ordinalproperty.h>                     // for FloatProperty
#include <inviwo/core/util/formats.h>                                   // for DataFormatBase
#include <inviwo/core/util/glmvec.h>                                    // for size2_t, dvec2
#include <inviwo/core/util/logcentral.h>                                // for LogCentral

#include <algorithm>      // for copy, max, max_el...
#include <cmath>          // for abs
#include <cstdlib>        // for size_t, abs
#include <functional>     // for __base
#include <memory>         // for shared_ptr, uniqu...
#include <string>         // for string
#include <string_view>    // for string_view
#include <type_traits>    // for remove_extent_t
#include <unordered_map>  // for unordered_map
#include <unordered_set>  // for unordered_set

#include <glm/ext/vector_float2.hpp>  // for vec2
#include <glm/ext/vector_uint2.hpp>   // for uvec2
#include <glm/vec2.hpp>               // for vec<>::(anonymous)
#include <glm/vec3.hpp>               // for vec<>::(anonymous)
#include <glm/vec4.hpp>               // for vec<>::(anonymous)

namespace inviwo {

const ProcessorInfo HeightFieldMapper::processorInfo_{
    "org.inviwo.HeightFieldMapper",  // Class identifier
    "Height Field Mapper",           // Display name
    "Heightfield",                   // Category
    CodeState::Experimental,         // Code state
    Tags::CPU,                       // Tags
    R"(Maps a heightfield onto a geometry and renders it to an image.
    The Height Field Mapper converts an arbitrary 2D input image to a grayscale float
    image to be used in the Height Field Renderer processor. Additionally, data values
    are mapped to either an user-defined range or are scaled to fit in a given maximum
    height based on the sea level.)"_unindentHelp};
const ProcessorInfo& HeightFieldMapper::getProcessorInfo() const { return processorInfo_; }

HeightFieldMapper::HeightFieldMapper()
    : Processor()
    , inport_("image",
              "The heightfield input. If the image has multiple channels "
              "only the red channel is used."_help,
              OutportDeterminesSize::Yes)
    , outport_("heightfield", "The scaled height field (single channel)."_help, DataFloat32::get())
    , scalingModeProp_("scalingmode", "Scaling Mode",
                       R"(The heightfield is scaled to either a fixed range (0 to 1),
                          to a user-specified range (__Height Range__), or based on
                          the sea level (__Sea Level__ and __Maximum Height__).)"_unindentHelp,
                       {{"scaleFixed", "Fixed Range [0:1]", HeightFieldScaling::FixedRange},
                        {"scaleRange", "Data Range", HeightFieldScaling::DataRange},
                        {"scaleSeaLevel", "Sea Level", HeightFieldScaling::SeaLevel}},
                       0)
    , heightRange_("heightrange", "Height Range", "Min/max range for data range scaling."_help,
                   0.0f, 1.0f, -1.0e1f, 1.0e1f)
    , maxHeight_("maxheight", "Maximum Height", "Max height used in sea level scaling."_help, 1.0f,
                 {0.0f, ConstraintBehavior::Immutable}, {1.0e1f, ConstraintBehavior::Ignore})
    , seaLevel_("sealevel", "Sea Level", "Sea level around which the heightfield is scaled."_help,
                0.0f, {-1.0e1f, ConstraintBehavior::Ignore}, {1.0e1f, ConstraintBehavior::Ignore}) {
    addPort(inport_);
    addPort(outport_);

    heightRange_.visibilityDependsOn(scalingModeProp_, [](const OptionPropertyInt& opt) {
        return opt == HeightFieldScaling::DataRange;
    });
    maxHeight_.visibilityDependsOn(scalingModeProp_, [](const OptionPropertyInt& opt) {
        return opt == HeightFieldScaling::SeaLevel;
    });
    seaLevel_.visibilityDependsOn(scalingModeProp_, [](const OptionPropertyInt& opt) {
        return opt == HeightFieldScaling::SeaLevel;
    });

    addProperties(scalingModeProp_, heightRange_, maxHeight_, seaLevel_);
    setAllPropertiesCurrentStateAsDefault();
}

HeightFieldMapper::~HeightFieldMapper() {}

void HeightFieldMapper::process() {
    if (!inport_.isReady()) return;

    auto srcImg = inport_.getData();
    if (!srcImg) {
        log::warn("No valid input image given");
        return;
    }

    // check the number of channels
    const DataFormatBase* format = srcImg->getDataFormat();
    std::size_t numInputChannels = format->getComponents();
    size2_t dim = srcImg->getDimensions();

    Image* outImg = nullptr;

    // check format of output image
    if (!outImg || (outImg->getDataFormat()->getId() != DataFormatId::Float32)) {
        // replace with new floating point image
        Image* img = new Image(dim, DataFloat32::get());
        outport_.setData(img);
        outImg = img;
    } else if (outImg->getDimensions() != dim) {
        // adjust dimensions of output image
        outImg->setDimensions(dim);
    }

    LayerRAM* dstLayer = outImg->getColorLayer(0)->getEditableRepresentation<LayerRAM>();
    float* data = static_cast<float*>(dstLayer->getData());

    // convert input image to float image
    const LayerRAM* srcLayer = srcImg->getColorLayer(0)->getRepresentation<LayerRAM>();

    // special case: input image is already FLOAT32 with 1 channel
    if ((numInputChannels == 1) && (format->getId() == DataFormatId::Float32)) {
        const float* srcData = static_cast<const float*>(srcLayer->getData());
        std::copy(srcData, srcData + dim.x * dim.y, data);
    } else {
        switch (numInputChannels) {
            case 2:
                for (unsigned int y = 0; y < dim.y; ++y) {
                    for (unsigned int x = 0; x < dim.x; ++x) {
                        data[y * dim.x + x] =
                            static_cast<float>(srcLayer->getAsNormalizedDVec2(glm::uvec2(x, y)).r);
                    }
                }
                break;
            case 3:
                for (unsigned int y = 0; y < dim.y; ++y) {
                    for (unsigned int x = 0; x < dim.x; ++x) {
                        data[y * dim.x + x] =
                            static_cast<float>(srcLayer->getAsNormalizedDVec3(glm::uvec2(x, y)).r);
                    }
                }
                break;
            case 4:
                for (unsigned int y = 0; y < dim.y; ++y) {
                    for (unsigned int x = 0; x < dim.x; ++x) {
                        data[y * dim.x + x] =
                            static_cast<float>(srcLayer->getAsNormalizedDVec4(glm::uvec2(x, y)).r);
                    }
                }
                break;
            case 1:
            default:
                for (unsigned int y = 0; y < dim.y; ++y) {
                    for (unsigned int x = 0; x < dim.x; ++x) {
                        data[y * dim.x + x] =
                            static_cast<float>(srcLayer->getAsNormalizedDouble(glm::uvec2(x, y)));
                    }
                }
                break;
        }
    }

    // rescale data set
    std::size_t numValues = dim.x * dim.y;
    // determine min/max values
    float minVal = *std::min_element(data, data + numValues);
    float maxVal = *std::max_element(data, data + numValues);

    switch (scalingModeProp_.get()) {
        case HeightFieldScaling::SeaLevel: {
            // scale heightfield based on sea level and min/max height
            float sealevel = seaLevel_.get();
            float aboveSea = maxVal - sealevel;
            float belowSea = minVal - sealevel;

            float factor = maxHeight_.get() / std::max(aboveSea, std::abs(belowSea));
            for (std::size_t i = 0; i < numValues; ++i) {
                data[i] = (data[i] - sealevel) * factor;
            }
        } break;
        case HeightFieldScaling::DataRange: {
            // scale data to [heightRange_.min : heightRange_.max]
            glm::vec2 range(heightRange_.get());

            float factor = (range.y - range.x) / (maxVal - minVal);
            for (std::size_t i = 0; i < numValues; ++i) {
                data[i] = (data[i] - minVal) * factor + range.x;
            }
        } break;
        case HeightFieldScaling::FixedRange:
        default: {
            // scale data to [0:1] range
            float delta = 1.0f / (maxVal - minVal);
            for (std::size_t i = 0; i < numValues; ++i) {
                data[i] = (data[i] - minVal) * delta;
            }
        } break;
    }
}

}  // namespace inviwo
