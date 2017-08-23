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

#ifndef IVW_CLIPPINGWIDGET_H
#define IVW_CLIPPINGWIDGET_H

#include <modules/userinterfacegl/userinterfaceglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/simplelightingproperty.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/datastructures/geometry/geometrytype.h>
#include <inviwo/core/interaction/cameratrackball.h>
#include <inviwo/core/interaction/pickingmapper.h>
#include <modules/opengl/shader/shader.h>

#include <array>

namespace inviwo {

class Mesh;
class MeshDrawerGL;

/** \docpage{org.inviwo.CropWidget, Clipping Widget}
 * ![](org.inviwo.CropWidget.png?classIdentifier=org.inviwo.CropWidget)
 * Processor for providing interaction handles for cropping a volume.
 *
 * ### Inports
 *   * __inport__ input image 
 *   * __volume__ input volume used to determine the bounding box
 *
 * ### Outports
 *   * __outport__ output image with the interaction handles rendered 
 *                 on top of the input image
 * 
 * ### Properties
 *   * __UI Settings__ various properties for adjusting the visual appearance
 *   * __Active Axis__ 
 *   * __Crop X__  crop range along the x axis
 *   * __Crop Y__  crop range along the y axis
 *   * __Crop Z__  crop range along the z axis
 */

class IVW_MODULE_USERINTERFACEGL_API CropWidget : public Processor { 
public:
    enum class InteractionElement { LowerBound, UpperBound, Middle, None };

    CropWidget();
    virtual ~CropWidget();

    virtual void process() override;

    virtual void initializeResources() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    void initMesh();
    void createLineStripMesh();
    void updateAxis();
    void updateBoundingCube();
    void objectPicked(PickingEvent *p);
    void rangePositionHandlePicked(PickingEvent *p, bool upperLimit);

    struct AnnotationInfo {
        vec3 pos;
        vec3 axis;
        mat4 rotMatrix;
        mat4 flipMatrix;

        vec3 startNDC;
        vec3 endNDC;
    };

    AnnotationInfo getAxis();

    ImageInport inport_;
    VolumeInport volume_;
    ImageOutport outport_;
    MeshOutport meshOut_;

    CompositeProperty uiSettings_;
    BoolProperty showWidget_;
    BoolProperty showCropPlane_;
    FloatVec4Property handleColor_;
    FloatVec4Property cropLineColor_;
    FloatProperty lineWidth_;

    TemplateOptionProperty<CartesianCoordinateAxis> axis_;
    //OptionPropertyInt alignment_;
    FloatProperty offset_;
    FloatProperty scale_;
    std::array<IntMinMaxProperty, 3> clipRanges_;
    CameraProperty camera_;
    
    SimpleLightingProperty lightingProperty_;
    CameraTrackball trackball_;

    PickingMapper picking_;
    Shader shader_;
    Shader lineShader_;
    
    // number of available interaction elements. 
    static const int numInteractionWidgets = 3; //!< lower and upper bound arrows, middle

    struct PickIDs {
        std::size_t id;
        InteractionElement element;
    };
    std::array<PickIDs, numInteractionWidgets> pickingIDs_;

    AnnotationInfo currentAxisInfo_;

    bool isMouseBeingPressedAndHold_;
    ivec2 lastState_;

    std::shared_ptr<Mesh> interactionHandleMesh_;
    std::shared_ptr<Mesh> linestrip_;

    std::shared_ptr<MeshDrawerGL> interactionHandleDrawer_;

    mat3 volumeBasis_;
    vec3 volumeOffset_;
};

} // namespace inviwo

#endif // IVW_CLIPPINGWIDGET_H
