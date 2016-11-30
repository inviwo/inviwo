
#define VERTEX_POS    0
#define VERTEX_NORMAL 1
#define VERTEX_COLOR  2

#define UBO_SCENE     0

#define AO_RANDOMTEX_SIZE 4

#ifdef __cplusplus
namespace ssao
{
#endif

struct SceneData {
  mat4  viewProjMatrix;
  mat4  viewMatrix;
  mat4  viewMatrixIT;
  
  uvec2 viewport;
  uvec2 _pad;
};

struct HBAOData {
  float   RadiusToScreen;        // radius
  float   R2;     // 1/radius
  float   NegInvR2;     // radius * radius
  float   NDotVBias;
 
  vec2    InvFullResolution;
  vec2    InvQuarterResolution;
  
  float   AOMultiplier;
  float   PowExponent;
  vec2    _pad0;
  
  vec4    projInfo;
  vec2    projScale;
  int     projOrtho;
  int     _pad1;
  
  vec4    float2Offsets[AO_RANDOMTEX_SIZE*AO_RANDOMTEX_SIZE];
  vec4    jitters[AO_RANDOMTEX_SIZE*AO_RANDOMTEX_SIZE];
};

#ifdef __cplusplus
}
#endif

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