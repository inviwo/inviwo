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
#ifndef IVW_ANTI_ALIASING_GLSL
#define IVW_ANTI_ALIASING_GLSL


const float PI_4 = 0.785398163397448309616;
/*
 * \brief Compute pixel coverage based on distance to line and its normal.
 * Code based on 
 * Anti-aliased Euclidean distance transform
 * by Stefan Gustavsson and Robin Strand
 * (Note errata http://weber.itn.liu.se/~stegu/edtaa/errata.pdf)
 * @param df Distance from center of pixel to the line
 * @param G Line normal (orthogonal to line: |-> )
 * 
 * See plottinggl pcp_lines.geom/frag for an example
 */
float linePixelCoverage(float df, vec2 G) {
	float a;
	if (any(lessThan(G, vec2(1.0e-6)))) {
		// Horizontal/vertical line, enough with linear approximation
		a = 0.5 - df;
	} else {
		// The analytic function is antisymmetric around df = 0
		// Both with respect to sign and transposition.
		// Means that we only need to consider G.xy >= 0 and G.x > G.y
		G = abs(G);
		if (G.x < G.y) {
			G.xy = G.yx;
		}

		float a1 = 0.5*G.y / G.x;
		float d1 = G.y;
		float d2 = (1.0/sqrt(2.0)) * sin(PI_4 - asin(G.y));

		if (df <= (-d1 - d2)) {
			a = 0;
		} else if (-d1 - d2 <= df && df <= -d2) {
			a = pow(d1 + d2 + df, 2)* (a1 / (d1*d1));
		} else {//if (-d2 <= df && df <= d2) {
			float a2 = 1.0 - 4.0*a1;
			a = a1 + (a2/2.0) * (1.0 + df / d2);
		} 
		
		// Code below is the unoptimized version of above
		/* 
		float a1 = 0.5*G.y / G.x;
		float a2 = 1.0 - 4.0*a1;
		float d1 = G.y;
		float d2 = (1.0/sqrt(2.0)) * sin(PI_4 - asin(G.y));
		
		if (df <= (-d1 - d2)) {
			a = 0;
		} else if ((-d1 - d2) <= df && df <= -d2) {
			a = pow(d1 + d2 + df, 2)* (a1 / (d1*d1));
		} else if (-d2 <= df && df < d2) {
			a = a1 + (a2/2.0) * (1.0 + df / d2);
		} else if (d2 <= df && df <= (d1 + d2)) {
			a = 1.0 - pow(d1 + d2 - df, 2) * (a1 / (d1*d1));
		} else if (df >= (d1 + d2)) {
			a = 1;
		}
		*/
	}

	// clamping might not be necessary
	a = clamp(a, 0, 1);
	return a;
}

#endif
