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

#pragma once

#include <modules/basegl/baseglmoduledefine.h>  // for IVW_MODULE_BASEGL_API

#include <inviwo/core/ports/imageport.h>               // for ImageMultiInport, ImageOutport
#include <inviwo/core/processors/processor.h>          // for Processor
#include <inviwo/core/processors/processorinfo.h>      // for ProcessorInfo
#include <inviwo/core/properties/compositeproperty.h>  // for CompositeProperty
#include <inviwo/core/properties/minmaxproperty.h>     // for IntMinMaxProperty
#include <inviwo/core/properties/optionproperty.h>     // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>    // for FloatProperty
#include <inviwo/core/util/glmvec.h>                   // for ivec2
#include <inviwo/core/util/staticstring.h>             // for operator+
#include <modules/basegl/viewmanager.h>                // for ViewManager
#include <modules/opengl/shader/shader.h>              // for Shader

#include <functional>   // for __base
#include <string>       // for operator==, string
#include <string_view>  // for operator==
#include <vector>       // for operator!=, vector, operator==

namespace inviwo {
class Event;
class Inport;
class Outport;

//
// clang-format off
/** \docpage{org.inviwo.ImageLayoutGL, Image Layout}
 * ![](org.inviwo.ImageLayoutGL.png?classIdentifier=org.inviwo.ImageLayoutGL)
 *
 * Provides layouting for multiple input images. The order of the input images will determine the result.
 * A mouse click activates the respective area for handling mouse/key interactions.
 * Available layouts include
 * <table>
 *   <tr><td><tt>Single</tt></td><td>The first input image fills the entire output.</td></tr>
 *   <tr><td><tt>Horizontal Split</tt></td><td>Two images are put on top of each other.</td></tr>
 *   <tr><td><tt>Vertical Split</tt></td><td>Two images are put next to each other side by side.</td></tr>
 *   <tr><td><tt>Cross Split</tt></td><td>Two-by-two layout of up to four images filled from left to right and top to bottom.</td></tr>
 *   <tr><td><tt>Three Left, One Right</tt></td><td>The first 3 images are vertically arranged on the left, the fourth is shown on the right.</td></tr>
 *   <tr><td><tt>Three Right, One Left</tt></td><td>The first 3 images are vertically arranged on the right, the fourth is shown on the left.</td></tr>
 *   <tr><td><tt>Horizontal Split Multiple</tt></td><td>Two or more images are put on top of each other.</td></tr>
 *   <tr><td><tt>Vertical Split Multiple</tt></td><td>Two or more images are put next to each other side by side.</td></tr>
 * </table>
 *
 * Minimum left/right/top/bottom sizes will be respected until the output size is smaller than the minimum.
 * Maximum left/right/top/bottom sizes will be respected until there is a conflict, for example max left and right set to 500 
 * but output size is larger than 500+500, at which point left/bottom will have precedence.
 *
 * ### Inports
 *   * __Image Inport__ Multi-inport for multiple images.
 * 
 * ### Outports
 *   * __outport__      Resulting layout of input images
 * 
 * ### Properties
 *   * __Layout__         Applied layout <tt>Single</tt>, <tt>Horizontal Split</tt>, <tt>Cross Split</tt>, <tt>Three Left, One Right</tt>, <tt>Three Right, One Left</tt>, <tt>Horizontal Split Multiple</tt> or <tt>Vertical Split Multiple</tt>
 *   * __Split Position__ Position of the layout splitter.
  *   * __Left/Right/Top/Bottom min/max__ Minimum and maximum size in pixels of the corresponding side.
 *
 */
// clang-format on
class IVW_MODULE_BASEGL_API ImageLayoutGL : public Processor {
public:
    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    enum class Layout {
        Single,
        HorizontalSplit,
        VerticalSplit,
        CrossSplit,
        ThreeLeftOneRight,
        ThreeRightOneLeft,
        HorizontalSplitMultiple,
        VerticalSplitMultiple,
    };

    ImageLayoutGL();
    virtual ~ImageLayoutGL();

    virtual void propagateEvent(Event*, Outport* source) override;

    /**
     * Return true if dimensions of connected port is greater than zero.
     * @param inport This processor's inport
     * @param outport Another processor's outport
     */
    virtual bool isConnectionActive(Inport* inport, Outport* outport) const override;

protected:
    virtual void process() override;

    void updateViewports(ivec2 size, bool force = false);
    void onStatusChange(bool propagate = true);

private:
    ImageMultiInport multiinport_;
    ImageOutport outport_;

    OptionProperty<Layout> layout_;
    FloatProperty horizontalSplitter_;
    FloatProperty verticalSplitter_;
    FloatProperty vertical3Left1RightSplitter_;
    FloatProperty vertical3Right1LeftSplitter_;

    CompositeProperty bounds_;
    // Bounds for vertical splitters
    IntMinMaxProperty leftMinMax_;
    IntMinMaxProperty rightMinMax_;
    // Bounds for horizontal splitters
    IntMinMaxProperty topMinMax_;
    IntMinMaxProperty bottomMinMax_;

    Shader shader_;
    ViewManager viewManager_;
    Layout currentLayout_;
    ivec2 currentDim_;
};

}  // namespace inviwo
