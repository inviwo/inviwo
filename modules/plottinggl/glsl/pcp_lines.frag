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

flat in vec4 pickColor_;
in float scalarMeta_;
in float orthogonalLineDistance_; // Distance from line center [pixel]
in vec2 lineEdgeNormal_; // Normalized line edge normal (orthogonal to line)
flat in vec2 line_;
uniform vec4 color;
uniform vec4 selectColor;
uniform float mixColor;
uniform float mixAlpha;
uniform float mixSelection;

uniform float antialiasing; // width of antialised edged [pixel]
uniform float lineWidth; // line width [pixel]
uniform ivec2 dims;

uniform sampler2D tf;


void main() {
    vec4 res = texture(tf, vec2(scalarMeta_, 0.5f));

    res = mix(res, color, vec4(vec3(mixColor), mixAlpha));
    res = mix(res, selectColor, vec4(mixSelection));

	// Analytic anti-aliasing
    #define ANALYTIC_ANTIALIASING
    //#ifdef ANALYTIC_ANTIALIASING
    if (lineWidth < 10) {
        
    
    vec2 n = normalize(lineEdgeNormal_);
	float df = abs(orthogonalLineDistance_) - 0.5*lineWidth;
    vec2 o_line = (0.5*(1+line_)*(dims-1));
    vec2 o_u = o_line + 0.5*lineWidth*n;
    vec2 o_l = o_line - 0.5*lineWidth*n;
    float du = dot(gl_FragCoord.xy-o_u, n);
    float dl = dot(gl_FragCoord.xy-o_l, n);
    //float df = abs(du) < abs(dl) ? du : dl;
	if (abs(df) <= sqrt(2)) {
    //if (df > 0) {
        vec2 pixelSpacing = 1.0f / dims.xy;
		//res.w *= linePixelCoverage(df, normalize(lineEdgeNormal_));
        //res.rgb = vec3(0, 1, 0);
    } else if (df < -sqrt(2)) {
        //df /= 100;
        //res.rgb = vec3(0, abs(df)/(0.5*lineWidth), abs(df)/(0.5*lineWidth));
	} else if (abs(df) > 0.5*sqrt(2)) {
		// pixel is outside of line
        //res.rgb = vec3(1, 0, 1);
        //res.w = 1;
    } else {
        
    }
    //res.rgb = vec3(1);
    
    //res.w = abs(dl);
                  res.w = linePixelCoverage(df, normalize(lineEdgeNormal_));
   //         res= vec4(0.5*(1+line_), 0, 1.0);
     
    /*
    // Based on
    // 2D Shape Rendering by Distance Fields
    // Distance field crossing 0 at edges:
    //      <-line width->
    //   - | + positive  | -
    //    edge         edge
    //
    float D = 0.5*lineWidth - abs(orthogonalLineDistance_);
    // Perform anisotropic analytic antialiasing.
    // We can greatlty simplify the analytic computation
    // becasue we know the partial derivative of our
    // distance field, i.e. distance to line center, changes
    // with exactly 1 per pixel.
    // Hence, sqrt( (dF/dx)^2 + (dF/dy)^2 ) = sqrt(1+1)
    // Multiply with 0.7 instead of 0.5 (linear ramp) to compensate
    // for smoothstep's different endpoint smoothness and slope
    //float aastep = 0.7 * length(vec2(dFdx(D), dFdy(D)));
    float aastep = antialiasing * sqrt(1.0+1.0);
    // 1 where D > 0, 0 where D < 0, with analytic AA around D = 0.
    float d = smoothstep(-aastep, aastep, D);
    res.w *= d;
    //res = vec4(vec3(d), 1.0);
    */
    } else {
    //#else
        // Filtered anti-aliasing
        float linewidthHalf = lineWidth * 0.5;
        float distance = abs(orthogonalLineDistance_);
        float d = distance - (linewidthHalf);
        // antialiasing around the edges
        //if( d > -antialiasing) {
        if( d > 0) {
            // apply antialiasing by modifying the alpha [Rougier, Journal of Computer Graphics Techniques 2013]
            d /= antialiasing;
            /*
            if (d < 0) {
                // increases from -d to 0 (edge approaching pixel center)
                res.w = 1-(exp(-d*d));
            } else {
                // decreases from 0 to d (edge
                res.w = (exp(-d*d));
            }
            */
            //res.w *= (exp(-d*antialiasing*antialiasing));
            res.w *= (exp(-d*d));
        } else if (d > antialiasing) {
            //res.w = 0;
            //discard;
        }
    }

    //#endif
    PickingData = pickColor_;
    FragData0 = res;
}
