/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2025 Inviwo Foundation
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

#include <modules/oit/oitmodule.h>

#include <inviwo/core/common/inviwomodule.h>              // for InviwoModule, Mod...
#include <inviwo/core/ports/outportiterable.h>            // for OutportIterableIm...
#include <inviwo/core/util/exception.h>                   // for Exception
#include <modules/oit/ports/rasterizationport.h>          // for RasterizationOutport
#include <modules/oit/processors/calcnormalsprocessor.h>  // for CalcNormalsProcessor
#include <modules/oit/processors/linerasterizer.h>        // for LineRasterizer
#include <modules/oit/processors/meshrasterizer.h>        // for MeshRasterizer
#include <modules/oit/processors/meshvolumerenderer.h>
#include <modules/oit/processors/rasterizationrenderer.h>  // for RasterizationRend...
#include <modules/oit/processors/rasterizer.h>
#include <modules/oit/processors/sphererasterizer.h>        // for SphereRasterizer
#include <modules/oit/processors/transformrasterization.h>  // for TransformRasteriz...
#include <modules/oit/processors/volumerasterizer.h>
#include <modules/opengl/shader/shadermanager.h>  // for ShaderManager

#include <functional>  // for __base
#include <memory>      // for unique_ptr

#include <fmt/core.h>  // for basic_string_view

namespace inviwo {
class InviwoApplication;

OITModule::OITModule(InviwoApplication* app) : InviwoModule(app, "Oit") {

    // Add a directory to the search path of the Shadermanager
    ShaderManager::getPtr()->addShaderSearchPath(getPath(ModulePath::GLSL));

    // Processors
    registerProcessor<LineRasterizer>();
    registerProcessor<MeshRasterizer>();
    registerProcessor<RasterizationRenderer>();
    registerProcessor<MeshVolumeRenderer>();
    registerProcessor<TransformRasterization>();
    registerProcessor<CalcNormalsProcessor>();
    registerProcessor<SphereRasterizer>();
    registerProcessor<VolumeRasterizer>();

    // Ports
    registerPort<RasterizationInport>();
    registerPort<RasterizationOutport>();
}

}  // namespace inviwo
