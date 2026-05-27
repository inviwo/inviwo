/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2026 Inviwo Foundation
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

#include <modules/userinterfacegl/userinterfaceglmoduledefine.h>

#include <inviwo/core/datastructures/geometry/geometrytype.h>
#include <inviwo/core/interaction/cameratrackball.h>
#include <inviwo/core/interaction/pickingmapper.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/simplelightingproperty.h>
#include <inviwo/core/util/glmmat.h>
#include <inviwo/core/util/glmvec.h>
#include <modules/opengl/shader/shader.h>
#include <modules/basegl/properties/linesettingsproperty.h>
#include <modules/basegl/rendering/linerenderer.h>

#include <array>
#include <functional>
#include <optional>
#include <memory>

namespace inviwo {

class Mesh;
class PickingEvent;

class IVW_MODULE_USERINTERFACEGL_API CropWidget : public Processor {
public:
    enum class InteractionElement : std::uint8_t { LowerBound, UpperBound, Middle, None };

    CropWidget();
    virtual ~CropWidget();

    virtual void process() override;

    virtual void initializeResources() override;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    struct AnnotationInfo {
        CartesianCoordinateAxis axis;
        dvec3 startNDC{0.0};
        dvec3 endNDC{0.0};
    };

    void initMesh();
    void createLineStripMesh();
    void updateAxisRanges();
    void objectPicked(PickingEvent* p);
    void rangePositionHandlePicked(size_t axisIndex, PickingEvent* p, InteractionElement element);

    void renderAxis(size_t axisIndex, dvec3 start, dvec3 stop, const dmat4& rotMat,
                    const dmat4& flipMat);

    ImageInport inport_;
    VolumeInport volume_;
    ImageOutport outport_;

    BoolProperty showWidget_;
    CompositeProperty uiSettings_;
    BoolProperty showCropPlane_;
    FloatVec4Property handleColor_;
    LineSettingsProperty cropLineSettings_;

    DoubleProperty offset_;
    DoubleProperty scale_;
    std::array<IntMinMaxProperty, 3> ranges_;
    std::array<AnnotationInfo, 3> axisInfo_;
    BoolProperty relativeRangeAdjustment_;
    CompositeProperty outputProps_;

    CameraProperty camera_;

    SimpleLightingProperty lightingProperty_;
    CameraTrackball trackball_;

    PickingMapper picking_;
    Shader shader_;

    // number of available interaction elements.
    static const size_t numInteractionWidgets = 3;  //!< lower and upper bound arrows, middle handle

    struct PickIDs {
        std::size_t id;
        InteractionElement element;
    };
    std::array<PickIDs, 3 * numInteractionWidgets> pickingIDs_;

    bool isMouseBeingPressedAndHold_;
    ivec2 lastState_;

    std::array<std::shared_ptr<const Mesh>, 2> interactionHandleMesh_;

    std::shared_ptr<Mesh> linestrip_;

    algorithm::LineRenderer lineRenderer_;

    std::function<std::optional<dmat4>()> getBoundingBox_;
};

}  // namespace inviwo
