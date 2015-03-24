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

#include "textureutils.h"
#include "canvasgl.h"
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/image/imagegl.h>
#include <modules/opengl/image/layergl.h>
#include <modules/opengl/glwrap/bufferobjectarray.h>

namespace inviwo {

namespace utilgl {

void activateTarget(ImageOutport& outport, ImageType type) {
    Image* outImage = outport.getData();
    ImageGL* outImageGL = outImage->getEditableRepresentation<ImageGL>();
    outImageGL->activateBuffer(type);
}

void activateAndClearTarget(ImageOutport& outport, ImageType type) {
    activateTarget(outport, type);
    clearCurrentTarget();
}

void activateTargetAndCopySource(ImageOutport& outport, ImageInport& inport, ImageType type){
    Image* outImage = outport.getData();
    ImageGL* outImageGL = outImage->getEditableRepresentation<ImageGL>();
    inport.passOnDataToOutport(&outport);
    outImageGL->activateBuffer(type);
}

void clearCurrentTarget() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }

void deactivateCurrentTarget() { FrameBufferObject::deactivateFBO(); }

void updateAndActivateTarget(ImageOutport& outport, ImageInport& inport) {
    Image* outImage = outport.getData();
    ImageGL* outImageGL = outImage->getEditableRepresentation<ImageGL>();
    outImageGL->updateFrom(inport.getData()->getRepresentation<ImageGL>());
    outImageGL->activateBuffer();
}

void bindTextures(const Image* image, bool color, bool depth, bool picking, GLenum colorTexUnit,
                    GLenum depthTexUnit, GLenum pickingTexUnit) {
    const ImageGL* imageGL = image->getRepresentation<ImageGL>();
    if (color) {
        const LayerGL* layer = imageGL->getColorLayerGL();
        if (layer) {
            layer->bindTexture(colorTexUnit);
        }
    }
    if (depth) {
        const LayerGL* layer = imageGL->getDepthLayerGL();
        if (layer) {
            layer->bindTexture(depthTexUnit);
        }
    }
    if (picking) {
        const LayerGL* layer = imageGL->getPickingLayerGL();
        if (layer) {
            layer->bindTexture(pickingTexUnit);
        }
    }
}


void bindColorTexture(const Image* image, GLenum texUnit) {
    bindTextures(image, true, false, false, texUnit, 0, 0);
}
void bindColorTexture(const ImageInport& inport, GLenum texUnit) {
    bindTextures(inport.getData(), true, false, false, texUnit, 0, 0);
}

void bindColorTexture(const ImageOutport& outport, GLenum texUnit) {
    bindTextures(outport.getConstData(), true, false, false, texUnit, 0, 0);
}


void bindDepthTexture(const Image* image, GLenum texUnit) {
    bindTextures(image, false, true, false, 0, texUnit, 0);
}
void bindDepthTexture(const ImageInport& inport, GLenum texUnit) {
    bindTextures(inport.getData(), false, true, false, 0, texUnit, 0);
}
void bindDepthTexture(const ImageOutport& outport, GLenum texUnit) {
    bindTextures(outport.getConstData(), false, true, false, 0, texUnit, 0);
}


void bindPickingTexture(const Image* image, GLenum texUnit) {
    bindTextures(image, false, false, true, 0, 0, texUnit);
}
void bindPickingTexture(const ImageInport& inport, GLenum texUnit) {
    bindTextures(inport.getData(), false, false, true, 0, 0, texUnit);
}
void bindPickingTexture(const ImageOutport& outport, GLenum texUnit) {
    bindTextures(outport.getConstData(), false, false, true, 0, 0, texUnit);
}

void bindTextures(const Image* image, GLenum colorTexUnit, GLenum depthTexUnit) {
    bindTextures(image, true, true, false, colorTexUnit, depthTexUnit, 0);
}

void bindTextures(const ImageInport& inport, GLenum colorTexUnit, GLenum depthTexUnit) {
    bindTextures(inport.getData(), true, true, false, colorTexUnit, depthTexUnit, 0);
}

void bindTextures(const ImageOutport& outport, GLenum colorTexUnit, GLenum depthTexUnit) {
    bindTextures(outport.getConstData(), true, true, false, colorTexUnit, depthTexUnit, 0);
}

void bindTextures(const Image* image, GLenum colorTexUnit, GLenum depthTexUnit,
                    GLenum pickingTexUnit) {
    bindTextures(image, true, true, true, colorTexUnit, depthTexUnit, pickingTexUnit);
}

void bindTextures(const ImageInport& inport, GLenum colorTexUnit, GLenum depthTexUnit,
                    GLenum pickingTexUnit) {
    bindTextures(inport.getData(), true, true, true, colorTexUnit, depthTexUnit, pickingTexUnit);
}

void bindTextures(const ImageOutport& outport, GLenum colorTexUnit, GLenum depthTexUnit,
                    GLenum pickingTexUnit) {
    bindTextures(outport.getConstData(), true, true, true, colorTexUnit, depthTexUnit,
                   pickingTexUnit);
}


void bindColorTexture(const Image* image, const TextureUnit& texUnit) {
    bindTextures(image, true, false, false, texUnit.getEnum(), 0, 0);
}
void bindColorTexture(const ImageInport& inport, const TextureUnit& texUnit) {
    bindTextures(inport.getData(), true, false, false, texUnit.getEnum(), 0, 0);
}
void bindColorTexture(const ImageOutport& outport, const TextureUnit& texUnit) {
    bindTextures(outport.getConstData(), true, false, false, texUnit.getEnum(), 0, 0);
}

void bindDepthTexture(const Image* image, const TextureUnit& texUnit) {
    bindTextures(image, false, true, false, 0, texUnit.getEnum(), 0);
}
void bindDepthTexture(const ImageInport& inport, const TextureUnit& texUnit) {
    bindTextures(inport.getData(), false, true, false, 0, texUnit.getEnum(), 0);
}
void bindDepthTexture(const ImageOutport& outport, const TextureUnit& texUnit) {
    bindTextures(outport.getConstData(), false, true, false, 0, texUnit.getEnum(), 0);
}


void bindPickingTexture(const Image* image, const TextureUnit& texUnit) {
    bindTextures(image, false, false, true, 0, 0, texUnit.getEnum());
}
void bindPickingTexture(const ImageInport& inport, const TextureUnit& texUnit) {
    bindTextures(inport.getData(), false, false, true, 0, 0, texUnit.getEnum());
}
void bindPickingTexture(const ImageOutport& outport, const TextureUnit& texUnit) {
    bindTextures(outport.getConstData(), false, false, true, 0, 0, texUnit.getEnum());
}

TextureUnit bindColorTexture(const Image* image) {
    TextureUnit unit;
    bindColorTexture(image, unit);
    return std::move(unit);
}
TextureUnit bindColorTexture(const ImageInport& inport) {
    TextureUnit unit;
    bindColorTexture(inport, unit);
    return std::move(unit);
}
TextureUnit bindColorTexture(const ImageOutport& outport) {
    TextureUnit unit;
    bindColorTexture(outport, unit);
    return std::move(unit);
}

TextureUnit bindDepthTexture(const Image* image) {
    TextureUnit unit;
    bindDepthTexture(image, unit);
    return std::move(unit);
}
TextureUnit bindDepthTexture(const ImageInport& inport) {
    TextureUnit unit;
    bindDepthTexture(inport, unit);
    return std::move(unit);
}
TextureUnit bindDepthTexture(const ImageOutport& outport) {
    TextureUnit unit;
    bindDepthTexture(outport, unit);
    return std::move(unit);
}

TextureUnit bindPickingTexture(const Image* image) {
    TextureUnit unit;
    bindPickingTexture(image, unit);
    return std::move(unit);
}
TextureUnit bindPickingTexture(const ImageInport& inport) {
    TextureUnit unit;
    bindPickingTexture(inport, unit);
    return std::move(unit);
}
TextureUnit bindPickingTexture(const ImageOutport& outport) {
    TextureUnit unit;
    bindPickingTexture(outport, unit);
    return std::move(unit);
}

TextureUnitContainer bindColorDepthTextures(const Image* image) {
    TextureUnitContainer cont(2);
    bindTextures(image, cont[0], cont[1]);
    return std::move(cont);
}
TextureUnitContainer bindColorDepthTextures(const ImageInport& image) {
    TextureUnitContainer cont(2);
    bindTextures(image, cont[0], cont[1]);
    return std::move(cont);
}
TextureUnitContainer bindColorDepthTextures(const ImageOutport& image) {
    TextureUnitContainer cont(2);
    bindTextures(image, cont[0], cont[1]);
    return std::move(cont);
}

TextureUnitContainer bindColorDepthPickingTextures(const Image* image) {
    TextureUnitContainer cont(2);
    bindTextures(image, cont[0], cont[1], cont[2]);
    return std::move(cont);
}
TextureUnitContainer bindColorDepthPickingTextures(const ImageInport& image) {
    TextureUnitContainer cont(2);
    bindTextures(image, cont[0], cont[1], cont[2]);
    return std::move(cont);
}
TextureUnitContainer bindColorDepthPickingTextures(const ImageOutport& image) {
    TextureUnitContainer cont(2);
    bindTextures(image, cont[0], cont[1], cont[2]);
    return std::move(cont);
}

void bindTextures(const Image* image, const TextureUnit& colorTexUnit,
                    const TextureUnit& depthTexUnit) {
    bindTextures(image, true, true, false, colorTexUnit.getEnum(), depthTexUnit.getEnum(), 0);
}

void bindTextures(const ImageInport& inport, const TextureUnit& colorTexUnit,
                    const TextureUnit& depthTexUnit) {
    bindTextures(inport.getData(), true, true, false, colorTexUnit.getEnum(),
                   depthTexUnit.getEnum(), 0);
}

void bindTextures(const ImageOutport& outport, const TextureUnit& colorTexUnit,
                    const TextureUnit& depthTexUnit) {
    bindTextures(outport.getConstData(), true, true, false, colorTexUnit.getEnum(),
                   depthTexUnit.getEnum(), 0);
}

void bindTextures(const Image* image, const TextureUnit& colorTexUnit,
                    const TextureUnit& depthTexUnit, const TextureUnit& pickingTexUnit) {
    bindTextures(image, true, true, true, colorTexUnit.getEnum(), depthTexUnit.getEnum(),
                   pickingTexUnit.getEnum());
}

void bindTextures(const ImageInport& inport, const TextureUnit& colorTexUnit,
                    const TextureUnit& depthTexUnit, const TextureUnit& pickingTexUnit) {
    bindTextures(inport.getData(), true, true, true, colorTexUnit.getEnum(),
                   depthTexUnit.getEnum(), pickingTexUnit.getEnum());
}

void bindTextures(const ImageOutport& outport, const TextureUnit& colorTexUnit,
                    const TextureUnit& depthTexUnit, const TextureUnit& pickingTexUnit) {
    bindTextures(outport.getConstData(), true, true, true, colorTexUnit.getEnum(),
                   depthTexUnit.getEnum(), pickingTexUnit.getEnum());
}

void unbindTextures(const Image* image, bool color, bool depth, bool picking) {
    const ImageGL* imageGL = image->getRepresentation<ImageGL>();
    if (color) {
        const LayerGL* layer = imageGL->getColorLayerGL();
        if (layer) {
            layer->unbindTexture();
        }
    }
    if (depth) {
        const LayerGL* layer = imageGL->getDepthLayerGL();
        if (layer) {
            layer->unbindTexture();
        }
    }
    if (picking) {
        const LayerGL* layer = imageGL->getPickingLayerGL();
        if (layer) {
            layer->unbindTexture();
        }
    }
}

void unbindColorTexture(const ImageInport& inport) {
    unbindTextures(inport.getData(), true, false, false);
}

void unbindColorTexture(const ImageOutport& outport) {
    unbindTextures(outport.getConstData(), true, false, false);
}

void unbindDepthTexture(const ImageInport& inport) {
    unbindTextures(inport.getData(), false, true, false);
}

void unbindDepthTexture(const ImageOutport& outport) {
    unbindTextures(outport.getConstData(), false, true, false);
}

void unbindPickingTexture(const ImageInport& inport) {
    unbindTextures(inport.getData(), false, false, true);
}

void unbindPickingTexture(const ImageOutport& outport) {
    unbindTextures(outport.getConstData(), false, false, true);
}

void unbindTextures(const Image* image) { unbindTextures(image, true, true, true); }

void unbindTextures(const ImageInport& inport) {
    unbindTextures(inport.getData(), true, true, true);
}

void unbindTextures(const ImageOutport& outport) {
    unbindTextures(outport.getConstData(), true, true, true);
}

void setShaderUniforms(Shader* shader, const Image* image, const std::string samplerID) {
    const StructuredCoordinateTransformer<2>& ct =
        image->getColorLayer()->getCoordinateTransformer();

    shader->setUniform(samplerID + ".dataToModel", ct.getDataToModelMatrix());
    shader->setUniform(samplerID + ".modelToData", ct.getModelToDataMatrix());

    shader->setUniform(samplerID + ".dataToWorld", ct.getDataToWorldMatrix());
    shader->setUniform(samplerID + ".worldToData", ct.getWorldToDataMatrix());

    shader->setUniform(samplerID + ".modelToWorld", ct.getModelToWorldMatrix());
    shader->setUniform(samplerID + ".worldToModel", ct.getWorldToModelMatrix());

    shader->setUniform(samplerID + ".worldToTexture", ct.getWorldToTextureMatrix());
    shader->setUniform(samplerID + ".textureToWorld", ct.getTextureToWorldMatrix());

    shader->setUniform(samplerID + ".textureToIndex", ct.getTextureToIndexMatrix());
    shader->setUniform(samplerID + ".indexToTexture", ct.getIndexToTextureMatrix());

    vec2 dimensions = vec2(image->getDimensions());
    shader->setUniform(samplerID + ".dimensions", dimensions);
    shader->setUniform(samplerID + ".reciprocalDimensions", vec2(1.0f) / dimensions);
}

void setShaderUniforms(Shader* shader, const ImageInport& inport, const std::string samplerID) {
    setShaderUniforms(shader, inport.getData(),
                      samplerID.empty() ? inport.getIdentifier() + "Parameters" : samplerID);
}

void setShaderUniforms(Shader* shader, const ImageOutport& outport, const std::string samplerID) {
    setShaderUniforms(shader, outport.getConstData(),
                      samplerID.empty() ? outport.getIdentifier() + "Parameters" : samplerID);
}

BufferObjectArray* enableImagePlaneRect() {
    BufferObjectArray* rectArray = new BufferObjectArray();
    CanvasGL::attachImagePlanRect(rectArray);
    glDepthFunc(GL_ALWAYS);
    rectArray->bind();
    return rectArray;
}

void disableImagePlaneRect(BufferObjectArray* rectArray) {
    rectArray->unbind();
    glDepthFunc(GL_LESS);
    delete rectArray;
}

void singleDrawImagePlaneRect() {
    BufferObjectArray* rectArray = enableImagePlaneRect();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    disableImagePlaneRect(rectArray);
}

void multiDrawImagePlaneRect(int instances) {
    BufferObjectArray* rectArray = enableImagePlaneRect();
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, instances);
    disableImagePlaneRect(rectArray);
}

void bindTexture(const TransferFunctionProperty& tfp, const TextureUnit& texUnit) {
    const Layer* tfLayer = tfp.get().getData();
    if (tfLayer) {
        const LayerGL* transferFunctionGL = tfLayer->getRepresentation<LayerGL>();
        transferFunctionGL->bindTexture(texUnit.getEnum());
    }
}

TextureUnit bindTexture(const TransferFunctionProperty& tf) {
    TextureUnit unit;
    bindTexture(tf, unit);
    return std::move(unit);
}

TextureUnit bindAndSetUniforms(Shader* shader, const TransferFunctionProperty& tf) {
    auto unit = bindTexture(tf);
    shader->setUniform(tf.getIdentifier(), unit.getUnitNumber());
    return std::move(unit);
}

void bindTexture(const Volume* volume, const TextureUnit& texUnit) {
    const VolumeGL* volumeGL = volume->getRepresentation<VolumeGL>();
    if (volumeGL) {
        volumeGL->bindTexture(texUnit.getEnum());
    } else {
        LogErrorCustom("TextureUtils", "Could not get a GL representation from volume");
    }
}

void bindTexture(const VolumeInport& inport, const TextureUnit& texUnit) {
    bindTexture(inport.getData(), texUnit);
}

TextureUnit bindTexture(const Volume* volume) {
    TextureUnit unit;
    bindTexture(volume, unit);
    return std::move(unit);
}

TextureUnit bindTexture(const VolumeInport& inport) {
    return bindTexture(inport.getData());
}

TextureUnitContainer bindAndSetUniforms(Shader* shader, const Image* image, const std::string& id,
                                        ImageType type) {
    switch (type) {
        case COLOR_ONLY: {
            TextureUnitContainer cont(1);
            bindColorTexture(image, cont[0]);
            utilgl::setShaderUniforms(shader, image, id + "Parameters");
            shader->setUniform(id + "Color", cont[0].getUnitNumber());
            return cont;
        }
        case COLOR_DEPTH: {
            TextureUnitContainer cont = bindColorDepthTextures(image);
            utilgl::setShaderUniforms(shader, image, id + "Parameters");
            shader->setUniform(id + "Color", cont[0].getUnitNumber());
            shader->setUniform(id + "Depth", cont[1].getUnitNumber());
            return cont;
        }
        case COLOR_PICKING: {
            TextureUnitContainer cont(2);
            bindColorTexture(image, cont[0]);
            bindPickingTexture(image, cont[1]);
            utilgl::setShaderUniforms(shader, image, id + "Parameters");
            shader->setUniform(id + "Color", cont[0].getUnitNumber());
            shader->setUniform(id + "Picking", cont[1].getUnitNumber());
            return cont;
        }
        case COLOR_DEPTH_PICKING: {
            TextureUnitContainer cont = bindColorDepthPickingTextures(image);
            utilgl::setShaderUniforms(shader, image, id + "Parameters");
            shader->setUniform(id + "Color", cont[0].getUnitNumber());
            shader->setUniform(id + "Depth", cont[1].getUnitNumber());
            shader->setUniform(id + "Picking", cont[2].getUnitNumber());
            return cont;
        }
    }
}
    

TextureUnitContainer bindAndSetUniforms(Shader* shader, ImageInport& port, ImageType type) {
    return bindAndSetUniforms(shader, port.getData(), port.getIdentifier(), type);
}

TextureUnitContainer bindAndSetUniforms(Shader* shader, ImageOutport& port, ImageType type) {
    return bindAndSetUniforms(shader, port.getData(), port.getIdentifier(), type);
}


}

}  // namespace
