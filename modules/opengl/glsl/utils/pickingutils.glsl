/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2016 Inviwo Foundation
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

#ifndef IVW_PICKING_UTILS_GLSL
#define IVW_PICKING_UTILS_GLSL

uint reverseByte(uint b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

vec3 pickingIndexToColor(uint id) {
    uint index = id + 1;

    uint r = 0;
    uint g = 0;
    uint b = 0;

    for (int i = 0; i < 8; ++i) {
        r |= ((index & (1 << (3 * i + 2))) >> (2 * i + 2));
        g |= ((index & (1 << (3 * i + 1))) >> (2 * i + 1));
        b |= ((index & (1 << (3 * i + 0))) >> (2 * i + 0));
    }

    return vec3(reverseByte(r), reverseByte(g), reverseByte(b))/255.0;
}

uint pickingColorToIndex(vec3 color) {
    const uint r = reverseByte(uint(color[0]*255.0));
    const uint g = reverseByte(uint(color[1]*255.0));
    const uint b = reverseByte(uint(color[2]*255.0));

    uint index = 0;
    for (int i = 0; i < 8; ++i) {
        index |= (((b & (1 << i)) << (0 + 2 * i)));
        index |= (((g & (1 << i)) << (1 + 2 * i)));
        index |= (((r & (1 << i)) << (2 + 2 * i)));
    }
    return index - 1;
}


#endif // IVW_PICKING_UTILS_GLSL