/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include "utils/structs.glsl"
#include "utils/sampler3d.glsl"
#include "utils/classification.glsl"
#include "utils/pickingutils.glsl"

uniform sampler3D volume;
uniform VolumeParameters volumeParameters;

uniform vec3 pickColor = vec3(0.0, 0.0, 0.0);

#ifdef BACKGROUND_AVAILABLE
uniform ImageParameters backgroundParameters;
uniform sampler2D backgroundColor;
uniform sampler2D backgroundPicking;
uniform sampler2D backgroundDepth;
#endif

uniform sampler2D transferFunction;

in vec3 texCoord;

void main() {
    vec4 voxel = getNormalizedVoxel(volume, volumeParameters, texCoord);
    voxel = applyTF(transferFunction, voxel);
    PickingData = vec4(pickColor,1.0);

#ifdef BACKGROUND_AVAILABLE
    vec4 background = texture(backgroundColor, gl_FragCoord.xy * backgroundParameters.reciprocalDimensions);
    voxel = mix(background, voxel, voxel.a);
    voxel.a = max(background.a, voxel.a);
#endif // BACKGROUND_AVAILABLE    

    FragData0 = voxel;
}
