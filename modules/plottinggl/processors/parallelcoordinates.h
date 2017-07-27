/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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

#ifndef IVW_PARALLELCOORDINATES_H
#define IVW_PARALLELCOORDINATES_H

#include <modules/plottinggl/plottingglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/interaction/pickingmapper.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/rendering/meshdrawer.h>
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>
#include <modules/plotting/datastructures/dataframe.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/rendering/texturequadrenderer.h>
#include <modules/fontrendering/textrenderer.h>
#include <modules/plotting/properties/dataframeproperty.h>
#include <modules/plotting/properties/marginproperty.h>

namespace inviwo {
class Mesh;
class PickingEvent;

/** \docpage{org.inviwo.ParallelCoordinates, Parallel Coordinates}
 * ![](org.inviwo.ParallelCoordinates.png?classIdentifier=org.inviwo.ParallelCoordinates)
 * This processor plots a given DataFrame using a Parallel Coordinate Plot.
 *
 * ### Inports
 *   * __DataFrame__  data input for plotting
 *   * __BrushingAndLinking__   inport for brushing & linking interactions
 *
 * ### Outports
 *   * __outport__   rendered image of the parallel coordinate plot
 *
 */

namespace plot {

class IVW_MODULE_PLOTTINGGL_API ParallelCoordinates : public Processor {
public:
    struct AxisBase {
        AxisBase(size_t columnId, std::string name, BoolCompositeProperty *boolCompositeProperty,
                 BoolProperty *usePercentiles, std::shared_ptr<const BufferBase> buffer)
            : columnId_(columnId)
            , property_(boolCompositeProperty)
            , usePercentiles_(usePercentiles)
            , buffer_(buffer)
            , name_(name) {
        }
        
        virtual ~AxisBase() = default;
        
        size_t columnId_;
        BoolCompositeProperty *property_;
        BoolProperty *usePercentiles_;
        std::shared_ptr<const BufferBase> buffer_;
        std::string name_;

        std::shared_ptr<Texture2D> labelTexture_;
        std::shared_ptr<Texture2D> minValTexture_;
        std::shared_ptr<Texture2D> maxValTexture_;

        virtual float at(size_t idx) const = 0;
        virtual float getNormalized(float v) const = 0;
        virtual float getValue(float v) const = 0;
        virtual vec2 getRange() const = 0;

        virtual float getRangeMin() const = 0;
        virtual float getRangeMax() const = 0;

        float getNormalizedAt(size_t idx) const { return getNormalized(at(idx)); }

        virtual void updateBrushing(std::unordered_set<size_t> &brushed) = 0;
        virtual void updateRange(bool upper, float y) = 0;
    };

    enum class BlendMode { None = 0, Additive = 1, Sutractive = 2, Regular = 3 };

    enum class LabelPosition { None, Above, Below };

public:
    ParallelCoordinates();
    virtual ~ParallelCoordinates();

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    void linePicked(PickingEvent *p);
    void handlePicked(PickingEvent *p);

private:
    void createOrUpdateProperties();

    void buildLineMesh();
    void drawAxis(size2_t size, std::vector<AxisBase *> enabledAxis, vec4 extraMargins);
    void drawHandles(size2_t size, std::vector<AxisBase *> enabledAxis, vec4 extraMargins);
    void drawLines(size2_t size, std::vector<AxisBase *> enabledAxis, vec4 extraMargins);

    void buildTextCache(size2_t size, std::vector<AxisBase *> enabledAxis);

    void renderText(size2_t size, std::vector<AxisBase *> enabledAxis, vec4 extraMargins);

    void updateBrushing();

    DataInport<DataFrame> dataFrame_;
    BrushingAndLinkingInport brushingAndLinking_;
    ImageOutport outport_;

    CompositeProperty axisProperties_;

    CompositeProperty colors_;
    FloatVec4Property axisColor_;
    FloatVec4Property handleColor_;
    TransferFunctionProperty tf_;
    TransferFunctionProperty tfSelection_;

    CompositeProperty filteringOptions_;
    BoolProperty showFiltered_;
    FloatVec4Property filterColor_;
    FloatProperty filterIntensity_;

    TemplateOptionProperty<BlendMode> blendMode_;
    FloatProperty alpha_;
    FloatProperty falllofPower_;
    FloatProperty lineWidth_;
    FloatProperty selectedLineWidth_;

    FloatVec2Property handleSize_;

    MarginProperty margins_;

    StringProperty selectedAxisName_;
    IntProperty selectedAxisId_;
    DataFrameColumnProperty selectedColorAxis_;

    CompositeProperty text_;
    TemplateOptionProperty<LabelPosition> labelPosition_;
    BoolProperty showValue_;
    FloatVec4Property color_;
    OptionPropertyInt fontSize_;
    OptionPropertyInt valuesFontSize_;

    Shader lineShader_;
    Shader axisShader_;
    Shader handleShader_;

    std::unique_ptr<Mesh> handle_;
    std::unique_ptr<MeshDrawer> handleDrawer_;

    std::unique_ptr<Mesh> axis_;
    std::unique_ptr<MeshDrawer> axisDrawer_;

    std::unique_ptr<Mesh> lines_;
    std::unique_ptr<MeshDrawerGL> linesDrawer_;

    std::vector<std::unique_ptr<AxisBase>> axisVector_;

    PickingMapper linePicking_;
    PickingMapper handlePicking_;

    TextRenderer textRenderer_;
    TextureQuadRenderer textureRenderer_;

    std::shared_ptr<Image> handleImg_;

    bool recreateLines_;
    bool textCacheDirty_;
    bool brushingDirty_;

    vec4 extraMarginsLabel_;
    vec4 extraMarginsValues_;
    vec4 extraMarginsHandles_;
    vec4 extraMargins_;
};

}  // namespace plot

}  // namespace inviwo

#endif  // IVW_PARALLELCOORDINATES_H
