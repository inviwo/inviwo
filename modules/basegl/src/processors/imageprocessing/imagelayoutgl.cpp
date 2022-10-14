/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2023 Inviwo Foundation
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

#include <modules/basegl/processors/imageprocessing/imagelayoutgl.h>

#include <inviwo/core/datastructures/image/imagetypes.h>  // for ImageType, ImageType::ColorDept...
#include <inviwo/core/interaction/events/event.h>         // for Event
#include <inviwo/core/interaction/events/resizeevent.h>   // for ResizeEvent
#include <inviwo/core/ports/imageport.h>                  // for ImageMultiInport, ImageOutport
#include <inviwo/core/ports/outport.h>                    // for Outport
#include <inviwo/core/processors/processor.h>             // for Processor
#include <inviwo/core/processors/processorinfo.h>         // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>        // for CodeState, CodeState::Experimental
#include <inviwo/core/processors/processortags.h>         // for Tags, Tags::GL
#include <inviwo/core/properties/compositeproperty.h>     // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>     // for InvalidationLevel, Invalidation...
#include <inviwo/core/properties/minmaxproperty.h>        // for IntMinMaxProperty, MinMaxProperty
#include <inviwo/core/properties/optionproperty.h>        // for OptionPropertyOption, OptionPro...
#include <inviwo/core/properties/ordinalproperty.h>       // for FloatProperty, OrdinalProperty
#include <inviwo/core/properties/propertysemantics.h>     // for PropertySemantics, PropertySema...
#include <inviwo/core/util/assertion.h>                   // for IVW_ASSERT
#include <inviwo/core/util/glmvec.h>                      // for ivec2, ivec4, size2_t, vec2, uvec2
#include <inviwo/core/util/rendercontext.h>               // for RenderContext
#include <inviwo/core/util/statecoordinator.h>            // for StateCoordinator
#include <inviwo/core/util/staticstring.h>                // for operator+
#include <inviwo/core/util/stdextensions.h>               // for all_of
#include <modules/basegl/viewmanager.h>                   // for ViewManager, ViewManager::View
#include <modules/opengl/inviwoopengl.h>                  // for glViewport
#include <modules/opengl/shader/shader.h>                 // for Shader
#include <modules/opengl/texture/textureunit.h>           // for TextureUnit
#include <modules/opengl/texture/textureutils.h>          // for activateAndClearTarget, bindTex...

#include <algorithm>    // for max, min, find
#include <cstddef>      // for size_t
#include <iterator>     // for distance
#include <limits>       // for numeric_limits
#include <memory>       // for shared_ptr
#include <type_traits>  // for remove_extent_t

#include <glm/common.hpp>             // for clamp, min
#include <glm/vec2.hpp>               // for vec<>::(anonymous), vec, operator-
#include <glm/vector_relational.hpp>  // for any, lessThanEqual, equal

namespace inviwo {
class Inport;

const ProcessorInfo ImageLayoutGL::processorInfo_{
    "org.inviwo.ImageLayoutGL",  // Class identifier
    "Image Layout",              // Display name
    "Image Operation",           // Category
    CodeState::Experimental,     // Code state
    Tags::GL,                    // Tags
    R"(Provides layouting for multiple input images. The order of the input images will determine
    the result. A mouse click activates the respective area for handling mouse/key interactions.
    Available layouts include:

    * __Single__ The first input image fills the entire output.
    * __Horizontal Split__ Two images are put on top of each other.
    * __Vertical Split__ Two images are put next to each other side by side.
    * __Cross Split__ Two-by-two layout of up to four images filled from left to right and top
      to bottom.
    * __Three Left One Right__ The first 3 images are vertically arranged on the left, the fourth
      is shown on the right.
    * __Three Right One Left__ The first 3 images are vertically arranged on the right, the fourth
      is shown on the left.
    * __Horizontal Split Multiple__ Two or more images are put on top of each other.
    * __Vertical Split Multiple__ Two or more images are put next to each other side by side.


    Minimum left/right/top/bottom sizes will be respected until the output size is smaller than
    the minimum. Maximum left/right/top/bottom sizes will be respected until there is a conflict,
    for example max left and right set to 500 but output size is larger than 500+500, at which
    point left/bottom will have precedence.
    )"_unindentHelp};

const ProcessorInfo ImageLayoutGL::getProcessorInfo() const { return processorInfo_; }

ImageLayoutGL::ImageLayoutGL()
    : Processor()
    , multiinport_("multiinport", "Multi-inport for multiple images."_help)
    , outport_("outport", "Resulting layout of input images"_help)
    , layout_("layout", "Layout", "Applied layout"_help,
              {{"single", "Single", Layout::Single},
               {"horizontalSplit", "Horizontal Split", Layout::HorizontalSplit},
               {"verticalSplit", "Vertical Split", Layout::VerticalSplit},
               {"crossSplit", "Cross Split", Layout::CrossSplit},
               {"threeRightOneLeftSplit", "Three Left, One Right", Layout::ThreeLeftOneRight},
               {"threeLeftOneRightSplit", "Three Right, One Left", Layout::ThreeRightOneLeft},
               {"horizontalSplitMultiple", "Horizontal Split Multiple",
                Layout::HorizontalSplitMultiple},
               {"verticalSplitMultiple", "Vertical Split Multiple", Layout::VerticalSplitMultiple}},
              3)

    , horizontalSplitter_("horizontalSplitter", "Horizontal Split", 0.5f, 0.f, 1.f)
    , verticalSplitter_("verticalSplitter", "Vertical Split", 0.5f, 0.f, 1.f)
    , vertical3Left1RightSplitter_("vertical3Left1RightSplitter", "Split Position", 1.0f / 3.0f,
                                   0.f, 1.f)
    , vertical3Right1LeftSplitter_("vertical3Right1LeftSplitter", "Split Position", 2.0f / 3.0f,
                                   0.f, 1.f)
    , bounds_("bounds", "Min/Max dimensions (px)",
              "Minimum and maximum size in pixels of the corresponding side."_help)
    , leftMinMax_("leftMinMax", "Left", 0, std::numeric_limits<int>::max(), 0,
                  std::numeric_limits<int>::max(), 1, 0, InvalidationLevel::InvalidOutput,
                  PropertySemantics::Text)  // note, min 1 to avoid zero size
    , rightMinMax_("rightMinMax", "Right", 0, std::numeric_limits<int>::max(), 0,
                   std::numeric_limits<int>::max(), 1, 0, InvalidationLevel::InvalidOutput,
                   PropertySemantics::Text)
    , topMinMax_("topMinMax", "Top", 0, std::numeric_limits<int>::max(), 0,
                 std::numeric_limits<int>::max(), 0, 0, InvalidationLevel::InvalidOutput,
                 PropertySemantics::Text)
    , bottomMinMax_("bottomMinMax", "Bottom", 0, std::numeric_limits<int>::max(), 0,
                    std::numeric_limits<int>::max(), 1, 0, InvalidationLevel::InvalidOutput,
                    PropertySemantics::Text)
    , shader_("img_texturequad.vert", "img_copy.frag")
    , viewManager_()
    , currentLayout_(Layout::CrossSplit)
    , currentDim_(0u, 0u) {

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });

    addPort(multiinport_);
    multiinport_.setIsReadyUpdater([this]() {
        const auto& outports = multiinport_.getConnectedOutports();
        size_t minNum = std::min(outports.size(), viewManager_.size());
        return multiinport_.isConnected() &&
               util::all_of(outports.begin(), outports.begin() + minNum,
                            [](Outport* p) { return p->isReady(); });
    });
    // Ensure that viewports are up-to-date
    // before isConnectionActive is called
    multiinport_.onConnect([this]() { updateViewports(currentDim_, true); });
    multiinport_.onDisconnect([this]() { updateViewports(currentDim_, true); });

    addPort(outport_);

    addProperty(layout_);

    horizontalSplitter_.setVisible(false);
    horizontalSplitter_.onChange([this]() { onStatusChange(); });
    addProperty(horizontalSplitter_);

    verticalSplitter_.setVisible(false);
    verticalSplitter_.onChange([this]() { onStatusChange(); });
    addProperty(verticalSplitter_);

    vertical3Left1RightSplitter_.setVisible(false);
    vertical3Left1RightSplitter_.onChange([this]() { onStatusChange(); });
    addProperty(vertical3Left1RightSplitter_);

    vertical3Right1LeftSplitter_.setVisible(false);
    vertical3Right1LeftSplitter_.onChange([this]() { onStatusChange(); });
    addProperty(vertical3Right1LeftSplitter_);

    leftMinMax_.onChange([this]() {
        ResizeEvent e(currentDim_);
        propagateEvent(&e, &outport_);
    });
    rightMinMax_.onChange([this]() {
        ResizeEvent e(currentDim_);
        propagateEvent(&e, &outport_);
    });
    topMinMax_.onChange([this]() {
        ResizeEvent e(currentDim_);
        propagateEvent(&e, &outport_);
    });
    bottomMinMax_.onChange([this]() {
        ResizeEvent e(currentDim_);
        propagateEvent(&e, &outport_);
    });
    bounds_.addProperties(leftMinMax_, rightMinMax_, topMinMax_, bottomMinMax_);
    addProperties(bounds_);

    layout_.onChange([this]() { onStatusChange(); });

    onStatusChange(false);
}

ImageLayoutGL::~ImageLayoutGL() = default;

void ImageLayoutGL::propagateEvent(Event* event, Outport* source) {
    if (!event->markAsVisited(this)) return;

    invokeEvent(event);
    if (event->hasBeenUsed()) return;

    if (event->hash() == ResizeEvent::chash()) {
        auto resizeEvent = static_cast<ResizeEvent*>(event);
        // Note, no auto since we want a copy of the views
        std::vector<ViewManager::View> prevViews = viewManager_.getViews();
        updateViewports(resizeEvent->size(), true);
        auto& outports = multiinport_.getConnectedOutports();
        size_t minNum = std::min(outports.size(), viewManager_.size());

        auto changedFromZeroDim = [](int prev, int current) {
            return (prev <= 0 && current > 0) || (prev > 0 && current <= 0);
        };

        bool updated = false;
        for (size_t i = 0; i < minNum; ++i) {
            ResizeEvent e(uvec2(viewManager_[i].size));
            multiinport_.propagateEvent(&e, outports[i]);
            // Only evaluate connections if they will be displayed
            if (i < prevViews.size() &&
                (changedFromZeroDim(prevViews[i].size.x, viewManager_[i].size.x) ||
                 changedFromZeroDim(prevViews[i].size.y, viewManager_[i].size.y))) {
                updated = true;
            } else if (glm::any(glm::lessThanEqual(viewManager_[i].size, ivec2(0)))) {
                // New view has zero size
                updated = true;
            }
        }
        for (size_t i = minNum; i < outports.size(); ++i) {
            ResizeEvent e(size2_t(0));
            multiinport_.propagateEvent(&e, outports[i]);
        }
        if (updated || prevViews.size() != viewManager_.size()) {
            multiinport_.readyUpdate();
            notifyObserversActiveConnectionsChange(this);
        }
    } else {
        auto& data = multiinport_.getConnectedOutports();
        auto prop = [&](Event* newEvent, size_t ind) {
            if (ind < viewManager_.size() && ind < data.size()) {
                multiinport_.propagateEvent(newEvent, data[ind]);
            }
        };
        auto propagated = viewManager_.propagateEvent(event, prop);
        if (!propagated && event->shouldPropagateTo(&multiinport_, this, source)) {
            multiinport_.propagateEvent(event);
        }
    }
}

void ImageLayoutGL::onStatusChange(bool propagate) {
    bool hVisible = false;        // horizontalSplitter_
    bool vVisible = false;        // verticalSplitter_
    bool v3LeftVisible = false;   // vertical3Left1RightSplitter_
    bool v3RightVisible = false;  // vertical3Right1LeftSplitter_

    bool boundsVisible = true;
    bool lVisible = false;  // leftMinMax_
    bool rVisible = false;  // rightMinMax_
    bool tVisible = false;  // topMinMax_
    bool bVisible = false;  // bottomMinMax_

    switch (layout_.getSelectedValue()) {
        case Layout::HorizontalSplit:
            hVisible = true;
            tVisible = true;
            bVisible = true;

            break;
        case Layout::VerticalSplit:
            vVisible = true;
            lVisible = true;
            rVisible = true;
            break;
        case Layout::CrossSplit:
            hVisible = true;
            vVisible = true;

            lVisible = true;
            rVisible = true;
            tVisible = true;
            bVisible = true;
            break;
        case Layout::ThreeLeftOneRight:
            v3LeftVisible = true;
            lVisible = true;
            rVisible = true;
            break;
        case Layout::ThreeRightOneLeft:
            v3RightVisible = true;
            lVisible = true;
            rVisible = true;
            break;
        case Layout::HorizontalSplitMultiple:
        case Layout::VerticalSplitMultiple:
        case Layout::Single:
        default:
            boundsVisible = false;
            break;
    }

    horizontalSplitter_.setVisible(hVisible);
    verticalSplitter_.setVisible(vVisible);
    vertical3Left1RightSplitter_.setVisible(v3LeftVisible);
    vertical3Right1LeftSplitter_.setVisible(v3RightVisible);

    bounds_.setVisible(boundsVisible);
    leftMinMax_.setVisible(lVisible);
    rightMinMax_.setVisible(rVisible);
    topMinMax_.setVisible(tVisible);
    bottomMinMax_.setVisible(bVisible);

    if (propagate) {
        RenderContext::getPtr()->activateDefaultRenderContext();
        ResizeEvent e(currentDim_);
        propagateEvent(&e, &outport_);
    }
}

bool ImageLayoutGL::isConnectionActive([[maybe_unused]] Inport* from, Outport* to) const {
    IVW_ASSERT(from == &multiinport_,
               "ImageLayoutGL was designed for one inport but isConnectionActive was called with "
               "another inport");
    const auto ports = multiinport_.getConnectedOutports();
    auto portIt = std::find(ports.begin(), ports.end(), to);
    auto id = static_cast<size_t>(std::distance(ports.begin(), portIt));
    if (id < viewManager_.size()) {
        // Note: We cannot use Outport dimensions since it might not exist
        return glm::compMul(viewManager_.getViews()[id].size) != 0;
    } else {
        // More connections than views
        return false;
    }
}

void ImageLayoutGL::process() {
    TextureUnit::setZeroUnit();
    auto images = multiinport_.getVectorData();

    TextureUnit colorUnit, depthUnit, pickingUnit;
    utilgl::activateAndClearTarget(outport_, ImageType::ColorDepthPicking);

    shader_.activate();
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

    ivec2 dim = outport_.getData()->getDimensions();
    glViewport(0, 0, dim.x, dim.y);

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
    TextureUnit::setZeroUnit();
}

void ImageLayoutGL::updateViewports(ivec2 dim, bool force) {
    if (!force && (currentDim_ == dim) && (currentLayout_ == layout_.get())) return;  // no changes

    viewManager_.clear();
    const int smallWindowHeight = static_cast<int>(dim.y / 3.0f);

    const int extra1 = dim.y % 3 >= 1 ? 1 : 0;  // add extra pixels to the small "windows" if the
    const int extra2 = dim.y % 3 >= 2 ? 1 : 0;  // size is not divisible by 3 to avoid black borders

    auto rightMinMax = 1.f - vec2(rightMinMax_.getEnd() / static_cast<float>(dim.x),
                                  rightMinMax_.getStart() / static_cast<float>(dim.x));
    auto bottomMinMax = 1.f - vec2(topMinMax_.getEnd() / static_cast<float>(dim.y),
                                   topMinMax_.getStart() / static_cast<float>(dim.y));
    // Bounds cannot be smaller/larger than output size
    auto leftBounds = glm::min(*leftMinMax_, ivec2(dim.x));
    auto bottomBounds = glm::min(*bottomMinMax_, ivec2(dim.y));

    const int midx = glm::clamp(
        static_cast<int>(glm::clamp(*verticalSplitter_, rightMinMax.x, rightMinMax.y) * dim.x),
        leftBounds.x, leftBounds.y);
    const int midy = glm::clamp(
        static_cast<int>(glm::clamp(*horizontalSplitter_, bottomMinMax.x, bottomMinMax.y) * dim.y),
        bottomBounds.x, bottomBounds.y);

    const int leftWindow3L1RX = glm::clamp(
        static_cast<int>(glm::clamp(*vertical3Left1RightSplitter_, rightMinMax.x, rightMinMax.y) *
                         dim.x),
        leftBounds.x, leftBounds.y);
    const int leftWindow3R1LX = glm::clamp(
        static_cast<int>(glm::clamp(*vertical3Right1LeftSplitter_, rightMinMax.x, rightMinMax.y) *
                         dim.x),
        leftBounds.x, leftBounds.y);

    const int portCount = static_cast<int>(multiinport_.getConnectedOutports().size());

    const int widthMultiple = portCount > 1 ? dim.x / portCount : dim.x;
    const int heightMultiple = portCount > 1 ? dim.y / portCount : dim.y;

    auto extraPixelsH = dim.y % std::max(1, portCount);
    auto extraPixelsW = dim.x % std::max(1, portCount);

    int startH = 0;
    int startW = 0;

    switch (layout_.getSelectedValue()) {
        case Layout::HorizontalSplit:

            // #########
            // #   1   #
            // #-------#
            // #   2   #
            // #########
            // X, Y, W, H

            viewManager_.push_back(ivec4(0, midy, dim.x, dim.y - midy));
            viewManager_.push_back(ivec4(0, 0, dim.x, midy));
            break;
        case Layout::VerticalSplit:

            // #########
            // #   |   #
            // # 1 | 2 #
            // #   |   #
            // #########
            // X, Y, W, H

            viewManager_.push_back(ivec4(0, 0, midx, dim.y));
            viewManager_.push_back(ivec4(midx, 0, dim.x - midx, dim.y));
            break;
        case Layout::CrossSplit:

            // #########
            // # 1 | 2 #
            // #-------#
            // # 3 | 4 #
            // #########
            // X, Y, W, H

            viewManager_.push_back(ivec4(0, midy, midx, dim.y - midy));
            viewManager_.push_back(ivec4(midx, midy, dim.x - midx, dim.y - midy));
            viewManager_.push_back(ivec4(0, 0, midx, midy));
            viewManager_.push_back(ivec4(midx, 0, dim.x - midx, midy));
            break;
        case Layout::ThreeLeftOneRight:

            // #############
            // # 1 |       #
            // #---|       #
            // # 2 |   4   #
            // #---|       #
            // # 3 |       #
            // #############
            // X, Y, W, H

            viewManager_.push_back(ivec4(0, 2 * smallWindowHeight + extra1 + extra2,
                                         leftWindow3L1RX, smallWindowHeight));
            viewManager_.push_back(
                ivec4(0, smallWindowHeight + extra1, leftWindow3L1RX, smallWindowHeight + extra2));
            viewManager_.push_back(ivec4(0, 0, leftWindow3L1RX, smallWindowHeight + extra1));
            viewManager_.push_back(ivec4(leftWindow3L1RX, 0, dim.x - leftWindow3L1RX, dim.y));
            break;
        case Layout::ThreeRightOneLeft:

            // #############
            // #       | 1 #
            // #       |---#
            // #   4   | 2 #
            // #       |---#
            // #       | 3 #
            // #############
            // X, Y, W, H

            viewManager_.push_back(ivec4(leftWindow3R1LX, 2 * smallWindowHeight + extra1 + extra2,
                                         dim.x - leftWindow3R1LX, smallWindowHeight));
            viewManager_.push_back(ivec4(leftWindow3R1LX, smallWindowHeight + extra1,
                                         dim.x - leftWindow3R1LX, smallWindowHeight + extra2));
            viewManager_.push_back(
                ivec4(leftWindow3R1LX, 0, dim.x - leftWindow3R1LX, smallWindowHeight + extra1));
            viewManager_.push_back(ivec4(0, 0, leftWindow3R1LX, dim.y));
            break;
        case Layout::HorizontalSplitMultiple:

            // #########
            // #   1   #
            // #-------#
            // #   2   #
            // #-------#
            // #  ...  #
            // #-------#
            // #   N   #
            // #########
            // X, Y, W, H

            for (auto i = 0; i < std::max(1, portCount); i++) {
                auto height = (i < extraPixelsH) ? heightMultiple + 1 : heightMultiple;
                viewManager_.push_back(ivec4(0, startH, dim.x, height));
                startH += height;
            }
            break;
        case Layout::VerticalSplitMultiple:

            // #################
            // #   |   #   |   #
            // # 1 | 2 # . | N #
            // #   |   #   |   #
            // #################
            // X, Y, W, H

            for (auto i = 0; i < std::max(1, portCount); i++) {
                auto width = (i < extraPixelsW) ? widthMultiple + 1 : widthMultiple;
                viewManager_.push_back(ivec4(startW, 0, width, dim.y));
                startW += width;
            }
            break;
        case Layout::Single:
        default:
            viewManager_.push_back(ivec4(0, 0, dim.x, dim.y));
    }

    currentDim_ = dim;
    currentLayout_ = layout_.get();

    isReady_.update();
}

}  // namespace inviwo
