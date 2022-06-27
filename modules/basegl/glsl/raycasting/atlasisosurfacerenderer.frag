#include "utils/structs.glsl"
#include "utils/sampler3d.glsl"
#include "utils/depth.glsl"
#include "utils/compositing.glsl"

uniform VolumeParameters volumeParameters;
uniform sampler3D volume;

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