/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2022 Inviwo Foundation
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

#include <modules/base/processors/pixelvalue.h>

#include <inviwo/core/datastructures/image/layer.h>                     // for Layer
#include <inviwo/core/datastructures/image/layerram.h>                  // for LayerRAM
#include <inviwo/core/datastructures/image/layerutil.h>                 // for applySwizzleMask
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/interaction/events/event.h>                       // for Event
#include <inviwo/core/interaction/events/mousebuttons.h>                // for MouseButtons, Mou...
#include <inviwo/core/interaction/events/mouseevent.h>                  // for MouseEvent
#include <inviwo/core/ports/imageport.h>                                // for ImageInport, Imag...
#include <inviwo/core/processors/processor.h>                           // for Processor
#include <inviwo/core/processors/processorinfo.h>                       // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                      // for CodeState, CodeSt...
#include <inviwo/core/processors/processortags.h>                       // for Tags, Tags::CPU
#include <inviwo/core/properties/eventproperty.h>                       // for EventProperty
#include <inviwo/core/properties/invalidationlevel.h>                   // for InvalidationLevel
#include <inviwo/core/properties/ordinalproperty.h>                     // for OrdinalProperty
#include <inviwo/core/properties/property.h>                            // for Property
#include <inviwo/core/properties/propertysemantics.h>                   // for PropertySemantics
#include <inviwo/core/properties/stringproperty.h>                      // for StringProperty
#include <inviwo/core/properties/valuewrapper.h>                        // for PropertySerializa...
#include <inviwo/core/util/formatdispatching.h>                         // for All, PrecisionVal...
#include <inviwo/core/util/glmconvert.h>                                // for glm_convert
#include <inviwo/core/util/glmutils.h>                                  // for value_type
#include <inviwo/core/util/glmvec.h>                                    // for dvec4, vec4, size2_t
#include <inviwo/core/util/indexmapper.h>                               // for IndexMapper, Inde...
#include <inviwo/core/util/stringconversion.h>                          // for toString

#include <cstddef>                                                      // for size_t
#include <functional>                                                   // for __base
#include <limits>                                                       // for numeric_limits
#include <memory>                                                       // for shared_ptr, uniqu...
#include <string>                                                       // for string
#include <string_view>                                                  // for string_view
#include <type_traits>                                                  // for remove_extent_t
#include <unordered_set>                                                // for unordered_set

#include <flags/flags.h>                                                // for any
#include <glm/gtx/io.hpp>                                               // for operator<<
#include <glm/vec2.hpp>                                                 // for operator*, operator-
#include <glm/vec4.hpp>                                                 // for vec
#include <glm/vector_relational.hpp>                                    // for any, greaterThan
#include <half/half.hpp>                                                // for operator<<

namespace inviwo {
class Image;

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo PixelValue::processorInfo_{
    "org.inviwo.PixelValue",  // Class identifier
    "Pixel Value",            // Display name
    "Information",            // Category
    CodeState::Stable,        // Code state
    Tags::CPU,                // Tags
};
const ProcessorInfo PixelValue::getProcessorInfo() const { return processorInfo_; }

PixelValue::PixelValue()
    : Processor()
    , inport_("inport_", true)
    , outport_("outport", false)
    , coordinates_("Coordinates", "Coordinates", size2_t(0),
                   size2_t(std::numeric_limits<size_t>::lowest()),
                   size2_t(std::numeric_limits<size_t>::max()), size2_t(1),
                   InvalidationLevel::Valid, PropertySemantics::Text)
    , pixelValues_(
          {{"pixelValue1", "Pixel Value", dvec4(0), dvec4(std::numeric_limits<double>::lowest()),
            dvec4(std::numeric_limits<double>::max()),
            dvec4(std::numeric_limits<double>::epsilon()), InvalidationLevel::Valid,
            PropertySemantics::Text},
           {"pixelValue2", "Pixel Value (Layer 2)", dvec4(0),
            dvec4(std::numeric_limits<double>::lowest()), dvec4(std::numeric_limits<double>::max()),
            dvec4(std::numeric_limits<double>::epsilon()), InvalidationLevel::Valid,
            PropertySemantics::Text},
           {"pixelValue3", "Pixel Value (Layer 3)", dvec4(0),
            dvec4(std::numeric_limits<double>::lowest()), dvec4(std::numeric_limits<double>::max()),
            dvec4(std::numeric_limits<double>::epsilon()), InvalidationLevel::Valid,
            PropertySemantics::Text},
           {"pixelValue4", "Pixel Value (Layer 4)", dvec4(0),
            dvec4(std::numeric_limits<double>::lowest()), dvec4(std::numeric_limits<double>::max()),
            dvec4(std::numeric_limits<double>::epsilon()), InvalidationLevel::Valid,
            PropertySemantics::Text},
           {"pixelValue5", "Pixel Value (Layer 5)", dvec4(0),
            dvec4(std::numeric_limits<double>::lowest()), dvec4(std::numeric_limits<double>::max()),
            dvec4(std::numeric_limits<double>::epsilon()), InvalidationLevel::Valid,
            PropertySemantics::Text},
           {"pixelValue6", "Pixel Value (Layer 6)", dvec4(0),
            dvec4(std::numeric_limits<double>::lowest()), dvec4(std::numeric_limits<double>::max()),
            dvec4(std::numeric_limits<double>::epsilon()), InvalidationLevel::Valid,
            PropertySemantics::Text},
           {"pixelValue7", "Pixel Value (Layer 7)", dvec4(0),
            dvec4(std::numeric_limits<double>::lowest()), dvec4(std::numeric_limits<double>::max()),
            dvec4(std::numeric_limits<double>::epsilon()), InvalidationLevel::Valid,
            PropertySemantics::Text},
           {"pixelValue8", "Pixel Value (Layer 8)", dvec4(0),
            dvec4(std::numeric_limits<double>::lowest()), dvec4(std::numeric_limits<double>::max()),
            dvec4(std::numeric_limits<double>::epsilon()), InvalidationLevel::Valid,
            PropertySemantics::Text}

          })
    , pixelValuesNormalized_({
          {"pixelValue1Normalized", "Normalized Pixel Value", vec4(0), vec4(0), vec4(1),
           vec4(std::numeric_limits<float>::epsilon()), InvalidationLevel::Valid,
           PropertySemantics::Color},
          {"pixelValue2Normalized", "Normalized Pixel Value (Layer 2)", vec4(0), vec4(0), vec4(1),
           vec4(std::numeric_limits<float>::epsilon()), InvalidationLevel::Valid,
           PropertySemantics::Color},
          {"pixelValue3Normalized", "Normalized Pixel Value (Layer 3)", vec4(0), vec4(0), vec4(1),
           vec4(std::numeric_limits<float>::epsilon()), InvalidationLevel::Valid,
           PropertySemantics::Color},
          {"pixelValue4Normalized", "Normalized Pixel Value (Layer 4)", vec4(0), vec4(0), vec4(1),
           vec4(std::numeric_limits<float>::epsilon()), InvalidationLevel::Valid,
           PropertySemantics::Color},
          {"pixelValue5Normalized", "Normalized Pixel Value (Layer 5)", vec4(0), vec4(0), vec4(1),
           vec4(std::numeric_limits<float>::epsilon()), InvalidationLevel::Valid,
           PropertySemantics::Color},
          {"pixelValue6Normalized", "Normalized Pixel Value (Layer 6)", vec4(0), vec4(0), vec4(1),
           vec4(std::numeric_limits<float>::epsilon()), InvalidationLevel::Valid,
           PropertySemantics::Color},
          {"pixelValue7Normalized", "Normalized Pixel Value (Layer 7)", vec4(0), vec4(0), vec4(1),
           vec4(std::numeric_limits<float>::epsilon()), InvalidationLevel::Valid,
           PropertySemantics::Color},
          {"pixelValue8Normalized", "Normalized Pixel Value (Layer 8)", vec4(0), vec4(0), vec4(1),
           vec4(std::numeric_limits<float>::epsilon()), InvalidationLevel::Valid,
           PropertySemantics::Color},

      })

    , pickingValue_(
          "pickingValue", "Picking Value", dvec4(0), dvec4(std::numeric_limits<double>::lowest()),
          dvec4(std::numeric_limits<double>::max()), dvec4(std::numeric_limits<double>::epsilon()),
          InvalidationLevel::Valid, PropertySemantics::Text)
    , depthValue_("depthValue_", "Depth Value", 0.0, std::numeric_limits<double>::lowest(),
                  std::numeric_limits<double>::max(), std::numeric_limits<double>::epsilon(),
                  InvalidationLevel::Valid, PropertySemantics::Text)
    , pixelStrValues_({
          {"pixelStrValue", "Pixel Value (as string)", "", InvalidationLevel::Valid},
          {"pixelStrValue2", "Pixel Value (as string, layer 2)", "", InvalidationLevel::Valid},
          {"pixelStrValue3", "Pixel Value (as string, layer 3)", "", InvalidationLevel::Valid},
          {"pixelStrValue4", "Pixel Value (as string, layer 4)", "", InvalidationLevel::Valid},
          {"pixelStrValue5", "Pixel Value (as string, layer 5)", "", InvalidationLevel::Valid},
          {"pixelStrValue6", "Pixel Value (as string, layer 6)", "", InvalidationLevel::Valid},
          {"pixelStrValue7", "Pixel Value (as string, layer 7)", "", InvalidationLevel::Valid},
          {"pixelStrValue8", "Pixel Value (as string, layer 8)", "", InvalidationLevel::Valid},
      })

    , pickingStrValue_("pickingStrValue", "Picking Value (as string)", "", InvalidationLevel::Valid)
    , depthStrValue_("depthStrValue", "Depth Value (as string)", "", InvalidationLevel::Valid)

    , mouseMove_(
          "mouseMove", "Mouse Move", [this](Event* e) { mouseMoveEvent(e); },
          MouseButtons(flags::any), MouseState::Move)

{
    addPort(inport_);
    addPort(outport_);

    for (int i = 0; i < 8; i++) {
        pixelValues_[i].setVisible(i == 0);
        pixelStrValues_[i].setVisible(i == 0);
        pixelValuesNormalized_[i].setVisible(i == 0);
        addProperty(pixelValues_[i]);
        addProperty(pixelStrValues_[i]);
        addProperty(pixelValuesNormalized_[i]);
    }

    addProperty(pickingValue_);
    addProperty(pickingStrValue_);
    addProperty(depthValue_);
    addProperty(depthStrValue_);
    addProperty(coordinates_);
    addProperty(mouseMove_);

    inport_.onChange([&]() {
        size_t numCh = 1;
        if (inport_.hasData()) {
            numCh = inport_.getData()->getNumberOfColorLayers();
        }
        for (size_t i = 0; i < 8; i++) {
            pixelValues_[i].setVisible(i < numCh);
            pixelStrValues_[i].setVisible(i < numCh);
            pixelValuesNormalized_[i].setVisible(i < numCh);
        }
    });

    for (auto& p : getProperties()) {
        p->setSerializationMode(PropertySerializationMode::None);
    }
}

void PixelValue::process() { outport_.setData(inport_.getData()); }

void PixelValue::mouseMoveEvent(Event* theevent) {
    if (!inport_.hasData()) return;

    if (auto mouseEvent = theevent->getAs<MouseEvent>()) {
        auto img = inport_.getData();
        auto dims = img->getDimensions();
        auto numCh = img->getNumberOfColorLayers();
        auto p = mouseEvent->posNormalized();
        if (glm::any(glm::lessThan(p, dvec2(0, 0))) || glm::any(glm::greaterThan(p, dvec2(1, 1)))) {
            // This can happen a lot when having a image layout
            return;
        }
        const size2_t pos{p * dvec2(dims - size2_t(1))};
        coordinates_.set(pos);
        for (size_t i = 0; i < numCh; i++) {
            img->getColorLayer(i)
                ->getRepresentation<LayerRAM>()
                ->dispatch<void, dispatching::filter::All>([&](const auto layer) {
                    using ValueType = util::PrecisionValueType<decltype(layer)>;
                    using Comp = typename util::value_type<ValueType>::type;
                    const auto data = layer->getDataTyped();
                    const auto im = util::IndexMapper2D(dims);

                    auto value = data[im(pos)];
                    auto v = util::glm_convert<glm::vec<4, Comp>>(value);
                    v = util::applySwizzleMask(v, img->getColorLayer(i)->getSwizzleMask());

                    if constexpr (std::is_same_v<Comp, unsigned char> ||
                                  std::is_same_v<Comp, char>) {
                        pixelStrValues_[i].set(toString(ivec4{v}));
                    } else {
                        pixelStrValues_[i].set(toString(v));
                    }

                    auto vf = util::glm_convert<glm::vec<4, float>>(v);
                    if constexpr (std::is_integral_v<Comp>) {
                        vf /= std::numeric_limits<Comp>::max();
                    }
                    pixelValuesNormalized_[i].set(vf);
                });
        }

        auto pickV = img->getPickingLayer()->getRepresentation<LayerRAM>()->getAsDVec4(pos);
        auto depthV = img->getDepthLayer()->getRepresentation<LayerRAM>()->getAsDouble(pos);

        pickingValue_.set(pickV);
        depthValue_.set(depthV);

        pickingStrValue_.set(toString(pickV));
        depthStrValue_.set(toString(depthV));
    }
}

}  // namespace inviwo
