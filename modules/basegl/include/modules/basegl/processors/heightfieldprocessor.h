/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2022 Inviwo Foundation
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

#include <inviwo/core/interaction/cameratrackball.h>        // for CameraTrackball
#include <inviwo/core/ports/imageport.h>                    // for ImageInport, ImageOutport
#include <inviwo/core/ports/meshport.h>                     // for MeshFlatMultiInport
#include <inviwo/core/processors/processor.h>               // for Processor
#include <inviwo/core/processors/processorinfo.h>           // for ProcessorInfo
#include <inviwo/core/properties/cameraproperty.h>          // for CameraProperty
#include <inviwo/core/properties/optionproperty.h>          // for OptionPropertyInt
#include <inviwo/core/properties/ordinalproperty.h>         // for FloatProperty
#include <inviwo/core/properties/simplelightingproperty.h>  // for SimpleLightingProperty
#include <modules/opengl/shader/shader.h>                   // for Shader

namespace inviwo {

namespace HeightFieldShading {
enum Type {
    ConstantColor,
    ColorTexture,
    HeightField,
};
}

/**
 * \brief Maps a heightfield onto a geometry and renders it to an image.
 */
class IVW_MODULE_BASEGL_API HeightFieldProcessor : public Processor {
public:
    HeightFieldProcessor();
    ~HeightFieldProcessor();

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void initializeResources() override;
    virtual void process() override;

private:
    MeshFlatMultiInport inport_;
    ImageInport inportHeightfield_;  //!< inport for the 2D heightfield texture
    ImageInport inportTexture_;      //!< inport for the 2D color texture (optional)
    ImageInport inportNormalMap_;    //!< inport for the 2D normal map texture (optional)
    ImageInport imageInport_;
    ImageOutport outport_;

    FloatProperty heightScale_;             //!< scaling factor for the input heightfield
    OptionPropertyInt terrainShadingMode_;  //!< shading mode for coloring the heightfield

    CameraProperty camera_;
    CameraTrackball trackball_;

    SimpleLightingProperty lightingProperty_;

    Shader shader_;
};

}  // namespace inviwo
