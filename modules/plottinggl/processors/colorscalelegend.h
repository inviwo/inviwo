/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
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

#ifndef IVW_COLORSCALELEGEND_H
#define IVW_COLORSCALELEGEND_H

#include <modules/plottinggl/plottingglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/isotfproperty.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/datastructures/image/image.h>
#include <modules/opengl/rendering/texturequadrenderer.h>
#include <modules/basegl/viewmanager.h>
#include <modules/opengl/shader/shader.h>
#include <inviwo/core/properties/optionproperty.h>
#include <modules/plottinggl/utils/axisrenderer.h>
#include <inviwo/core/properties/compositeproperty.h>

namespace inviwo {

/** \docpage{org.inviwo.ColorScaleLegend, Color Scale Legend}
 * ![](org.inviwo.ColorScaleLegend.png?classIdentifier=org.inviwo.ColorScaleLegend)
 * Explanation of how to use the processor.
 *
 * ### Inports
 *   * __<ImageInport>__ Inport image.
 *   * __<VolumeInport>__ Inport volume, for datarange extraction.
 *
 * ### Outports
 *   * __<ImageOutport>__ Outport image.
 *
 * ### Properties
 *   * __TF & Isovalues__ The transfer function to render onto the legend.
 *   * __Positioning & Size__
 *		+ __Legend Placement__ Defines to which side of the canvas the legend should be
 *          aligned or if the position and rotation should be customly set by the user
 *		+ __Legend Rotation__ (Only available if "Custom" is chosen as placement) Sets the
 *          legend rotation
 *		+ __Position__ (Only available if "Custom" is chosen as placement) Sets the legend
 *          position in screen coordinates (0 to 1)
 *		+ __Margin__ (Only available if "Custom" is chosen as placement) Sets the legend
 *          margin to canvas borders in pixels
 *		+ __Legend Size__ Sets the legend width and height in pixels
 *   * __Style__
 *		+ __Legend Title__ Sets the axis caption
 *		+ __Color__ Sets the border, axis, title and label colors
 *		+ __Background__ Sets the legend background, either to none or to checkerboard
 *          pattern
 *		+ __Checker Board Size__ Sets the pattern size of the checkerboard
 *      + __Border Width__ Sets the border width in pixels
 */

/**
 * \class ColorScaleLegend
 * \brief VERY_BRIEFLY_DESCRIBE_THE_PROCESSOR
 * Adds a legend displaying the transfer function of this processor to the image output.
 * DESCRIBE_THE_PROCESSOR_FROM_A_DEVELOPER_PERSPECTIVE
 */
class IVW_MODULE_PLOTTINGGL_API ColorScaleLegend : public Processor {
public:
    ColorScaleLegend();
    virtual ~ColorScaleLegend() = default;

    virtual void initializeResources() override;
    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    enum class BackgroundStyle { CheckerBoard, NoBackground };

    void setAxisPosition();
    void updatePositionBoundaries();
    void setLegendPosition();
    void setLegendRotation();
    vec2 getRealSize();

    ImageInport inport_;
    ImageOutport outport_;
    VolumeInport volumeInport_;

    IsoTFProperty isotfComposite_;

    CompositeProperty positioning_;
    CompositeProperty style_;

    // position properties
    TemplateOptionProperty<int> legendPlacement_;
    TemplateOptionProperty<int> rotation_;
    FloatVec2Property position_;
    IntProperty margin_;
    IntVec2Property legendSize_;

    // style customization properties
    StringProperty title_;
    FloatVec4Property color_;
    IntProperty fontSize_;
    TemplateOptionProperty<BackgroundStyle> backgroundStyle_;
    FloatProperty checkerBoardSize_;
    IntProperty borderWidth_;

    // shader variables
    TextureQuadRenderer textureRenderer_;
    Shader shader_;

    // axis properties
    plot::AxisProperty axis_;
    plot::AxisRenderer axisRenderer_;
    ivec2 axisStart_, axisEnd_;
    ivec2 bottomLeft_, bottomRight_, topLeft_, topRight_;
};

}  // namespace inviwo

#endif  // IVW_COLORSCALELEGEND_H
