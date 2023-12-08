#include "random.glsl"
#include "transmittance/transmittancemethods.glsl"
#include "util/vectormatrixmethods.glsl"

vec3 estimateDirectLight(float rayStep, sampler3D volume, VolumeParameters volParam, sampler2D tf,
                         vec3 samplePos, vec3 cameraDir, LightParameters light, uint hashSeed,
                         int rcChannel, float extinctionUpper) {

    vec3 toLightTextureVector =
        (volParam.worldToTexture * vec4(light.position, 1f)).xyz - samplePos;
    vec3 toLightTextureDir = normalize(toLightTextureVector);
    float t0 = 0.0f;
    float t1 = 1.f;

    RayBBIntersection_TextureSpace(samplePos, toLightTextureDir, t0, t1);

    float meanfreepath_l = WoodcockTracking(samplePos, toLightTextureDir, t0, t1, hashSeed, volume,
                                            volParam, tf, extinctionUpper);
    float Tl = meanfreepath_l >= t1 ? 1.0f : 0.0f;

    if (Tl == 0.0f) {
        return vec3(0f);
    }

    vec3 gradient = gradientCentralDiff(vec4(0f), volume, volParam, samplePos, 0);
    gradient = normalize(gradient);

    vec4 voxel = getNormalizedVoxel(volume, volParam, samplePos);
    gradient *= sign(voxel[0] / (1.0 - volParam.formatScaling) - volParam.formatOffset);

    vec4 tfSample = applyTF(tf, voxel);

    vec3 sampleAmbient = tfSample.rgb;
    vec3 sampleDiffuse = tfSample.rgb;
    vec3 sampleSpecular = tfSample.rgb;

    vec3 sampleWorldPos = (volParam.textureToWorld * vec4(samplePos, 1f)).xyz;
    vec3 color = shadeBlinnPhong(light, sampleAmbient, sampleDiffuse, sampleSpecular,
                                 sampleWorldPos, gradient, cameraDir);

    return color * Tl;
}