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

smooth in vec2 uv;

const uint num_pts = 5;
const vec3 pts[ num_pts ] = {
    vec3( 0.208,    0.498,    0.213 ),
    vec3( 0.141,    0.498,    0.414 ),
    vec3( 0.137,    0.498,    0.586 ),
    vec3( 0.208,    0.498,    0.801 ),
    vec3( 0.224,    0.498,    0.826 )
};
float accumulated_distance[ num_pts ] = {0.0, 0.0, 0.0, 0.0, 0.0};

void calc_accumulated_distance() // remove from shader
{
    accumulated_distance[0] = 0.0;
    for (uint idx = 1; idx < num_pts; ++idx)
    {
        const vec3 p1 = pts[idx - 1];
        const vec3 p2 = pts[idx];
        accumulated_distance[idx] = accumulated_distance[idx - 1] + distance(p1, p2);
    }

    for (uint idx = 1; idx < num_pts; ++idx)
    {
        accumulated_distance[idx] /= accumulated_distance[num_pts - 1];
    }
}

vec3 get_normal_xy(vec3 direction)
{
    vec3 normal = vec3(-direction.y, direction.x, 0.0);
    return normalize(normal);
}

vec3 get_normal_xz(vec3 direction)
{
    vec3 normal = vec3(-direction.z, 0.0, direction.x);
    return normalize(normal);
}

vec3 get_normal_yz(vec3 direction)
{
    vec3 normal = vec3(0.0, -direction.z, direction.y);
    return normalize(normal);
}

vec3 interpolate_linear(float t, out vec3 normal, out vec3 direction, out vec3 binormal)
{
    vec3 pt = vec3(0.0);

    if (t <= 0.0)
    {
        pt = pts[0];

        direction = normalize(pts[1] - pts[0]);
    }
    else if (t >= 1.0)
    {
        pt = pts[num_pts - 1];

        direction = normalize(pts[num_pts - 2] - pts[num_pts - 1]);
    }
    else
    {
        for (uint idx = 1; idx < num_pts; ++idx)
        {
            if (accumulated_distance[idx] > t) // t is between i-1 and i
            {
                vec3 p1 = pts[idx - 1];
                vec3 p2 = pts[idx];

                float d1 = accumulated_distance[idx - 1];
                float d2 = accumulated_distance[idx];

                float t_normalized = (t - d1) / (d2 - d1);
                pt = mix(p1, p2, t_normalized);

                direction = normalize(p2 - p1);

                break;
            }
        }
    }

    normal = get_normal_xz(direction);
    binormal = normalize(cross(normal, direction));

    return pt;
}

void main() {
    calc_accumulated_distance(); // pre-processing of line, move to CPU code to calc. only once

    float depth_offset_start = -0.45;
    float depth_offset_end = 0.0;
    float dist_to_centerline = 0.04;

    vec3 normal = vec3(0.0);
    vec3 direction = vec3(0.0);
    vec3 binormal = vec3(0.0);
    vec3 pt_centerline = interpolate_linear(uv.x, normal, direction, binormal);
    vec3 entry_pt = pt_centerline
                        + dist_to_centerline * normal
                        + mix(depth_offset_start, depth_offset_end, uv.y) * binormal;
    
    FragData0 = vec4(entry_pt, 1.0);
}
