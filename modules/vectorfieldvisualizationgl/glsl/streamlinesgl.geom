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

#extension GL_EXT_gpu_shader4: enable
#extension GL_EXT_geometry_shader4: enable
#extension GL_EXT_geometry_shader4: enable  


#include "utils/sampler3d.glsl"
#include "utils/structs.glsl"

uniform GeometryParameters geometry;
uniform CameraParameters camera;

uniform VolumeParameters vectorvolumeParameters;
uniform sampler3D vectorvolume;
uniform float stepSize;

uniform mat3 invbasis;
 
layout(points) in;
layout(line_strip, max_vertices = MAX_VERTEX) out;

out float mag_;

void vertex2(vec3 v){
    mat4 m = camera.worldToClip * vectorvolumeParameters.textureToWorld;
    gl_Position =  m * vec4(v,1.0);
    EmitVertex();
}

void vertex(vec3 v){
    mat4 m = camera.worldToClip;
    gl_Position =  m * vec4(v,1.0);
    EmitVertex();
}




void step(int dir){
    vec3 pos = (gl_in[0].gl_Position * vectorvolumeParameters.textureToWorld).xyz;
    vertex(pos);
    float stepL = stepSize * dir; 
    for(int i = 0;i<STEPS;i++){
        for(int j = 0;j<STRIDE;j++){ 
            vec3 worldVelocity = getVoxel(vectorvolume, vectorvolumeParameters, 
                            (vectorvolumeParameters.worldToTexture * vec4(pos,1.0)).xyz).xyz;
			mag_ = length(worldVelocity);
			worldVelocity = normalize(worldVelocity) * stepL;

            pos +=  worldVelocity;
            if(pos.x < 0) return;
            if(pos.y < 0) return;
            if(pos.z < 0) return;
            if(pos.x > 1) return;
            if(pos.y > 1) return;
            if(pos.z > 1) return;
        }
        vertex(pos);
    }
}




void step2(int dir){
    vec3 pos = gl_in[0].gl_Position.xyz;
    vertex(pos);
    float stepL = stepSize * dir;
    for(int i = 0;i<STEPS;i++){
        for(int j = 0;j<STRIDE;j++){ 
            vec3 worldVelocity = getVoxel(vectorvolume, vectorvolumeParameters, pos).xyz;
			mag_ = length(worldVelocity);
			worldVelocity = normalize(worldVelocity);


            vec3 vel = invbasis * ( worldVelocity * stepL);
            pos +=  vel;
            if(pos.x < 0) return;
            if(pos.y < 0) return;
            if(pos.z < 0) return;
            if(pos.x > 1) return;
            if(pos.y > 1) return;
            if(pos.z > 1) return;
        }
        vertex(pos);
    }
}





void main(){
   
    #ifdef FWD
    step(1);
    EndPrimitive();
    #endif
    #ifdef BWD
    step(-1);
    EndPrimitive();
    #endif
    
}
