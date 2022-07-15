#include "utils/structs.glsl"
#include "utils/sampler3d.glsl"
#include "utils/depth.glsl"
#include "utils/compositing.glsl"


uniform VolumeParameters volumeParameters;
uniform sampler3D volume;
uniform sampler3D linearVolume;

uniform ImageParameters entryParameters;
uniform sampler2D entryColor;

uniform ImageParameters exitParameters;
uniform sampler2D exitColor;

uniform ImageParameters outportParameters;

uniform vec4 selectedColor;
uniform vec4 highlightedColor;
uniform vec4 filteredColor;
uniform bool showHighlighted;
uniform bool showSelected;
uniform bool showFiltered;
uniform float scaling;
uniform float sampleRate;



#define ERT_THRESHOLD 0.99

vec3 gradientCentralDiff(vec4 intensity, sampler3D volume, VolumeParameters volumeParams, vec3 samplePos, int channel) {
    // Of order O(h^2) central differences
    vec3 cDs;
    float scale = 1.0;
    // Value at f(x+h)
    cDs.x = getNormalizedVoxelChannel(volume, volumeParams, samplePos + volumeParams.textureSpaceGradientSpacing[0]*scale,channel);
    cDs.y = getNormalizedVoxelChannel(volume, volumeParams, samplePos + volumeParams.textureSpaceGradientSpacing[1]*scale,channel);
    cDs.z = getNormalizedVoxelChannel(volume, volumeParams, samplePos + volumeParams.textureSpaceGradientSpacing[2]*scale,channel);
    // Value at f(x-h)
    cDs.x = cDs.x - getNormalizedVoxelChannel(volume, volumeParams, samplePos - volumeParams.textureSpaceGradientSpacing[0]*scale,channel);
    cDs.y = cDs.y - getNormalizedVoxelChannel(volume, volumeParams, samplePos - volumeParams.textureSpaceGradientSpacing[1]*scale,channel);
    cDs.z = cDs.z - getNormalizedVoxelChannel(volume, volumeParams, samplePos - volumeParams.textureSpaceGradientSpacing[2]*scale,channel);
    // Note that this computation is performed in world space
    // f' = ( f(x+h)-f(x-h) ) / 2*volumeParams.worldSpaceGradientSpacing
    return (cDs)/(2.0*volumeParams.worldSpaceGradientSpacing);
}

vec4 raycast(vec3 entryPoint, vec3 exitPoint, float samplingRate)
{
    vec4 result = vec4(0);
    vec3 rayDirection = exitPoint - entryPoint;
    float tEnd = length(rayDirection); // Euclidean distance through cube
    float tIncr = min(tEnd, tEnd / (samplingRate * length(rayDirection * volumeParameters.dimensions)));
    float samples = ceil(tEnd / tIncr); // Max amount of samples possible within cube
    tIncr = tEnd / samples;
    float t = 0.5f * tIncr; // Start first sample depth in half step inside
    rayDirection = normalize(rayDirection);
    vec3 position;
    uint value = 0;
    float v;
    uint prevValue = 0;
    float tDepth = -1.0;
   
    while(t < tEnd)
    {
        position = entryPoint + t * rayDirection;
        // Sample voxel
        v = getNormalizedVoxel(volume, volumeParameters, position).x;
        value = uint(v*3.0 + 0.5);

        // Draw if new voxel value
        if(value != prevValue)
        {
            vec4 color = vec4(0);
            if(showHighlighted && (value == 3 || prevValue == 3))
            {
                color = highlightedColor;
            }
            else if(showSelected && (value == 1 || prevValue == 1))
            {
                color = selectedColor;
            }
            else if(showFiltered && (value == 2 || prevValue == 2))
            {
                color = filteredColor;
            }
            //result = compositeDVR(result, color, t - tIncr, tDepth, tIncr);
            color.rgb *= color.a;
            result += (1.0-result.a) * color;
            vec3 atlasGradient = normalize(gradientCentralDiff(vec4(1.0), linearVolume, volumeParameters, position, 0));
            
            if(value > prevValue)
            {
                
            }
            return vec4(atlasGradient, 1.0);
        }


        // Terminate ray early
        if(result.a > ERT_THRESHOLD)
        {
           t = tEnd;
        }
        else
        {
            // Traverse along ray
            t += tIncr;
            prevValue = value;
        }
    }
    return result;
}

void main() {
    vec2 texCoords = gl_FragCoord.xy * outportParameters.reciprocalDimensions;
    vec3 entryPoint = texture(entryColor, texCoords).rgb;
    vec3 exitPoint = texture(exitColor, texCoords).rgb;

    vec4 color = vec4(0);
    
    if(entryPoint == exitPoint) {
        discard;
    }
    if(entryPoint != exitPoint) {
        color = raycast(entryPoint, exitPoint, sampleRate);
    }
    FragData0 = color;
}