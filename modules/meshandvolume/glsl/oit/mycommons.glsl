/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2022 Inviwo Foundation
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

#ifndef ABUFFERCOMMONS_H
#define ABUFFERCOMMONS_H

// max depth of the chain, all others are discarded during the final rendering
#define ABUFFER_SIZE 32

// Helpers

uint compressColor10bits(vec3 color) {
    uint c = (int((color.r * 1023)) & 0x3ff) << 22;
    c += (int((color.g * 1023)) & 0x3ff) << 12;
    c += (int((color.b * 1023)) & 0x3ff) << 2;
    return c;
}
vec3 uncompressColor10bits(uint c) {
    vec3 color;
    color.r = float((c >> 22) & 0x3ff) / 1023.0f;
    color.g = float((c >> 12) & 0x3ff) / 1023.0f;
    color.b = float((c >> 2) & 0x3ff) / 1023.0f;
    return color;
}

// uvec2.x is color.r(20bits)+color.g(12bits), uvec2.y is color.g(8bits)+color.b(20bits)
uvec2 compressColor20bits(vec3 color) {
    uvec2 c = uvec2(0);
    c.x += (int((color.r * 1048575)) & 0xfffff) << 12;
    c.x += (int((color.g * 1048575)) & 0xfffff) >> 8;
    c.y += (int((color.g * 1048575)) & 0xfffff) << 24;
    c.y += (int((color.b * 1048575)) & 0xfffff) << 4;
    return c;
}
vec3 uncompressColor20bits(uvec2 c) {
    vec3 color;
    color.r = float((c.x >> 12) & 0xfffff) / 1048575.0f;
    color.g = float((c.x << 8) & 0xfffff) / 1048575.0f;
    color.g += float((c.y >> 24) & 0xfffff) / 1048575.0f;
    color.b = float((c.y >> 4) & 0xfffff) / 1048575.0f;
    return color;
}

#endif