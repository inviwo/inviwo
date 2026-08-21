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

#include <inviwo/core/datastructures/image/imagetypes.h>
#include <inviwo/core/interaction/events/event.h>
#include <inviwo/core/interaction/events/resizeevent.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/outport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/processors/processorstate.h>
#include <inviwo/core/processors/processortags.h>
#include <inviwo/core/processors/processorutils.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/constraintbehavior.h>
#include <inviwo/core/properties/invalidationlevel.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/propertyowner.h>
#include <inviwo/core/properties/propertysemantics.h>
#include <inviwo/core/util/assertion.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/rendercontext.h>
#include <modules/basegl/datastructures/splittersettings.h>
#include <modules/basegl/properties/splitterproperty.h>
#include <modules/basegl/rendering/splitterrenderer.h>
#include <modules/basegl/viewmanager.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/sharedopenglresources.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>
#include <ranges>
#include <algorithm>

#include <fmt/base.h>
#include <glm/vec2.hpp>
#include <glm/gtx/vec_swizzle.hpp>
#include <glm/vector_relational.hpp>

namespace inviwo {
class Deserializer;

namespace layout {

MultiInput::MultiInput(std::function<void(bool)> aUpdate,
                       std::function<const std::vector<View>&()> aViews)
    : update{std::move(aUpdate)}
    , views{std::move(aViews)}
    , sortOrder{Sorting::ConnectionOrder}
    , inport("inport")
    , outports{}
    , data{}
    , callbacks{} {
    inport.setIsReadyUpdater([this]() {
        return inport.isConnected() &&
               std::ranges::any_of(std::views::zip(outports, views()), [](auto item) {
                   auto [port, view] = item;
                   return !view.empty() && port->isReady();
               });
    });

    inport.onConnect([this]() {
        sort();
        update(true);
    });
    inport.onDisconnect([this]() {
        sort();
        update(false);
    });
}

void MultiInput::addPorts(Processor* p) { p->addPort(inport); }
void MultiInput::removePorts(Processor* p) { p->removePort(&inport); }

size_t MultiInput::size() const { return outports.size(); }

void MultiInput::setSorting(Sorting sSortOrder) {
    sortOrder = sSortOrder;
    sort();

    removeObservations();
    callbacks.clear();

    switch (sortOrder) {
        case Sorting::ConnectionOrder:
            // No additional action needed
            break;
        case Sorting::ProcessorIdentifier:
            for (auto* port : outports) {
                callbacks.emplace_back(port->getProcessor()->onIdentifierChange(
                    [this](std::string_view, std::string_view) { sortInputChanged(); }));
            }
            break;
        case Sorting::ProcessorDisplayName:
            for (auto* port : outports) {
                callbacks.emplace_back(port->getProcessor()->onDisplayNameChange(
                    [this](std::string_view, std::string_view) { sortInputChanged(); }));
            }
            break;

        case Sorting::ProcessorXPosition:
            for (auto* port : outports) {
                util::getMetaData(port->getProcessor())->addObserver(this);
            }
            break;
        default:
            break;
    }
}

void MultiInput::sortInputChanged() {
    sort();

    rendercontext::activateDefault();
    propagateSizes();
}

void MultiInput::onProcessorMetaDataPositionChange() { sortInputChanged(); }

void MultiInput::sort() {
    outports.assign_range(inport.getConnectedOutports());

    switch (sortOrder) {
        case Sorting::ConnectionOrder:
            // No additional sorting needed
            break;
        case Sorting::ProcessorIdentifier:
            std::ranges::stable_sort(outports, std::ranges::less{},
                                     [](Outport* p) { return p->getProcessor()->getIdentifier(); });
            break;
        case Sorting::ProcessorDisplayName:
            std::ranges::stable_sort(outports, std::ranges::less{}, [](Outport* p) {
                return p->getProcessor()->getDisplayName();
            });
            break;

        case Sorting::ProcessorXPosition:
            std::ranges::stable_sort(outports, std::ranges::less{}, [](Outport* p) {
                return util::getPosition(p->getProcessor()).x;
            });
            break;
        default:
            break;
    }
}

const std::vector<std::shared_ptr<const Image>>& MultiInput::getData() {
    data.clear();

    for (auto&& [view, outport] : std::views::zip(views(), outports)) {
        if (!view.empty() && outport->isReady()) {
            data.emplace_back(static_cast<ImageOutport*>(outport)->getDataForPort(&inport));
        } else {
            data.emplace_back(nullptr);
        }
    }
    return data;
}

void MultiInput::propagateSizes() {
    for (auto&& [view, outport] : std::views::zip(views(), outports)) {
        // Propagate a 1x1 size to "non-active" ports, to ensure that we don't leave a "large"
        // entry in requestedDimensions_ in the ImageOutport.
        ResizeEvent e{view.empty() ? size2_t{1, 1} : size2_t{view.size}};
        inport.propagateEvent(&e, outport);
    }
}

void MultiInput::propagateEvent(Event* event, size_t index) {
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
    auto portIt = std::ranges::find(outports, to);
    return static_cast<size_t>(std::ranges::distance(outports.begin(), portIt));
}

bool MultiInput::updateLabels(std::vector<std::string>& labels, std::string_view format) const {
    bool modified = false;

    if (labels.size() != size()) {
        labels.resize(size());
        modified |= true;
    }
    try {
        StrBuffer buff;
        for (auto&& [outport, label, i] : std::views::zip(outports, labels, std::views::iota(0))) {
            if (outport && outport->getProcessor()) {
                buff.replace(fmt::runtime(format),
                             fmt::arg("id", outport->getProcessor()->getIdentifier()),
                             fmt::arg("name", outport->getProcessor()->getDisplayName()),
                             fmt::arg("index", i));

                if (label != buff.view()) {
                    label = buff.view();
                    modified |= true;
                }
            }
        }
    } catch (const fmt::format_error&) {
        throw Exception{SourceContext{},
                        "Invalid format string {}, only 'id', 'name', and 'index' are supported",
                        format};
    }

    return modified;
}

SequenceInput::SequenceInput(std::function<void(bool)> aUpdate,
                             std::function<const std::vector<View>&()> aViews)
    : update{std::move(aUpdate)}, views{std::move(aViews)}, inport("inport") {
    inport.setIsReadyUpdater([this]() {
        return (inport.isConnected() && util::all_of(inport.getConnectedOutports(),
                                                     [](Outport* p) { return p->isReady(); }));
    });

    inport.onChange([this]() { update(true); });
    inport.onConnect([this]() { update(true); });
    inport.onDisconnect([this]() { update(false); });
}

void SequenceInput::addPorts(Processor* p) { p->addPort(inport); }
void SequenceInput::removePorts(Processor* p) { p->removePort(&inport); }

size_t SequenceInput::size() const {
    if (auto input = inport.getData()) {
        return input->size();
    }
    return 0;
}

void SequenceInput::setSorting(Sorting) {}

const std::vector<std::shared_ptr<const Image>>& SequenceInput::getData() {
    data.clear();
    if (auto seq = inport.getData()) {
        data.insert(data.end(), seq->begin(), seq->end());
    }
    return data;
}

void SequenceInput::propagateSizes() {
    const auto max = std::ranges::fold_left(
        views() | std::views::transform([](const auto& v) { return v.size; }), ivec2{1, 1},
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

bool SequenceInput::updateLabels(std::vector<std::string>& labels, std::string_view format) const {
    bool modified = false;

    if (auto seq = inport.getData()) {
        if (labels.size() != seq->size()) {
            labels.resize(seq->size());
            modified |= true;
        }
        try {
            StrBuffer buff;
            for (auto&& [i, label] : std::views::zip(std::views::iota(0ul, seq->size()), labels)) {

                buff.replace(fmt::runtime(format), fmt::arg("index", i));
                if (label != buff.view()) {
                    label = buff;
                    modified |= true;
                }
            }
        } catch (const fmt::format_error&) {
            throw Exception{SourceContext{}, "Invalid format string {}, only 'index' is supported",
                            format};
        }
    } else {
        modified = !labels.empty();
        labels.clear();
    }

    return modified;
}

Input::Input(const std::function<void(bool)>& update,
             const std::function<const std::vector<View>&()>& views)
    : input_{std::in_place_type_t<MultiInput>{}, update, views} {}

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

void Input::propagateSizes() {
    std::visit([&](auto& i) { i.propagateSizes(); }, input_);
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

bool Input::updateLabels(std::vector<std::string>& labels, std::string_view format) const {
    return std::visit([&](auto& i) { return i.updateLabels(labels, format); }, input_);
}

void Input::setSorting(Sorting sortOrder) {
    return std::visit([&](auto& i) { return i.setSorting(sortOrder); }, input_);
}

void Input::setMode(Processor* p, InputMode mode, const std::function<void(bool)>& update,
                    const std::function<const std::vector<View>&()>& views) {
    if (input_.index() == static_cast<size_t>(mode)) return;

    removePorts(p);
    switch (mode) {
        case InputMode::Multi:
            input_.emplace<MultiInput>(update, views);
            break;
        case InputMode::Sequence:
            input_.emplace<SequenceInput>(update, views);
            break;
    }
    addPorts(p);
}

SplitterPositions::SplitterPositions(std::string_view identifier, std::string_view displayName,
                                     std::function<void(SplitsView)> onChange,
                                     std::function<double()> minSize)
    : splitters_{identifier, displayName}
    , nSplitters_{0}
    , onChange_{std::move(onChange)}
    , minSize_{std::move(minSize)}
    , isEnforcing_{false} {}

void SplitterPositions::enforceOrder(size_t changedIndex) {
    if (isEnforcing_) return;
    if (nSplitters_ == 0) {
        onChange_(splits());
        return;
    }
    if (changedIndex >= nSplitters_) {
        onChange_(splits());
        return;
    }

    const util::KeepTrueWhileInScope enforce{&isEnforcing_};
    const NetworkLock lock(&splitters_);
    const auto minSize = minSize_();

    // push everything to the left if necessary
    double sliderPos = position(changedIndex);
    for (auto i : std::views::iota(0uz, changedIndex) | std::views::reverse) {
        if (position(i) + minSize >= sliderPos) {
            set(i, sliderPos - minSize);
        }
        sliderPos = position(i);
    }

    // push everything to the right if necessary
    sliderPos = position(changedIndex);
    for (auto i : std::views::iota(changedIndex + 1, nSplitters_)) {
        if (sliderPos >= position(i) - minSize) {
            set(i, sliderPos + minSize);
        }
        sliderPos = position(i);
    }
    onChange_(splits());
}

bool SplitterPositions::updateSize(size_t newSize) {
    const util::KeepTrueWhileInScope enforce{&isEnforcing_};
    const NetworkLock lock(&splitters_);

    if (nSplitters_ == newSize) return false;

    for (size_t i = 0; i < newSize; ++i) {
        if (splitters_.size() <= i) {
            auto prop = std::make_unique<DoubleProperty>(
                fmt::format("splitter{}", i), fmt::format("Splitter {}", i + 1),
                OrdinalPropertyState<double>{
                    .value = static_cast<double>(i + 1) / static_cast<double>(newSize + 1),
                    .min = 0.0,
                    .minConstraint = ConstraintBehavior::Immutable,
                    .max = 1.0,
                    .maxConstraint = ConstraintBehavior::Immutable,
                    .increment = 0.001,
                    .invalidationLevel = InvalidationLevel::InvalidOutput});
            prop->onChange([i, this]() { enforceOrder(i); });
            splitters_.addProperty(std::move(prop));
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
    const auto minSize = minSize_();

    for (size_t i = 0; i < size(); i++) {
        set(i, std::max(static_cast<double>(i + 1) * minSize,
                        static_cast<double>(i + 1) / static_cast<double>(size() + 1)));
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

double aspect(const auto& dims) {
    return static_cast<double>(dims.x) / static_cast<double>(dims.y);
}
glm::dmat4 scale(const auto& srcDims, const auto& dstDims) {
    const auto sourceAspect = aspect(srcDims);
    const auto targetAspect = aspect(dstDims);
    return targetAspect < sourceAspect
               ? glm::scale(glm::dvec3(1.0, targetAspect / sourceAspect, 1.0))
               : glm::scale(glm::dvec3(sourceAspect / targetAspect, 1.0, 1.0));
}

}  // namespace
Layout::Layout()
    : Processor()
    , input_([this](bool connect) { updateSplitters(connect); },
             [this]() -> const std::vector<layout::View>& { return views_; })
    , outport_("outport")
    , inputMode_("inputMode", "Input Mode",
                 "Select the input Mode, either a multi inport or a sequence inport"_help,
                 {{"multi", "Multi", layout::InputMode::Multi},
                  {"sequence", "Sequence", layout::InputMode::Sequence}},
                 0)
    , sorting_{"sorting",
               "Input Sorting",
               "Select the ordering of the inputs"_help,
               {{"connectionOrder", "Connection Order", layout::Sorting::ConnectionOrder},
                {"processorIdentifier", "Processor Identifier",
                 layout::Sorting::ProcessorIdentifier},
                {"processorDisplayName", "Processor Display Name",
                 layout::Sorting::ProcessorDisplayName},
                {"processorXPosition", "Processor X Position",
                 layout::Sorting::ProcessorXPosition}},
               0}
    , splitterSettings_("splitterSettings", "Style", true, splitter::Style::Line,
                        vec4(0.27f, 0.3f, 0.31f, 1.0f), vec4(0.1f, 0.1f, 0.12f, 1.0f))
    , minWidth_("minWidth", "Minimum Width (px)", 10, 0, 4096, 1, InvalidationLevel::InvalidOutput,
                PropertySemantics::SpinBox)
    , horizontalSplitters_(
          "horizontalSplitters", "Horizontal Splits", [this](auto splits) { splittersChanged(); },
          [this]() {
              return static_cast<double>(minWidth_.get()) / static_cast<double>(currentDim_.x);
          })
    , verticalSplitters_(
          "verticalSplitters", "Vertical Splits", [this](auto splits) { splittersChanged(); },
          [this]() {
              return static_cast<double>(minWidth_.get()) / static_cast<double>(currentDim_.y);
          })
    , horizontalRenderer_(this)
    , verticalRenderer_(this)
    , splitEvenly_{"splitEvenly", "Split Evenly",
                   [this]() {
                       const NetworkLock lock(this);
                       horizontalSplitters_.spaceEvenly();
                       verticalSplitters_.spaceEvenly();
                       splittersChanged();
                   }}
    , labels_{"labels", "Labels", false}
    , format_{"format", "Format",
              "Format string, arguments 'id', 'name', and 'index' are supported,"
              " default: '{id}'"_help,
              "{id}"}
    , font_{"font", "Font Settings"}
    , color_{"color", "Color", util::ordinalColor(vec4(1.0f, 1.0f, 1.0f, 1.0f))}
    , position_{"position", "Position", vec2(0.0f, 0.0f)}
    , offset_{"offset", "Offset", ivec2(0, 0)}

    , currentDim_(0, 0)
    , shader_("standard.vert", "img_copy.frag")
    , deserialized_(false)
    , splits_{}
    , textRenderer_{[]() {
        // ensure the default context is active when creating the TextRenderer
        rendercontext::activateLocal();
        return TextRenderer{};
    }()}
    , textLabels_{}
    , textObjects_{}
    , textureRenderer_{} {

    input_.addPorts(this);
    addPort(outport_);

    splitterSettings_.setCollapsed(true);
    splitterSettings_.width_.set(4.0f);
    splitterSettings_.triSize_.set(0.0f);
    splitterSettings_.setCurrentStateAsDefault();

    font_.removeProperty(font_.anchorPos_);
    font_.removeProperty(font_.fontFace_);
    font_.removeProperty(font_.fontSize_);
    labels_.addProperties(format_, position_, offset_, font_.anchorPos_, font_.fontFace_,
                          font_.fontSize_, color_);
    labels_.setCollapsed(true);

    addProperties(inputMode_, sorting_, splitterSettings_, minWidth_,
                  horizontalSplitters_.splitters_, verticalSplitters_.splitters_, splitEvenly_,
                  labels_);

    horizontalRenderer_.setInvalidateAction(
        [this]() { invalidate(InvalidationLevel::InvalidOutput); });
    horizontalRenderer_.setDragAction(
        [this](float pos, int index) { horizontalSplitters_.set(index, pos); });

    verticalRenderer_.setInvalidateAction(
        [this]() { invalidate(InvalidationLevel::InvalidOutput); });
    verticalRenderer_.setDragAction(
        [this](float pos, int index) { verticalSplitters_.set(index, pos); });

    inputMode_.onChange([this]() {
        input_.setMode(
            this, inputMode_.get(), [this](bool connect) { updateSplitters(connect); },
            [this]() -> const std::vector<layout::View>& { return views_; });
    });
}

void Layout::process() {
    if (sorting_.isModified()) {
        input_.setSorting(sorting_.get());
    }

    const auto& images = input_.getData();
    deserialized_ = false;

    utilgl::activateAndClearTarget(outport_, ImageType::ColorDepthPicking);

    shader_.activate();
    const TextureUnit colorUnit;
    const TextureUnit depthUnit;
    const TextureUnit pickingUnit;
    shader_.setUniform("color_", colorUnit.getUnitNumber());
    shader_.setUniform("depth_", depthUnit.getUnitNumber());
    shader_.setUniform("picking_", pickingUnit.getUnitNumber());

    for (auto&& [image, view] : std::views::zip(images, views_)) {
        if (!view.empty() && image) {
            glViewport(view.pos.x, view.pos.y, view.size.x, view.size.y);
            shader_.setUniform("dataToClip", scale(image->getDimensions(), view.size));
            utilgl::bindTextures(*image, colorUnit, depthUnit, pickingUnit);
            utilgl::singleDrawImagePlaneRect();
        }
    }
    shader_.deactivate();

    auto noise = SharedOpenGLResources::getPtr()->getNoiseShader();
    noise->activate();
    for (auto&& [image, view] : std::views::zip(images, views_)) {
        if (!view.empty() && !image) {
            glViewport(view.pos.x, view.pos.y, view.size.x, view.size.y);
            utilgl::singleDrawImagePlaneRect();
        }
    }
    noise->deactivate();

    const ivec2 dim = outport_.getData()->getDimensions();
    glViewport(0, 0, dim.x, dim.y);

    if (splitterSettings_.enabled()) {
        auto toFloat = [](auto val) { return static_cast<float>(val); };

        auto hs = horizontalSplitters_.splits() | std::views::transform(toFloat);
        splits_.assign(hs.begin(), hs.end());
        horizontalRenderer_.render(splitterSettings_, splitter::Direction::Horizontal, splits_,
                                   outport_.getDimensions());

        auto vs = verticalSplitters_.splits() | std::views::transform(toFloat);
        splits_.assign(vs.begin(), vs.end());
        verticalRenderer_.render(splitterSettings_, splitter::Direction::Vertical, splits_,
                                 outport_.getDimensions());
    }

    if (labels_.isChecked()) {
        if (font_.fontFace_.isModified()) {
            textRenderer_.setFont(font_.fontFace_.get());
        }

        // check whether a property was modified
        if (input_.updateLabels(textLabels_, format_.get()) || color_.isModified() ||
            font_.anchorPos_.isModified() || font_.fontFace_.isModified() ||
            font_.fontSize_.isModified()) {
            updateLabelTextures();
        }

        utilgl::DepthFuncState depthFunc(GL_ALWAYS);
        utilgl::BlendModeState blending(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        for (auto&& [tex, view] : std::views::zip(textObjects_, views_)) {
            // use integer position for best results
            const vec2 size(tex.bbox.textExtent);
            const vec2 shift = 0.5f * size * (font_.anchorPos_.get() + vec2(1.0f, 1.0f));

            ivec2 pos(position_.get() * vec2(view.size));
            pos += offset_.get() - ivec2(shift);

            glViewport(view.pos.x, view.pos.y, view.size.x, view.size.y);
            textureRenderer_.render(tex.texture, pos + tex.bbox.glyphsOrigin, view.size);
        }
    }

    utilgl::deactivateCurrentTarget();
}

void Layout::propagateEvent(Event* event, Outport* source) {
    if (event->hasVisitedProcessor(this)) return;
    event->markAsVisited(this);

    invokeEvent(event);
    if (event->hasBeenUsed()) return;

    if (const auto* resizeEvent = event->getAs<ResizeEvent>()) {
        currentDim_ = resizeEvent->size();
        calculateViews(currentDim_);
        input_.propagateSizes();

    } else {
        auto propagated = eventTransformer_.propagateEvent(event, source);
        if (!propagated) {
            input_.propagateEvent(event, this, source);
        }
    }
}

bool Layout::isConnectionActive([[maybe_unused]] Inport* from, Outport* to) const {
    const auto id = input_.indexOf(to);
    if (id < views_.size()) {
        return !views_[id].empty();
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

    textLabels_.clear();

    const auto grid = getGrid(std::max(1uz, input_.size()));

    if (verticalSplitters_.updateSize(grid.x - 1uz) ||
        horizontalSplitters_.updateSize(grid.y - 1uz)) {

        if (!deserialized_ && connect) {
            horizontalSplitters_.spaceEvenly();
            verticalSplitters_.spaceEvenly();
        }

        splittersChanged();
    }
}

glm::dvec2 remapToSubImage(glm::dvec2 normCoord, glm::dvec2 size, glm::dvec2 subPos,
                           glm::dvec2 subSize) {
    // 1. Convert full-image normalized [-1,1] to pixel coordinates
    const glm::dvec2 pixel = (normCoord + 1.0) * 0.5 * size;
    // 2. Transform into sub-image local pixel space
    const glm::dvec2 localPixel = pixel - subPos;
    // 3. Convert local pixel to normalized [-1,1] in sub-image space
    return (localPixel / subSize) * 2.0 - 1.0;
}

void Layout::calculateViews(ivec2 imgSize) {
    std::vector<double> xpos;
    {
        xpos.emplace_back(0.0);
        auto vs = verticalSplitters_.splits();
        xpos.insert(xpos.end(), vs.begin(), vs.end());
        xpos.emplace_back(1.0);
    }
    std::vector<double> ypos;
    {
        ypos.emplace_back(1.0);
        auto hs =
            horizontalSplitters_.splits() | std::views::transform([](double y) { return 1.0 - y; });
        ypos.insert(ypos.end(), hs.begin(), hs.end());
        ypos.emplace_back(0.0);
    }

    const auto nonActive = views_ | std::views::transform([](auto& view) { return view.empty(); }) |
                           std::ranges::to<std::vector>();

    views_.clear();
    eventTransformer_.views.clear();
    size_t i = 0;
    for (auto&& [xStart, xStop] : std::views::zip(xpos, xpos | std::views::drop(1))) {
        for (auto&& [yStop, yStart] : std::views::zip(ypos, ypos | std::views::drop(1))) {
            const auto fracPos = dvec2{xStart, yStart};
            const auto pos = dvec2{imgSize} * fracPos;
            const auto fracViewSize = dvec2{xStop - xStart, yStop - yStart};
            const auto size = dvec2{imgSize} * fracViewSize;
            views_.push_back({ivec2{glm::round(pos)}, ivec2{glm::round(size)}});

            eventTransformer_.views.push_back(EventTransformer::View{
                .globalNdcToLocalNdc =
                    [imgSize, pos, size](const dvec3& globalNdc) {
                        return dvec3{remapToSubImage(glm::xy(globalNdc), imgSize, pos, size),
                                     globalNdc.z};
                    },
                .propagateEvent = [this, i](Event* e, Outport*) { input_.propagateEvent(e, i); },
                .size = [size]() { return size2_t{glm::round(size)}; }});
            ++i;
        }
    }

    if (!std::ranges::equal(views_ | std::views::transform([](auto& view) { return view.empty(); }),
                            nonActive)) {
        for (auto* port : getInports()) {
            port->readyUpdate();
        }
        notifyObserversActiveConnectionsChange(this);
    }
}

void Layout::splittersChanged() {
    const NetworkLock lock(this);
    rendercontext::activateDefault();
    ResizeEvent e(currentDim_);
    propagateEvent(&e, &outport_);
}

void Layout::updateLabelTextures() {
    textRenderer_.setFontSize(font_.fontSize_.get());
    textRenderer_.setLineSpacing(font_.lineSpacing_.get());
    textObjects_.resize(textLabels_.size());
    for (auto&& [text, tex] : std::views::zip(textLabels_, textObjects_)) {
        tex = util::createTextTextureObject(textRenderer_, text, color_.get(), tex.texture);
    }
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
