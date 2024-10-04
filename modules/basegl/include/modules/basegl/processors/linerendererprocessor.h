/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2024 Inviwo Foundation
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

#include <inviwo/core/interaction/cameratrackball.h>         // for CameraTrackball
#include <inviwo/core/ports/imageport.h>                     // for ImageInport, ImageOutport
#include <inviwo/core/ports/meshport.h>                      // for MeshFlatMultiInport
#include <inviwo/core/processors/processor.h>                // for Processor
#include <inviwo/core/processors/processorinfo.h>            // for ProcessorInfo
#include <inviwo/core/properties/boolproperty.h>             // for BoolProperty
#include <inviwo/core/properties/cameraproperty.h>           // for CameraProperty
#include <modules/basegl/properties/linesettingsproperty.h>  // for LineSettingsProperty
#include <modules/basegl/rendering/linerenderer.h>           // for LineRenderer
#include <modules/basegl/util/meshbnlgl.h>
#include <modules/basegl/datastructures/meshshadercache.h>

namespace inviwo {

/**
 * \class LineRendererProcessor
 * \brief Renders input geometry with 2D lines
 */
class IVW_MODULE_BASEGL_API LineRendererProcessor : public Processor {
public:
    LineRendererProcessor();
    virtual ~LineRendererProcessor() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    void drawMeshes();

    MeshFlatMultiInport inport_;
    ImageInport imageInport_;
    ImageOutport outport_;
    LineSettingsProperty lineSettings_;
    BoolProperty writeDepth_;
    CameraProperty camera_;
    CameraTrackball trackball_;
    MeshBnLGL bnl_;
    algorithm::LineRenderer lineRenderer_;
};

}  // namespace inviwo
