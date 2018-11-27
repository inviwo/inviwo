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

uniform vec3 middle;
uniform vec3 normal;
uniform vec3 up;
uniform vec3 right;
uniform float offset;
uniform float viewportAspect = 1.0f;
uniform float sliceAspect = 1.0f;

in vec2 uv;

void main() {
	// Viewport coordinates in [0,1]
    float x = uv.x;
    float y = uv.y;

    vec3 bottomLeft = middle - 0.5f * right - 0.5f * up;

	// Correct for uneven viewport aspect ratio

	float aspect = viewportAspect * sliceAspect;

	if (aspect < 1.0f) bottomLeft = middle - 0.5f * right - 0.5f * (1.0f/aspect) * up;
	else if (aspect > 1.0f) bottomLeft = middle - 0.5f * aspect * right - 0.5f * up;

	// Apply offset that controls the slab thickness
    bottomLeft += offset * normal;

    FragData0 = vec4(bottomLeft + x * right + y * up, 1.0f);
	if (aspect < 1.0f) FragData0 = vec4(bottomLeft + x * right + y * (1.0f/aspect) * up, 1.0f);
	else if (aspect > 1.0f) FragData0 = vec4(bottomLeft + x * aspect * right + y * up, 1.0f);
}