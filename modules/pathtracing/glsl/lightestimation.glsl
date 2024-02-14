#include "random.glsl"
#include "transmittance/transmittancemethods.glsl"
#include "util/rayboxintersection.glsl"

vec3 estimateDirectLightUniformGrid(sampler3D volume, VolumeParameters volParam, sampler2D tf,
                                    sampler3D opacity, VolumeParameters opacityParam,
                                    bool partitionedTransmittance, vec3 samplePos, vec3 cameraDir,
                                    LightParameters light, uint hashSeed, int rcChannel,
                                    int transmittanceMethod) {

    vec3 toLightPath = (volParam.worldToTexture * vec4(light.position, 1f)).xyz - samplePos;
    vec3 toLightDir = normalize(toLightPath);
    float t0 = 0.0f;
    float t1 = 1.f;

    rayBoxIntersection(vec3(0f), vec3(1f), samplePos, toLightDir, t0, t1);

    float Tl = 1.0f;

    if (partitionedTransmittance) {
        Tl =
            partitionedTransmittanceTracking(transmittanceMethod, samplePos, toLightDir, t0, t1,
                                             hashSeed, volume, volParam, tf, opacity, opacityParam);
    } else {
        //Tl = transmittance(transmittanceMethod, samplePos, toLightDir, t0, t1, hashSeed, volume,
        //                   volParam, tf);
    }

    // NOTE: There is a visual bug caused by a rayMinMax, transmittance with
    //       rayBoxIntersection modified start and end, and gradient sign shift being called.
    //       I can't explain why this chain of operations causes the side-effect, I just know that
    //       removing one of them solves it. The side-effect, simply explained, shadeing becomes
    //       darker for brighter spots depending on how far along z you view. Z sign flipping
    //       causing darker shading by disregarding 'shadows' from Tl. This gradient operations just
    //       shifts what viewing directions this error occurs in.
    // Tl = 1.0f;
    if (Tl == 0.0f) {
        return vec3(0f);
    }

    // NOTE: Shader macros have a tendency to not be recognized sometimes when first loading test
    // workspace NOTE: If this occurs, theres a chance the program crashes due to segfault. Is it
    // related?
    vec4 voxel = getNormalizedVoxel(volume, volParam, samplePos);
    vec3 gradient = COMPUTE_GRADIENT_FOR_CHANNEL(voxel, volume, volParam, samplePos, rcChannel);
    // vec3 gradient = gradientCentralDiff(vec4(0f), volume, volParam, samplePos, rcChannel);
    gradient = normalize(gradient);

    gradient *= sign(voxel[rcChannel] / (1.0 - volParam.formatScaling) - volParam.formatOffset);

    vec4 tfSample = applyTF(tf, voxel);

    vec3 sampleAmbient = tfSample.rgb;
    vec3 sampleDiffuse = tfSample.rgb;
    vec3 sampleSpecular = tfSample.rgb;

    vec3 sampleWorldPos = (volParam.textureToWorld * vec4(samplePos, 1f)).xyz;
    vec3 color = APPLY_LIGHTING(light, sampleAmbient, sampleDiffuse, sampleSpecular, sampleWorldPos,
                                -gradient, cameraDir);

    // vec3 color = shadeBlinnPhong(light, sampleAmbient, sampleDiffuse, sampleSpecular,
    //                              sampleWorldPos, -gradient, cameraDir);

    return color * Tl;
}
/*
vec3 estimateDirectLight(sampler3D volume, VolumeParameters volParam, sampler2D tf, vec3 samplePos,
                         vec3 cameraDir, LightParameters light, uint hashSeed, int rcChannel,
                         int transmittanceMethod) {

    vec3 toLightPath = (volParam.worldToTexture * vec4(light.position, 1f)).xyz - samplePos;
    vec3 toLightDir = normalize(toLightPath);
    float t0 = 0.0f;
    float t1 = 1.f;

    rayBoxIntersection(vec3(0f), vec3(1f), samplePos, toLightDir, t0, t1);

    float Tl = transmittance(transmittanceMethod, samplePos, toLightDir, t0, t1, hashSeed, volume,
                             volParam, tf);

    if (Tl == 0.0f) {
        return vec3(0f);
    }

    // NOTE: Shader macros have a tendency to not be recognized sometimes when first loading test
    // workspace NOTE: If this occurs, theres a chance the program crashes due to segfault. Is it
    // related?
    vec4 voxel = getNormalizedVoxel(volume, volParam, samplePos);
    // vec3 gradient = COMPUTE_GRADIENT_FOR_CHANNEL(voxel, volume, volParam, samplePos, rcChannel);
    vec3 gradient = gradientCentralDiff(vec4(0f), volume, volParam, samplePos, rcChannel);
    gradient = normalize(gradient);

    gradient *= sign(voxel[rcChannel] / (1.0 - volParam.formatScaling) - volParam.formatOffset);

    vec4 tfSample = applyTF(tf, voxel);

    vec3 sampleAmbient = tfSample.rgb;
    vec3 sampleDiffuse = tfSample.rgb;
    vec3 sampleSpecular = tfSample.rgb;

    vec3 sampleWorldPos = (volParam.textureToWorld * vec4(samplePos, 1f)).xyz;
    // vec3 color = APPLY_LIGHTING(light, sampleAmbient, sampleDiffuse, sampleSpecular,
    // sampleWorldPos,
    //                            -gradient, cameraDir);

    vec3 color = shadeBlinnPhong(light, sampleAmbient, sampleDiffuse, sampleSpecular,
                                 sampleWorldPos, -gradient, cameraDir);

    return color * Tl;
}
*/