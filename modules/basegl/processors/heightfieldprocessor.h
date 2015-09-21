/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#ifndef IVW_HEIGHTFIELDPROCESSOR_H
#define IVW_HEIGHTFIELDPROCESSOR_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/datastructures/light/baselightsource.h>

#include <modules/basegl/processors/meshrenderprocessorgl.h>
#include <modules/opengl/shader/shader.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/templateproperty.h>
#include <vector>

namespace inviwo {

namespace HeightFieldShading {
    enum Type {
        ConstantColor,
        ColorTexture,
        HeightField,
    };
}

/** \docpage{org.inviwo.HeightFieldRenderGL, Height Field Renderer}
 * Maps a heightfield onto a geometry and renders it to an image. 
 * ![](org.inviwo.HeightFieldRenderGL.png?classIdentifier=org.inviwo.HeightFieldRenderGL)
 * 
 * 
 * ### Inports
 *   * __GeometryMultiInport__ Input geometry which is modified by the heightfield.
 *   * __ImageInport__ The heightfield input (single-channel image). If the 
 *                     image has multiple channels only the red channel is used.
 *   * __ImageInport__ Color texture for color mapping (optional).
 *   * __ImageInport__ Normal map input (optional).
 *
 * ### Outports
 *   * __ImageOutport__ The rendered height field.
 * 
 * ### Properties
 *   * __Height Scale__ Scaling factor for the heightfield.
 *   * __Shading Mode__ Defines the color mapped onto the heightfield using either constant color, 
 *                      color input texture, or the heightfield texture.
 *
 * ### Example Network
 *   ![](heightfield-network.png)
 */

/**
 * \brief Maps a heightfield onto a geometry and renders it to an image.
 *
 */
class IVW_MODULE_BASEGL_API HeightFieldProcessor : public MeshRenderProcessorGL {
public:
    HeightFieldProcessor();
    ~HeightFieldProcessor();

    InviwoProcessorInfo();

    virtual bool isReady() const override {
        // only the mesh input port is mandatory
        return inport_.isReady(); 
    }

protected:
    virtual void process() override;

private:
    void heightfieldChanged();

    ImageInport inportHeightfield_; //!< inport for the 2D heightfield texture
    ImageInport inportTexture_;     //!< inport for the 2D color texture (optional)
    ImageInport inportNormalMap_;   //!< inport for the 2D normal map texture (optional)

    FloatProperty heightScale_;            //!< scaling factor for the input heightfield
    OptionPropertyInt terrainShadingMode_; //!< shading mode for coloring the heightfield
};

} // namespace

#endif // IVW_HEIGHTFIELDPROCESSOR_H
