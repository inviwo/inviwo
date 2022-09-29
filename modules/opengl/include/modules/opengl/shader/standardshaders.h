/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2022 Inviwo Foundation
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

#include <memory>                               // for shared_ptr
#include <utility>                              // for pair

namespace inviwo {
class ShaderResource;
class ShaderType;

namespace utilgl {

/**
 * Standard minimal vertex shader resource, used if no vertex shader exits in shader.h
 */
IVW_MODULE_OPENGL_API std::pair<ShaderType, std::shared_ptr<const ShaderResource>>
imgIdentityVert();

/**
 * Standard vertex shader resource for a quad
 */
IVW_MODULE_OPENGL_API std::pair<ShaderType, std::shared_ptr<const ShaderResource>> imgQuadVert();

/**
 * Standard vertex shader resource for a quad
 */
IVW_MODULE_OPENGL_API std::pair<ShaderType, std::shared_ptr<const ShaderResource>> imgQuadFrag();

}  // namespace utilgl

}  // namespace inviwo
