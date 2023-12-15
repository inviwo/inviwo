#include "random.glsl"
#include "transmittance/transmittancemethods.glsl"
#include "util/vectormatrixmethods.glsl"

vec3 estimateDirectLight(float rayStep, sampler3D volume, VolumeParameters volParam, sampler2D tf,
                         vec3 samplePos, vec3 cameraDir, LightParameters light, uint hashSeed,
                         int rcChannel) {

    vec3 toLightPath = (volParam.worldToTexture * vec4(light.position, 1f)).xyz - samplePos;
    vec3 toLightDir = normalize(toLightPath);
    float t0 = 0.0f;
    float t1 = 1.f;

    rayBoxIntersection(vec3(0f), vec3(1f), samplePos, toLightDir, t0, t1);

    float Tl =
        transmittance(RESIDUALRATIO, samplePos, toLightDir, t0, t1, hashSeed, volume, volParam, tf);

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