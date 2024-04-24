/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#include <modules/base/processors/transferfunction2dmesh.h>
#include <inviwo/core/algorithm/markdown.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/interaction/events/pickingevent.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/touchevent.h>
#include <inviwo/core/interaction/events/touchstate.h>
#include <inviwo/core/interaction/events/wheelevent.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TransferFunction2DMesh::processorInfo_{
    "org.inviwo.TransferFunction2DMesh",           // Class identifier
    "Transfer Function2D Mesh",                    // Display name
    "Volume Operation",                            // Category
    CodeState::Experimental,                       // Code state
    Tags::CPU | Tag{"TF"} | Tag{"Multispectral"},  // Tags
    R"(Processor for generating basic point-based 2D transfer functions.)"_unindentHelp,
};

const ProcessorInfo& TransferFunction2DMesh::getProcessorInfo() const { return processorInfo_; }

constexpr std::string_view tfHelp =
    R"(Text-based description of point-based TF primitives. Each primitive has
the form of

```
<x>, <y> #rrggbbaa <radius>
```
where `x` and `y` refer to a normalized position [0,1].
)";

namespace detail {

std::unique_ptr<BoolCompositeProperty> propertyTemplate() {
    auto prop = std::make_unique<BoolCompositeProperty>("tfPrimitive", "TF Point", false);
    prop->setChecked(true);

    auto pos = std::make_unique<FloatVec2Property>(
        "position", "Position",
        OrdinalPropertyState<vec2>{.value = vec2{0.5f},
                                   .min = vec2{0.0f},
                                   .minConstraint = ConstraintBehavior::Immutable,
                                   .max = vec2{1.0f},
                                   .maxConstraint = ConstraintBehavior::Immutable});
    prop->addProperty(std::move(pos));
    prop->addProperty(
        std::make_unique<FloatVec4Property>("color", "Color", util::ordinalColor(vec4{1.0f})));
    prop->addProperty(std::make_unique<FloatProperty>(
        "radius", "Radius", util::ordinalLength(0.1f, 1.0f).set(PropertySemantics::Default)));
    return prop;
}

}  // namespace detail

TransferFunction2DMesh::TransferFunction2DMesh()
    : Processor{}
    , outport_{"outport", "Mesh representing 2D transfer function primitives."_help}
    , primitives_{"primitives", "TF Primitives", detail::propertyTemplate()}
    , tfPrimitivesText_{"tfPrimitives",
                        "TF Primitives",
                        util::unindentMd2doc(tfHelp),
                        "",
                        InvalidationLevel::InvalidOutput,
                        PropertySemantics::Multiline}
    , enablePicking_{"enablePicking", "Enable Picking", true}
    , tfPicking_{this, 1, [&](PickingEvent* p) { handlePicking(p); }} {

    addPorts(outport_);
    addProperties(primitives_, enablePicking_);
}

void TransferFunction2DMesh::process() {
    const size_t numPrimitives = primitives_.size();

    std::vector<vec2> positions;
    std::vector<vec4> colors;
    std::vector<float> radii;
    std::vector<std::uint32_t> pickingIDs;

    std::uint32_t pickingID = static_cast<std::uint32_t>(tfPicking_.getPickingId(0));
    for (const auto* p : primitives_) {
        const auto* compositeProp = dynamic_cast<const BoolCompositeProperty*>(p);
        if (compositeProp && compositeProp->isChecked()) {
            if (const auto* pos = dynamic_cast<const FloatVec2Property*>(
                    compositeProp->getPropertyByIdentifier("position"))) {
                positions.push_back(pos->get());
            }
            if (const auto* color = dynamic_cast<const FloatVec4Property*>(
                    compositeProp->getPropertyByIdentifier("color"))) {
                colors.push_back(color->get());
            }
            if (const auto* radius = dynamic_cast<const FloatProperty*>(
                    compositeProp->getPropertyByIdentifier("radius"))) {
                radii.push_back(radius->get());
            }
            pickingIDs.push_back(pickingID);
        }
        ++pickingID;
    }

    auto mesh = std::make_shared<Mesh>(DrawType::Points, ConnectivityType::None);
    mesh->addBuffer(Mesh::BufferInfo(BufferType::PositionAttrib),
                    util::makeBuffer(std::move(positions)));
    mesh->addBuffer(Mesh::BufferInfo(BufferType::ColorAttrib), util::makeBuffer(std::move(colors)));
    mesh->addBuffer(Mesh::BufferInfo(BufferType::RadiiAttrib), util::makeBuffer(std::move(radii)));
    mesh->addBuffer(Mesh::BufferInfo(BufferType::PickingAttrib),
                    util::makeBuffer(std::move(pickingIDs)));

    const mat4 modelTrafo =
        glm::scale(vec3{2.0f, 2.0f, 1.0f}) * glm::translate(vec3{-0.5f, -0.5f, 0.0f});
    mesh->setModelMatrix(modelTrafo);

    tfPicking_.resize(numPrimitives);

    outport_.setData(mesh);
}

void TransferFunction2DMesh::handlePicking(PickingEvent* p) {
    if (p->getPickedId() >= primitives_.size()) {
        return;
    }
    if (enablePicking_) {

        if ((p->getState() & PickingState::Updated) &&
            p->getEvent()->hash() == MouseEvent::chash()) {
            auto me = p->getEventAs<MouseEvent>();
            if ((me->buttonState() & MouseButton::Left) && me->state() == MouseState::Move) {
                const auto delta = vec2{p->getNDC() - p->getPreviousNDC()} * 0.5f;

                if (auto* compositeProp = dynamic_cast<CompositeProperty*>(
                        primitives_.getProperties()[p->getPickedId()])) {

                    if (auto* pos = dynamic_cast<FloatVec2Property*>(
                            compositeProp->getPropertyByIdentifier("position"))) {
                        pos->set(pos->get() + delta);
                    }
                }

                invalidate(InvalidationLevel::InvalidOutput);
                p->markAsUsed();
            }
        } else if (p->getState() == PickingState::Updated &&
                   p->getEvent()->hash() == WheelEvent::chash()) {
            auto we = p->getEventAs<WheelEvent>();

            if (auto* compositeProp = dynamic_cast<CompositeProperty*>(
                    primitives_.getProperties()[p->getPickedId()])) {

                if (auto* radius = dynamic_cast<FloatProperty*>(
                        compositeProp->getPropertyByIdentifier("radius"))) {
                    const float wheelScaling = 0.025f;

                    const float r =
                        radius->get() + static_cast<float>(-we->delta().y * wheelScaling);
                    radius->set(std::max(r, 0.0001f));
                }
            }

            invalidate(InvalidationLevel::InvalidOutput);
            p->markAsUsed();
        }
    }
}

}  // namespace inviwo
