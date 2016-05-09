/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/interaction/events/mouseevent.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo PixelValue::processorInfo_{
    "org.inviwo.PixelValue",  // Class identifier
    "Pixel Value",            // Display name
    "Undefined",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
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

    , mouseMove_("mouseMove", "Mouse Move",
                 new MouseEvent(MouseEvent::MOUSE_BUTTON_ANY_AND_NONE,
                                InteractionEvent::MODIFIER_NONE, MouseEvent::MOUSE_STATE_MOVE),
                 new Action(this, &PixelValue::mouseMoveEvent))

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
}

void PixelValue::process() { outport_.setData(inport_.getData()); }

void PixelValue::mouseMoveEvent(Event* theevent) {
    if (auto mouseEvent = dynamic_cast<MouseEvent*>(theevent)) {
        auto img = inport_.getData();
        auto dims = img->getDimensions();
        auto numCh = img->getNumberOfColorLayers();
        auto p = mouseEvent->posNormalized();
        if (glm::any(glm::lessThan(p, vec2(0, 0))) || glm::any(glm::greaterThan(p, vec2(1, 1)))) {
            LogError("How often can this happen");
            return;
        }
        p.y = 1 - p.y;
        size2_t pos = static_cast<size2_t>(p * vec2(dims));
        coordinates_.set(pos);
        for (size_t i = 0; i < numCh; i++) {
            auto v = img->getColorLayer(i)->getRepresentation<LayerRAM>()->getAsDVec4(pos);
            pixelValues_[i].set(v);
            vec4 vf = static_cast<vec4>(v);
            auto df = img->getDataFormat();
            if (df->getNumericType() == NumericType::UnsignedInteger ||
                df->getNumericType() == NumericType::SignedInteger) {
                vf /= df->getMax();
            }
            pixelValuesNormalized_[i].set(vf);
            pixelStrValues_[i].set(toString(v));
        }

        auto pickV = img->getPickingLayer()->getRepresentation<LayerRAM>()->getAsDVec4(pos);
        auto depthV = img->getDepthLayer()->getRepresentation<LayerRAM>()->getAsDouble(pos);

        pickingValue_.set(pickV);
        depthValue_.set(depthV);

        pickingStrValue_.set(toString(pickV));
        depthStrValue_.set(toString(depthV));
    }
}

}  // namespace
