/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2025 Inviwo Foundation
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

#include <modules/opengl/openglmoduledefine.h>  // for IVW_MODULE_OPENGL_API

#include <inviwo/core/datastructures/image/imagetypes.h>  // for ImageType, ImageType::AllLayers
#include <inviwo/core/datastructures/image/image.h>       // IWYU pragma: kepp
#include <inviwo/core/datastructures/image/layer.h>       // IWYU pragma: kepp
#include <inviwo/core/datastructures/volume/volume.h>     // IWYU pragma: kepp

#include <inviwo/core/ports/datainport.h>  // IWYU pragma: kepp
#include <inviwo/core/ports/imageport.h>   // IWYU pragma: kepp
#include <inviwo/core/ports/layerport.h>   // IWYU pragma: kepp
#include <inviwo/core/ports/volumeport.h>  // IWYU pragma: kepp

#include <modules/opengl/inviwoopengl.h>  // for GLenum

#include <cstddef>      // for size_t
#include <memory>       // for unique_ptr
#include <string_view>  // for string_view

namespace inviwo {

class IsoTFProperty;
class Mesh;
class Shader;
class TextureBase;
class TextureUnit;
class TextureUnitContainer;
class TransferFunctionProperty;

namespace utilgl {

IVW_MODULE_OPENGL_API void activateTarget(Image& targetImage,
                                          ImageType type = ImageType::AllLayers);
IVW_MODULE_OPENGL_API void activateTarget(ImageOutport& targetOutport,
                                          ImageType type = ImageType::AllLayers);
IVW_MODULE_OPENGL_API void activateAndClearTarget(Image& targetImage,
                                                  ImageType type = ImageType::AllLayers);
IVW_MODULE_OPENGL_API void activateAndClearTarget(ImageOutport& targetOutport,
                                                  ImageType type = ImageType::AllLayers);

IVW_MODULE_OPENGL_API void activateTargetAndCopySource(Image& targetImage, const Image& sourceImage,
                                                       ImageType type = ImageType::AllLayers);
IVW_MODULE_OPENGL_API void activateTargetAndCopySource(ImageOutport& targetOutport,
                                                       const Image& sourceImage,
                                                       ImageType type = ImageType::AllLayers);

IVW_MODULE_OPENGL_API void activateTargetAndCopySource(Image& targetImage,
                                                       const ImageInport& sourceInport,
                                                       ImageType type = ImageType::AllLayers);
IVW_MODULE_OPENGL_API void activateTargetAndCopySource(ImageOutport& targetOutport,
                                                       const ImageInport& sourceInport,
                                                       ImageType type = ImageType::AllLayers);

IVW_MODULE_OPENGL_API void activateTargetAndClearOrCopySource(
    Image& targetImage, const ImageInport& sourceInport, ImageType type = ImageType::AllLayers);

IVW_MODULE_OPENGL_API void activateTargetAndClearOrCopySource(
    ImageOutport& targetOutport, const ImageInport& sourceInport,
    ImageType type = ImageType::AllLayers);

IVW_MODULE_OPENGL_API void clearCurrentTarget();
IVW_MODULE_OPENGL_API void deactivateCurrentTarget();

IVW_MODULE_OPENGL_API void updateAndActivateTarget(ImageOutport& targetOutport,
                                                   ImageInport& sourceInport);

// Bind textures with glenum
IVW_MODULE_OPENGL_API void bindTexture(const Layer& layer, GLenum texUnit);
IVW_MODULE_OPENGL_API void bindTexture(const LayerInport& inport, GLenum texUnit);
IVW_MODULE_OPENGL_API void bindTexture(const LayerOutport& outport, GLenum texUnit);

IVW_MODULE_OPENGL_API void bindTextures(const Image& image, bool color, bool depth, bool picking,
                                        GLenum colorTexUnit, GLenum depthTexUnit,
                                        GLenum pickingTexUnit);

IVW_MODULE_OPENGL_API void bindColorTexture(const Image& image, GLenum texUnit);
IVW_MODULE_OPENGL_API void bindColorTexture(const ImageInport& inport, GLenum texUnit);
IVW_MODULE_OPENGL_API void bindColorTexture(const ImageOutport& outport, GLenum texUnit);

IVW_MODULE_OPENGL_API void bindDepthTexture(const Image& image, GLenum texUnit);
IVW_MODULE_OPENGL_API void bindDepthTexture(const ImageInport& inport, GLenum texUnit);
IVW_MODULE_OPENGL_API void bindDepthTexture(const ImageOutport& outport, GLenum texUnit);

IVW_MODULE_OPENGL_API void bindPickingTexture(const Image& image, GLenum texUnit);
IVW_MODULE_OPENGL_API void bindPickingTexture(const ImageInport& inport, GLenum texUnit);
IVW_MODULE_OPENGL_API void bindPickingTexture(const ImageOutport& outport, GLenum texUnit);

IVW_MODULE_OPENGL_API void bindTextures(const Image& image, GLenum colorTexUnit,
                                        GLenum depthTexUnit);
IVW_MODULE_OPENGL_API void bindTextures(const ImageInport& inport, GLenum colorTexUnit,
                                        GLenum depthTexUnit);
IVW_MODULE_OPENGL_API void bindTextures(const ImageOutport& outport, GLenum colorTexUnit,
                                        GLenum depthTexUnit);

IVW_MODULE_OPENGL_API void bindTextures(const Image& image, GLenum colorTexUnit,
                                        GLenum depthTexUnit, GLenum pickingTexUnit);
IVW_MODULE_OPENGL_API void bindTextures(const ImageInport& inport, GLenum colorTexUnit,
                                        GLenum depthTexUnit, GLenum pickingTexUnit);
IVW_MODULE_OPENGL_API void bindTextures(const ImageOutport& outport, GLenum colorTexUnit,
                                        GLenum depthTexUnit, GLenum pickingTexUnit);

// Bind textures with TextureUnit
IVW_MODULE_OPENGL_API void bindTexture(const Layer& layer, const TextureUnit& texUnit);
IVW_MODULE_OPENGL_API void bindTexture(const LayerInport& inport, const TextureUnit& texUnit);
IVW_MODULE_OPENGL_API void bindTexture(const LayerOutport& outport, const TextureUnit& texUnit);

IVW_MODULE_OPENGL_API void bindColorTexture(const Image& image, const TextureUnit& texUnit);
IVW_MODULE_OPENGL_API void bindColorTexture(const ImageInport& inport, const TextureUnit& texUnit);
IVW_MODULE_OPENGL_API void bindColorTexture(const ImageOutport& outport,
                                            const TextureUnit& texUnit);
IVW_MODULE_OPENGL_API void bindDepthTexture(const Image& image, const TextureUnit& texUnit);
IVW_MODULE_OPENGL_API void bindDepthTexture(const ImageInport& inport, const TextureUnit& texUnit);
IVW_MODULE_OPENGL_API void bindDepthTexture(const ImageOutport& outport,
                                            const TextureUnit& texUnit);
IVW_MODULE_OPENGL_API void bindPickingTexture(const Image& image, const TextureUnit& texUnit);
IVW_MODULE_OPENGL_API void bindPickingTexture(const ImageInport& inport,
                                              const TextureUnit& texUnit);
IVW_MODULE_OPENGL_API void bindPickingTexture(const ImageOutport& outport,
                                              const TextureUnit& texUnit);

IVW_MODULE_OPENGL_API void bindTextures(const Image& image, const TextureUnit& colorTexUnit,
                                        const TextureUnit& depthTexUnit);
IVW_MODULE_OPENGL_API void bindTextures(const ImageInport& inport, const TextureUnit& colorTexUnit,
                                        const TextureUnit& depthTexUnit);
IVW_MODULE_OPENGL_API void bindTextures(const ImageOutport& outport,
                                        const TextureUnit& colorTexUnit,
                                        const TextureUnit& depthTexUnit);

IVW_MODULE_OPENGL_API void bindTextures(const Image& image, const TextureUnit& colorTexUnit,
                                        const TextureUnit& depthTexUnit,
                                        const TextureUnit& pickingTexUnit);
IVW_MODULE_OPENGL_API void bindTextures(const ImageInport& inport, const TextureUnit& colorTexUnit,
                                        const TextureUnit& depthTexUnit,
                                        const TextureUnit& pickingTexUnit);
IVW_MODULE_OPENGL_API void bindTextures(const ImageOutport& outport,
                                        const TextureUnit& colorTexUnit,
                                        const TextureUnit& depthTexUnit,
                                        const TextureUnit& pickingTexUnit);

// Unbind textures
IVW_MODULE_OPENGL_API void unbindTexture(const Layer& layer);
IVW_MODULE_OPENGL_API void unbindTexture(const LayerInport& inport);
IVW_MODULE_OPENGL_API void unbindTexture(const LayerOutport& outport);

IVW_MODULE_OPENGL_API void unbindTextures(const Image& image, bool color, bool depth, bool picking);

IVW_MODULE_OPENGL_API void unbindColorTexture(const ImageInport& inport);
IVW_MODULE_OPENGL_API void unbindColorTexture(const ImageOutport& outport);
IVW_MODULE_OPENGL_API void unbindDepthTexture(const ImageInport& inport);
IVW_MODULE_OPENGL_API void unbindDepthTexture(const ImageOutport& outport);
IVW_MODULE_OPENGL_API void unbindPickingTexture(const ImageInport& inport);
IVW_MODULE_OPENGL_API void unbindPickingTexture(const ImageOutport& outport);

IVW_MODULE_OPENGL_API void unbindTextures(const Image& image);
IVW_MODULE_OPENGL_API void unbindTextures(const ImageInport& inport);
IVW_MODULE_OPENGL_API void unbindTextures(const ImageOutport& outport);

// convenience texture bindings
IVW_MODULE_OPENGL_API void bindTexture(const TextureBase& texture, GLenum texUnit);
IVW_MODULE_OPENGL_API void bindTexture(const TextureBase& texture, const TextureUnit& texUnit);

IVW_MODULE_OPENGL_API void bindAndSetUniforms(Shader& shader, TextureUnitContainer& cont,
                                              const TextureBase& texture,
                                              std::string_view samplerID);

IVW_MODULE_OPENGL_API void bindTexture(TransferFunctionProperty& tf, const TextureUnit& texUnit);

IVW_MODULE_OPENGL_API void bindAndSetUniforms(Shader& shader, TextureUnitContainer& cont,
                                              TransferFunctionProperty& tf);

IVW_MODULE_OPENGL_API void bindTexture(IsoTFProperty& property, const TextureUnit& texUnit);

IVW_MODULE_OPENGL_API void bindAndSetUniforms(Shader& shader, TextureUnitContainer& cont,
                                              IsoTFProperty& property);

// Volume texture bindings
IVW_MODULE_OPENGL_API void bindTexture(const Volume& volume, const TextureUnit& texUnit);
IVW_MODULE_OPENGL_API void bindTexture(const VolumeInport& inport, const TextureUnit& texUnit);

// Shader defines.
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader, const Layer& layer,
                                             std::string_view samplerID);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader, const LayerInport& inport,
                                             std::string_view samplerID = "");
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader, const LayerOutport& outport,
                                             std::string_view samplerID = "");

IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader, const Image& image,
                                             std::string_view samplerID);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader, const ImageInport& inport,
                                             std::string_view samplerID = "");
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader, const ImageOutport& outport,
                                             std::string_view samplerID = "");

// Draw image plane.

/**
 * Creates an indexed triangle strip mesh
 * with points and texture coordinates.
 *  (-1,1)----(1, 1)
 *    |         |
 *  (-1,1)----(1,-1)
 *
 * @return std::unique_ptr<Mesh>
 */
IVW_MODULE_OPENGL_API std::unique_ptr<Mesh> planeRect();
IVW_MODULE_OPENGL_API void singleDrawImagePlaneRect();
IVW_MODULE_OPENGL_API void multiDrawImagePlaneRect(int instances);

// convenience Image port binding and setting uniforms
IVW_MODULE_OPENGL_API void bindAndSetUniforms(Shader& shader, TextureUnitContainer& cont,
                                              const Layer& layer, std::string_view id);
IVW_MODULE_OPENGL_API void bindAndSetUniforms(Shader& shader, TextureUnitContainer& cont,
                                              const LayerInport& inport);
IVW_MODULE_OPENGL_API void bindAndSetUniforms(Shader& shader, TextureUnitContainer& cont,
                                              const LayerOutport& outport);

IVW_MODULE_OPENGL_API void bindAndSetUniforms(Shader& shader, TextureUnitContainer& cont,
                                              const Image& image, std::string_view id,
                                              ImageType type);
IVW_MODULE_OPENGL_API void bindAndSetUniforms(Shader& shader, TextureUnitContainer& cont,
                                              const ImageInport& image, ImageType type);
IVW_MODULE_OPENGL_API void bindAndSetUniforms(Shader& shader, TextureUnitContainer& cont,
                                              const ImageOutport& image, ImageType type);
}  // namespace utilgl
}  // namespace inviwo
