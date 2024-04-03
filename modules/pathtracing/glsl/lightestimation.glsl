#include "random.glsl"
#include "transmittance/transmittance.glsl"
#include "util/rayboxintersection.glsl"

vec3 estimateDirectLightUniformGrid(sampler3D volume, VolumeParameters volParam, sampler2D tf,
                                    sampler3D opacity, VolumeParameters opacityParam,
                                    vec3 samplePos, vec3 cameraDir, LightParameters light,
                                    uint hashSeed, int rcChannel, int TRANSMITTANCEMETHOD) {

    vec3 lightTexPos = (volParam.worldToTexture * vec4(light.position, 1f)).xyz;


    vec3 toLightPath2 = lightTexPos - samplePos;

    vec3 toLightDir = normalize(toLightPath2);
    float t0 = 0.0f;
    float t1 = length(toLightPath2);

    float t0_prev = 0;
    float t1_prev = t1;
    
    // I think that this scaling needs to be done 100% correct
    // To get the right scale of light distances it is better to do this calc in world space.
    // if bb is smaller than 0 1, we see that shifting light position shifts the truthyness of rayBoxIntersection. Does that make any sense? Yes it does
    // I have no idea if there is anything wrong with this. 
    // So I assume it isn't
    if( !rayBoxIntersection(vec3(0.0f), vec3(1.0f), samplePos, toLightDir, t0, t1)) {
        return vec3(0);
    }

    float Tl = 1.0f;

    // wait, why tf is t0_prev and t0 different but not t1
    // when the light position is inside the volume we get 
    // but how does t0 become LARGER than 9? from the algo?


    vec3 auxReturn = vec3(0);
    // t0 and t1 from the rayBoxIntersection should not be used as start and end.
    // t0 is the first box hit, t1 is the last.
    // t0 should be 0 if the ray starts within the box.
    Tl = partitionedTransmittanceTesting(TRANSMITTANCEMETHOD, samplePos, toLightDir, t0, t1,
                                          hashSeed, volume, volParam, tf, opacity, opacityParam, auxReturn);
    
    
    //Tl = 1.0f;
    if(auxReturn != vec3(0)) {
        //return auxReturn;
    } else {
        //return vec3(0f);
    }

    if (Tl == 0.0f) {
        return vec3(0f);
    }


    // NOTE: Shader macros have a tendency to not be recognized sometimes when first loading test
    // workspace NOTE: If this occurs, theres a chance the program crashes due to segfault. Is it
    // related?
    vec4 voxel = getNormalizedVoxel(volume, volParam, samplePos);
    //vec3 gradient = COMPUTE_GRADIENT_FOR_CHANNEL(voxel, volume, volParam, samplePos, rcChannel);
    vec3 gradient = gradientCentralDiff(vec4(0f), volume, volParam, samplePos, rcChannel);
    gradient = normalize(gradient);

    gradient *= sign(voxel[rcChannel] / (1.0 - volParam.formatScaling) - volParam.formatOffset);

    vec4 tfSample = applyTF(tf, voxel);

    vec3 sampleAmbient = tfSample.rgb;
    vec3 sampleDiffuse = tfSample.rgb;
    vec3 sampleSpecular = tfSample.rgb;

    vec3 sampleWorldPos = (volParam.textureToWorld * vec4(samplePos, 1f)).xyz;
    //vec3 color = APPLY_LIGHTING(light, sampleAmbient, sampleDiffuse, sampleSpecular, sampleWorldPos,
    //                            -gradient, cameraDir);

    vec3 color = shadeBlinnPhong(light, sampleAmbient, sampleDiffuse, sampleSpecular,
                                 sampleWorldPos, -gradient, cameraDir);

    return color*Tl;
}

vec3 estimateDirectLight(sampler3D volume, VolumeParameters volParam, sampler2D tf, vec3 samplePos,
                         vec3 cameraDir, LightParameters light, uint hashSeed, int rcChannel,
                         int TRANSMITTANCEMETHOD) {

    vec3 toLightPath = (volParam.worldToTexture * vec4(light.position, 1f)).xyz - samplePos;
    vec3 toLightDir = normalize(toLightPath);
    float t0 = 0.0f;
    float t1 = 1.f;

    rayBoxIntersection(vec3(0f), vec3(1f), samplePos, toLightDir, t0, t1);

    float Tl = transmittance(TRANSMITTANCEMETHOD, samplePos, toLightDir, t0, t1, hashSeed, volume,
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
