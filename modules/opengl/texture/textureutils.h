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

#ifndef IVW_TEXTUREUTILS_H
#define IVW_TEXTUREUTILS_H

#include <modules/opengl/openglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/volumeport.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/texture/texture.h>
#include <modules/opengl/texture/textureunit.h>
#include <inviwo/core/properties/transferfunctionproperty.h>

namespace inviwo {

class BufferObjectArray;
class Shader;

namespace utilgl {

IVW_MODULE_OPENGL_API void activateTarget(Image& image, ImageType type = ALL_LAYERS);
IVW_MODULE_OPENGL_API void activateTarget(ImageOutport& outport, ImageType type = ALL_LAYERS);
IVW_MODULE_OPENGL_API void activateAndClearTarget(Image& image,
                                                  ImageType type = ALL_LAYERS);
IVW_MODULE_OPENGL_API void activateAndClearTarget(ImageOutport& outport,
                                                  ImageType type = ALL_LAYERS);

IVW_MODULE_OPENGL_API void activateTargetAndCopySource(ImageOutport& outport, ImageInport& inport,
                                                       ImageType type = ALL_LAYERS);

IVW_MODULE_OPENGL_API void clearCurrentTarget();
IVW_MODULE_OPENGL_API void deactivateCurrentTarget();

IVW_MODULE_OPENGL_API void updateAndActivateTarget(ImageOutport& outport, ImageInport& inport);

// Bind textures with glenum
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
IVW_MODULE_OPENGL_API void bindTexture(const Texture& texture, GLenum texUnit);
IVW_MODULE_OPENGL_API void bindTexture(const Texture& texture, const TextureUnit& texUnit);

IVW_MODULE_OPENGL_API void bindAndSetUniforms(Shader& shader, TextureUnitContainer& cont,
                                              const Texture& texture, const std::string samplerID);

IVW_MODULE_OPENGL_API void bindTexture(const TransferFunctionProperty& tf,
                                       const TextureUnit& texUnit);

IVW_MODULE_OPENGL_API void bindAndSetUniforms(Shader& shader, TextureUnitContainer& cont,
                                              const TransferFunctionProperty& tf);

// Volume texture bindings
IVW_MODULE_OPENGL_API void bindTexture(const Volume& volume, const TextureUnit& texUnit);
IVW_MODULE_OPENGL_API void bindTexture(const VolumeInport& inport, const TextureUnit& texUnit);

// Shader defines.
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader, const Image& image,
                                             const std::string samplerID);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader, const ImageInport& inport,
                                             const std::string samplerID = "");
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader, const ImageOutport& outport,
                                             const std::string samplerID = "");

// Draw image plane.
IVW_MODULE_OPENGL_API BufferObjectArray* enableImagePlaneRect();

IVW_MODULE_OPENGL_API void disableImagePlaneRect(BufferObjectArray*);

IVW_MODULE_OPENGL_API void singleDrawImagePlaneRect();

IVW_MODULE_OPENGL_API void multiDrawImagePlaneRect(int instances);

// convenience Image port binding and setting uniforms
IVW_MODULE_OPENGL_API void bindAndSetUniforms(Shader& shader, TextureUnitContainer& cont,
                                              const Image& image, const std::string& id,
                                              ImageType type);
IVW_MODULE_OPENGL_API void bindAndSetUniforms(Shader& shader, TextureUnitContainer& cont,
                                              ImageInport& image, ImageType type);
IVW_MODULE_OPENGL_API void bindAndSetUniforms(Shader& shader, TextureUnitContainer& cont,
                                              ImageOutport& image, ImageType type);
}
}  // namespace

#endif  // IVW_TEXTUREUTILS_H
