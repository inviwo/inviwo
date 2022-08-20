/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2022 Inviwo Foundation
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

#include <inviwo/core/util/assertion.h>
#include <inviwo/core/network/networklock.h>
#include <modules/opengl/texture/textureutils.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ColumnLayout::processorInfo_{
    "org.inviwo.ColumnLayout",  // Class identifier
    "Column Layout",            // Display name
    "Image Operation",          // Category
    CodeState::Stable,          // Code state
    Tags::GL | Tag("Layout"),   // Tags
};
const ProcessorInfo ColumnLayout::getProcessorInfo() const { return processorInfo_; }

const ProcessorInfo RowLayout::processorInfo_{
    "org.inviwo.RowLayout",    // Class identifier
    "Row Layout",              // Display name
    "Image Operation",         // Category
    CodeState::Stable,         // Code state
    Tags::GL | Tag("Layout"),  // Tags
};
const ProcessorInfo RowLayout::getProcessorInfo() const { return processorInfo_; }

Layout::Layout(splitter::Direction direction)
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , splitterSettings_("splitterSettings", "Style", true, splitter::Style::Line,
                        vec4(0.27f, 0.3f, 0.31f, 1.0f), vec4(0.1f, 0.1f, 0.12f, 1.0f))
    , minWidth_("minWidth", "Minimum Width (px)", 10, 0, 4096, 1, InvalidationLevel::InvalidOutput,
                PropertySemantics::SpinBox)
    , splitters_("splitters", "Split Positions")
    , renderer_(this)
    , direction_(direction)
    , currentDim_(0, 0)
    , shader_("img_texturequad.vert", "img_copy.frag")
    , deserialized_(false) {

    addPort(inport_);
    addPort(outport_);

    splitterSettings_.setCollapsed(true);
    splitterSettings_.width_.set(4.0f);
    splitterSettings_.triSize_.set(0.0f);
    splitterSettings_.setCurrentStateAsDefault();

    addProperties(splitterSettings_, minWidth_, splitters_);

    inport_.setIsReadyUpdater([this]() {
        // Ports with dimensions (1,1) or less are considered to be inactive
        return inport_.isConnected() &&
               util::all_of(inport_.getConnectedOutports(), [](Outport* p) {
                   auto ip = static_cast<ImageOutport*>(p);
                   return (ip->hasData() &&
                           glm::any(glm::lessThanEqual(ip->getDimensions(), size2_t(1))))
                              ? true
                              : p->isReady();
               });
    });

    inport_.onConnect([this]() { updateSplitters(true); });
    inport_.onDisconnect([this]() { updateSplitters(false); });

    renderer_.setInvalidateAction([this]() { invalidate(InvalidationLevel::InvalidOutput); });
    renderer_.setDragAction([this](float pos, int index) {
        if (index < static_cast<int>(splitters_.size())) {
            static_cast<FloatProperty*>(splitters_[index])->set(pos);
        }
    });
}

void Layout::process() {
    auto images = inport_.getVectorData();
    deserialized_ = false;

    utilgl::activateAndClearTarget(outport_, ImageType::ColorDepthPicking);

    shader_.activate();
    TextureUnit colorUnit, depthUnit, pickingUnit;
    shader_.setUniform("color_", colorUnit.getUnitNumber());
    shader_.setUniform("depth_", depthUnit.getUnitNumber());
    shader_.setUniform("picking_", pickingUnit.getUnitNumber());

    size_t minNum = std::min(images.size(), viewManager_.size());
    for (size_t i = 0; i < minNum; ++i) {
        if (glm::any(glm::lessThanEqual(viewManager_[i].size, ivec2(0)))) {
            continue;
        }
        utilgl::bindTextures(*images[i], colorUnit, depthUnit, pickingUnit);
        glViewport(viewManager_[i].pos.x, viewManager_[i].pos.y, viewManager_[i].size.x,
                   viewManager_[i].size.y);
        utilgl::singleDrawImagePlaneRect();
    }

    const ivec2 dim = outport_.getData()->getDimensions();
    glViewport(0, 0, dim.x, dim.y);

    if (splitterSettings_.enabled()) {
        std::vector<float> positions;
        for (size_t i = 1; i < minNum; ++i) {
            positions.push_back(static_cast<const FloatProperty*>(splitters_[i - 1])->get());
        }
        renderer_.render(splitterSettings_, direction_, positions, outport_.getDimensions());
    }

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

void Layout::propagateEvent(Event* event, Outport* source) {
    if (event->hasVisitedProcessor(this)) return;
    event->markAsVisited(this);

    invokeEvent(event);
    if (event->hasBeenUsed()) return;

    if (event->hash() == ResizeEvent::chash()) {
        auto resizeEvent = static_cast<ResizeEvent*>(event);

        updateViewports(resizeEvent->size());
        currentDim_ = resizeEvent->size();

        const auto& outports = inport_.getConnectedOutports();
        for (size_t i = 0; i < outports.size(); ++i) {
            const size2_t size = [&]() {
                if (glm::any(glm::lessThanEqual(viewManager_[i].size, ivec2(0)))) {
                    return size2_t(1);
                } else {
                    return size2_t(viewManager_[i].size);
                }
            }();

            ResizeEvent e(size);
            outports[i]->propagateEvent(&e, &inport_);
        }
    } else {
        auto& data = inport_.getConnectedOutports();
        auto prop = [&](Event* newEvent, size_t ind) {
            if (ind < viewManager_.size()) {
                inport_.propagateEvent(newEvent, data[ind]);
            }
        };
        auto propagated = viewManager_.propagateEvent(event, prop);
        if (!propagated && event->shouldPropagateTo(&inport_, this, source)) {
            inport_.propagateEvent(event);
        }
    }
}

bool Layout::isConnectionActive([[maybe_unused]] Inport* from, Outport* to) const {
    IVW_ASSERT(from == &inport_,
               "Layout was designed for one inport but isConnectionActive was called with "
               "another inport");
    const auto ports = inport_.getConnectedOutports();
    auto portIt = std::find(ports.begin(), ports.end(), to);
    auto id = static_cast<size_t>(std::distance(ports.begin(), portIt));
    if (id < viewManager_.size()) {
        // Note: We cannot use Outport dimensions since it might not exist
        return !glm::any(glm::equal(viewManager_.getViews()[id].size, ivec2(0)));
    } else {
        // More connections than views
        return false;
    }
}

void Layout::deserialize(Deserializer& d) {
    Processor::deserialize(d);

    int count = 0;
    for (auto p : splitters_) {
        p->onChange([count, this]() { updateSliders(count); });
        ++count;
    }

    deserialized_ = true;
}

void Layout::updateSplitters(bool connect) {
    const auto numViews = std::distance(inport_.begin(), inport_.end());

    size_t count = 0;
    for (ptrdiff_t i = 1; i < numViews; ++i) {
        count++;
        if (splitters_.size() < count) {
            auto prop = new FloatProperty(
                fmt::format("splitter{}", count - 1), fmt::format("Splitter {}", count),
                {i / static_cast<float>(numViews), 0.0f, ConstraintBehavior::Immutable, 1.0f,
                 ConstraintBehavior::Immutable, 0.01f, InvalidationLevel::InvalidOutput});
            prop->onChange([idx = static_cast<int>(i), this]() { updateSliders(idx - 1); });
            splitters_.addProperty(prop);
        } else if (!deserialized_ && connect) {
            // redistribute space evenly when new ports are connected
            static_cast<FloatProperty*>(splitters_[i - 1])->set(i / static_cast<float>(numViews));
        }
        splitters_[i - 1]->setVisible(true);
    }

    for (size_t i = count; i < splitters_.size(); i++) {
        splitters_[i]->setVisible(false);
    }

    updateViewports(currentDim_);

    if (inport_.isConnected()) {
        ResizeEvent e(currentDim_);
        propagateEvent(&e, &outport_);
    }
}

float Layout::getSplitPosition(int i) {
    if (i < 0) {
        return 0.0f;
    } else if (i >= static_cast<int>(splitters_.size())) {
        return 1.0f;
    } else {
        return static_cast<const FloatProperty*>(splitters_[i])->get();
    }
}

void Layout::updateSliders(int slider) {
    NetworkLock lock(this);

    // push everything to the left if necessary
    const float minDelta = static_cast<float>(minWidth_.get()) / currentDim_.x;
    float sliderPos = getSplitPosition(slider);
    for (int i = slider - 1; i >= 0; --i) {
        auto prop = static_cast<FloatProperty*>(splitters_[i]);
        if (*prop + minDelta < sliderPos) {
            break;
        }
        prop->set(sliderPos - minDelta);
        sliderPos = *prop;
    }

    // push everything to the right if necessary
    sliderPos = getSplitPosition(slider);
    for (size_t i = slider + 1; i < splitters_.size(); ++i) {
        auto prop = static_cast<FloatProperty*>(splitters_[i]);
        if (sliderPos < *prop - minDelta) {
            break;
        }
        prop->set(sliderPos + minDelta);
        sliderPos = *prop;
    }

    updateViewports(currentDim_);

    if (inport_.isConnected()) {
        ResizeEvent e(currentDim_);
        propagateEvent(&e, &outport_);
    }
}

ColumnLayout::ColumnLayout() : Layout(splitter::Direction::Vertical) {}

void ColumnLayout::updateViewports(ivec2 dim) {
    const auto numViews = static_cast<int>(std::distance(inport_.begin(), inport_.end()));

    std::vector<int> positions;
    positions.push_back(0);
    for (auto i = 0; i < numViews - 1; ++i) {
        positions.push_back(static_cast<int>(getSplitPosition(i) * dim.x));
    }
    positions.push_back(dim.x);

    viewManager_.clear();
    for (auto i = 0; i < std::max(1, numViews); ++i) {
        viewManager_.push_back(ivec4(positions[i], 0, positions[i + 1] - positions[i], dim.y));
    }
}

RowLayout::RowLayout() : Layout(splitter::Direction::Horizontal) {}

void RowLayout::updateViewports(ivec2 dim) {
    const auto numViews = static_cast<int>(std::distance(inport_.begin(), inport_.end()));

    std::vector<int> positions;
    positions.push_back(dim.y);
    for (auto i = 0; i < numViews - 1; ++i) {
        positions.push_back(static_cast<int>((1.0f - getSplitPosition(i)) * dim.y));
    }
    positions.push_back(0);

    viewManager_.clear();
    for (auto i = 0; i < std::max(1, numViews); ++i) {
        viewManager_.push_back(ivec4(0, positions[i + 1], dim.x, positions[i] - positions[i + 1]));
    }
}

}  // namespace inviwo
