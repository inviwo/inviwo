/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

 /*-----------------------------------------------------------------------
  Copyright (c) 2014, NVIDIA. All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Neither the name of its contributors may be used to endorse 
     or promote products derived from this software without specific
     prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
  PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
-----------------------------------------------------------------------*/

layout(location=0) uniform vec4      info; // xy
vec2 uvOffset = info.xy;
vec2 invResolution = info.zw;

layout(binding=0)  uniform sampler2D texLinearDepth;

// In accordance with GLSL specification Section 4.4.2, output variables cannot be 
// assigned to the same location.
//
// Since the outputs FragData0 and PickingData are preset to locations 0 and 2, 
// additional outputs must start at location 2.
layout(location=2,index=0) out float out_Color[6];


//----------------------------------------------------------------------------------

#if 1
void main() {
  vec2 uv = floor(gl_FragCoord.xy) * 4.0 + uvOffset + 0.5;
  uv *= invResolution;  
  
  vec4 S0 = textureGather(texLinearDepth, uv, 0);
  vec4 S1 = textureGatherOffset(texLinearDepth, uv, ivec2(2,0), 0);
  
  // fill default output, i.e. FragData0 and PickingData, first
  FragData0 = vec4(S0.w);
  PickingData = vec4(S0.z);
  // fill additional output variables
  out_Color[0] = S1.w;
  out_Color[1] = S1.z;
  out_Color[2] = S0.x;
  out_Color[3] = S0.y;
  out_Color[4] = S1.x;
  out_Color[5] = S1.y;
}
#else
void main() {
  vec2 uv = floor(gl_FragCoord.xy) * 4.0 + uvOffset;
  ivec2 tc = ivec2(uv);

  // fill default output, i.e. FragData0 and PickingData, first
  FragData0 = vec4(texelFetchOffset(texLinearDepth, tc, 0, ivec2(0,0)).x);
  PickingData = vec4(texelFetchOffset(texLinearDepth, tc, 0, ivec2(1,0)).x);
  // fill additional output variables
  out_Color[0] = texelFetchOffset(texLinearDepth, tc, 0, ivec2(2,0)).x;
  out_Color[1] = texelFetchOffset(texLinearDepth, tc, 0, ivec2(3,0)).x;
  out_Color[2] = texelFetchOffset(texLinearDepth, tc, 0, ivec2(0,1)).x;
  out_Color[3] = texelFetchOffset(texLinearDepth, tc, 0, ivec2(1,1)).x;
  out_Color[4] = texelFetchOffset(texLinearDepth, tc, 0, ivec2(2,1)).x;
  out_Color[5] = texelFetchOffset(texLinearDepth, tc, 0, ivec2(3,1)).x;
}

#endif
