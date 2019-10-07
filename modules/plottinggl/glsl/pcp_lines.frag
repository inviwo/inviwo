/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include "utils/antialiasing.glsl"

in vec4 pickColor_;
in float scalarMeta_;
in float orthogonalLineDistance_; // Distance from line center [pixel]
in vec2 lineEdgeNormal_; // Normalized line edge normal (orthogonal to line)

uniform vec4 color;
uniform vec4 selectColor;
uniform float mixColor;
uniform float mixAlpha;
uniform float mixSelection;

uniform float antialiasing; // width of antialised edged [pixel]
uniform float lineWidth; // line width [pixel]

uniform sampler2D tf;




void main() {
    vec4 res = texture(tf, vec2(scalarMeta_, 0.5f));

    res = mix(res, color, vec4(vec3(mixColor), mixAlpha));
    res = mix(res, selectColor, vec4(mixSelection));

	// Analytic anti-aliasing
	float alpha = 1.0;
	float df = abs(orthogonalLineDistance_) - 0.5*lineWidth;
	if (abs(df) <= 0.5*sqrt(2.0)) {
		res.w *= linePixelCoverage(df, lineEdgeNormal_);
	} else {
		// edge is more than one pixel away
	}
	/*
	// Filtered anti-aliasing
    float linewidthHalf = lineWidth * 0.5;
    float distance = abs(orthogonalLineDistance_);
    float d = distance - (linewidthHalf);
    // antialiasing around the edges
    if( d > 0) {
        // apply antialiasing by modifying the alpha [Rougier, Journal of Computer Graphics Techniques 2013]
        d /= antialiasing;
        res.w *= = exp(-d*d);
    } */
    //res = vec4(vec3(alpha), 1.0);
    PickingData = pickColor_;
    FragData0 = res;
}
