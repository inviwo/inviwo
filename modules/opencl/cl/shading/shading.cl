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

#ifndef SHADING_CL
#define SHADING_CL

#include "shading/phasefunctions.cl" 
#include "shading/blinnphongbrdf.cl"
#include "shading/cooktorrancebrdf.cl"
#include "shading/ashikiminbrdf.cl"
#include "shading/wardbrdf.cl"
#include "shading/abcbrdf.cl"
#include "datastructures/lightsource.cl"

#include "gradients.cl"
#include "samplers.cl"  
// Take a look at this pdf, it explains many of the models used:
// http://www.cs.ucl.ac.uk/staff/j.kautz/GameCourse/04_PointLights.pdf


// Helper functions to calculate the shading
float3 shadeDiffuseCalculation(LightParameters light_, float3 materialDiffuseColor, float3 normal,
                             float3 toLightDir) {
    return materialDiffuseColor * light_.diffuseColor * max(dot(normal, toLightDir), 0.0f);
}

float3 shadeSpecularBlinnPhongCalculation(LightParameters light_, float3 materialSpecularColor,
                                        float3 normal, float3 toLightDir, float3 toCameraDir) {
    float3 halfway = toCameraDir + toLightDir;

    // check for special case where the light source is exactly opposite
    // to the view direction, i.e. the length of the halfway vector is zero
    if (dot(halfway, halfway) < 1.e-6f) {  // check for squared length
        return (float3)(0.0f);
    } else {
        halfway = normalize(halfway);
        return materialSpecularColor * light_.specularColor *
               native_powr(max(dot(normal, halfway), 0.f), light_.specularExponent);
    }
}

float3 shadeSpecularPhongCalculation(LightParameters light_, float3 materialSpecularColor, float3 normal,
                                   float3 toLightDir, float3 toCameraDir) {
    // Compute reflection (not that glsl uses incident direction)
    // Equivalent to: 2.0*dot(toLightDir, normal)*normal - toLightDir;
    float3 r = reflect(-toLightDir, normal);
   
    return materialSpecularColor * light_.specularColor *
           native_powr(max(dot(r, toCameraDir), 0.f), light_.specularExponent * 0.25f);
}

// Functions to apply different shading modes.
// All positions and directions should be in world space!
float3 shadeAmbient(LightParameters light_, float3 materialAmbientColor) {
    return materialAmbientColor * light_.ambientColor;
}

float3 shadeDiffuse(LightParameters light_, float3 materialDiffuseColor, float3 position, float3 normal) {
    return shadeDiffuseCalculation(light_, materialDiffuseColor, normal,
                                   normalize(light_.position - position));
}

float3 shadeSpecular(LightParameters light_, float3 materialSpecularColor, float3 position, float3 normal,
                   float3 toCameraDir) {
    return shadeSpecularPhongCalculation(light_, materialSpecularColor, normal,
                                         normalize(light_.position - position), toCameraDir);
}

float3 shadeBlinnPhong(LightParameters light_, float3 materialAmbientColor, float3 materialDiffuseColor,
                     float3 materialSpecularColor, float3 position, float3 normal, float3 toCameraDir) {
    float3 toLightDir = normalize(light_.position - position);
    float3 resAmb = shadeAmbient(light_, materialAmbientColor);
    float3 resDiff = shadeDiffuseCalculation(light_, materialDiffuseColor, normal, toLightDir);
    float3 resSpec = shadeSpecularBlinnPhongCalculation(light_, materialSpecularColor, normal,
                                                      toLightDir, toCameraDir);
    return resAmb + resDiff + resSpec;
}

float3 shadePhong(LightParameters light_, float3 materialAmbientColor, float3 materialDiffuseColor,
                float3 materialSpecularColor, float3 position, float3 normal, float3 toCameraDir) {
    float3 toLightDir = normalize(light_.position - position);
    float3 resAmb = shadeAmbient(light_, materialAmbientColor);
    float3 resDiff = shadeDiffuseCalculation(light_, materialDiffuseColor, normal, toLightDir);
    float3 resSpec = shadeSpecularPhongCalculation(light_, materialSpecularColor, normal, toLightDir,
                                                 toCameraDir);
    return resAmb + resDiff + resSpec;
}

/////// Physically based shading //////////
#define PHASE_FUNCTION_HENYEY_GREENSTEIN 0
#define PHASE_FUNCTION_SCHLICK 1
#define PHASE_FUNCTION_BLINN_PHONG 2
#define PHASE_FUNCTION_WARD 3
#define PHASE_FUNCTION_COOK_TORRANCE 4
#define PHASE_FUNCTION_ABC_MICROFACET 5
#define PHASE_FUNCTION_ASHIKHMIN 6
#define PHASE_FUNCTION_MIX 7
#define PHASE_FUNCTION_ISOTROPIC 8


typedef enum ShadingType {
    HENYEY_GREENSTEIN=0,
    SCHLICK,
    BLINN_PHONG,
    WARD,
    COOK_TORRANCE,
    ABC_MICROFACET,
    ASHIKHMIN,
    MIX,
    ISOTROPIC
} ShadingType;

#define BLINN_EXPONENT 10.0f
#define GRADIENT_MAGNITUDE_THRESHOLD (MAX_GRAD_MAG*0.5f)
    // Material properties: 
    // x = fresnel factor f_0 = ((1-n)/(1+n))^2, 
    // y = exponent (blinn)
    // z = phase function scattering
    // w = Mix between phase function and BRDF (MIX shadingtype)
#define EXPONENT_SCALE 100.f

// Maps parameter x from 0-1. Used for exposure in Blinn-Phong and other parameters. 
inline float vrayParameterMapping(float x) {
    return 1.f/native_powr(1.f - x, 3.5f) - 1.f;
}
inline float WardParameterMapping(float x) {
    return fmax(x,1e-4f);
}
// Convert between the roughness m in Beckmann distribution and shininess exponent in Blinn-Phong distribution
float shininessToRoughness(float exponent) {
    return native_sqrt( 2.f/(exponent+2.f) );
}
float roughnessToShininess(float m) {
    return 2.f/(fmax(1e-4f, m*m))-2.f;
}
float2 BCParameterMapping(float2 bc) {
    return bc*(float2)(13000.f,1.f); 
}
float HenyehGreensteinToSchlick(float g) {
    // Should work for intermediate values of k in Schlick 
    return 1.55f*g-0.55f*pown(g, 3); 
    //return 1.5f*g-(1.f-1.5f)*g*g*g; 
}

inline float4 scaleShadingParameter(float4 material, const ShadingType shadingType) {
    float4 scaledMaterial = material;
#ifdef PERFORMANCE 
#if PHASE_FUNCTION == PHASE_FUNCTION_HENYEY_GREENSTEIN
    scaledMaterial.z = -1.f+2.f*material.z;  
#elif PHASE_FUNCTION == PHASE_FUNCTION_SCHLICK
    scaledMaterial.z = HenyehGreensteinToSchlick(-1.f+2.f*material.z); 
#elif PHASE_FUNCTION == PHASE_FUNCTION_BLINN_PHONG
    scaledMaterial.y = roughnessToShininess(material.y);  
#elif PHASE_FUNCTION == PHASE_FUNCTION_WARD
    scaledMaterial.y = WardParameterMapping(material.y);  
#elif PHASE_FUNCTION == PHASE_FUNCTION_COOK_TORRANCE
    scaledMaterial.y = WardParameterMapping(material.y); 
#elif PHASE_FUNCTION == PHASE_FUNCTION_ABC_MICROFACET
    scaledMaterial.yz = BCParameterMapping(material.yz);  
#elif PHASE_FUNCTION == PHASE_FUNCTION_ASHIKHMIN 
    scaledMaterial.y = roughnessToShininess(material.y);  
#elif PHASE_FUNCTION == PHASE_FUNCTION_MIX
    scaledMaterial.y = roughnessToShininess(material.y);  
#else  
    scaledMaterial.y = material.y; 
#endif
#else
    if ( shadingType == HENYEY_GREENSTEIN ) {
        scaledMaterial.z = -1.f+2.f*material.z;  
    }else if ( shadingType == SCHLICK ) {
        scaledMaterial.z = HenyehGreensteinToSchlick(-1.f+2.f*material.z); 
    }else if ( shadingType == BLINN_PHONG ) {
        scaledMaterial.y = roughnessToShininess(material.y);  
    } else if ( shadingType == WARD ) {
        scaledMaterial.y = WardParameterMapping(material.y);  
    } else if ( shadingType == COOK_TORRANCE ) {
        //scaledMaterial.y = roughnessToShininess(material.y);  
        scaledMaterial.y = WardParameterMapping(material.y); 
    } else if ( shadingType == ABC_MICROFACET ) {
        scaledMaterial.yz = BCParameterMapping(material.yz);  
    } else if ( shadingType == ASHIKHMIN ) {
        scaledMaterial.y = roughnessToShininess(material.y);  
    } else if ( shadingType == MIX ) {
        scaledMaterial.y = roughnessToShininess(material.y);  
    } else {
        scaledMaterial.y = material.y;  
    }
#endif
    // Scale index of refraction (IOR) f0 = ((n1-n2)/(n1+n2))^2, where n1 = 1
    // n2=20 gives f0=0.8185
    scaledMaterial.x *= 0.81859410430839002267573696145125f;
    //scaledMaterial.x *= 4.f;
    return scaledMaterial;
}

void sampleShadingFunction(read_only image3d_t volumeTex, float volumeSample, const float3 voxelSpacing, float4 material,
                 float3 sample, 
                 float3* direction, float2 rnd, const ShadingType shadingType) {

    // Material properties: 
    // x = fresnel factor f_0 = ((1-n)/(1+n))^2, 
    // y = exponent (blinn)
    // z = phase function scattering

    if ( shadingType == HENYEY_GREENSTEIN ) {
        *direction = HenyeyGreensteinSample(-*direction, material.z, rnd);            
    } else if ( shadingType == SCHLICK ) { 
         *direction = SchlickSample(-*direction, material.z, rnd); 
    } else if ( shadingType == BLINN_PHONG ) {
        float3 gradient = normalize(calcGradient(as_float4(sample), volumeTex, as_float4(voxelSpacing)).xyz);
        float3 wo = -*direction;
        float3 Nu = normalize(cross(gradient, wo));
        float3 Nv = normalize(cross(gradient, Nu));
        wo = worldToShading(Nu, Nv, gradient, wo); 
        *direction = shadingToWorld(Nu, Nv, gradient, BlinnSample(wo, gradient, material.y, rnd));
    } else if ( shadingType == WARD ) {
        float3 gradient = normalize(calcGradient(as_float4(sample), volumeTex, as_float4(voxelSpacing)).xyz);
        float3 wo = -*direction;
        float3 Nu = normalize(cross(gradient, wo));
        float3 Nv = normalize(cross(gradient, Nu));
        wo = worldToShading(Nu, Nv, gradient, wo); 
        *direction = shadingToWorld(Nu, Nv, gradient, WardSample(wo, gradient, material.y,material.y, rnd));
    } else if ( shadingType == COOK_TORRANCE ) {
        float3 gradient = normalize(calcGradient(as_float4(sample), volumeTex, as_float4(voxelSpacing)).xyz);
        float3 wo = -*direction;
        float3 Nu = normalize(cross(gradient, wo));
        float3 Nv = normalize(cross(gradient, Nu));
        wo = worldToShading(Nu, Nv, gradient, wo); 
        *direction = shadingToWorld(Nu, Nv, gradient, CookTorranceSample(wo, material.y, rnd));
    } else if ( shadingType == ABC_MICROFACET ) {
        float3 gradient = normalize(calcGradient(as_float4(sample), volumeTex, as_float4(voxelSpacing)).xyz);
        float3 wo = -*direction;
        float3 Nu = normalize(cross(gradient, wo));
        float3 Nv = normalize(cross(gradient, Nu));
        wo = worldToShading(Nu, Nv, gradient, wo); 
        *direction = shadingToWorld(Nu, Nv, gradient, ABCMicrofacetSample(wo, material.z, material.w, rnd));
    } else if ( shadingType == ASHIKHMIN ) {
        float3 gradient = normalize(calcGradient(as_float4(sample), volumeTex, as_float4(voxelSpacing)).xyz);
        float3 wo = -*direction;
        float3 Nu = normalize(cross(gradient, wo)); 
        float3 Nv = normalize(cross(gradient, Nu));
        wo = worldToShading(Nu, Nv, gradient, wo); 
        *direction = shadingToWorld(Nu, Nv, gradient, AshikhminSample(wo, gradient, material.y, rnd));
    } else if ( shadingType == MIX ) {
        float3 gradient = (calcGradient(as_float4(sample), volumeTex, as_float4(voxelSpacing)).xyz); 
        float gradientMagnitude = length(gradient);      
        if(material.w < gradientMagnitude) {    
            *direction = uniformSampleSphere(rnd);   
        } else {
            gradient = normalize(gradient);
            float3 wo = -*direction;
            float3 Nu = normalize(cross(gradient, wo)); 
            float3 Nv = normalize(cross(gradient, Nu));
            wo = worldToShading(Nu, Nv, gradient, wo); 
            *direction = shadingToWorld(Nu, Nv, gradient, AshikhminSample(wo, gradient, material.y, rnd)); 
        }
    } else { // Isotropic phase function
        *direction = uniformSampleSphere(rnd);  
    }

}

// directionInOut should contain toCameraDirection upon input and will contain toLightDirection upon output.
void sampleShadingFunctionPdf(read_only image3d_t volumeTex, float volumeSample, const float3 voxelSpacing, float4 material, float3 pos, 
                 float3* directionInOut, float *pdf, float2 rnd, const ShadingType shadingType) {
    // directionInOut : A direction towards light source or camera upon input. Will be overwritten to new sample direction
    if ( shadingType == HENYEY_GREENSTEIN ) {
        float3 newDir = HenyeyGreensteinSample(-*directionInOut, material.z, rnd);       
        *pdf = HenyeyGreensteinPhaseFunction(-*directionInOut, newDir, material.z); 
        *directionInOut = newDir;
    } else if ( shadingType == SCHLICK ) { 
         float3 newDir = SchlickSample(-*directionInOut, material.z, rnd); 
          *pdf = SchlickPhaseFunction(-*directionInOut, newDir, material.z); 
          *directionInOut = newDir;
    } else if ( shadingType == BLINN_PHONG ) {
        float3 gradient = normalize(calcGradient(as_float4(pos), volumeTex, as_float4(voxelSpacing)).xyz);
        float3 wo = -*directionInOut;
        float3 Nu = normalize(cross(gradient, wo));
        float3 Nv = normalize(cross(gradient, Nu));
        wo = worldToShading(Nu, Nv, gradient, wo);
        float3 wi = BlinnSample(wo, gradient, material.y, rnd);
        *pdf = BlinnPdf(wo, wi, material.y);
        *directionInOut = shadingToWorld(Nu, Nv, gradient, wi);
    } else if ( shadingType == WARD ) {
        float3 gradient = normalize(calcGradient(as_float4(pos), volumeTex, as_float4(voxelSpacing)).xyz);
        float3 wo = -*directionInOut;
        float3 Nu = normalize(cross(gradient, wo));
        float3 Nv = normalize(cross(gradient, Nu));
        wo = worldToShading(Nu, Nv, gradient, wo);
        float3 wi = WardSample(wo, gradient, material.y,material.y, rnd);
        *pdf = WardPdf(wo, wi, gradient, material.y, material.y);
        *directionInOut = shadingToWorld(Nu, Nv, gradient, wi);
    } else if ( shadingType == COOK_TORRANCE ) {
        float3 gradient = normalize(calcGradient(as_float4(pos), volumeTex, as_float4(voxelSpacing)).xyz);
        float3 wo = -*directionInOut;
        float3 Nu = normalize(cross(gradient, wo));
        float3 Nv = normalize(cross(gradient, Nu));
        wo = worldToShading(Nu, Nv, gradient, wo); 
        float3 wi = CookTorranceSample(wo, material.y, rnd);
        *pdf = CookTorrancePdf(wo, wi, gradient, material.y);
        *directionInOut = shadingToWorld(Nu, Nv, gradient, wi);
    } else if ( shadingType == ABC_MICROFACET ) {
        float3 gradient = normalize(calcGradient(as_float4(pos), volumeTex, as_float4(voxelSpacing)).xyz);
        float3 wo = -*directionInOut;
        float3 Nu = normalize(cross(gradient, wo));
        float3 Nv = normalize(cross(gradient, Nu));
        wo = worldToShading(Nu, Nv, gradient, wo); 
        float3 wi = ABCMicrofacetSample(wo, material.z, material.w, rnd);
        // TODO: Implement ABC pdf
        *pdf = CookTorrancePdf(wo, wi, gradient, material.w);
        *directionInOut = shadingToWorld(Nu, Nv, gradient, wi);
    } else if ( shadingType == ASHIKHMIN ) {
        float3 gradient = normalize(calcGradient(as_float4(pos), volumeTex, as_float4(voxelSpacing)).xyz);
        float3 wo = -*directionInOut;
        float3 Nu = normalize(cross(gradient, wo));
        float3 Nv = normalize(cross(gradient, Nu));
        wo = worldToShading(Nu, Nv, gradient, wo); 
        float3 wi = AshikhminSample(wo, gradient, material.y, rnd);
        *pdf = AshikhminPdf(wo, wi, material.y);
        *directionInOut = shadingToWorld(Nu, Nv, gradient, wi);
    } else if ( shadingType == MIX ) {
        float3 gradient = (calcGradient(as_float4(pos), volumeTex, as_float4(voxelSpacing)).xyz); 
        float gradientMagnitude = length(gradient);
        if(material.w < gradientMagnitude) {    
            *directionInOut = uniformSampleSphere(rnd);   
            *pdf = uniformSpherePdf();
        } else {
            gradient = normalize(gradient);
            float3 wo = -*directionInOut;
            float3 Nu = normalize(cross(gradient, wo)); 
            float3 Nv = normalize(cross(gradient, Nu));
            wo = worldToShading(Nu, Nv, gradient, wo); 
            float3 wi = AshikhminSample(wo, gradient, material.y, rnd);
            *pdf = AshikhminPdf(wo, wi, material.y);
            *directionInOut = shadingToWorld(Nu, Nv, gradient, wi);
        }
    } else { 
        *directionInOut = normalize(uniformSampleSphere(rnd));  
        *pdf = uniformSpherePdf();
    }

}
float shadingFunctionPdf(read_only image3d_t volumeTex, float volumeSample, const float3 voxelSpacing, float4 material,
                 float3 pos, float3 toCameraDir,
                 float3 toLightDirection, const ShadingType shadingType) {

    float pdf;
    if ( shadingType == HENYEY_GREENSTEIN ) {
        pdf = HenyeyGreensteinPhaseFunction(-toCameraDir, toLightDirection, material.z);    
    } else if ( shadingType == SCHLICK ) { 
        pdf = SchlickPhaseFunction(-toCameraDir, toLightDirection, material.z); 
    } else if ( shadingType == BLINN_PHONG ) {
        float3 gradient = normalize(calcGradient(as_float4(pos), volumeTex, as_float4(voxelSpacing)).xyz);
        float3 wo = toCameraDir;
        float3 Nu = normalize(cross(gradient, wo));
        float3 Nv = normalize(cross(gradient, Nu));
        wo = worldToShading(Nu, Nv, gradient, wo);
        float3 wi = worldToShading(Nu, Nv, gradient, toLightDirection);
        pdf = BlinnPdf(wo, wi, material.y);
    } else if ( shadingType == WARD ) {
        float3 gradient = normalize(calcGradient(as_float4(pos), volumeTex, as_float4(voxelSpacing)).xyz);
        float3 wo = toCameraDir;
        float3 Nu = normalize(cross(gradient, wo));
        float3 Nv = normalize(cross(gradient, Nu));
        wo = worldToShading(Nu, Nv, gradient, wo);
        float3 wi = worldToShading(Nu, Nv, gradient, toLightDirection);
        pdf = WardPdf(wo, wi, gradient, material.y, material.y);
    } else if ( shadingType == COOK_TORRANCE ) {
        float3 gradient = normalize(calcGradient(as_float4(pos), volumeTex, as_float4(voxelSpacing)).xyz);
        float3 wo = toCameraDir;
        float3 Nu = normalize(cross(gradient, wo));
        float3 Nv = normalize(cross(gradient, Nu));
        wo = worldToShading(Nu, Nv, gradient, wo); 
        float3 wi = worldToShading(Nu, Nv, gradient, toLightDirection);
        pdf = CookTorrancePdf(wo, wi, gradient, material.y);
    } else if ( shadingType == ABC_MICROFACET ) {
        float3 gradient = normalize(calcGradient(as_float4(pos), volumeTex, as_float4(voxelSpacing)).xyz);
        float3 wo = toCameraDir;
        float3 Nu = normalize(cross(gradient, wo));
        float3 Nv = normalize(cross(gradient, Nu));
        wo = worldToShading(Nu, Nv, gradient, wo); 
        float3 wi = worldToShading(Nu, Nv, gradient, toLightDirection);
        // TODO: Implement ABC pdf
        pdf = CookTorrancePdf(wo, wi, gradient, material.w);
    } else if ( shadingType == ASHIKHMIN ) {
        float3 gradient = normalize(calcGradient(as_float4(pos), volumeTex, as_float4(voxelSpacing)).xyz);
        float3 wo = toCameraDir;
        float3 Nu = normalize(cross(gradient, wo));
        float3 Nv = normalize(cross(gradient, Nu));
        wo = worldToShading(Nu, Nv, gradient, wo); 
        float3 wi = worldToShading(Nu, Nv, gradient, toLightDirection);
        pdf = AshikhminPdf(wo, wi, material.y);
    } else if ( shadingType == MIX ) {
        float3 gradient = (calcGradient(as_float4(pos), volumeTex, as_float4(voxelSpacing)).xyz); 
        float gradientMagnitude = length(gradient);
        if(material.w < gradientMagnitude) {      
            pdf = uniformSpherePdf();
        } else {
            gradient = normalize(gradient);
            float3 wo = toCameraDir;
            float3 Nu = normalize(cross(gradient, wo));
            float3 Nv = normalize(cross(gradient, Nu));
            wo = worldToShading(Nu, Nv, gradient, wo); 
            float3 wi = worldToShading(Nu, Nv, gradient, toLightDirection);
            pdf = AshikhminPdf(wo, wi, material.y);
        }
    } else { 
        pdf = uniformSpherePdf();
    }
    return pdf;
}

float3 applyShading(const float3 toCameraDir, const float3 toLightDir, const float3 materialDiffuse, const float3 materialSpecular, const float4 material, float3 gradient, const ShadingType shadingType) {
    float3 f; 
    //float3 Nu = cross(gradient, toCameraDir);
    //if ( dot(Nu, Nu) < 1.e-3f ) {
    //    Nu = cross( gradient, (float3)( 1.0f, 0.0f, 0.0f ) ); 
    //    //if ( dot(Nu, Nu) < 1.e-3f ) {
    //    //    Nu = normalize(cross( gradient, (float3)( 0.0f, 1.0f, 0.0f ) ));
    //    //}
    //    //if ( dot(Nu, Nu) < 1.e-3f ) {
    //    //    Nu = normalize(cross( gradient, (float3)( 0.0f, 0.0f, 1.0f ) ));
    //    //}
    //}
    //Nu = normalize(Nu);
    //float3 Nv = normalize(cross(gradient, Nu));
    //float3 wo = worldToShading(Nu, Nv, gradient, toCameraDir); 
    //float3 wi = worldToShading(Nu, Nv, gradient, toLightDir); 
#ifdef PERFORMANCE 
#if PHASE_FUNCTION == PHASE_FUNCTION_HENYEY_GREENSTEIN
    f = materialDiffuse*HenyeyGreensteinPhaseFunction(toCameraDir, toLightDir, material.z);
#elif PHASE_FUNCTION == PHASE_FUNCTION_SCHLICK
    f = materialDiffuse*SchlickPhaseFunction(toCameraDir, toLightDir, material.z); 
#elif PHASE_FUNCTION == PHASE_FUNCTION_BLINN_PHONG
    float3 Nu = cross(gradient, toCameraDir);
    if ( dot(Nu, Nu) < 1.e-3f ) {
        Nu = cross( gradient, (float3)( 1.0f, 0.0f, 0.0f ) ); 
    }
    Nu = fast_normalize(Nu);
    float3 Nv = fast_normalize(cross(gradient, Nu));
    float3 wo = worldToShading(Nu, Nv, gradient, toCameraDir); 
    float3 wi = worldToShading(Nu, Nv, gradient, toLightDir); 
    f = materialDiffuse*isotropicPhaseFunction()+materialSpecular*BlinnBRDF(wo, wi, material.y)*fabs(wi.z);
#elif PHASE_FUNCTION == PHASE_FUNCTION_WARD
    float3 Nu = cross(gradient, toCameraDir);
    if ( dot(Nu, Nu) < 1.e-3f ) {
        Nu = cross( gradient, (float3)( 1.0f, 0.0f, 0.0f ) ); 
    }
    Nu = fast_normalize(Nu);
    float3 Nv = fast_normalize(cross(gradient, Nu));
    float3 wo = worldToShading(Nu, Nv, gradient, toCameraDir); 
    float3 wi = worldToShading(Nu, Nv, gradient, toLightDir); 
    f = materialDiffuse*isotropicPhaseFunction()+materialSpecular*WardBRDF(wo, wi, material.y, material.y)*fabs(wi.z);
#elif PHASE_FUNCTION == PHASE_FUNCTION_COOK_TORRANCE
    float3 Nu = cross(gradient, toCameraDir);
    if ( dot(Nu, Nu) < 1.e-3f ) {
        Nu = cross( gradient, (float3)( 1.0f, 0.0f, 0.0f ) ); 
    }
    Nu = fast_normalize(Nu);
    float3 Nv = fast_normalize(cross(gradient, Nu));
    float3 wo = worldToShading(Nu, Nv, gradient, toCameraDir); 
    float3 wi = worldToShading(Nu, Nv, gradient, toLightDir); 
    f = materialDiffuse*isotropicPhaseFunction()+materialSpecular*CookTorranceBRDF(wo, wi, material.x, material.y)*fabs(wi.z);
#elif PHASE_FUNCTION == PHASE_FUNCTION_ABC_MICROFACET
    float3 Nu = cross(gradient, toCameraDir);
    if ( dot(Nu, Nu) < 1.e-3f ) {
        Nu = cross( gradient, (float3)( 1.0f, 0.0f, 0.0f ) ); 
    }
    Nu = fast_normalize(Nu);
    float3 Nv = fast_normalize(cross(gradient, Nu));
    float3 wo = worldToShading(Nu, Nv, gradient, toCameraDir); 
    float3 wi = worldToShading(Nu, Nv, gradient, toLightDir); 
    f = materialDiffuse*isotropicPhaseFunction()+materialSpecular*ABCMicrofacetBRDF(wo, wi, material.x, material.y, material.z)*fabs(wi.z);
#elif PHASE_FUNCTION == PHASE_FUNCTION_ASHIKHMIN
    float3 Nu = cross(gradient, toCameraDir);
    if ( dot(Nu, Nu) < 1.e-3f ) {
        Nu = cross( gradient, (float3)( 1.0f, 0.0f, 0.0f ) ); 
    }
    Nu = fast_normalize(Nu);
    float3 Nv = fast_normalize(cross(gradient, Nu));
    float3 wo = worldToShading(Nu, Nv, gradient, toCameraDir); 
    float3 wi = worldToShading(Nu, Nv, gradient, toLightDir); 
    f = materialDiffuse*isotropicPhaseFunction()+materialSpecular*AshikimBRDF(wo, wi, gradient, material.x, material.y)*fabs(wi.z);
#elif PHASE_FUNCTION == PHASE_FUNCTION_MIX
    float3 Nu = cross(gradient, toCameraDir);
    if ( dot(Nu, Nu) < 1.e-3f ) {
        Nu = cross( gradient, (float3)( 1.0f, 0.0f, 0.0f ) ); 
    }
    Nu = fast_normalize(Nu);
    float3 Nv = fast_normalize(cross(gradient, Nu));
    float3 wo = worldToShading(Nu, Nv, gradient, toCameraDir); 
    float3 wi = worldToShading(Nu, Nv, gradient, toLightDir); 
    f = mix(materialDiffuse*isotropicPhaseFunction(), materialDiffuse*lambertianBRDF()+materialSpecular*AshikimBRDF(wo, wi, gradient, material.x, material.y), material.w)*fabs(wi.z);
#else  
    f = materialDiffuse*isotropicPhaseFunction();   
#endif
#else
    if ( shadingType == HENYEY_GREENSTEIN ) { 
        f = materialDiffuse*HenyeyGreensteinPhaseFunction(toCameraDir, -toLightDir, material.z);
    } else if ( shadingType == SCHLICK ) {
        f = materialDiffuse*SchlickPhaseFunction(toCameraDir, -toLightDir, material.z); 
    } else if ( shadingType == BLINN_PHONG ) {
        float3 Nu = cross(gradient, toCameraDir);
        if ( dot(Nu, Nu) < 1.e-3f ) {
            Nu = cross( gradient, (float3)( 1.0f, 0.0f, 0.0f ) ); 
        }
        Nu = fast_normalize(Nu);
        float3 Nv = fast_normalize(cross(gradient, Nu));
        float3 wo = worldToShading(Nu, Nv, gradient, toCameraDir); 
        float3 wi = worldToShading(Nu, Nv, gradient, toLightDir); 
        f = materialDiffuse*isotropicPhaseFunction()+materialSpecular*BlinnBRDF(wo, wi, material.x, material.y)*fabs(wi.z);
    } else if ( shadingType == WARD ) {
        float3 Nu = cross(gradient, toCameraDir);
        if ( dot(Nu, Nu) < 1.e-3f ) {
            Nu = cross( gradient, (float3)( 1.0f, 0.0f, 0.0f ) ); 
        }
        Nu = fast_normalize(Nu);
        float3 Nv = fast_normalize(cross(gradient, Nu));
        float3 wo = worldToShading(Nu, Nv, gradient, toCameraDir); 
        float3 wi = worldToShading(Nu, Nv, gradient, toLightDir); 
        f = materialDiffuse*isotropicPhaseFunction()+materialSpecular*WardBRDF(wo, wi, material.x, material.y, material.y)*fabs(wi.z);
    } else if ( shadingType == COOK_TORRANCE ) {
        float3 Nu = cross(gradient, toCameraDir);
        if ( dot(Nu, Nu) < 1.e-3f ) {
            Nu = cross( gradient, (float3)( 1.0f, 0.0f, 0.0f ) ); 
        }
        Nu = fast_normalize(Nu);
        float3 Nv = fast_normalize(cross(gradient, Nu));
        float3 wo = worldToShading(Nu, Nv, gradient, toCameraDir); 
        float3 wi = worldToShading(Nu, Nv, gradient, toLightDir); 
        f = materialDiffuse*isotropicPhaseFunction()+materialSpecular*CookTorranceBRDF(wo, wi, material.x, material.y)*fabs(wi.z);
    } else if ( shadingType == ABC_MICROFACET ) {
        float3 Nu = cross(gradient, toCameraDir);
        if ( dot(Nu, Nu) < 1.e-3f ) {
            Nu = cross( gradient, (float3)( 1.0f, 0.0f, 0.0f ) ); 
        }
        Nu = fast_normalize(Nu);
        float3 Nv = fast_normalize(cross(gradient, Nu));
        float3 wo = worldToShading(Nu, Nv, gradient, toCameraDir); 
        float3 wi = worldToShading(Nu, Nv, gradient, toLightDir); 
        f = materialDiffuse*isotropicPhaseFunction()+materialSpecular*ABCMicrofacetBRDF(wo, wi, material.x, material.y, material.z)*fabs(wi.z);
    } else if ( shadingType == ASHIKHMIN ) {
        float3 Nu = cross(gradient, toCameraDir);
        if ( dot(Nu, Nu) < 1.e-3f ) {
            Nu = cross( gradient, (float3)( 1.0f, 0.0f, 0.0f ) ); 
        }
        Nu = fast_normalize(Nu);
        float3 Nv = fast_normalize(cross(gradient, Nu));
        float3 wo = worldToShading(Nu, Nv, gradient, toCameraDir); 
        float3 wi = worldToShading(Nu, Nv, gradient, toLightDir); 
        f = materialDiffuse*isotropicPhaseFunction()+materialSpecular*AshikimBRDF(wo, wi, gradient, material.x, material.y)*fabs(wi.z);  
    } else if ( shadingType == MIX ) {
        float3 Nu = cross(gradient, toCameraDir);
        if ( dot(Nu, Nu) < 1.e-3f ) {
            Nu = cross( gradient, (float3)( 1.0f, 0.0f, 0.0f ) ); 
        }
        Nu = fast_normalize(Nu);
        float3 Nv = fast_normalize(cross(gradient, Nu));
        float3 wo = worldToShading(Nu, Nv, gradient, toCameraDir); 
        float3 wi = worldToShading(Nu, Nv, gradient, toLightDir); 
        f = mix(materialDiffuse*isotropicPhaseFunction(), materialDiffuse*isotropicPhaseFunction()+materialSpecular*AshikimBRDF(wo, wi, gradient, material.x, material.y)*fabs(wi.z), material.w);
    } else {
        f = materialDiffuse*isotropicPhaseFunction(); 
    } 
#endif
    // No need to use pdf since we are not generating samples based on phase function/BRDF
    return f;
}     

#endif
