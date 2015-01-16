/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

// Trilinear interpolation 
// Example of how to get data values       
// float s[8];int i = 0;
// for(int z = zMin; z <= zMin+1; ++z) {
//     for(int y = yMin; y <= yMin+1; ++y) {
//          for(int x = xMin; x <= xMin+1; ++x) {
//              s[i++] = read_imagef(volume, smpUNormNoClampNearest, (int4)(x,y,z,0)).x;
//          }
//     }
// }
float trilinearInterploation(const float3 s, const float d[8]) {
	// Interpolate along z
	float i1 = mix(d[0], d[4], s.z);
	float i2 = mix(d[1], d[5], s.z);
	float i3 = mix(d[2], d[6], s.z);
	float i4 = mix(d[3], d[7], s.z);
	// interpolate along y 
	float w1 = mix(i1, i3, s.y);
	float w2 = mix(i2, i4, s.y);

	return mix(w1, w2, s.x);
}