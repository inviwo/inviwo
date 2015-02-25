/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include <modules/basegl/baseglmodule.h>
#include <modules/opengl/glwrap/shadermanager.h>
#include <modules/basegl/processors/background.h>
#include <modules/basegl/processors/drawlines.h>
#include <modules/basegl/processors/drawpoints.h>
#include <modules/basegl/processors/entryexitpoints.h>
#include <modules/basegl/processors/findedges.h>
#include <modules/basegl/processors/firstivwprocessor.h>
#include <modules/basegl/processors/heightfieldprocessor.h>
#include <modules/basegl/processors/imagecompositeprocessorgl.h>
#include <modules/basegl/processors/imagelayoutgl.h>
#include <modules/basegl/processors/imagemixer.h>
#include <modules/basegl/processors/lightingraycaster.h>
#include <modules/basegl/processors/lightvolumegl.h>
#include <modules/basegl/processors/multichannelraycaster.h>
#include <modules/basegl/processors/geometrypicking.h>
#include <modules/basegl/processors/redgreenprocessor.h>
#include <modules/basegl/processors/volumeraycaster.h>
#include <modules/basegl/processors/isoraycaster.h>
#include <modules/basegl/processors/volumeslicegl.h>
#include <modules/basegl/processors/imageprocessing/imagegamma.h>
#include <modules/basegl/processors/imageprocessing/imagegrayscale.h>
#include <modules/basegl/processors/imageprocessing/imageinvert.h>
#include <modules/basegl/processors/imageprocessing/imagemapping.h>
#include <modules/basegl/processors/imageprocessing/imageresample.h>
#ifdef BASEGL_INCLUDE_SHADER_RESOURCES
#include <modules/basegl/shader_resources.h>
#endif

namespace inviwo {

BaseGLModule::BaseGLModule() : InviwoModule() {
    setIdentifier("BaseGL");

#ifdef BASEGL_INCLUDE_SHADER_RESOURCES
    BASEGL_addGeneratedShaderResources();
#else
    ShaderManager::getPtr()->addShaderSearchPath(InviwoApplication::PATH_MODULES, "basegl/glsl");
#endif
    
    registerProcessor(Background);
    registerProcessor(DrawLines);
    registerProcessor(DrawPoints);
    registerProcessor(EntryExitPoints);
    registerProcessor(FindEdges);
    registerProcessor(FirstIvwProcessor);
    registerProcessor(GeometryPicking);
    registerProcessor(HeightFieldProcessor);
    registerProcessor(ImageCompositeProcessorGL);
    registerProcessor(ImageLayoutGL);
    registerProcessor(ImageMixer);
    registerProcessor(LightingRaycaster);
    registerProcessor(LightVolumeGL);
    registerProcessor(MultichannelRaycaster);
    registerProcessor(RedGreenProcessor);
    registerProcessor(VolumeRaycaster);
    registerProcessor(ISORaycaster);
    registerProcessor(VolumeSliceGL);

    // image processing
    registerProcessor(ImageGamma);
    registerProcessor(ImageGrayscale);
    registerProcessor(ImageInvert);
    registerProcessor(ImageMapping);
    registerProcessor(ImageResample);
}

} // namespace
