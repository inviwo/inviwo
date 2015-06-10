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

#include "volumecombiner.h"
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/glwrap/textureunit.h>
#include <modules/opengl/glwrap/shader.h>
#include <modules/opengl/volumeutils.h>

namespace inviwo {

ProcessorClassIdentifier(VolumeCombiner, "org.inviwo.VolumeCombiner");
ProcessorDisplayName(VolumeCombiner, "Volume Combiner");
ProcessorTags(VolumeCombiner, Tags::GL);
ProcessorCategory(VolumeCombiner, "Volume Operation");
ProcessorCodeState(VolumeCombiner, CODE_STATE_EXPERIMENTAL);

VolumeCombiner::VolumeCombiner() 
    : VolumeGLProcessor("volume_combiner.frag")
    , vol2_("vol2") 
    , scaleVol1_("scaleVol1", "Volume 1 data scaling", 1.0f, 0.0f, 100.0f)
    , scaleVol2_("scaleVol2", "Volume 2 data scaling", 1.0f, 0.0f, 100.0f)
{
    addPort(vol2_);
    addProperty(scaleVol1_);
    addProperty(scaleVol2_);
}

VolumeCombiner::~VolumeCombiner() {}

void VolumeCombiner::preProcess() {
    TextureUnit vol2Unit;

    const VolumeGL* volGL = vol2_.getData()->getRepresentation<VolumeGL>();
    volGL->bindTexture(vol2Unit.getEnum());

    shader_->setUniform("volume2_", vol2Unit.getUnitNumber());
    utilgl::setShaderUniforms(shader_, vol2_, "volume2Parameters_");

    shader_->setUniform("scaleVol1_", scaleVol1_.get());
    shader_->setUniform("scaleVol2_", scaleVol2_.get());
}

}  // namespace
