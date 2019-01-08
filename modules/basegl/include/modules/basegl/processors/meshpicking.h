/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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

#ifndef IVW_POSITIONWIDGETPROCESSOR_H
#define IVW_POSITIONWIDGETPROCESSOR_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <modules/opengl/inviwoopengl.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/meshport.h>
#include <modules/opengl/image/imagecompositor.h>
#include <inviwo/core/interaction/pickingmapper.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/interaction/cameratrackball.h>
#include <modules/opengl/shader/shader.h>

namespace inviwo {

class MeshDrawerGL;
class PickingEvent;

/** \docpage{org.inviwo.GeometryPicking, Mesh Picking}
 * ![](org.inviwo.GeometryPicking.png?classIdentifier=org.inviwo.GeometryPicking)
 * Composite image with mesh where mesh repositioned through picking
 *
 * Use Left Mouse Button to move the mesh around in the scene
 *
 * ### Inports
 *   * __MeshInport__ The input mesh.
 *   * __ImageInport__ The input image.
 *
 * ### Outports
 *   * __ImageOutport__ The output image.
 *
 * ### Properties
 *   * __Position__ Defines size of all lines.
 *   * __Camera__ Camera of the scene.
 *   * __Trackball__ Camera trackball.
 */

/**
 * \brief Composite image with geometry where geometry repositioned through picking
 *
 */

class IVW_MODULE_BASEGL_API MeshPicking : public Processor {
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    MeshPicking();
    virtual ~MeshPicking();

    virtual void process() override;

    void handlePickingEvent(PickingEvent*);

    void updatePosition(PickingEvent* p);

private:
    MeshInport meshInport_;
    ImageInport imageInport_;
    ImageOutport outport_;
    ImageCompositor compositor_;

    OptionPropertyInt cullFace_;

    FloatVec3Property position_;
    FloatVec4Property highlightColor_;

    CameraProperty camera_;
    CameraTrackball trackball_;

    PickingMapper picking_;

    Shader shader_;

    std::shared_ptr<const Mesh> mesh_;
    std::unique_ptr<MeshDrawerGL> drawer_;

    bool highlight_ = false;
};

}  // namespace inviwo

#endif  // IVW_POSITIONWIDGETPROCESSOR_H
