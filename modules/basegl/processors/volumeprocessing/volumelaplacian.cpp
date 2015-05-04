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

#include "volumelaplacian.h"

#include <modules/opengl/volume/volumegl.h>

namespace inviwo {

ProcessorClassIdentifier(VolumeLaplacian, "org.inviwo.Volume Laplacian");
ProcessorDisplayName(VolumeLaplacian,  "Volume Laplacian");
ProcessorTags(VolumeLaplacian, Tags::None);
ProcessorCategory(VolumeLaplacian, "Volume Operation");
ProcessorCodeState(VolumeLaplacian, CODE_STATE_BROKEN);

VolumeLaplacian::VolumeLaplacian()
    : VolumeGLProcessor("volume_laplacian.frag") {
}

void VolumeLaplacian::process() {
    //VolumeGLProcessor::process();

    /*
    switch (inport_.getData()->getDataFormat()->getId()) {
    #define DataFormatIdMacro(i) \
    case i: \
        processRepresentation<Data##i::type, Data##i::bits>(); \
        break;
    #include <inviwo/core/util/formatsdefinefunc.h>

    default:
        break;
    }
    }
    */

    CallFunctionWithTemplateArgsForType(processRepresentation, inport_.getData()->getDataFormat()->getId());
}

} // namespace

