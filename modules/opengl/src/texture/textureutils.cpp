/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2023 Inviwo Foundation
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

#include <modules/opengl/texture/textureutils.h>

#include <inviwo/core/datastructures/buffer/buffer.h>                   // for makeBuffer, Buffer
#include <inviwo/core/datastructures/buffer/bufferram.h>                // for BufferRAMPrecision
#include <inviwo/core/datastructures/coordinatetransformer.h>           // for StructuredCoordin...
#include <inviwo/core/datastructures/geometry/geometrytype.h>           // for BufferType, Buffe...
#include <inviwo/core/datastructures/geometry/mesh.h>                   // for Mesh, Mesh::MeshInfo
#include <inviwo/core/datastructures/image/image.h>                     // for Image
#include <inviwo/core/datastructures/image/imagetypes.h>                // for ImageType, ImageT...
#include <inviwo/core/datastructures/image/layer.h>                     // for Layer
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/transferfunction.h>                // for TransferFunction
#include <inviwo/core/datastructures/volume/volume.h>                   // for Volume
#include <inviwo/core/ports/imageport.h>                                // for ImageOutport, Ima...
#include <inviwo/core/ports/volumeport.h>                               // for VolumeInport
#include <inviwo/core/processors/processor.h>                           // for Processor
#include <inviwo/core/properties/isotfproperty.h>                       // for IsoTFProperty
#include <inviwo/core/properties/transferfunctionproperty.h>            // for TransferFunctionP...
#include <inviwo/core/util/glmvec.h>                                    // for vec2
#include <inviwo/core/util/logcentral.h>                                // for LogCentral, LogEr...
#include <inviwo/core/util/stringconversion.h>                          // for StrBuffer
#include <modules/opengl/buffer/framebufferobject.h>                    // for FrameBufferObject
#include <modules/opengl/geometry/meshgl.h>                             // for MeshGL
#include <modules/opengl/image/imagegl.h>                               // for ImageGL
#include <modules/opengl/image/layergl.h>                               // for LayerGL
#include <modules/opengl/inviwoopengl.h>                                // for GLenum, glActiveT...
#include <modules/opengl/openglutils.h>                                 // for DepthFuncState
#include <modules/opengl/shader/shader.h>                               // for Shader
#include <modules/opengl/sharedopenglresources.h>                       // for SharedOpenGLResou...
#include <modules/opengl/texture/texture.h>                             // for Texture
#include <modules/opengl/texture/textureunit.h>                         // for TextureUnit, Text...
#include <modules/opengl/volume/volumegl.h>                             // for VolumeGL

#include <sstream>        // for operator<<, basic...
#include <string>         // for char_traits, string
#include <type_traits>    // for remove_extent_t
#include <unordered_map>  // for unordered_map
#include <unordered_set>  // for unordered_set
#include <utility>        // for move

#include <fmt/core.h>      // for basic_string_view
#include <glm/mat3x3.hpp>  // for mat
#include <glm/vec2.hpp>    // for vec, operator/

#include <inviwo/tracy/tracy.h>
#include <inviwo/tracy/tracyopengl.h>

namespace inviwo {

namespace utilgl {

void activateTarget(Image& targetImage, ImageType type) {
    TRACY_ZONE_SCOPED_C(0x000088);
    auto outImageGL = targetImage.getEditableRepresentation<ImageGL>();
    outImageGL->activateBuffer(type);
}

void activateTarget(ImageOutport& targetOutport, ImageType type) {
    TRACY_ZONE_SCOPED_C(0x000088);
    if (!targetOutport.hasEditableData()) {
        targetOutport.setData(
            std::make_shared<Image>(targetOutport.getDimensions(), targetOutport.getDataFormat()));
    }
    auto outImage = targetOutport.getEditableData();
    activateTarget(*outImage, type);
}

void activateAndClearTarget(Image& targetImage, ImageType type) {
    TRACY_ZONE_SCOPED_C(0x000088);
    activateTarget(targetImage, type);
    clearCurrentTarget();
}

void activateAndClearTarget(ImageOutport& targetOutport, ImageType type) {
    TRACY_ZONE_SCOPED_C(0x000088);
    activateTarget(targetOutport, type);
    clearCurrentTarget();
}

void activateTargetAndCopySource(Image& targetImage, const Image& sourceImage, ImageType type) {
    TRACY_ZONE_SCOPED_C(0x000088);
    auto outImageGL = targetImage.getEditableRepresentation<ImageGL>();
    sourceImage.getRepresentation<ImageGL>()->copyRepresentationsTo(outImageGL);
    outImageGL->activateBuffer(type);
}

void activateTargetAndCopySource(ImageOutport& targetOutport, const Image& sourceImage,
                                 ImageType type) {
    TRACY_ZONE_SCOPED_C(0x000088);
    if (!targetOutport.hasEditableData()) {
        targetOutport.setData(
            std::make_shared<Image>(targetOutport.getDimensions(), targetOutport.getDataFormat()));
    }
    auto outImage = targetOutport.getEditableData();
    activateTargetAndCopySource(*outImage, sourceImage, type);
}

void activateTargetAndCopySource(Image& targetImage, const ImageInport& sourceInport,
                                 ImageType type) {
    TRACY_ZONE_SCOPED_C(0x000088);
    auto outImageGL = targetImage.getEditableRepresentation<ImageGL>();

    if (auto inImage = sourceInport.getData()) {
        inImage->getRepresentation<ImageGL>()->copyRepresentationsTo(outImageGL);
    } else {
        LogWarnCustom("TextureUtils", "Trying to copy empty image inport: \""
                                          << sourceInport.getIdentifier() << "\" in processor: \""
                                          << sourceInport.getProcessor()->getIdentifier() << "\"");
    }
    outImageGL->activateBuffer(type);
}

void activateTargetAndCopySource(ImageOutport& targetOutport, const ImageInport& sourceInport,
                                 ImageType type) {
    TRACY_ZONE_SCOPED_C(0x000088);
    if (!targetOutport.hasEditableData()) {
        targetOutport.setData(
            std::make_shared<Image>(targetOutport.getDimensions(), targetOutport.getDataFormat()));
    }
    auto outImage = targetOutport.getEditableData();
    activateTargetAndCopySource(*outImage, sourceInport, type);
}

void activateTargetAndClearOrCopySource(Image& targetImage, const ImageInport& sourceInport,
                                        ImageType type) {
    TRACY_ZONE_SCOPED_C(0x000088);

    if (sourceInport.isReady()) {
        utilgl::activateTargetAndCopySource(targetImage, sourceInport, type);
    } else {
        utilgl::activateAndClearTarget(targetImage, type);
    }
}

void activateTargetAndClearOrCopySource(ImageOutport& targetOutport,
                                        const ImageInport& sourceInport, ImageType type) {
    TRACY_ZONE_SCOPED_C(0x000088);
    TRACY_GPU_ZONE_C("Activate and Cleanr Or Copy", 0x000088);
    if (sourceInport.isReady()) {
        utilgl::activateTargetAndCopySource(targetOutport, sourceInport, type);
    } else {
        utilgl::activateAndClearTarget(targetOutport, type);
    }
}

void clearCurrentTarget() {
    TRACY_ZONE_SCOPED_C(0x000088);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void deactivateCurrentTarget() {
    TRACY_ZONE_SCOPED_C(0x000088);
    TRACY_GPU_ZONE_C("Deactivate FBO", 0x000088);
    FrameBufferObject::deactivateFBO();
}

void updateAndActivateTarget(ImageOutport& targetOutport, ImageInport& sourceInport) {
    TRACY_ZONE_SCOPED_C(0x000088);
    if (!targetOutport.hasEditableData()) {
        targetOutport.setData(
            std::make_shared<Image>(targetOutport.getDimensions(), targetOutport.getDataFormat()));
    }
    auto outImage = targetOutport.getEditableData();
    auto outImageGL = outImage->getEditableRepresentation<ImageGL>();
    outImageGL->updateFrom(sourceInport.getData()->getRepresentation<ImageGL>());
    outImageGL->activateBuffer();
}

void bindTextures(const Image& image, bool color, bool depth, bool picking, GLenum colorTexUnit,
                  GLenum depthTexUnit, GLenum pickingTexUnit) {
    TRACY_ZONE_SCOPED_C(0x000088);
    if (color) {
        if (auto layer = image.getColorLayer()) {
            layer->getRepresentation<LayerGL>()->bindTexture(colorTexUnit);
        }
    }
    if (depth) {
        if (auto layer = image.getDepthLayer()) {
            layer->getRepresentation<LayerGL>()->bindTexture(depthTexUnit);
        }
    }
    if (picking) {
        if (auto layer = image.getPickingLayer()) {
            layer->getRepresentation<LayerGL>()->bindTexture(pickingTexUnit);
        }
    }
}

void bindColorTexture(const Image& image, GLenum texUnit) {
    bindTextures(image, true, false, false, texUnit, 0, 0);
}
void bindColorTexture(const ImageInport& inport, GLenum texUnit) {
    bindTextures(*inport.getData(), true, false, false, texUnit, 0, 0);
}

void bindColorTexture(const ImageOutport& outport, GLenum texUnit) {
    bindTextures(*outport.getData(), true, false, false, texUnit, 0, 0);
}

void bindDepthTexture(const Image& image, GLenum texUnit) {
    bindTextures(image, false, true, false, 0, texUnit, 0);
}
void bindDepthTexture(const ImageInport& inport, GLenum texUnit) {
    bindTextures(*inport.getData(), false, true, false, 0, texUnit, 0);
}
void bindDepthTexture(const ImageOutport& outport, GLenum texUnit) {
    bindTextures(*outport.getData(), false, true, false, 0, texUnit, 0);
}

void bindPickingTexture(const Image& image, GLenum texUnit) {
    bindTextures(image, false, false, true, 0, 0, texUnit);
}
void bindPickingTexture(const ImageInport& inport, GLenum texUnit) {
    bindTextures(*inport.getData(), false, false, true, 0, 0, texUnit);
}
void bindPickingTexture(const ImageOutport& outport, GLenum texUnit) {
    bindTextures(*outport.getData(), false, false, true, 0, 0, texUnit);
}

void bindTextures(const Image& image, GLenum colorTexUnit, GLenum depthTexUnit) {
    bindTextures(image, true, true, false, colorTexUnit, depthTexUnit, 0);
}

void bindTextures(const ImageInport& inport, GLenum colorTexUnit, GLenum depthTexUnit) {
    bindTextures(*inport.getData(), true, true, false, colorTexUnit, depthTexUnit, 0);
}

void bindTextures(const ImageOutport& outport, GLenum colorTexUnit, GLenum depthTexUnit) {
    bindTextures(*outport.getData(), true, true, false, colorTexUnit, depthTexUnit, 0);
}

void bindTextures(const Image& image, GLenum colorTexUnit, GLenum depthTexUnit,
                  GLenum pickingTexUnit) {
    bindTextures(image, true, true, true, colorTexUnit, depthTexUnit, pickingTexUnit);
}

void bindTextures(const ImageInport& inport, GLenum colorTexUnit, GLenum depthTexUnit,
                  GLenum pickingTexUnit) {
    bindTextures(*inport.getData(), true, true, true, colorTexUnit, depthTexUnit, pickingTexUnit);
}

void bindTextures(const ImageOutport& outport, GLenum colorTexUnit, GLenum depthTexUnit,
                  GLenum pickingTexUnit) {
    bindTextures(*outport.getData(), true, true, true, colorTexUnit, depthTexUnit, pickingTexUnit);
}

void bindColorTexture(const Image& image, const TextureUnit& texUnit) {
    bindTextures(image, true, false, false, texUnit.getEnum(), 0, 0);
}
void bindColorTexture(const ImageInport& inport, const TextureUnit& texUnit) {
    bindTextures(*inport.getData(), true, false, false, texUnit.getEnum(), 0, 0);
}
void bindColorTexture(const ImageOutport& outport, const TextureUnit& texUnit) {
    bindTextures(*outport.getData(), true, false, false, texUnit.getEnum(), 0, 0);
}

void bindDepthTexture(const Image& image, const TextureUnit& texUnit) {
    bindTextures(image, false, true, false, 0, texUnit.getEnum(), 0);
}
void bindDepthTexture(const ImageInport& inport, const TextureUnit& texUnit) {
    bindTextures(*inport.getData(), false, true, false, 0, texUnit.getEnum(), 0);
}
void bindDepthTexture(const ImageOutport& outport, const TextureUnit& texUnit) {
    bindTextures(*outport.getData(), false, true, false, 0, texUnit.getEnum(), 0);
}

void bindPickingTexture(const Image& image, const TextureUnit& texUnit) {
    bindTextures(image, false, false, true, 0, 0, texUnit.getEnum());
}
void bindPickingTexture(const ImageInport& inport, const TextureUnit& texUnit) {
    bindTextures(*inport.getData(), false, false, true, 0, 0, texUnit.getEnum());
}
void bindPickingTexture(const ImageOutport& outport, const TextureUnit& texUnit) {
    bindTextures(*outport.getData(), false, false, true, 0, 0, texUnit.getEnum());
}

void bindTextures(const Image& image, const TextureUnit& colorTexUnit,
                  const TextureUnit& depthTexUnit) {
    bindTextures(image, true, true, false, colorTexUnit.getEnum(), depthTexUnit.getEnum(), 0);
}

void bindTextures(const ImageInport& inport, const TextureUnit& colorTexUnit,
                  const TextureUnit& depthTexUnit) {
    bindTextures(*inport.getData(), true, true, false, colorTexUnit.getEnum(),
                 depthTexUnit.getEnum(), 0);
}

void bindTextures(const ImageOutport& outport, const TextureUnit& colorTexUnit,
                  const TextureUnit& depthTexUnit) {
    bindTextures(*outport.getData(), true, true, false, colorTexUnit.getEnum(),
                 depthTexUnit.getEnum(), 0);
}

void bindTextures(const Image& image, const TextureUnit& colorTexUnit,
                  const TextureUnit& depthTexUnit, const TextureUnit& pickingTexUnit) {
    bindTextures(image, true, true, true, colorTexUnit.getEnum(), depthTexUnit.getEnum(),
                 pickingTexUnit.getEnum());
}

void bindTextures(const ImageInport& inport, const TextureUnit& colorTexUnit,
                  const TextureUnit& depthTexUnit, const TextureUnit& pickingTexUnit) {
    bindTextures(*inport.getData(), true, true, true, colorTexUnit.getEnum(),
                 depthTexUnit.getEnum(), pickingTexUnit.getEnum());
}

void bindTextures(const ImageOutport& outport, const TextureUnit& colorTexUnit,
                  const TextureUnit& depthTexUnit, const TextureUnit& pickingTexUnit) {
    bindTextures(*outport.getData(), true, true, true, colorTexUnit.getEnum(),
                 depthTexUnit.getEnum(), pickingTexUnit.getEnum());
}

void unbindTextures(const Image& image, bool color, bool depth, bool picking) {
    TRACY_ZONE_SCOPED_C(0x000088);
    if (color) {
        if (auto layer = image.getColorLayer()) {
            layer->getRepresentation<LayerGL>()->unbindTexture();
        }
    }
    if (depth) {
        if (auto layer = image.getDepthLayer()) {
            layer->getRepresentation<LayerGL>()->unbindTexture();
        }
    }
    if (picking) {
        if (auto layer = image.getPickingLayer()) {
            layer->getRepresentation<LayerGL>()->unbindTexture();
        }
    }
}

void unbindColorTexture(const ImageInport& inport) {
    unbindTextures(*inport.getData(), true, false, false);
}

void unbindColorTexture(const ImageOutport& outport) {
    unbindTextures(*outport.getData(), true, false, false);
}

void unbindDepthTexture(const ImageInport& inport) {
    unbindTextures(*inport.getData(), false, true, false);
}

void unbindDepthTexture(const ImageOutport& outport) {
    unbindTextures(*outport.getData(), false, true, false);
}

void unbindPickingTexture(const ImageInport& inport) {
    unbindTextures(*inport.getData(), false, false, true);
}

void unbindPickingTexture(const ImageOutport& outport) {
    unbindTextures(*outport.getData(), false, false, true);
}

void unbindTextures(const Image& image) { unbindTextures(image, true, true, true); }

void unbindTextures(const ImageInport& inport) {
    unbindTextures(*inport.getData(), true, true, true);
}

void unbindTextures(const ImageOutport& outport) {
    unbindTextures(*outport.getData(), true, true, true);
}

void setShaderUniforms(Shader& shader, const Image& image, std::string_view samplerID) {
    TRACY_ZONE_SCOPED_C(0x000088);
    const StructuredCoordinateTransformer<2>& ct =
        image.getColorLayer()->getCoordinateTransformer();

    StrBuffer buff;
    shader.setUniform(buff.replace("{}.dataToModel", samplerID), ct.getDataToModelMatrix());
    shader.setUniform(buff.replace("{}.modelToData", samplerID), ct.getModelToDataMatrix());

    shader.setUniform(buff.replace("{}.dataToWorld", samplerID), ct.getDataToWorldMatrix());
    shader.setUniform(buff.replace("{}.worldToData", samplerID), ct.getWorldToDataMatrix());

    shader.setUniform(buff.replace("{}.modelToWorld", samplerID), ct.getModelToWorldMatrix());
    shader.setUniform(buff.replace("{}.worldToModel", samplerID), ct.getWorldToModelMatrix());

    shader.setUniform(buff.replace("{}.worldToTexture", samplerID), ct.getWorldToTextureMatrix());
    shader.setUniform(buff.replace("{}.textureToWorld", samplerID), ct.getTextureToWorldMatrix());

    shader.setUniform(buff.replace("{}.textureToIndex", samplerID), ct.getTextureToIndexMatrix());
    shader.setUniform(buff.replace("{}.indexToTexture", samplerID), ct.getIndexToTextureMatrix());

    vec2 dimensions = vec2(image.getDimensions());
    shader.setUniform(buff.replace("{}.dimensions", samplerID), dimensions);
    shader.setUniform(buff.replace("{}.reciprocalDimensions", samplerID), vec2(1.0f) / dimensions);
}

void setShaderUniforms(Shader& shader, const ImageInport& inport, std::string_view samplerID) {
    if (samplerID.empty()) {
        setShaderUniforms(shader, *inport.getData(),
                          StrBuffer{"{}Parameters", inport.getIdentifier()});
    } else {

        setShaderUniforms(shader, *inport.getData(), samplerID);
    }
}

void setShaderUniforms(Shader& shader, const ImageOutport& outport, std::string_view samplerID) {
    if (samplerID.empty()) {
        setShaderUniforms(shader, *outport.getData(),
                          StrBuffer{"{}Parameters", outport.getIdentifier()});
    } else {
        setShaderUniforms(shader, *outport.getData(), samplerID);
    }
}

std::unique_ptr<Mesh> planeRect() {
    auto verticesBuffer =
        util::makeBuffer<vec2>({{-1.0f, -1.0f}, {1.0f, -1.0f}, {-1.0f, 1.0f}, {1.0f, 1.0f}});
    auto texCoordsBuffer =
        util::makeBuffer<vec2>({{0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f}});
    auto indices_ = util::makeIndexBuffer({0, 1, 2, 3});

    auto m = std::make_unique<Mesh>();
    m->addBuffer(BufferType::PositionAttrib, verticesBuffer);
    m->addBuffer(BufferType::TexCoordAttrib, texCoordsBuffer);
    m->addIndices(Mesh::MeshInfo(DrawType::Triangles, ConnectivityType::Strip), indices_);

    return m;
}

void singleDrawImagePlaneRect() {
    TRACY_ZONE_SCOPED_C(0x000088);
    auto rect = SharedOpenGLResources::getPtr()->imagePlaneRect();
    utilgl::Enable<MeshGL> enable(rect);
    utilgl::DepthFuncState depth(GL_ALWAYS);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void multiDrawImagePlaneRect(int instances) {
    TRACY_ZONE_SCOPED_C(0x000088);
    auto rect = SharedOpenGLResources::getPtr()->imagePlaneRect();
    utilgl::Enable<MeshGL> enable(rect);
    utilgl::DepthFuncState depth(GL_ALWAYS);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, instances);
}

void bindTexture(const TextureBase& texture, GLenum texUnit) {
	TRACY_ZONE_SCOPED_C(0x000088);
    glActiveTexture(texUnit);
    texture.bind();
    glActiveTexture(GL_TEXTURE0);
}

void bindTexture(const TextureBase& texture, const TextureUnit& texUnit) {
	TRACY_ZONE_SCOPED_C(0x000088);
    glActiveTexture(texUnit.getEnum());
    texture.bind();
    glActiveTexture(GL_TEXTURE0);
}

void bindAndSetUniforms(Shader& shader, TextureUnitContainer& cont, const TextureBase& texture,
                        std::string_view samplerID) {
    TRACY_ZONE_SCOPED_C(0x000088);
    TextureUnit unit;
    bindTexture(texture, unit);
    shader.setUniform(samplerID, unit);
    cont.push_back(std::move(unit));
}

void bindTexture(const TransferFunctionProperty& tfp, const TextureUnit& texUnit) {
    TRACY_ZONE_SCOPED_C(0x000088);
    if (auto tfLayer = tfp.get().getData()) {
        auto transferFunctionGL = tfLayer->getRepresentation<LayerGL>();
        transferFunctionGL->bindTexture(texUnit.getEnum());
    }
}

void bindAndSetUniforms(Shader& shader, TextureUnitContainer& cont,
                        const TransferFunctionProperty& tf) {
    TRACY_ZONE_SCOPED_C(0x000088);
    TextureUnit unit;
    bindTexture(tf, unit);
    shader.setUniform(tf.getIdentifier(), unit);
    cont.push_back(std::move(unit));
}

void bindTexture(const IsoTFProperty& property, const TextureUnit& texUnit) {
    TRACY_ZONE_SCOPED_C(0x000088);
    if (auto tfLayer = property.tf_.get().getData()) {
        auto transferFunctionGL = tfLayer->getRepresentation<LayerGL>();
        transferFunctionGL->bindTexture(texUnit.getEnum());
    }
}

void bindAndSetUniforms(Shader& shader, TextureUnitContainer& cont, const IsoTFProperty& property) {
    TRACY_ZONE_SCOPED_C(0x000088);
    TextureUnit unit;
    bindTexture(property, unit);
    shader.setUniform(property.tf_.getIdentifier(), unit);
    cont.push_back(std::move(unit));
}

void bindTexture(const Volume& volume, const TextureUnit& texUnit) {
    TRACY_ZONE_SCOPED_C(0x000088);
    if (auto volumeGL = volume.getRepresentation<VolumeGL>()) {
        volumeGL->bindTexture(texUnit.getEnum());
    } else {
        LogErrorCustom("TextureUtils", "Could not get a GL representation from volume");
    }
}

void bindTexture(const VolumeInport& inport, const TextureUnit& texUnit) {
    bindTexture(*inport.getData(), texUnit);
}

void bindAndSetUniforms(Shader& shader, TextureUnitContainer& cont, const Image& image,
                        std::string_view id, ImageType type) {
    TRACY_ZONE_SCOPED_C(0x000088);

    StrBuffer buff;

    switch (type) {
        case ImageType::ColorOnly: {
            TextureUnit unit;
            bindColorTexture(image, unit);
            utilgl::setShaderUniforms(shader, image, buff.replace("{}Parameters", id));
            shader.setUniform(buff.replace("{}Color", id), unit);
            cont.push_back(std::move(unit));
            break;
        }
        case ImageType::ColorDepth: {
            TextureUnit unit1, unit2;
            bindTextures(image, unit1, unit2);
            utilgl::setShaderUniforms(shader, image, buff.replace("{}Parameters", id));
            shader.setUniform(buff.replace("{}Color", id), unit1);
            shader.setUniform(buff.replace("{}Depth", id), unit2);
            cont.push_back(std::move(unit1));
            cont.push_back(std::move(unit2));
            break;
        }
        case ImageType::ColorPicking: {
            TextureUnit unit1, unit2;
            bindColorTexture(image, unit1);
            bindPickingTexture(image, unit2);
            utilgl::setShaderUniforms(shader, image, buff.replace("{}Parameters", id));
            shader.setUniform(buff.replace("{}Color", id), unit1);
            shader.setUniform(buff.replace("{}Picking", id), unit2);
            cont.push_back(std::move(unit1));
            cont.push_back(std::move(unit2));
            break;
        }
        case ImageType::ColorDepthPicking: {
            TextureUnit unit1, unit2, unit3;
            bindTextures(image, unit1, unit2, unit3);
            utilgl::setShaderUniforms(shader, image, buff.replace("{}Parameters", id));
            shader.setUniform(buff.replace("{}Color", id), unit1);
            shader.setUniform(buff.replace("{}Depth", id), unit2);
            shader.setUniform(buff.replace("{}Picking", id), unit3);
            cont.push_back(std::move(unit1));
            cont.push_back(std::move(unit2));
            cont.push_back(std::move(unit3));
            break;
        }
    }
}

void bindAndSetUniforms(Shader& shader, TextureUnitContainer& cont, const ImageInport& port,
                        ImageType type) {
    bindAndSetUniforms(shader, cont, *port.getData(), port.getIdentifier(), type);
}

void bindAndSetUniforms(Shader& shader, TextureUnitContainer& cont, const ImageOutport& port,
                        ImageType type) {
    bindAndSetUniforms(shader, cont, *port.getData(), port.getIdentifier(), type);
}
}  // namespace utilgl

}  // namespace inviwo
