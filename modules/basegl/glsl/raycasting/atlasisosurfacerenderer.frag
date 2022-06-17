#include "utils/structs.glsl"
#include "utils/sampler3d.glsl"
#include "utils/depth.glsl"

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

vec4 raycast(vec3 entryPoint, vec3 exitPoint, float samplingRate)
{
    vec4 result = vec4(0);
    vec3 rayDirection = exitPoint - entryPoint;
    float tEnd = length(rayDirection); // Euclidean distance through cube
    float tIncr = min(tEnd, tEnd / ((1.0/samplingRate) * length(rayDirection * volumeParameters.dimensions)));
    float samples = ceil(tEnd / tIncr); // Max amount of samples possible within cube
    tIncr = tEnd / samples;
    float t = 0.5 * tIncr; // Start first sample depth in half step inside
    rayDirection = normalize(rayDirection);
    vec3 position; // Current sample position
    uint value = 0;
    float v = 0.0;
    uint prevValue = 0;
    vec4 prevResult;
    position =  entryPoint+ t * rayDirection; // What should this be?

    while(t < tEnd)
    {
        // Check early termination
        if(result.a >= 0.99)
        {
           t = tEnd;
           return result;
        }

        position += t * rayDirection;
        v = getNormalizedVoxel(volume, volumeParameters, position).x;
        value = uint(v * scaling + 0.5);

        // Do nothing
        if(value == 0 || value == prevValue){}
        else if(showHighlighted && value == 3)
        {
            result += highlightedColor;
            prevValue = value;
        }
        else if(showSelected && value == 1)
        {
            result += selectedColor;
            prevValue = value;
        }
        else if(showFiltered && value == 2)
        {
            result += filteredColor;
            prevValue = value;
        }
        t += tIncr;
    }
    return result;
}

void main() {
    //float samplingRate = 2.0;
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