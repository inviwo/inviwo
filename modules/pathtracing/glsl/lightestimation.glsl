#include "random.glsl"
#include "transmittance/transmittance.glsl"
#include "util/rayboxintersection.glsl"

vec3 estimateDirectLightUniformGrid(sampler3D volume, VolumeParameters volParam, sampler2D tf,
                                    sampler3D opacity, VolumeParameters opacityParam, vec3 cellDim,
                                    vec3 samplePos, vec3 cameraDir, LightParameters light,
                                    uint hashSeed, int rcChannel, int TRANSMITTANCEMETHOD) {

    vec3 lightTexPos = (volParam.worldToTexture * vec4(light.position, 1f)).xyz;

    vec3 toLightPath = lightTexPos - samplePos;

    float t0 = 0.0f;
    float t1 = length(toLightPath);
    vec3 toLightDir = normalize(toLightPath);


    if (!rayBoxIntersection(vec3(0.0f), vec3(1.0f), samplePos, toLightDir, t0, t1)) {
        return vec3(0);
    }

    float Tl = 1.0f;

    vec3 auxReturn = vec3(0);
    Tl = partitionedTransmittanceTracking(TRANSMITTANCEMETHOD, samplePos, toLightDir, t0, t1,
                                         hashSeed, volume, volParam, tf,
                                         opacity, opacityParam, cellDim,
                                         auxReturn);

    if (Tl == 0.0f) {
        return vec3(0f);
    }

    vec4 voxel = getNormalizedVoxel(volume, volParam, samplePos);
    vec3 gradient = COMPUTE_GRADIENT_FOR_CHANNEL(voxel, volume, volParam, samplePos, rcChannel);
    gradient = normalize(gradient);

    gradient *= sign(voxel[rcChannel] / (1.0 - volParam.formatScaling) - volParam.formatOffset);
    vec4 tfSample = applyTF(tf, voxel);

    vec3 sampleAmbient = tfSample.rgb;
    vec3 sampleDiffuse = tfSample.rgb;
    vec3 sampleSpecular = tfSample.rgb;

    vec3 sampleWorldPos = (volParam.textureToWorld * vec4(samplePos, 1f)).xyz;
    vec3 color = APPLY_LIGHTING(light, sampleAmbient, sampleDiffuse, sampleSpecular, sampleWorldPos,
                                -gradient, cameraDir);

    return color * Tl;
}

vec3 estimateDirectLight(sampler3D volume, VolumeParameters volParam, sampler2D tf, vec3 samplePos,
                         vec3 cameraDir, LightParameters light, uint hashSeed, int rcChannel,
                         int TRANSMITTANCEMETHOD) {

    vec3 toLightPath = (volParam.worldToTexture * vec4(light.position, 1f)).xyz - samplePos;
    float t0 = 0.0f;
    float t1 = length(toLightPath);
    vec3 toLightDir = normalize(toLightPath);
    
    

    if (!rayBoxIntersection(vec3(0.0f), vec3(1.0f), samplePos, toLightDir, t0, t1)) {
        return vec3(0);
    }

    float Tl = transmittance(TRANSMITTANCEMETHOD, samplePos, toLightDir, t0, t1, hashSeed, volume,
                             volParam, tf);

    if (Tl == 0.0f) {
        return vec3(0f);
    }

    vec4 voxel = getNormalizedVoxel(volume, volParam, samplePos);
    vec3 gradient = COMPUTE_GRADIENT_FOR_CHANNEL(voxel, volume, volParam, samplePos, rcChannel);
    gradient = normalize(gradient);

    gradient *= sign(voxel[rcChannel] / (1.0 - volParam.formatScaling) - volParam.formatOffset);

    vec4 tfSample = applyTF(tf, voxel);

    vec3 sampleAmbient = tfSample.rgb;
    vec3 sampleDiffuse = tfSample.rgb;
    vec3 sampleSpecular = tfSample.rgb;

    vec3 sampleWorldPos = (volParam.textureToWorld * vec4(samplePos, 1f)).xyz;
    vec3 color = APPLY_LIGHTING(light, sampleAmbient, sampleDiffuse, sampleSpecular,
                                sampleWorldPos,  -gradient, cameraDir);

    return color * Tl;
}
