/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2018 Inviwo Foundation
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

const float TWO_PI = 6.28318530718;

// NUM_PTS should be set from host code
// whenever it changes the shader must be rebuilt
#ifndef NUM_PTS
#define NUM_PTS 2
#endif

uniform vec3 pts[ NUM_PTS ];
uniform float accumulated_distance[ NUM_PTS ];
uniform vec3 up_vector;
uniform float radius;
uniform float angle_offset;

smooth in vec2 uv;

/**
 * Return interpolated point in polyline pts.
 * Rely on precomputed accumulated distances.
 * @param t between 0 and 1 
 * @param normal
 * @param direction */
vec3 interpolate_linear(float t, out vec3 normal, out vec3 direction)
{
    vec3 pt = vec3(0.0);

    if (t <= 0.0)
    {
        pt = pts[0];

		direction = normalize(pts[1] - pts[0]);
		normal = normalize(vec3(direction.y, -direction.x, direction.z));
    }
    else if (t >= 1.0)
    {
        pt = pts[NUM_PTS - 1];

		direction = normalize(pts[NUM_PTS - 2] - pts[NUM_PTS - 1]);
		normal = normalize(vec3(direction.y, -direction.x, direction.z));
    }
    else
    {
        for (uint idx = 1; idx < NUM_PTS; ++idx)
        {
            if (accumulated_distance[idx] > t) // t is between i-1 and i
            {
                vec3 p1 = pts[idx - 1];
                vec3 p2 = pts[idx];
				direction = normalize(p2 - p1);

                float d1 = accumulated_distance[idx - 1];
                float d2 = accumulated_distance[idx];

                float t_normalized = (t - d1) / (d2 - d1);
				
				// smoothing direction
				vec3 p3 = idx < NUM_PTS-1 ? pts[idx+1] : p2;
				vec3 next_direction = normalize(p3-p2);
				direction = mix(direction, next_direction, t_normalized);

				normal = normalize(cross(up_vector, direction));
				//normal = normalize(vec3(direction.y, -direction.x, direction.z)); // ToDo: calc. actual normal

                pt = mix(p1, p2, t_normalized);

                break;
            }
        }
    }

    return pt;
}

void main() {
	vec3 normal;
	vec3 direction;
    vec3 entry_pt = interpolate_linear(uv.x, normal, direction);
	
	float angle = uv.y * TWO_PI;
	vec3 binormal = cross(normal, direction);
	vec3 exit_pt = entry_pt + (normal * sin(angle + angle_offset) + binormal * cos(angle + angle_offset)) * radius;
    FragData0 = vec4(exit_pt, 0);
	
	// uncomment for hardcoded 2-point-polyline with radius 0.5
	/*vec3 start = vec3(0, 0.5, 0.5);
	vec3 end = vec3(1, 0.5, 0.5);
	vec3 pos = mix(start, end, uv.x);
	float angle = uv.y * TWO_PI;
	vec3 normal = vec3(0, 1, 0);
	vec3 binormal = vec3(0, 0, 1);
	float radius = 0.5;
    FragData0 = vec4(pos + (normal * sin(angle) + binormal * cos(angle)) * radius, 0);*/
}
