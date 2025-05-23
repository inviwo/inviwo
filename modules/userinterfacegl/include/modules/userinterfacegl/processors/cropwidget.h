/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
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

#include <modules/userinterfacegl/userinterfaceglmoduledefine.h>  // for IVW_MODULE_USERINTERFAC...

#include <inviwo/core/datastructures/geometry/geometrytype.h>  // for CartesianCoordinateAxis
#include <inviwo/core/interaction/cameratrackball.h>           // for CameraTrackball
#include <inviwo/core/interaction/pickingmapper.h>             // for PickingMapper
#include <inviwo/core/ports/imageport.h>                       // for ImageInport, ImageOutport
#include <inviwo/core/ports/volumeport.h>                      // for VolumeInport
#include <inviwo/core/processors/processor.h>                  // for Processor
#include <inviwo/core/processors/processorinfo.h>              // for ProcessorInfo
#include <inviwo/core/properties/boolproperty.h>               // for BoolProperty
#include <inviwo/core/properties/cameraproperty.h>             // for CameraProperty
#include <inviwo/core/properties/compositeproperty.h>          // for CompositeProperty
#include <inviwo/core/properties/minmaxproperty.h>             // for IntMinMaxProperty
#include <inviwo/core/properties/ordinalproperty.h>            // for FloatProperty, FloatVec...
#include <inviwo/core/properties/simplelightingproperty.h>     // for SimpleLightingProperty
#include <inviwo/core/util/glmmat.h>                           // for mat4, mat3
#include <inviwo/core/util/glmvec.h>                           // for vec3, ivec2
#include <modules/opengl/shader/shader.h>                      // for Shader
#include <modules/basegl/properties/linesettingsproperty.h>    // for LineSettingsProperty
#include <modules/basegl/rendering/linerenderer.h>             // for LineRenderer

#include <array>    // for array
#include <cstddef>  // for size_t
#include <memory>   // for shared_ptr

#include <glm/mat4x4.hpp>  // for mat
#include <glm/vec3.hpp>    // for vec

namespace inviwo {

class Mesh;
class PickingEvent;

class IVW_MODULE_USERINTERFACEGL_API CropWidget : public Processor {
public:
    enum class InteractionElement { LowerBound, UpperBound, Middle, None };

    CropWidget();
    virtual ~CropWidget();

    virtual void process() override;

    virtual void initializeResources() override;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    struct AnnotationInfo {
        vec3 pos;
        vec3 axis;
        mat4 rotMatrix;
        mat4 flipMatrix;

        vec3 startNDC;
        vec3 endNDC;
    };
    struct CropAxis {
        CartesianCoordinateAxis axis;

        CompositeProperty composite;
        BoolProperty enabled;
        IntMinMaxProperty range;
        IntMinMaxProperty outputRange;

        AnnotationInfo info;
    };

    void initMesh();
    void createLineStripMesh();
    void updateAxisRanges();
    void updateBoundingCube();
    void objectPicked(PickingEvent* p);
    void rangePositionHandlePicked(CropAxis& cropAxis, PickingEvent* p, InteractionElement element);

    AnnotationInfo getAxis(CartesianCoordinateAxis majorAxis);
    void renderAxis(const CropAxis& axis);

    ImageInport inport_;
    VolumeInport volume_;
    ImageOutport outport_;

    CompositeProperty uiSettings_;
    BoolProperty showWidget_;
    BoolProperty showCropPlane_;
    FloatVec4Property handleColor_;
    LineSettingsProperty cropLineSettings_;

    FloatProperty offset_;
    FloatProperty scale_;
    std::array<CropAxis, 3> cropAxes_;
    BoolProperty relativeRangeAdjustment_;
    CompositeProperty outputProps_;

    CameraProperty camera_;

    SimpleLightingProperty lightingProperty_;
    CameraTrackball trackball_;

    PickingMapper picking_;
    Shader shader_;

    // number of available interaction elements.
    static const int numInteractionWidgets = 3;  //!< lower and upper bound arrows, middle handle

    struct PickIDs {
        std::size_t id;
        InteractionElement element;
    };
    std::array<PickIDs, 3 * numInteractionWidgets> pickingIDs_;

    bool isMouseBeingPressedAndHold_;
    ivec2 lastState_;

    std::array<std::shared_ptr<const Mesh>, 2> interactionHandleMesh_;

    std::shared_ptr<Mesh> linestrip_;

    mat3 volumeBasis_;
    vec3 volumeOffset_;
    algorithm::LineRenderer lineRenderer_;
};

}  // namespace inviwo
