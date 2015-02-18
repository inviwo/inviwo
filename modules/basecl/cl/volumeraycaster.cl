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

#include "samplers.cl"
#include "gradients.cl"
#include "transformations.cl"
#include "shading/shading.cl"

__constant float REF_SAMPLING_INTERVAL = 150.f;
#define ERT_THRESHOLD 1.0

__kernel void raycaster(read_only image3d_t volume, __constant VolumeParameters* volumeParams
                        , read_only image2d_t entryPoints 
                        , read_only image2d_t exitPoints
                        , read_only image2d_t transferFunction 
                        , float3 cameraPosition
                        , float samplingRate
                        , __constant LightParameters* light
                        , write_only image2d_t output
                        , int2 outputRegionOffset
                        , int2 outputRegionSize) 
{
    int2 globalId = (int2)(get_global_id(0), get_global_id(1));      

    if (any(globalId >= outputRegionSize)) { 
        return;
    }

    float4 entry = read_imagef(entryPoints, smpUNormNoClampNearest, globalId);   
    float4 exit = read_imagef(exitPoints, smpUNormNoClampNearest, globalId);
    float3 direction = exit.xyz - entry.xyz;   
    float tEnd = fast_length(direction); 

    float4 result = (float4)(0.f); 
    if(tEnd > 0.f) {     
        float tIncr = min(tEnd, tEnd/(samplingRate*length(direction*convert_float3(get_image_dim(volume).xyz)))); 
        direction = fast_normalize(direction);
        float samples = ceil(tEnd/tIncr);
        tIncr = tEnd/samples; 
        // Start integrating at the center of the bins
        float t = 0.5f*tIncr; 
        float4 emissionAbsorption;
        float3 toCameraDir = normalize(cameraPosition - transformPoint(volumeParams->textureToWorld, entry.xyz));
        while(t < tEnd) {
            float3 pos = entry.xyz+t*direction;
            float volumeSample = getNormalizedVoxel(volume, volumeParams, as_float4(pos)).x; 
            // xyz == emission, w = absorption
            emissionAbsorption = read_imagef(transferFunction, smpNormClampEdgeLinear, (float2)(volumeSample, 0.5f));
            float3 gradient = gradientCentralDiff(volume, volumeParams, as_float4(pos));
            gradient = normalize(gradient);

            // Shading
            // World space position
            float3 worldSpacePosition = transformPoint(volumeParams->textureToWorld, pos);
            // Note that the gradient is reversed since we define the normal of a surface as
            // the direction towards a lower intensity medium (gradient points in the inreasing direction)
            #if SHADING_MODE == 1
                    emissionAbsorption.xyz = shadeAmbient(*light, emissionAbsorption.xyz);
            #elif SHADING_MODE == 2
                    emissionAbsorption.xyz = shadeDiffuse(*light, emissionAbsorption.xyz, worldSpacePosition, -gradient);
            #elif SHADING_MODE == 3
                    emissionAbsorption.xyz = shadeSpecular(*light, (float3)(1.f), worldSpacePosition, -gradient, toCameraDir);
            #elif SHADING_MODE == 4
                   emissionAbsorption.xyz = shadeBlinnPhong(*light,  emissionAbsorption.xyz, emissionAbsorption.xyz, (float3)(1.f), worldSpacePosition, -gradient, toCameraDir);
            #elif SHADING_MODE == 5
                   emissionAbsorption.xyz = shadePhong(*light, emissionAbsorption.xyz, emissionAbsorption.xyz, (float3)(1.f), worldSpacePosition, -gradient, toCameraDir);
            #endif
            
            // Taylor expansion approximation
            float opacity = 1.f - native_powr(1.f - emissionAbsorption.w, tIncr * REF_SAMPLING_INTERVAL);
			result.xyz = result.xyz + (1.f - result.w) * opacity * emissionAbsorption.xyz;
            result.w = result.w + (1.f - result.w) * opacity;	

            if (result.w > ERT_THRESHOLD) t = tEnd;   
            else t += tIncr;   
        }

    }
         
    write_imagef(output, outputRegionOffset+globalId,  result);     
  
}
  