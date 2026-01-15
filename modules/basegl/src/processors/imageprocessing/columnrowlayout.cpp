/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2026 Inviwo Foundation
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

#include <modules/basegl/processors/imageprocessing/columnrowlayout.h>

#include <inviwo/core/datastructures/image/imagetypes.h>     // for ImageType, ImageType::ColorD...
#include <inviwo/core/interaction/events/event.h>            // for Event
#include <inviwo/core/interaction/events/resizeevent.h>      // for ResizeEvent
#include <inviwo/core/network/networklock.h>                 // for NetworkLock
#include <inviwo/core/ports/imageport.h>                     // for ImageMultiInport, ImageOutport
#include <inviwo/core/ports/outport.h>                       // for Outport
#include <inviwo/core/processors/processor.h>                // for Processor
#include <inviwo/core/processors/processorinfo.h>            // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>           // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>            // for Tags, Tag, Tags::GL
#include <inviwo/core/properties/compositeproperty.h>        // for CompositeProperty
#include <inviwo/core/properties/constraintbehavior.h>       // for ConstraintBehavior, Constrai...
#include <inviwo/core/properties/invalidationlevel.h>        // for InvalidationLevel, Invalidat...
#include <inviwo/core/properties/ordinalproperty.h>          // for FloatProperty, IntProperty
#include <inviwo/core/properties/property.h>                 // for Property
#include <inviwo/core/properties/propertyowner.h>            // for PropertyOwner
#include <inviwo/core/properties/propertysemantics.h>        // for PropertySemantics, PropertyS...
#include <inviwo/core/util/assertion.h>                      // for IVW_ASSERT
#include <inviwo/core/util/glmvec.h>                         // for ivec2, size2_t, ivec4, vec4
#include <inviwo/core/util/stdextensions.h>                  // for all_of
#include <modules/basegl/datastructures/splittersettings.h>  // for Direction, Direction::Horizo...
#include <modules/basegl/properties/splitterproperty.h>      // for SplitterProperty
#include <modules/basegl/rendering/splitterrenderer.h>       // for SplitterRenderer
#include <modules/basegl/viewmanager.h>                      // for ViewManager, ViewManager::View
#include <modules/opengl/inviwoopengl.h>                     // for glViewport
#include <modules/opengl/shader/shader.h>                    // for Shader
#include <modules/opengl/texture/textureunit.h>              // for TextureUnit
#include <modules/opengl/texture/textureutils.h>             // for activateAndClearTarget, bind...

#include <algorithm>    // for max, find, min
#include <cstddef>      // for size_t, ptrdiff_t
#include <functional>   // for __base
#include <iterator>     // for distance
#include <memory>       // for shared_ptr
#include <string>       // for string
#include <string_view>  // for string_view
#include <type_traits>  // for remove_extent_t
#include <vector>       // for vector
#include <ranges>

#include <fmt/core.h>                 // for format
#include <glm/vec2.hpp>               // for vec<>::(anonymous)
#include <glm/vector_relational.hpp>  // for any, lessThanEqual, equal

namespace inviwo {
class Deserializer;

namespace layout {

MultiInput::MultiInput(const std::function<void(bool)>& update) : inport("inport") {
    inport.setIsReadyUpdater([this]() {
        // Ports with dimensions (1,1) or less are considered to be inactive
        return inport.isConnected() && util::all_of(inport.getConnectedOutports(), [](Outport* p) {
                   // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
                   auto* ip = static_cast<ImageOutport*>(p);
                   return (ip->hasData() &&
                           glm::any(glm::lessThanEqual(ip->getDimensions(), size2_t(1))))
                              ? true
                              : p->isReady();
               });
    });

    inport.onConnect([update]() { update(true); });
    inport.onDisconnect([update]() { update(false); });
}

void MultiInput::addPorts(Processor* p) { p->addPort(inport); }
void MultiInput::removePorts(Processor* p) { p->removePort(&inport); }

size_t MultiInput::size() const { return std::distance(inport.begin(), inport.end()); }

const std::vector<std::shared_ptr<const Image>>& MultiInput::getData() {
    data.clear();
    data.insert(data.end(), inport.begin(), inport.end());
    return data;
}

void MultiInput::propagateSizes(ViewManager& vm) {
    const auto& outports = inport.getConnectedOutports();
    for (auto&& [view, outport] : std::views::zip(vm.getViews(), outports)) {
        if (!view.empty()) {
            ResizeEvent e{view.size};
            inport.propagateEvent(&e, outport);
        }
    }
}

void MultiInput::propagateEvent(Event* event, size_t index) {
    const auto& outports = inport.getConnectedOutports();
    if (index < outports.size()) {
        inport.propagateEvent(event, outports[index]);
    }
}

void MultiInput::propagateEvent(Event* event, Processor* p, Outport* source) {
    if (event->shouldPropagateTo(&inport, p, source)) {
        inport.propagateEvent(event);
    }
}

size_t MultiInput::indexOf(Outport* to) const {
    const auto& ports = inport.getConnectedOutports();
    auto portIt = std::ranges::find(ports, to);
    return static_cast<size_t>(std::distance(ports.begin(), portIt));
}

SequenceInput::SequenceInput(const std::function<void(bool)>& update) : inport("inport") {
    inport.setIsReadyUpdater([this]() {
        return (inport.isConnected() && util::all_of(inport.getConnectedOutports(),
                                                     [](Outport* p) { return p->isReady(); }));
    });

    inport.onChange([update]() { update(true); });
    inport.onConnect([update]() { update(true); });
    inport.onDisconnect([update]() { update(false); });
}

void SequenceInput::addPorts(Processor* p) { p->addPort(inport); }
void SequenceInput::removePorts(Processor* p) { p->removePort(&inport); }

size_t SequenceInput::size() const {
    if (auto input = inport.getData()) {
        return input->size();
    }
    return 0;
}

const std::vector<std::shared_ptr<const Image>>& SequenceInput::getData() {
    data.clear();
    if (auto seq = inport.getData()) {
        data.insert(data.end(), seq->begin(), seq->end());
    }
    return data;
}

void SequenceInput::propagateSizes(ViewManager& vm) {
    const auto max = std::ranges::fold_left(
        vm.getViews() | std::views::transform([](const auto& v) { return v.size; }), ivec2{1, 1},
        [](const ivec2& a, const ivec2& b) { return glm::max(a, b); });

    ResizeEvent e{max};
    inport.propagateEvent(&e, inport.getConnectedOutport());
}

void SequenceInput::propagateEvent(Event* event, size_t index) {
    if (index == 0) {
        inport.propagateEvent(event, inport.getConnectedOutport());
    }
}

void SequenceInput::propagateEvent(Event* event, Processor* p, Outport* source) {
    if (event->shouldPropagateTo(&inport, p, source)) {
        inport.propagateEvent(event);
    }
}

size_t SequenceInput::indexOf(Outport*) { return 0; }

Input::Input(const std::function<void(bool)>& update)
    : input_{std::in_place_type_t<MultiInput>{}, update} {}

void Input::addPorts(Processor* p) {
    std::visit([&](auto& i) { i.addPorts(p); }, input_);
}
void Input::removePorts(Processor* p) {
    std::visit([&](auto& i) { i.removePorts(p); }, input_);
}

size_t Input::size() const {
    return std::visit([](auto& i) { return i.size(); }, input_);
}

const std::vector<std::shared_ptr<const Image>>& Input::getData() {
    return std::visit([&](auto& i) -> decltype(auto) { return i.getData(); }, input_);
}

void Input::propagateSizes(ViewManager& vm) {
    std::visit([&](auto& i) { i.propagateSizes(vm); }, input_);
}

void Input::propagateEvent(Event* event, size_t index) {
    std::visit([&](auto& i) { i.propagateEvent(event, index); }, input_);
}

void Input::propagateEvent(Event* event, Processor* p, Outport* source) {
    std::visit([&](auto& i) { i.propagateEvent(event, p, source); }, input_);
}

size_t Input::indexOf(Outport* to) const {
    return std::visit([&](auto& i) { return i.indexOf(to); }, input_);
}

void Input::setMode(Processor* p, InputMode mode, const std::function<void(bool)>& update) {
    if (input_.index() == static_cast<size_t>(mode)) return;

    removePorts(p);
    switch (mode) {
        case InputMode::Multi:
            input_.emplace<MultiInput>(update);
            break;
        case InputMode::Sequence:
            input_.emplace<SequenceInput>(update);
            break;
    }
    addPorts(p);
}

SplitterPositions::SplitterPositions(std::string_view identifier, std::string_view displayName,
                                     std::function<float()> minSpacing)
    : splitters_{identifier, displayName}
    , nSplitters_{0}
    , minSpacing_{std::move(minSpacing)}
    , isEnforcing_{false} {}

void SplitterPositions::enforceOrder(size_t fixedSliderIndex) {
    if (isEnforcing_) return;
    const util::KeepTrueWhileInScope enforce{&isEnforcing_};
    const NetworkLock lock(&splitters_);
    const auto minSpacing = minSpacing_();

    // push everything to the left if necessary
    float sliderPos = position(fixedSliderIndex);
    for (auto i : std::views::iota(0uz, fixedSliderIndex) | std::views::reverse) {
        if (position(i) + minSpacing >= sliderPos) {
            set(i, sliderPos - minSpacing);
        }
        sliderPos = position(i);
    }

    // push everything to the right if necessary
    sliderPos = position(fixedSliderIndex);
    for (auto i : std::views::iota(fixedSliderIndex + 1, size())) {
        if (sliderPos >= position(i) - minSpacing) {
            set(i, sliderPos + minSpacing);
        }
        sliderPos = position(i);
    }
}

bool SplitterPositions::updateSize(size_t newSize) {
    const util::KeepTrueWhileInScope enforce{&isEnforcing_};
    const NetworkLock lock(&splitters_);

    if (nSplitters_ == newSize) return false;

    for (size_t i = 0; i < newSize; ++i) {
        if (splitters_.size() <= i) {
            auto* prop = new FloatProperty{
                fmt::format("splitter{}", i), fmt::format("Splitter {}", i + 1),
                OrdinalPropertyState<float>{
                    .value = static_cast<float>(i + 1) / static_cast<float>(newSize + 1),
                    .min = 0.0f,
                    .minConstraint = ConstraintBehavior::Immutable,
                    .max = 1.0f,
                    .maxConstraint = ConstraintBehavior::Immutable,
                    .increment = 0.01f,
                    .invalidationLevel = InvalidationLevel::InvalidOutput}};
            prop->onChange([i, this]() { enforceOrder(i); });
            splitters_.addProperty(prop);
        }
        get(i)->setVisible(true);
    }
    for (size_t i = newSize; i < splitters_.size(); i++) {
        get(i)->setVisible(false);
    }
    nSplitters_ = newSize;
    return true;
}

void SplitterPositions::spaceEvenly() {
    const util::KeepTrueWhileInScope enforce{&isEnforcing_};
    const NetworkLock lock(&splitters_);
    const auto minSpacing = minSpacing_();

    for (size_t i = 0; i < size(); i++) {
        set(i, std::max(static_cast<float>(i + 1) * minSpacing,
                        static_cast<float>(i + 1) / static_cast<float>(size() + 1)));
    }
}

void SplitterPositions::deserialized() {
    for (size_t i = 0; i < splitters_.size(); i++) {
        if (splitters_[i]->getVisible()) ++nSplitters_;
        splitters_[i]->onChange([i, this]() { enforceOrder(i); });
    }
}

}  // namespace layout

namespace {

float aspect(const auto& dims) { return static_cast<float>(dims.x) / static_cast<float>(dims.y); }
glm::mat4 scale(const auto& srcDims, const auto& dstDims) {
    const auto sourceAspect = aspect(srcDims);
    const auto targetAspect = aspect(dstDims);
    return targetAspect < sourceAspect
               ? glm::scale(glm::vec3(1.0f, targetAspect / sourceAspect, 1.0f))
               : glm::scale(glm::vec3(sourceAspect / targetAspect, 1.0f, 1.0f));
}

}  // namespace
Layout::Layout()
    : Processor()
    , input_([this](bool connect) { updateSplitters(connect); })
    , outport_("outport")
    , inputMode_("inputMode", "Input Mode",
                 "Select the input Mode, either a multi inport or a sequence inport"_help,
                 {{"multi", "Multi", layout::InputMode::Multi},
                  {"sequence", "Sequence", layout::InputMode::Sequence}},
                 0)
    , splitterSettings_("splitterSettings", "Style", true, splitter::Style::Line,
                        vec4(0.27f, 0.3f, 0.31f, 1.0f), vec4(0.1f, 0.1f, 0.12f, 1.0f))
    , minWidth_("minWidth", "Minimum Width (px)", 10, 0, 4096, 1, InvalidationLevel::InvalidOutput,
                PropertySemantics::SpinBox)
    , horizontalSplitters_("horizontalSplitters", "Horizontal Splits",
                           [this]() {
                               return static_cast<float>(minWidth_.get()) /
                                      static_cast<float>(currentDim_.x);
                           })
    , verticalSplitters_("verticalSplitters", "Vertical Splits",
                         [this]() {
                             return static_cast<float>(minWidth_.get()) /
                                    static_cast<float>(currentDim_.y);
                         })
    , horizontalRenderer_(this)
    , verticalRenderer_(this)
    , splitEvenly_{"splitEvenly", "Split Evenly",
                   [this]() {
                       const NetworkLock lock(this);
                       horizontalSplitters_.spaceEvenly();
                       verticalSplitters_.spaceEvenly();
                   }}
    , currentDim_(0, 0)
    , shader_("standard.vert", "img_copy.frag")
    , deserialized_(false) {

    input_.addPorts(this);
    addPort(outport_);

    splitterSettings_.setCollapsed(true);
    splitterSettings_.width_.set(4.0f);
    splitterSettings_.triSize_.set(0.0f);
    splitterSettings_.setCurrentStateAsDefault();

    addProperties(inputMode_, splitterSettings_, minWidth_, horizontalSplitters_.splitters_,
                  verticalSplitters_.splitters_, splitEvenly_);

    horizontalSplitters_.splitters_.onChange([this]() { splittersChanged(); });
    verticalSplitters_.splitters_.onChange([this]() { splittersChanged(); });

    horizontalRenderer_.setInvalidateAction(
        [this]() { invalidate(InvalidationLevel::InvalidOutput); });
    horizontalRenderer_.setDragAction(
        [this](float pos, int index) { horizontalSplitters_.set(index, pos); });

    verticalRenderer_.setInvalidateAction(
        [this]() { invalidate(InvalidationLevel::InvalidOutput); });
    verticalRenderer_.setDragAction(
        [this](float pos, int index) { verticalSplitters_.set(index, pos); });

    inputMode_.onChange([this]() {
        input_.setMode(this, inputMode_.get(), [this](bool connect) { updateSplitters(connect); });
    });
}

void Layout::process() {
    const auto& images = input_.getData();
    deserialized_ = false;

    utilgl::activateAndClearTarget(outport_, ImageType::ColorDepthPicking);

    shader_.activate();
    TextureUnit colorUnit, depthUnit, pickingUnit;
    shader_.setUniform("color_", colorUnit.getUnitNumber());
    shader_.setUniform("depth_", depthUnit.getUnitNumber());
    shader_.setUniform("picking_", pickingUnit.getUnitNumber());

    for (auto&& [image, view] : std::views::zip(images, viewManager_.getViews())) {
        if (view.empty()) continue;

        shader_.setUniform("dataToClip", scale(image->getDimensions(), view.size));
        utilgl::bindTextures(*image, colorUnit, depthUnit, pickingUnit);
        glViewport(view.pos.x, view.pos.y, view.size.x, view.size.y);
        utilgl::singleDrawImagePlaneRect();
    }

    const ivec2 dim = outport_.getData()->getDimensions();
    glViewport(0, 0, dim.x, dim.y);

    if (splitterSettings_.enabled()) {
        auto hs = horizontalSplitters_.splits();
        splits_.assign(hs.begin(), hs.end());
        horizontalRenderer_.render(splitterSettings_, splitter::Direction::Horizontal, splits_,
                                   outport_.getDimensions());

        auto vs = verticalSplitters_.splits();
        splits_.assign(vs.begin(), vs.end());
        verticalRenderer_.render(splitterSettings_, splitter::Direction::Vertical, splits_,
                                 outport_.getDimensions());
    }

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

void Layout::propagateEvent(Event* event, Outport* source) {
    if (event->hasVisitedProcessor(this)) return;
    event->markAsVisited(this);

    invokeEvent(event);
    if (event->hasBeenUsed()) return;

    if (auto* resizeEvent = event->getAs<ResizeEvent>()) {
        currentDim_ = resizeEvent->size();
        calculateViews(currentDim_);
        input_.propagateSizes(viewManager_);

    } else {
        auto propagated = viewManager_.propagateEvent(
            event, [this](Event* e, size_t i) { input_.propagateEvent(e, i); });

        if (!propagated) {
            input_.propagateEvent(event, this, source);
        }
    }
}

bool Layout::isConnectionActive([[maybe_unused]] Inport* from, Outport* to) const {
    const auto id = input_.indexOf(to);
    if (id < viewManager_.size()) {
        return !viewManager_[id].empty();
    } else {
        return false;
    }
}

void Layout::deserialize(Deserializer& d) {
    Processor::deserialize(d);

    horizontalSplitters_.deserialized();
    verticalSplitters_.deserialized();

    deserialized_ = true;
}

void Layout::updateSplitters(bool connect) {
    const NetworkLock lock(this);

    const auto grid = getGrid(std::max(1uz, input_.size()));

    if (verticalSplitters_.updateSize(grid.x - 1uz) ||
        horizontalSplitters_.updateSize(grid.y - 1uz)) {

        if (!deserialized_ && connect) {
            horizontalSplitters_.spaceEvenly();
            verticalSplitters_.spaceEvenly();
        }

        ResizeEvent e(currentDim_);
        propagateEvent(&e, &outport_);
    }
}

void Layout::calculateViews(ivec2 imgSize) {
    std::vector<float> xpos;
    {
        xpos.emplace_back(0.0f);
        auto vs = verticalSplitters_.splits();
        xpos.insert(xpos.end(), vs.begin(), vs.end());
        xpos.emplace_back(1.0f);
    }
    std::vector<float> ypos;
    {
        ypos.emplace_back(1.0f);
        auto hs =
            horizontalSplitters_.splits() | std::views::transform([](float y) { return 1.0f - y; });
        ypos.insert(ypos.end(), hs.begin(), hs.end());
        ypos.emplace_back(0.0f);
    }
    viewManager_.clear();
    for (auto&& [xStart, xStop] : std::views::zip(xpos, xpos | std::views::drop(1))) {
        for (auto&& [yStop, yStart] : std::views::zip(ypos, ypos | std::views::drop(1))) {
            const auto pos = ivec2{vec2{imgSize} * vec2{xStart, yStart}};
            const auto size = ivec2{vec2{imgSize} * vec2{xStop - xStart, yStop - yStart}};
            viewManager_.push_back({pos, size});
        }
    }
    notifyObserversActiveConnectionsChange(this);
}

void Layout::splittersChanged() {
    const NetworkLock lock(this);
    ResizeEvent e(currentDim_);
    propagateEvent(&e, &outport_);
}

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ColumnLayout::processorInfo_{
    "org.inviwo.ColumnLayout",  // Class identifier
    "Column Layout",            // Display name
    "Image Operation",          // Category
    CodeState::Stable,          // Code state
    Tags::GL | Tag("Layout"),   // Tags
    "Vertical layout which puts multiple input images on top of each other. "
    "Interactions are forwarded to the respective areas."_help,
};
const ProcessorInfo& ColumnLayout::getProcessorInfo() const { return processorInfo_; }
ColumnLayout::ColumnLayout() : Layout() {}
ivec2 ColumnLayout::getGrid(size_t inputs) const { return {1, inputs}; }

const ProcessorInfo RowLayout::processorInfo_{
    "org.inviwo.RowLayout",    // Class identifier
    "Row Layout",              // Display name
    "Image Operation",         // Category
    CodeState::Stable,         // Code state
    Tags::GL | Tag("Layout"),  // Tags
    "Horizontal layout which puts multiple input images next to each other. "
    "Interactions are forwarded to the respective areas"_help,
};
const ProcessorInfo& RowLayout::getProcessorInfo() const { return processorInfo_; }
RowLayout::RowLayout() : Layout() {}
ivec2 RowLayout::getGrid(size_t inputs) const { return {inputs, 1}; }

const ProcessorInfo GridLayout::processorInfo_{
    "org.inviwo.GridLayout",   // Class identifier
    "Grid Layout",             // Display name
    "Image Operation",         // Category
    CodeState::Stable,         // Code state
    Tags::GL | Tag("Layout"),  // Tags
    "Grid layout which puts multiple input images into a grid. "
    "Interactions are forwarded to the respective areas"_help,
};
const ProcessorInfo& GridLayout::getProcessorInfo() const { return processorInfo_; }

GridLayout::GridLayout() : Layout() {}
ivec2 GridLayout::getGrid(size_t inputs) const {
    const auto side = static_cast<size_t>(std::ceil(std::sqrt(inputs)));

    return {side, side};
}

}  // namespace inviwo
