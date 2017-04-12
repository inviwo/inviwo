/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2017 Inviwo Foundation
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
#include <modules/opengl/texture/textureunit.h>
#include <inviwo/core/interaction/events/gestureevent.h>
#include <inviwo/core/interaction/events/touchevent.h>
#include <modules/opengl/shader/shader.h>
#include <inviwo/core/interaction/interactionhandler.h>
#include <modules/opengl/texture/textureutils.h>

namespace inviwo {

const ProcessorInfo ImageLayoutGL::processorInfo_{
    "org.inviwo.ImageLayoutGL",  // Class identifier
    "Image Layout",              // Display name
    "Image Operation",           // Category
    CodeState::Experimental,     // Code state
    Tags::GL,                    // Tags
};
const ProcessorInfo ImageLayoutGL::getProcessorInfo() const { return processorInfo_; }

ImageLayoutGL::ImageLayoutGL()
    : Processor()
    , multiinport_("multiinport")
    , outport_("outport")
    , layout_("layout", "Layout",
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
    , shader_("img_texturequad.vert", "img_copy.frag")
    , viewManager_()
    , currentLayout_(Layout::CrossSplit)
    , currentDim_(0u, 0u) {

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });

    addPort(multiinport_);

    multiinport_.onConnect([this]() {
        ResizeEvent e(currentDim_);
        propagateEvent(&e, &outport_);
    });

    multiinport_.onDisconnect([this]() {
        ResizeEvent e(currentDim_);
        propagateEvent(&e, &outport_);
    });

    addPort(outport_);

    addProperty(layout_);

    horizontalSplitter_.setVisible(false);
    horizontalSplitter_.onChange(this, &ImageLayoutGL::onStatusChange);
    addProperty(horizontalSplitter_);

    verticalSplitter_.setVisible(false);
    verticalSplitter_.onChange(this, &ImageLayoutGL::onStatusChange);
    addProperty(verticalSplitter_);

    vertical3Left1RightSplitter_.setVisible(false);
    vertical3Left1RightSplitter_.onChange(this, &ImageLayoutGL::onStatusChange);
    addProperty(vertical3Left1RightSplitter_);

    vertical3Right1LeftSplitter_.setVisible(false);
    vertical3Right1LeftSplitter_.onChange(this, &ImageLayoutGL::onStatusChange);
    addProperty(vertical3Right1LeftSplitter_);

    layout_.onChange(this, &ImageLayoutGL::onStatusChange);
}

ImageLayoutGL::~ImageLayoutGL() = default;

void ImageLayoutGL::propagateEvent(Event* event, Outport* source) {
    if (event->hasVisitedProcessor(this)) return;
    event->markAsVisited(this);

    invokeEvent(event);
    if (event->hasBeenUsed()) return;

    if (event->hash() == ResizeEvent::chash()) {
        auto resizeEvent = static_cast<ResizeEvent*>(event);
        updateViewports(resizeEvent->size(), true);
        auto& outports = multiinport_.getConnectedOutports();
        size_t minNum = std::min(outports.size(), viewManager_.size());

        for (size_t i = 0; i < minNum; ++i) {
            ResizeEvent e(uvec2(viewManager_[i].size));
            multiinport_.propagateEvent(&e, outports[i]);
        }
    } else {
        auto& data = multiinport_.getConnectedOutports();
        auto prop = [&](Event* newEvent, size_t ind) {
            if (ind < data.size()) {
                multiinport_.propagateEvent(newEvent, data[ind]);
            }
        };
        auto propagated = viewManager_.propagateEvent(event, prop);
        if (!propagated && event->shouldPropagateTo(&multiinport_, this, source)) {
            multiinport_.propagateEvent(event);
        }
    }
}

void ImageLayoutGL::onStatusChange() {
    horizontalSplitter_.setVisible(false);
    verticalSplitter_.setVisible(false);
    vertical3Left1RightSplitter_.setVisible(false);
    vertical3Right1LeftSplitter_.setVisible(false);

    switch (layout_.getSelectedValue()) {
        case Layout::HorizontalSplit:
            horizontalSplitter_.setVisible(true);
            break;
        case Layout::VerticalSplit:
            verticalSplitter_.setVisible(true);
            break;
        case Layout::CrossSplit:
            horizontalSplitter_.setVisible(true);
            verticalSplitter_.setVisible(true);
            break;
        case Layout::ThreeLeftOneRight:
            vertical3Left1RightSplitter_.setVisible(true);
            break;
        case Layout::ThreeRightOneLeft:
            vertical3Right1LeftSplitter_.setVisible(true);
            break;
        case Layout::HorizontalSplitMultiple:
        case Layout::VerticalSplitMultiple:
        case Layout::Single:
        default:
            break;
    }

    ResizeEvent e(currentDim_);
    propagateEvent(&e, &outport_);
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
    const int smallWindowHeight = dim.y / 3.0f;

    const int extra1 = dim.y % 3 >= 1 ? 1 : 0;  // add extra pixels to the small "windows" if the
    const int extra2 = dim.y % 3 >= 2 ? 1 : 0;  // size is not divisible by 3 to avoid black borders

    const int midx = verticalSplitter_ * dim.x;
    const int midy = horizontalSplitter_ * dim.y;

    const int leftWindow3L1RX = vertical3Left1RightSplitter_ * dim.x;
    const int leftWindow3R1LX = vertical3Right1LeftSplitter_ * dim.x;

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
}

}  // namespace
