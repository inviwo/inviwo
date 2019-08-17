/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#include <modules/base/algorithm/volume/marchingcubes.h>
#include <modules/base/algorithm/volume/surfaceextraction.h>

namespace inviwo {

namespace marchingcubes {

struct Triangle {
    size_t e0a;
    size_t e0b;
    size_t e1a;
    size_t e1b;
    size_t e2a;
    size_t e2b;
};

//  v7 ----- v6
//  /|       /|
// v3 ----- v2|
// |v4 - - -|v5
// |/       |/
// v0 ----- v1

const static std::array<size3_t, 8> offs = {size3_t{0, 0, 0}, size3_t{1, 0, 0}, size3_t{1, 1, 0},
                                            size3_t{0, 1, 0}, size3_t{0, 0, 1}, size3_t{1, 0, 1},
                                            size3_t{1, 1, 1}, size3_t{0, 1, 1}};

std::array<std::vector<Triangle>, 256> cases = {
    std::vector<Triangle>{},
    std::vector<Triangle>{Triangle{0, 1, 0, 4, 3, 0}},
    std::vector<Triangle>{Triangle{0, 1, 1, 2, 1, 5}},
    std::vector<Triangle>{Triangle{1, 2, 0, 4, 3, 0}, Triangle{1, 5, 0, 4, 1, 2}},
    std::vector<Triangle>{Triangle{1, 2, 2, 3, 2, 6}},
    std::vector<Triangle>{Triangle{0, 1, 0, 4, 3, 0}, Triangle{1, 2, 2, 3, 2, 6}},
    std::vector<Triangle>{Triangle{1, 5, 2, 3, 2, 6}, Triangle{0, 1, 2, 3, 1, 5}},
    std::vector<Triangle>{Triangle{2, 3, 0, 4, 3, 0}, Triangle{2, 3, 2, 6, 0, 4},
                          Triangle{2, 6, 1, 5, 0, 4}},
    std::vector<Triangle>{Triangle{3, 0, 3, 7, 2, 3}},
    std::vector<Triangle>{Triangle{0, 1, 3, 7, 2, 3}, Triangle{0, 4, 3, 7, 0, 1}},
    std::vector<Triangle>{Triangle{1, 2, 1, 5, 0, 1}, Triangle{2, 3, 3, 0, 3, 7}},
    std::vector<Triangle>{Triangle{1, 2, 3, 7, 2, 3}, Triangle{1, 2, 1, 5, 3, 7},
                          Triangle{1, 5, 0, 4, 3, 7}},
    std::vector<Triangle>{Triangle{3, 0, 2, 6, 1, 2}, Triangle{3, 7, 2, 6, 3, 0}},
    std::vector<Triangle>{Triangle{0, 1, 2, 6, 1, 2}, Triangle{0, 1, 0, 4, 2, 6},
                          Triangle{0, 4, 3, 7, 2, 6}},
    std::vector<Triangle>{Triangle{3, 0, 1, 5, 0, 1}, Triangle{3, 0, 3, 7, 1, 5},
                          Triangle{3, 7, 2, 6, 1, 5}},
    std::vector<Triangle>{Triangle{1, 5, 0, 4, 2, 6}, Triangle{2, 6, 0, 4, 3, 7}},
    std::vector<Triangle>{Triangle{4, 5, 7, 4, 0, 4}},
    std::vector<Triangle>{Triangle{4, 5, 3, 0, 0, 1}, Triangle{7, 4, 3, 0, 4, 5}},
    std::vector<Triangle>{Triangle{0, 1, 1, 2, 1, 5}, Triangle{0, 4, 4, 5, 7, 4}},
    std::vector<Triangle>{Triangle{4, 5, 1, 2, 1, 5}, Triangle{4, 5, 7, 4, 1, 2},
                          Triangle{7, 4, 3, 0, 1, 2}},
    std::vector<Triangle>{Triangle{1, 2, 2, 3, 2, 6}, Triangle{0, 4, 4, 5, 7, 4}},
    std::vector<Triangle>{Triangle{3, 0, 4, 5, 7, 4}, Triangle{3, 0, 0, 1, 4, 5},
                          Triangle{1, 2, 2, 3, 2, 6}},
    std::vector<Triangle>{Triangle{1, 5, 2, 3, 2, 6}, Triangle{1, 5, 0, 1, 2, 3},
                          Triangle{0, 4, 4, 5, 7, 4}},
    std::vector<Triangle>{Triangle{2, 3, 2, 6, 1, 5}, Triangle{2, 3, 1, 5, 7, 4},
                          Triangle{2, 3, 7, 4, 3, 0}, Triangle{7, 4, 1, 5, 4, 5}},
    std::vector<Triangle>{Triangle{0, 4, 4, 5, 7, 4}, Triangle{3, 0, 3, 7, 2, 3}},
    std::vector<Triangle>{Triangle{3, 7, 4, 5, 7, 4}, Triangle{3, 7, 2, 3, 4, 5},
                          Triangle{2, 3, 0, 1, 4, 5}},
    std::vector<Triangle>{Triangle{1, 5, 0, 1, 1, 2}, Triangle{0, 4, 4, 5, 7, 4},
                          Triangle{2, 3, 3, 0, 3, 7}},
    std::vector<Triangle>{Triangle{4, 5, 7, 4, 3, 7}, Triangle{1, 5, 4, 5, 3, 7},
                          Triangle{1, 5, 3, 7, 2, 3}, Triangle{1, 5, 2, 3, 1, 2}},
    std::vector<Triangle>{Triangle{3, 0, 2, 6, 1, 2}, Triangle{3, 0, 3, 7, 2, 6},
                          Triangle{7, 4, 0, 4, 4, 5}},
    std::vector<Triangle>{Triangle{1, 2, 3, 7, 2, 6}, Triangle{1, 2, 4, 5, 3, 7},
                          Triangle{1, 2, 0, 1, 4, 5}, Triangle{7, 4, 3, 7, 4, 5}},
    std::vector<Triangle>{Triangle{4, 5, 7, 4, 0, 4}, Triangle{1, 5, 0, 1, 3, 7},
                          Triangle{1, 5, 3, 7, 2, 6}, Triangle{3, 7, 0, 1, 3, 0}},
    std::vector<Triangle>{Triangle{4, 5, 7, 4, 3, 7}, Triangle{4, 5, 3, 7, 1, 5},
                          Triangle{1, 5, 3, 7, 2, 6}},
    std::vector<Triangle>{Triangle{1, 5, 5, 6, 4, 5}},
    std::vector<Triangle>{Triangle{1, 5, 5, 6, 4, 5}, Triangle{0, 1, 0, 4, 3, 0}},
    std::vector<Triangle>{Triangle{0, 1, 5, 6, 4, 5}, Triangle{1, 2, 5, 6, 0, 1}},
    std::vector<Triangle>{Triangle{0, 4, 5, 6, 4, 5}, Triangle{0, 4, 3, 0, 5, 6},
                          Triangle{3, 0, 1, 2, 5, 6}},
    std::vector<Triangle>{Triangle{1, 2, 2, 3, 2, 6}, Triangle{1, 5, 5, 6, 4, 5}},
    std::vector<Triangle>{Triangle{3, 0, 0, 1, 0, 4}, Triangle{1, 2, 2, 3, 2, 6},
                          Triangle{4, 5, 1, 5, 5, 6}},
    std::vector<Triangle>{Triangle{5, 6, 2, 3, 2, 6}, Triangle{5, 6, 4, 5, 2, 3},
                          Triangle{4, 5, 0, 1, 2, 3}},
    std::vector<Triangle>{Triangle{2, 3, 2, 6, 5, 6}, Triangle{3, 0, 2, 3, 5, 6},
                          Triangle{3, 0, 5, 6, 4, 5}, Triangle{3, 0, 4, 5, 0, 4}},
    std::vector<Triangle>{Triangle{1, 5, 5, 6, 4, 5}, Triangle{2, 3, 3, 0, 3, 7}},
    std::vector<Triangle>{Triangle{0, 1, 3, 7, 2, 3}, Triangle{0, 1, 0, 4, 3, 7},
                          Triangle{4, 5, 1, 5, 5, 6}},
    std::vector<Triangle>{Triangle{0, 1, 5, 6, 4, 5}, Triangle{0, 1, 1, 2, 5, 6},
                          Triangle{2, 3, 3, 0, 3, 7}},
    std::vector<Triangle>{Triangle{2, 3, 1, 2, 5, 6}, Triangle{2, 3, 5, 6, 0, 4},
                          Triangle{2, 3, 0, 4, 3, 7}, Triangle{4, 5, 0, 4, 5, 6}},
    std::vector<Triangle>{Triangle{2, 6, 3, 0, 3, 7}, Triangle{2, 6, 1, 2, 3, 0},
                          Triangle{1, 5, 5, 6, 4, 5}},
    std::vector<Triangle>{Triangle{4, 5, 1, 5, 5, 6}, Triangle{0, 1, 0, 4, 1, 2},
                          Triangle{0, 4, 2, 6, 1, 2}, Triangle{0, 4, 3, 7, 2, 6}},
    std::vector<Triangle>{Triangle{5, 6, 4, 5, 0, 1}, Triangle{5, 6, 0, 1, 3, 7},
                          Triangle{5, 6, 3, 7, 2, 6}, Triangle{3, 7, 0, 1, 3, 0}},
    std::vector<Triangle>{Triangle{5, 6, 4, 5, 0, 4}, Triangle{5, 6, 0, 4, 2, 6},
                          Triangle{2, 6, 0, 4, 3, 7}},
    std::vector<Triangle>{Triangle{1, 5, 7, 4, 0, 4}, Triangle{5, 6, 7, 4, 1, 5}},
    std::vector<Triangle>{Triangle{1, 5, 3, 0, 0, 1}, Triangle{1, 5, 5, 6, 3, 0},
                          Triangle{5, 6, 7, 4, 3, 0}},
    std::vector<Triangle>{Triangle{0, 1, 7, 4, 0, 4}, Triangle{0, 1, 1, 2, 7, 4},
                          Triangle{1, 2, 5, 6, 7, 4}},
    std::vector<Triangle>{Triangle{1, 2, 5, 6, 3, 0}, Triangle{3, 0, 5, 6, 7, 4}},
    std::vector<Triangle>{Triangle{1, 5, 7, 4, 0, 4}, Triangle{1, 5, 5, 6, 7, 4},
                          Triangle{2, 6, 1, 2, 2, 3}},
    std::vector<Triangle>{Triangle{2, 6, 1, 2, 2, 3}, Triangle{1, 5, 5, 6, 0, 1},
                          Triangle{5, 6, 3, 0, 0, 1}, Triangle{5, 6, 7, 4, 3, 0}},
    std::vector<Triangle>{Triangle{0, 4, 0, 1, 2, 3}, Triangle{0, 4, 2, 3, 5, 6},
                          Triangle{0, 4, 5, 6, 7, 4}, Triangle{2, 6, 5, 6, 2, 3}},
    std::vector<Triangle>{Triangle{2, 3, 2, 6, 5, 6}, Triangle{2, 3, 5, 6, 3, 0},
                          Triangle{3, 0, 5, 6, 7, 4}},
    std::vector<Triangle>{Triangle{7, 4, 1, 5, 5, 6}, Triangle{7, 4, 0, 4, 1, 5},
                          Triangle{3, 0, 3, 7, 2, 3}},
    std::vector<Triangle>{Triangle{1, 5, 5, 6, 7, 4}, Triangle{1, 5, 7, 4, 2, 3},
                          Triangle{1, 5, 2, 3, 0, 1}, Triangle{2, 3, 7, 4, 3, 7}},
    std::vector<Triangle>{Triangle{2, 3, 3, 0, 3, 7}, Triangle{0, 1, 1, 2, 0, 4},
                          Triangle{1, 2, 7, 4, 0, 4}, Triangle{1, 2, 5, 6, 7, 4}},
    std::vector<Triangle>{Triangle{3, 7, 2, 3, 1, 2}, Triangle{3, 7, 1, 2, 7, 4},
                          Triangle{7, 4, 1, 2, 5, 6}},
    std::vector<Triangle>{Triangle{1, 5, 5, 6, 0, 4}, Triangle{0, 4, 5, 6, 7, 4},
                          Triangle{2, 6, 1, 2, 3, 0}, Triangle{2, 6, 3, 0, 3, 7}},
    std::vector<Triangle>{Triangle{5, 6, 7, 4, 0, 1}, Triangle{5, 6, 0, 1, 1, 5},
                          Triangle{7, 4, 3, 7, 0, 1}, Triangle{1, 2, 0, 1, 2, 6},
                          Triangle{3, 7, 2, 6, 0, 1}},
    std::vector<Triangle>{Triangle{3, 7, 2, 6, 0, 1}, Triangle{3, 7, 0, 1, 3, 0},
                          Triangle{2, 6, 5, 6, 0, 1}, Triangle{0, 4, 0, 1, 7, 4},
                          Triangle{5, 6, 7, 4, 0, 1}},
    std::vector<Triangle>{Triangle{3, 7, 2, 6, 5, 6}, Triangle{7, 4, 3, 7, 5, 6}},
    std::vector<Triangle>{Triangle{2, 6, 6, 7, 5, 6}},
    std::vector<Triangle>{Triangle{0, 1, 0, 4, 3, 0}, Triangle{5, 6, 2, 6, 6, 7}},
    std::vector<Triangle>{Triangle{1, 5, 0, 1, 1, 2}, Triangle{5, 6, 2, 6, 6, 7}},
    std::vector<Triangle>{Triangle{1, 2, 0, 4, 3, 0}, Triangle{1, 2, 1, 5, 0, 4},
                          Triangle{5, 6, 2, 6, 6, 7}},
    std::vector<Triangle>{Triangle{1, 2, 6, 7, 5, 6}, Triangle{2, 3, 6, 7, 1, 2}},
    std::vector<Triangle>{Triangle{1, 2, 6, 7, 5, 6}, Triangle{1, 2, 2, 3, 6, 7},
                          Triangle{3, 0, 0, 1, 0, 4}},
    std::vector<Triangle>{Triangle{1, 5, 6, 7, 5, 6}, Triangle{1, 5, 0, 1, 6, 7},
                          Triangle{0, 1, 2, 3, 6, 7}},
    std::vector<Triangle>{Triangle{5, 6, 1, 5, 0, 4}, Triangle{5, 6, 0, 4, 2, 3},
                          Triangle{5, 6, 2, 3, 6, 7}, Triangle{3, 0, 2, 3, 0, 4}},
    std::vector<Triangle>{Triangle{2, 3, 3, 0, 3, 7}, Triangle{2, 6, 6, 7, 5, 6}},
    std::vector<Triangle>{Triangle{3, 7, 0, 1, 0, 4}, Triangle{3, 7, 2, 3, 0, 1},
                          Triangle{2, 6, 6, 7, 5, 6}},
    std::vector<Triangle>{Triangle{0, 1, 1, 2, 1, 5}, Triangle{2, 3, 3, 0, 3, 7},
                          Triangle{5, 6, 2, 6, 6, 7}},
    std::vector<Triangle>{Triangle{5, 6, 2, 6, 6, 7}, Triangle{1, 2, 1, 5, 2, 3},
                          Triangle{1, 5, 3, 7, 2, 3}, Triangle{1, 5, 0, 4, 3, 7}},
    std::vector<Triangle>{Triangle{6, 7, 3, 0, 3, 7}, Triangle{6, 7, 5, 6, 3, 0},
                          Triangle{5, 6, 1, 2, 3, 0}},
    std::vector<Triangle>{Triangle{0, 1, 0, 4, 3, 7}, Triangle{0, 1, 3, 7, 5, 6},
                          Triangle{0, 1, 5, 6, 1, 2}, Triangle{5, 6, 3, 7, 6, 7}},
    std::vector<Triangle>{Triangle{3, 0, 3, 7, 6, 7}, Triangle{0, 1, 3, 0, 6, 7},
                          Triangle{0, 1, 6, 7, 5, 6}, Triangle{0, 1, 5, 6, 1, 5}},
    std::vector<Triangle>{Triangle{6, 7, 5, 6, 1, 5}, Triangle{6, 7, 1, 5, 3, 7},
                          Triangle{3, 7, 1, 5, 0, 4}},
    std::vector<Triangle>{Triangle{5, 6, 2, 6, 6, 7}, Triangle{4, 5, 7, 4, 0, 4}},
    std::vector<Triangle>{Triangle{4, 5, 3, 0, 0, 1}, Triangle{4, 5, 7, 4, 3, 0},
                          Triangle{6, 7, 5, 6, 2, 6}},
    std::vector<Triangle>{Triangle{1, 2, 1, 5, 0, 1}, Triangle{5, 6, 2, 6, 6, 7},
                          Triangle{0, 4, 4, 5, 7, 4}},
    std::vector<Triangle>{Triangle{2, 6, 6, 7, 5, 6}, Triangle{1, 2, 1, 5, 7, 4},
                          Triangle{1, 2, 7, 4, 3, 0}, Triangle{7, 4, 1, 5, 4, 5}},
    std::vector<Triangle>{Triangle{6, 7, 1, 2, 2, 3}, Triangle{6, 7, 5, 6, 1, 2},
                          Triangle{4, 5, 7, 4, 0, 4}},
    std::vector<Triangle>{Triangle{1, 2, 2, 3, 5, 6}, Triangle{5, 6, 2, 3, 6, 7},
                          Triangle{3, 0, 0, 1, 4, 5}, Triangle{3, 0, 4, 5, 7, 4}},
    std::vector<Triangle>{Triangle{0, 4, 4, 5, 7, 4}, Triangle{1, 5, 0, 1, 5, 6},
                          Triangle{0, 1, 6, 7, 5, 6}, Triangle{0, 1, 2, 3, 6, 7}},
    std::vector<Triangle>{Triangle{7, 4, 3, 0, 1, 5}, Triangle{7, 4, 1, 5, 4, 5},
                          Triangle{3, 0, 2, 3, 1, 5}, Triangle{5, 6, 1, 5, 6, 7},
                          Triangle{2, 3, 6, 7, 1, 5}},
    std::vector<Triangle>{Triangle{3, 0, 3, 7, 2, 3}, Triangle{7, 4, 0, 4, 4, 5},
                          Triangle{2, 6, 6, 7, 5, 6}},
    std::vector<Triangle>{Triangle{5, 6, 2, 6, 6, 7}, Triangle{4, 5, 7, 4, 2, 3},
                          Triangle{4, 5, 2, 3, 0, 1}, Triangle{2, 3, 7, 4, 3, 7}},
    std::vector<Triangle>{Triangle{0, 1, 1, 2, 1, 5}, Triangle{4, 5, 7, 4, 0, 4},
                          Triangle{2, 3, 3, 0, 3, 7}, Triangle{5, 6, 2, 6, 6, 7}},
    std::vector<Triangle>{Triangle{1, 5, 2, 3, 1, 2}, Triangle{1, 5, 3, 7, 2, 3},
                          Triangle{1, 5, 4, 5, 3, 7}, Triangle{7, 4, 3, 7, 4, 5},
                          Triangle{5, 6, 2, 6, 6, 7}},
    std::vector<Triangle>{Triangle{0, 4, 4, 5, 7, 4}, Triangle{3, 0, 3, 7, 5, 6},
                          Triangle{3, 0, 5, 6, 1, 2}, Triangle{5, 6, 3, 7, 6, 7}},
    std::vector<Triangle>{Triangle{5, 6, 1, 2, 3, 7}, Triangle{5, 6, 3, 7, 6, 7},
                          Triangle{1, 2, 0, 1, 3, 7}, Triangle{7, 4, 3, 7, 4, 5},
                          Triangle{0, 1, 4, 5, 3, 7}},
    std::vector<Triangle>{Triangle{0, 1, 5, 6, 1, 5}, Triangle{0, 1, 6, 7, 5, 6},
                          Triangle{0, 1, 3, 0, 6, 7}, Triangle{3, 7, 6, 7, 3, 0},
                          Triangle{0, 4, 4, 5, 7, 4}},
    std::vector<Triangle>{Triangle{6, 7, 5, 6, 1, 5}, Triangle{6, 7, 1, 5, 3, 7},
                          Triangle{4, 5, 7, 4, 1, 5}, Triangle{7, 4, 3, 7, 1, 5}},
    std::vector<Triangle>{Triangle{2, 6, 4, 5, 1, 5}, Triangle{6, 7, 4, 5, 2, 6}},
    std::vector<Triangle>{Triangle{4, 5, 2, 6, 6, 7}, Triangle{4, 5, 1, 5, 2, 6},
                          Triangle{0, 1, 0, 4, 3, 0}},
    std::vector<Triangle>{Triangle{2, 6, 0, 1, 1, 2}, Triangle{2, 6, 6, 7, 0, 1},
                          Triangle{6, 7, 4, 5, 0, 1}},
    std::vector<Triangle>{Triangle{0, 4, 3, 0, 1, 2}, Triangle{0, 4, 1, 2, 6, 7},
                          Triangle{0, 4, 6, 7, 4, 5}, Triangle{6, 7, 1, 2, 2, 6}},
    std::vector<Triangle>{Triangle{1, 2, 4, 5, 1, 5}, Triangle{1, 2, 2, 3, 4, 5},
                          Triangle{2, 3, 6, 7, 4, 5}},
    std::vector<Triangle>{Triangle{3, 0, 0, 1, 0, 4}, Triangle{1, 2, 2, 3, 1, 5},
                          Triangle{2, 3, 4, 5, 1, 5}, Triangle{2, 3, 6, 7, 4, 5}},
    std::vector<Triangle>{Triangle{0, 1, 2, 3, 4, 5}, Triangle{4, 5, 2, 3, 6, 7}},
    std::vector<Triangle>{Triangle{0, 4, 3, 0, 2, 3}, Triangle{0, 4, 2, 3, 4, 5},
                          Triangle{4, 5, 2, 3, 6, 7}},
    std::vector<Triangle>{Triangle{2, 6, 4, 5, 1, 5}, Triangle{2, 6, 6, 7, 4, 5},
                          Triangle{3, 7, 2, 3, 3, 0}},
    std::vector<Triangle>{Triangle{0, 1, 0, 4, 2, 3}, Triangle{2, 3, 0, 4, 3, 7},
                          Triangle{4, 5, 1, 5, 2, 6}, Triangle{4, 5, 2, 6, 6, 7}},
    std::vector<Triangle>{Triangle{3, 0, 3, 7, 2, 3}, Triangle{0, 1, 1, 2, 6, 7},
                          Triangle{0, 1, 6, 7, 4, 5}, Triangle{6, 7, 1, 2, 2, 6}},
    std::vector<Triangle>{Triangle{6, 7, 4, 5, 1, 2}, Triangle{6, 7, 1, 2, 2, 6},
                          Triangle{4, 5, 0, 4, 1, 2}, Triangle{2, 3, 1, 2, 3, 7},
                          Triangle{0, 4, 3, 7, 1, 2}},
    std::vector<Triangle>{Triangle{1, 5, 6, 7, 4, 5}, Triangle{1, 5, 3, 0, 6, 7},
                          Triangle{1, 5, 1, 2, 3, 0}, Triangle{3, 7, 6, 7, 3, 0}},
    std::vector<Triangle>{Triangle{0, 4, 3, 7, 1, 2}, Triangle{0, 4, 1, 2, 0, 1},
                          Triangle{3, 7, 6, 7, 1, 2}, Triangle{1, 5, 1, 2, 4, 5},
                          Triangle{6, 7, 4, 5, 1, 2}},
    std::vector<Triangle>{Triangle{3, 0, 3, 7, 6, 7}, Triangle{3, 0, 6, 7, 0, 1},
                          Triangle{0, 1, 6, 7, 4, 5}},
    std::vector<Triangle>{Triangle{6, 7, 4, 5, 0, 4}, Triangle{3, 7, 6, 7, 0, 4}},
    std::vector<Triangle>{Triangle{7, 4, 2, 6, 6, 7}, Triangle{7, 4, 0, 4, 2, 6},
                          Triangle{0, 4, 1, 5, 2, 6}},
    std::vector<Triangle>{Triangle{0, 1, 7, 4, 3, 0}, Triangle{0, 1, 2, 6, 7, 4},
                          Triangle{0, 1, 1, 5, 2, 6}, Triangle{6, 7, 7, 4, 2, 6}},
    std::vector<Triangle>{Triangle{2, 6, 6, 7, 7, 4}, Triangle{1, 2, 2, 6, 7, 4},
                          Triangle{1, 2, 7, 4, 0, 4}, Triangle{1, 2, 0, 4, 0, 1}},
    std::vector<Triangle>{Triangle{2, 6, 6, 7, 7, 4}, Triangle{2, 6, 7, 4, 1, 2},
                          Triangle{1, 2, 7, 4, 3, 0}},
    std::vector<Triangle>{Triangle{1, 2, 2, 3, 6, 7}, Triangle{1, 2, 6, 7, 0, 4},
                          Triangle{1, 2, 0, 4, 1, 5}, Triangle{0, 4, 6, 7, 7, 4}},
    std::vector<Triangle>{Triangle{2, 3, 6, 7, 1, 5}, Triangle{2, 3, 1, 5, 1, 2},
                          Triangle{6, 7, 7, 4, 1, 5}, Triangle{0, 1, 1, 5, 3, 0},
                          Triangle{7, 4, 3, 0, 1, 5}},
    std::vector<Triangle>{Triangle{7, 4, 0, 4, 0, 1}, Triangle{7, 4, 0, 1, 6, 7},
                          Triangle{6, 7, 0, 1, 2, 3}},
    std::vector<Triangle>{Triangle{7, 4, 3, 0, 2, 3}, Triangle{6, 7, 7, 4, 2, 3}},
    std::vector<Triangle>{Triangle{2, 3, 3, 0, 3, 7}, Triangle{2, 6, 6, 7, 0, 4},
                          Triangle{2, 6, 0, 4, 1, 5}, Triangle{0, 4, 6, 7, 7, 4}},
    std::vector<Triangle>{Triangle{2, 3, 0, 1, 7, 4}, Triangle{2, 3, 7, 4, 3, 7},
                          Triangle{0, 1, 1, 5, 7, 4}, Triangle{6, 7, 7, 4, 2, 6},
                          Triangle{1, 5, 2, 6, 7, 4}},
    std::vector<Triangle>{Triangle{1, 2, 0, 4, 0, 1}, Triangle{1, 2, 7, 4, 0, 4},
                          Triangle{1, 2, 2, 6, 7, 4}, Triangle{6, 7, 7, 4, 2, 6},
                          Triangle{2, 3, 3, 0, 3, 7}},
    std::vector<Triangle>{Triangle{3, 7, 2, 3, 1, 2}, Triangle{3, 7, 1, 2, 7, 4},
                          Triangle{2, 6, 6, 7, 1, 2}, Triangle{6, 7, 7, 4, 1, 2}},
    std::vector<Triangle>{Triangle{0, 4, 1, 5, 6, 7}, Triangle{0, 4, 6, 7, 7, 4},
                          Triangle{1, 5, 1, 2, 6, 7}, Triangle{3, 7, 6, 7, 3, 0},
                          Triangle{1, 2, 3, 0, 6, 7}},
    std::vector<Triangle>{Triangle{0, 1, 1, 5, 1, 2}, Triangle{3, 7, 6, 7, 7, 4}},
    std::vector<Triangle>{Triangle{7, 4, 0, 4, 0, 1}, Triangle{7, 4, 0, 1, 6, 7},
                          Triangle{3, 0, 3, 7, 0, 1}, Triangle{3, 7, 6, 7, 0, 1}},
    std::vector<Triangle>{Triangle{7, 4, 3, 7, 6, 7}},
    std::vector<Triangle>{Triangle{7, 4, 6, 7, 3, 7}},
    std::vector<Triangle>{Triangle{3, 0, 0, 1, 0, 4}, Triangle{3, 7, 7, 4, 6, 7}},
    std::vector<Triangle>{Triangle{0, 1, 1, 2, 1, 5}, Triangle{3, 7, 7, 4, 6, 7}},
    std::vector<Triangle>{Triangle{0, 4, 1, 2, 1, 5}, Triangle{0, 4, 3, 0, 1, 2},
                          Triangle{3, 7, 7, 4, 6, 7}},
    std::vector<Triangle>{Triangle{2, 6, 1, 2, 2, 3}, Triangle{6, 7, 3, 7, 7, 4}},
    std::vector<Triangle>{Triangle{1, 2, 2, 3, 2, 6}, Triangle{3, 0, 0, 1, 0, 4},
                          Triangle{6, 7, 3, 7, 7, 4}},
    std::vector<Triangle>{Triangle{2, 3, 1, 5, 0, 1}, Triangle{2, 3, 2, 6, 1, 5},
                          Triangle{6, 7, 3, 7, 7, 4}},
    std::vector<Triangle>{Triangle{6, 7, 3, 7, 7, 4}, Triangle{2, 3, 2, 6, 3, 0},
                          Triangle{2, 6, 0, 4, 3, 0}, Triangle{2, 6, 1, 5, 0, 4}},
    std::vector<Triangle>{Triangle{7, 4, 2, 3, 3, 0}, Triangle{6, 7, 2, 3, 7, 4}},
    std::vector<Triangle>{Triangle{7, 4, 0, 1, 0, 4}, Triangle{7, 4, 6, 7, 0, 1},
                          Triangle{6, 7, 2, 3, 0, 1}},
    std::vector<Triangle>{Triangle{2, 3, 7, 4, 6, 7}, Triangle{2, 3, 3, 0, 7, 4},
                          Triangle{0, 1, 1, 2, 1, 5}},
    std::vector<Triangle>{Triangle{1, 2, 6, 7, 2, 3}, Triangle{1, 2, 0, 4, 6, 7},
                          Triangle{1, 2, 1, 5, 0, 4}, Triangle{0, 4, 7, 4, 6, 7}},
    std::vector<Triangle>{Triangle{2, 6, 7, 4, 6, 7}, Triangle{2, 6, 1, 2, 7, 4},
                          Triangle{1, 2, 3, 0, 7, 4}},
    std::vector<Triangle>{Triangle{2, 6, 7, 4, 6, 7}, Triangle{1, 2, 7, 4, 2, 6},
                          Triangle{1, 2, 0, 4, 7, 4}, Triangle{1, 2, 0, 1, 0, 4}},
    std::vector<Triangle>{Triangle{0, 1, 3, 0, 7, 4}, Triangle{0, 1, 7, 4, 2, 6},
                          Triangle{0, 1, 2, 6, 1, 5}, Triangle{6, 7, 2, 6, 7, 4}},
    std::vector<Triangle>{Triangle{7, 4, 6, 7, 2, 6}, Triangle{7, 4, 2, 6, 0, 4},
                          Triangle{0, 4, 2, 6, 1, 5}},
    std::vector<Triangle>{Triangle{6, 7, 0, 4, 4, 5}, Triangle{3, 7, 0, 4, 6, 7}},
    std::vector<Triangle>{Triangle{3, 0, 6, 7, 3, 7}, Triangle{3, 0, 0, 1, 6, 7},
                          Triangle{0, 1, 4, 5, 6, 7}},
    std::vector<Triangle>{Triangle{0, 4, 6, 7, 3, 7}, Triangle{0, 4, 4, 5, 6, 7},
                          Triangle{1, 5, 0, 1, 1, 2}},
    std::vector<Triangle>{Triangle{1, 5, 4, 5, 6, 7}, Triangle{1, 5, 6, 7, 3, 0},
                          Triangle{1, 5, 3, 0, 1, 2}, Triangle{3, 7, 3, 0, 6, 7}},
    std::vector<Triangle>{Triangle{6, 7, 0, 4, 4, 5}, Triangle{6, 7, 3, 7, 0, 4},
                          Triangle{2, 3, 2, 6, 1, 2}},
    std::vector<Triangle>{Triangle{1, 2, 2, 3, 2, 6}, Triangle{3, 0, 0, 1, 3, 7},
                          Triangle{0, 1, 6, 7, 3, 7}, Triangle{0, 1, 4, 5, 6, 7}},
    std::vector<Triangle>{Triangle{4, 5, 3, 7, 0, 4}, Triangle{4, 5, 6, 7, 3, 7},
                          Triangle{0, 1, 2, 3, 1, 5}, Triangle{2, 3, 2, 6, 1, 5}},
    std::vector<Triangle>{Triangle{2, 6, 1, 5, 3, 0}, Triangle{2, 6, 3, 0, 2, 3},
                          Triangle{1, 5, 4, 5, 3, 0}, Triangle{3, 7, 3, 0, 6, 7},
                          Triangle{4, 5, 6, 7, 3, 0}},
    std::vector<Triangle>{Triangle{0, 4, 2, 3, 3, 0}, Triangle{0, 4, 4, 5, 2, 3},
                          Triangle{4, 5, 6, 7, 2, 3}},
    std::vector<Triangle>{Triangle{0, 1, 4, 5, 2, 3}, Triangle{4, 5, 6, 7, 2, 3}},
    std::vector<Triangle>{Triangle{1, 2, 1, 5, 0, 1}, Triangle{2, 3, 3, 0, 4, 5},
                          Triangle{2, 3, 4, 5, 6, 7}, Triangle{4, 5, 3, 0, 0, 4}},
    std::vector<Triangle>{Triangle{1, 2, 1, 5, 4, 5}, Triangle{1, 2, 4, 5, 2, 3},
                          Triangle{2, 3, 4, 5, 6, 7}},
    std::vector<Triangle>{Triangle{0, 4, 1, 2, 3, 0}, Triangle{0, 4, 6, 7, 1, 2},
                          Triangle{0, 4, 4, 5, 6, 7}, Triangle{6, 7, 2, 6, 1, 2}},
    std::vector<Triangle>{Triangle{2, 6, 1, 2, 0, 1}, Triangle{2, 6, 0, 1, 6, 7},
                          Triangle{6, 7, 0, 1, 4, 5}},
    std::vector<Triangle>{Triangle{4, 5, 6, 7, 3, 0}, Triangle{4, 5, 3, 0, 0, 4},
                          Triangle{6, 7, 2, 6, 3, 0}, Triangle{0, 1, 3, 0, 1, 5},
                          Triangle{2, 6, 1, 5, 3, 0}},
    std::vector<Triangle>{Triangle{2, 6, 1, 5, 4, 5}, Triangle{6, 7, 2, 6, 4, 5}},
    std::vector<Triangle>{Triangle{4, 5, 1, 5, 5, 6}, Triangle{7, 4, 6, 7, 3, 7}},
    std::vector<Triangle>{Triangle{0, 1, 0, 4, 3, 0}, Triangle{4, 5, 1, 5, 5, 6},
                          Triangle{3, 7, 7, 4, 6, 7}},
    std::vector<Triangle>{Triangle{5, 6, 0, 1, 1, 2}, Triangle{5, 6, 4, 5, 0, 1},
                          Triangle{7, 4, 6, 7, 3, 7}},
    std::vector<Triangle>{Triangle{3, 7, 7, 4, 6, 7}, Triangle{0, 4, 3, 0, 4, 5},
                          Triangle{3, 0, 5, 6, 4, 5}, Triangle{3, 0, 1, 2, 5, 6}},
    std::vector<Triangle>{Triangle{1, 5, 5, 6, 4, 5}, Triangle{2, 6, 1, 2, 2, 3},
                          Triangle{7, 4, 6, 7, 3, 7}},
    std::vector<Triangle>{Triangle{6, 7, 3, 7, 7, 4}, Triangle{1, 2, 2, 3, 2, 6},
                          Triangle{0, 1, 0, 4, 3, 0}, Triangle{4, 5, 1, 5, 5, 6}},
    std::vector<Triangle>{Triangle{7, 4, 6, 7, 3, 7}, Triangle{5, 6, 4, 5, 2, 6},
                          Triangle{4, 5, 2, 3, 2, 6}, Triangle{4, 5, 0, 1, 2, 3}},
    std::vector<Triangle>{Triangle{3, 0, 4, 5, 0, 4}, Triangle{3, 0, 5, 6, 4, 5},
                          Triangle{3, 0, 2, 3, 5, 6}, Triangle{2, 6, 5, 6, 2, 3},
                          Triangle{3, 7, 7, 4, 6, 7}},
    std::vector<Triangle>{Triangle{7, 4, 2, 3, 3, 0}, Triangle{7, 4, 6, 7, 2, 3},
                          Triangle{5, 6, 4, 5, 1, 5}},
    std::vector<Triangle>{Triangle{1, 5, 5, 6, 4, 5}, Triangle{0, 1, 0, 4, 6, 7},
                          Triangle{0, 1, 6, 7, 2, 3}, Triangle{6, 7, 0, 4, 7, 4}},
    std::vector<Triangle>{Triangle{3, 0, 6, 7, 2, 3}, Triangle{3, 0, 7, 4, 6, 7},
                          Triangle{1, 2, 5, 6, 0, 1}, Triangle{5, 6, 4, 5, 0, 1}},
    std::vector<Triangle>{Triangle{6, 7, 2, 3, 0, 4}, Triangle{6, 7, 0, 4, 7, 4},
                          Triangle{2, 3, 1, 2, 0, 4}, Triangle{4, 5, 0, 4, 5, 6},
                          Triangle{1, 2, 5, 6, 0, 4}},
    std::vector<Triangle>{Triangle{1, 5, 5, 6, 4, 5}, Triangle{2, 6, 1, 2, 6, 7},
                          Triangle{1, 2, 7, 4, 6, 7}, Triangle{1, 2, 3, 0, 7, 4}},
    std::vector<Triangle>{Triangle{1, 2, 6, 7, 2, 6}, Triangle{1, 2, 7, 4, 6, 7},
                          Triangle{1, 2, 0, 1, 7, 4}, Triangle{0, 4, 7, 4, 0, 1},
                          Triangle{1, 5, 5, 6, 4, 5}},
    std::vector<Triangle>{Triangle{4, 5, 0, 1, 2, 6}, Triangle{4, 5, 2, 6, 5, 6},
                          Triangle{0, 1, 3, 0, 2, 6}, Triangle{6, 7, 2, 6, 7, 4},
                          Triangle{3, 0, 7, 4, 2, 6}},
    std::vector<Triangle>{Triangle{7, 4, 6, 7, 2, 6}, Triangle{7, 4, 2, 6, 0, 4},
                          Triangle{5, 6, 4, 5, 2, 6}, Triangle{4, 5, 0, 4, 2, 6}},
    std::vector<Triangle>{Triangle{6, 7, 1, 5, 5, 6}, Triangle{6, 7, 3, 7, 1, 5},
                          Triangle{3, 7, 0, 4, 1, 5}},
    std::vector<Triangle>{Triangle{3, 0, 6, 7, 3, 7}, Triangle{0, 1, 6, 7, 3, 0},
                          Triangle{0, 1, 5, 6, 6, 7}, Triangle{0, 1, 1, 5, 5, 6}},
    std::vector<Triangle>{Triangle{0, 1, 3, 7, 0, 4}, Triangle{0, 1, 5, 6, 3, 7},
                          Triangle{0, 1, 1, 2, 5, 6}, Triangle{5, 6, 6, 7, 3, 7}},
    std::vector<Triangle>{Triangle{6, 7, 3, 7, 3, 0}, Triangle{6, 7, 3, 0, 5, 6},
                          Triangle{5, 6, 3, 0, 1, 2}},
    std::vector<Triangle>{Triangle{1, 2, 2, 3, 2, 6}, Triangle{1, 5, 5, 6, 3, 7},
                          Triangle{1, 5, 3, 7, 0, 4}, Triangle{3, 7, 5, 6, 6, 7}},
    std::vector<Triangle>{Triangle{0, 1, 3, 7, 3, 0}, Triangle{0, 1, 6, 7, 3, 7},
                          Triangle{0, 1, 1, 5, 6, 7}, Triangle{5, 6, 6, 7, 1, 5},
                          Triangle{1, 2, 2, 3, 2, 6}},
    std::vector<Triangle>{Triangle{3, 7, 0, 4, 5, 6}, Triangle{3, 7, 5, 6, 6, 7},
                          Triangle{0, 4, 0, 1, 5, 6}, Triangle{2, 6, 5, 6, 2, 3},
                          Triangle{0, 1, 2, 3, 5, 6}},
    std::vector<Triangle>{Triangle{6, 7, 3, 7, 3, 0}, Triangle{6, 7, 3, 0, 5, 6},
                          Triangle{2, 3, 2, 6, 3, 0}, Triangle{2, 6, 5, 6, 3, 0}},
    std::vector<Triangle>{Triangle{5, 6, 0, 4, 1, 5}, Triangle{5, 6, 2, 3, 0, 4},
                          Triangle{5, 6, 6, 7, 2, 3}, Triangle{3, 0, 0, 4, 2, 3}},
    std::vector<Triangle>{Triangle{1, 5, 5, 6, 6, 7}, Triangle{1, 5, 6, 7, 0, 1},
                          Triangle{0, 1, 6, 7, 2, 3}},
    std::vector<Triangle>{Triangle{1, 2, 5, 6, 0, 4}, Triangle{1, 2, 0, 4, 0, 1},
                          Triangle{5, 6, 6, 7, 0, 4}, Triangle{3, 0, 0, 4, 2, 3},
                          Triangle{6, 7, 2, 3, 0, 4}},
    std::vector<Triangle>{Triangle{1, 2, 5, 6, 6, 7}, Triangle{2, 3, 1, 2, 6, 7}},
    std::vector<Triangle>{Triangle{1, 2, 3, 0, 6, 7}, Triangle{1, 2, 6, 7, 2, 6},
                          Triangle{3, 0, 0, 4, 6, 7}, Triangle{5, 6, 6, 7, 1, 5},
                          Triangle{0, 4, 1, 5, 6, 7}},
    std::vector<Triangle>{Triangle{2, 6, 1, 2, 0, 1}, Triangle{2, 6, 0, 1, 6, 7},
                          Triangle{1, 5, 5, 6, 0, 1}, Triangle{5, 6, 6, 7, 0, 1}},
    std::vector<Triangle>{Triangle{0, 1, 3, 0, 0, 4}, Triangle{5, 6, 6, 7, 2, 6}},
    std::vector<Triangle>{Triangle{2, 6, 5, 6, 6, 7}},
    std::vector<Triangle>{Triangle{3, 7, 5, 6, 2, 6}, Triangle{7, 4, 5, 6, 3, 7}},
    std::vector<Triangle>{Triangle{3, 7, 5, 6, 2, 6}, Triangle{3, 7, 7, 4, 5, 6},
                          Triangle{0, 4, 3, 0, 0, 1}},
    std::vector<Triangle>{Triangle{5, 6, 3, 7, 7, 4}, Triangle{5, 6, 2, 6, 3, 7},
                          Triangle{1, 2, 1, 5, 0, 1}},
    std::vector<Triangle>{Triangle{2, 6, 7, 4, 5, 6}, Triangle{2, 6, 3, 7, 7, 4},
                          Triangle{1, 5, 0, 4, 1, 2}, Triangle{0, 4, 3, 0, 1, 2}},
    std::vector<Triangle>{Triangle{3, 7, 1, 2, 2, 3}, Triangle{3, 7, 7, 4, 1, 2},
                          Triangle{7, 4, 5, 6, 1, 2}},
    std::vector<Triangle>{Triangle{0, 1, 0, 4, 3, 0}, Triangle{1, 2, 2, 3, 7, 4},
                          Triangle{1, 2, 7, 4, 5, 6}, Triangle{7, 4, 2, 3, 3, 7}},
    std::vector<Triangle>{Triangle{1, 5, 7, 4, 5, 6}, Triangle{1, 5, 2, 3, 7, 4},
                          Triangle{1, 5, 0, 1, 2, 3}, Triangle{2, 3, 3, 7, 7, 4}},
    std::vector<Triangle>{Triangle{7, 4, 5, 6, 2, 3}, Triangle{7, 4, 2, 3, 3, 7},
                          Triangle{5, 6, 1, 5, 2, 3}, Triangle{3, 0, 2, 3, 0, 4},
                          Triangle{1, 5, 0, 4, 2, 3}},
    std::vector<Triangle>{Triangle{2, 3, 5, 6, 2, 6}, Triangle{2, 3, 3, 0, 5, 6},
                          Triangle{3, 0, 7, 4, 5, 6}},
    std::vector<Triangle>{Triangle{0, 4, 2, 3, 0, 1}, Triangle{0, 4, 5, 6, 2, 3},
                          Triangle{0, 4, 7, 4, 5, 6}, Triangle{2, 6, 2, 3, 5, 6}},
    std::vector<Triangle>{Triangle{1, 5, 0, 1, 1, 2}, Triangle{5, 6, 2, 6, 3, 0},
                          Triangle{5, 6, 3, 0, 7, 4}, Triangle{3, 0, 2, 6, 2, 3}},
    std::vector<Triangle>{Triangle{1, 5, 0, 4, 2, 3}, Triangle{1, 5, 2, 3, 1, 2},
                          Triangle{0, 4, 7, 4, 2, 3}, Triangle{2, 6, 2, 3, 5, 6},
                          Triangle{7, 4, 5, 6, 2, 3}},
    std::vector<Triangle>{Triangle{1, 2, 3, 0, 5, 6}, Triangle{3, 0, 7, 4, 5, 6}},
    std::vector<Triangle>{Triangle{0, 1, 0, 4, 7, 4}, Triangle{0, 1, 7, 4, 1, 2},
                          Triangle{1, 2, 7, 4, 5, 6}},
    std::vector<Triangle>{Triangle{1, 5, 0, 1, 3, 0}, Triangle{1, 5, 3, 0, 5, 6},
                          Triangle{5, 6, 3, 0, 7, 4}},
    std::vector<Triangle>{Triangle{1, 5, 0, 4, 7, 4}, Triangle{5, 6, 1, 5, 7, 4}},
    std::vector<Triangle>{Triangle{5, 6, 0, 4, 4, 5}, Triangle{5, 6, 2, 6, 0, 4},
                          Triangle{2, 6, 3, 7, 0, 4}},
    std::vector<Triangle>{Triangle{5, 6, 0, 1, 4, 5}, Triangle{5, 6, 3, 7, 0, 1},
                          Triangle{5, 6, 2, 6, 3, 7}, Triangle{3, 7, 3, 0, 0, 1}},
    std::vector<Triangle>{Triangle{0, 1, 1, 2, 1, 5}, Triangle{0, 4, 4, 5, 2, 6},
                          Triangle{0, 4, 2, 6, 3, 7}, Triangle{2, 6, 4, 5, 5, 6}},
    std::vector<Triangle>{Triangle{2, 6, 3, 7, 4, 5}, Triangle{2, 6, 4, 5, 5, 6},
                          Triangle{3, 7, 3, 0, 4, 5}, Triangle{1, 5, 4, 5, 1, 2},
                          Triangle{3, 0, 1, 2, 4, 5}},
    std::vector<Triangle>{Triangle{2, 3, 5, 6, 1, 2}, Triangle{2, 3, 0, 4, 5, 6},
                          Triangle{2, 3, 3, 7, 0, 4}, Triangle{4, 5, 5, 6, 0, 4}},
    std::vector<Triangle>{Triangle{0, 1, 4, 5, 3, 7}, Triangle{0, 1, 3, 7, 3, 0},
                          Triangle{4, 5, 5, 6, 3, 7}, Triangle{2, 3, 3, 7, 1, 2},
                          Triangle{5, 6, 1, 2, 3, 7}},
    std::vector<Triangle>{Triangle{0, 1, 2, 3, 5, 6}, Triangle{0, 1, 5, 6, 1, 5},
                          Triangle{2, 3, 3, 7, 5, 6}, Triangle{4, 5, 5, 6, 0, 4},
                          Triangle{3, 7, 0, 4, 5, 6}},
    std::vector<Triangle>{Triangle{1, 5, 4, 5, 5, 6}, Triangle{2, 3, 3, 7, 3, 0}},
    std::vector<Triangle>{Triangle{2, 3, 5, 6, 2, 6}, Triangle{3, 0, 5, 6, 2, 3},
                          Triangle{3, 0, 4, 5, 5, 6}, Triangle{3, 0, 0, 4, 4, 5}},
    std::vector<Triangle>{Triangle{5, 6, 2, 6, 2, 3}, Triangle{5, 6, 2, 3, 4, 5},
                          Triangle{4, 5, 2, 3, 0, 1}},
    std::vector<Triangle>{Triangle{3, 0, 2, 6, 2, 3}, Triangle{3, 0, 5, 6, 2, 6},
                          Triangle{3, 0, 0, 4, 5, 6}, Triangle{4, 5, 5, 6, 0, 4},
                          Triangle{0, 1, 1, 2, 1, 5}},
    std::vector<Triangle>{Triangle{5, 6, 2, 6, 2, 3}, Triangle{5, 6, 2, 3, 4, 5},
                          Triangle{1, 2, 1, 5, 2, 3}, Triangle{1, 5, 4, 5, 2, 3}},
    std::vector<Triangle>{Triangle{0, 4, 4, 5, 5, 6}, Triangle{0, 4, 5, 6, 3, 0},
                          Triangle{3, 0, 5, 6, 1, 2}},
    std::vector<Triangle>{Triangle{0, 1, 4, 5, 5, 6}, Triangle{1, 2, 0, 1, 5, 6}},
    std::vector<Triangle>{Triangle{0, 4, 4, 5, 5, 6}, Triangle{0, 4, 5, 6, 3, 0},
                          Triangle{1, 5, 0, 1, 5, 6}, Triangle{0, 1, 3, 0, 5, 6}},
    std::vector<Triangle>{Triangle{1, 5, 4, 5, 5, 6}},
    std::vector<Triangle>{Triangle{4, 5, 3, 7, 7, 4}, Triangle{4, 5, 1, 5, 3, 7},
                          Triangle{1, 5, 2, 6, 3, 7}},
    std::vector<Triangle>{Triangle{0, 1, 0, 4, 3, 0}, Triangle{4, 5, 1, 5, 7, 4},
                          Triangle{1, 5, 3, 7, 7, 4}, Triangle{1, 5, 2, 6, 3, 7}},
    std::vector<Triangle>{Triangle{1, 2, 2, 6, 3, 7}, Triangle{1, 2, 3, 7, 4, 5},
                          Triangle{1, 2, 4, 5, 0, 1}, Triangle{7, 4, 4, 5, 3, 7}},
    std::vector<Triangle>{Triangle{3, 0, 1, 2, 4, 5}, Triangle{3, 0, 4, 5, 0, 4},
                          Triangle{1, 2, 2, 6, 4, 5}, Triangle{7, 4, 4, 5, 3, 7},
                          Triangle{2, 6, 3, 7, 4, 5}},
    std::vector<Triangle>{Triangle{4, 5, 3, 7, 7, 4}, Triangle{1, 5, 3, 7, 4, 5},
                          Triangle{1, 5, 2, 3, 3, 7}, Triangle{1, 5, 1, 2, 2, 3}},
    std::vector<Triangle>{Triangle{1, 5, 7, 4, 4, 5}, Triangle{1, 5, 3, 7, 7, 4},
                          Triangle{1, 5, 1, 2, 3, 7}, Triangle{2, 3, 3, 7, 1, 2},
                          Triangle{0, 1, 0, 4, 3, 0}},
    std::vector<Triangle>{Triangle{3, 7, 7, 4, 4, 5}, Triangle{3, 7, 4, 5, 2, 3},
                          Triangle{2, 3, 4, 5, 0, 1}},
    std::vector<Triangle>{Triangle{3, 7, 7, 4, 4, 5}, Triangle{3, 7, 4, 5, 2, 3},
                          Triangle{0, 4, 3, 0, 4, 5}, Triangle{3, 0, 2, 3, 4, 5}},
    std::vector<Triangle>{Triangle{2, 3, 1, 5, 2, 6}, Triangle{2, 3, 7, 4, 1, 5},
                          Triangle{2, 3, 3, 0, 7, 4}, Triangle{7, 4, 4, 5, 1, 5}},
    std::vector<Triangle>{Triangle{1, 5, 2, 6, 7, 4}, Triangle{1, 5, 7, 4, 4, 5},
                          Triangle{2, 6, 2, 3, 7, 4}, Triangle{0, 4, 7, 4, 0, 1},
                          Triangle{2, 3, 0, 1, 7, 4}},
    std::vector<Triangle>{Triangle{3, 0, 7, 4, 2, 6}, Triangle{3, 0, 2, 6, 2, 3},
                          Triangle{7, 4, 4, 5, 2, 6}, Triangle{1, 2, 2, 6, 0, 1},
                          Triangle{4, 5, 0, 1, 2, 6}},
    std::vector<Triangle>{Triangle{1, 2, 2, 6, 2, 3}, Triangle{0, 4, 7, 4, 4, 5}},
    std::vector<Triangle>{Triangle{4, 5, 1, 5, 1, 2}, Triangle{4, 5, 1, 2, 7, 4},
                          Triangle{7, 4, 1, 2, 3, 0}},
    std::vector<Triangle>{Triangle{4, 5, 1, 5, 1, 2}, Triangle{4, 5, 1, 2, 7, 4},
                          Triangle{0, 1, 0, 4, 1, 2}, Triangle{0, 4, 7, 4, 1, 2}},
    std::vector<Triangle>{Triangle{4, 5, 0, 1, 3, 0}, Triangle{7, 4, 4, 5, 3, 0}},
    std::vector<Triangle>{Triangle{4, 5, 0, 4, 7, 4}},
    std::vector<Triangle>{Triangle{1, 5, 2, 6, 0, 4}, Triangle{2, 6, 3, 7, 0, 4}},
    std::vector<Triangle>{Triangle{3, 0, 0, 1, 1, 5}, Triangle{3, 0, 1, 5, 3, 7},
                          Triangle{3, 7, 1, 5, 2, 6}},
    std::vector<Triangle>{Triangle{0, 1, 1, 2, 2, 6}, Triangle{0, 1, 2, 6, 0, 4},
                          Triangle{0, 4, 2, 6, 3, 7}},
    std::vector<Triangle>{Triangle{3, 0, 1, 2, 2, 6}, Triangle{3, 7, 3, 0, 2, 6}},
    std::vector<Triangle>{Triangle{1, 2, 2, 3, 3, 7}, Triangle{1, 2, 3, 7, 1, 5},
                          Triangle{1, 5, 3, 7, 0, 4}},
    std::vector<Triangle>{Triangle{3, 0, 0, 1, 1, 5}, Triangle{3, 0, 1, 5, 3, 7},
                          Triangle{1, 2, 2, 3, 1, 5}, Triangle{2, 3, 3, 7, 1, 5}},
    std::vector<Triangle>{Triangle{0, 1, 2, 3, 3, 7}, Triangle{0, 4, 0, 1, 3, 7}},
    std::vector<Triangle>{Triangle{3, 0, 2, 3, 3, 7}},
    std::vector<Triangle>{Triangle{2, 3, 3, 0, 0, 4}, Triangle{2, 3, 0, 4, 2, 6},
                          Triangle{2, 6, 0, 4, 1, 5}},
    std::vector<Triangle>{Triangle{1, 5, 2, 6, 2, 3}, Triangle{0, 1, 1, 5, 2, 3}},
    std::vector<Triangle>{Triangle{2, 3, 3, 0, 0, 4}, Triangle{2, 3, 0, 4, 2, 6},
                          Triangle{0, 1, 1, 2, 0, 4}, Triangle{1, 2, 2, 6, 0, 4}},
    std::vector<Triangle>{Triangle{1, 2, 2, 6, 2, 3}},
    std::vector<Triangle>{Triangle{1, 2, 3, 0, 0, 4}, Triangle{1, 5, 1, 2, 0, 4}},
    std::vector<Triangle>{Triangle{0, 1, 1, 5, 1, 2}},
    std::vector<Triangle>{Triangle{0, 1, 3, 0, 0, 4}},
    std::vector<Triangle>{}};

void evaluateCube(K3DTree<size_t, float> &vertexTree, IndexBufferRAM *indexBuffer,
                  std::vector<vec3> &positions, std::vector<vec3> &normals,
                  const std::array<vec3, 8> &pos, const std::array<double, 8> &values) {
    int index = 0;

    //  v7 ----- v6
    //  /|       /|
    // v3 ----- v2|
    // |v4 - - -|v5
    // |/       |/
    // v0 ----- v1

    if (values[0] > 0) index = index | 1;
    if (values[1] > 0) index = index | 2;
    if (values[2] > 0) index = index | 4;
    if (values[3] > 0) index = index | 8;
    if (values[4] > 0) index = index | 16;
    if (values[5] > 0) index = index | 32;
    if (values[6] > 0) index = index | 64;
    if (values[7] > 0) index = index | 128;

    for (auto t : cases[index]) {
        auto interpolate = [&](auto a, auto b) {
            auto v0 = values[a];
            auto v1 = values[b];
            float t = static_cast<float>(v0 / (v0 - v1));
            return pos[a] + t * (pos[b] - pos[a]);
        };
        vec3 p0 = interpolate(t.e0a, t.e0b);
        vec3 p1 = interpolate(t.e1a, t.e1b);
        vec3 p2 = interpolate(t.e2a, t.e2b);

        marching::addTriangle(vertexTree, indexBuffer, positions, normals, p0, p1, p2);
    }
}

}  // namespace marchingcubes

namespace util {
std::shared_ptr<Mesh> marchingcubes(std::shared_ptr<const Volume> volume, double iso,
                                    const vec4 &color, bool invert, bool enclose,
                                    std::function<void(float)> progressCallback,
                                    std::function<bool(const size3_t &)> maskingCallback) {

    return volume->getRepresentation<VolumeRAM>()->dispatch<std::shared_ptr<Mesh>>([&](auto ram) {
        using T = util::PrecisionValueType<decltype(ram)>;
        if (progressCallback) progressCallback(0.0f);

        if (!maskingCallback) {
            throw Exception("Masking callback not set", IVW_CONTEXT_CUSTOM("util::marchingcubes"));
        }

        K3DTree<size_t, float> vertexTree;

        auto mesh = std::make_shared<BasicMesh>();
        auto indexBuffer = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);

        std::vector<vec3> positions;
        std::vector<vec3> normals;

        mesh->setModelMatrix(volume->getModelMatrix());
        mesh->setWorldMatrix(volume->getWorldMatrix());

        const T *src = ram->getDataTyped();

        const size3_t dim{volume->getDimensions()};
        double dx, dy, dz;
        dx = 1.0 / static_cast<double>(std::max(size_t(1), (dim.x - 1)));
        dy = 1.0 / static_cast<double>(std::max(size_t(1), (dim.y - 1)));
        dz = 1.0 / static_cast<double>(std::max(size_t(1), (dim.z - 1)));

        const auto volSize = dim.x * dim.y * dim.z;
        indexBuffer->getDataContainer().reserve(volSize * 6);
        positions.reserve(volSize * 6);
        normals.reserve(volSize * 6);

        for (size_t k = 0; k < dim.z - 1; k++) {
            for (size_t j = 0; j < dim.y - 1; j++) {
                for (size_t i = 0; i < dim.x - 1; i++) {
                    if (!maskingCallback({i, j, k})) continue;
                    double x = dx * i;
                    double y = dy * j;
                    double z = dz * k;

                    std::array<vec3, 8> pos;
                    std::array<double, 8> values;

                    for (int l = 0; l < 8; l++) {
                        const auto &o = marchingcubes::offs[l];
                        pos[l] = glm::vec3(x + dx * o.x, y + dy * o.y, z + dz * o.z);
                        values[l] = marching::getValue(src, size3_t(i, j, k) + o, dim, iso, invert);
                    }

                    marchingcubes::evaluateCube(vertexTree, indexBuffer.get(), positions, normals,
                                                pos, values);
                }
            }
            if (progressCallback) {
                progressCallback(static_cast<float>(k + 1) / static_cast<float>(dim.z - 1));
            }
        }

        if (enclose) {
            marching::encloseSurfce(src, dim, indexBuffer.get(), positions, normals, iso, invert,
                                    dx, dy, dz);
        }

        ivwAssert(positions.size() == normals.size(), "positions_ and normals_ must be equal");
        std::vector<BasicMesh::Vertex> vertices;
        vertices.reserve(positions.size());

        for (auto pit = positions.begin(), nit = normals.begin(); pit != positions.end();
             ++pit, ++nit) {
            vertices.push_back({*pit, glm::normalize(*nit), *pit, color});
        }

        mesh->addVertices(vertices);

        if (progressCallback) progressCallback(1.0f);

        return mesh;
    });
}
}  // namespace util

}  // namespace inviwo
