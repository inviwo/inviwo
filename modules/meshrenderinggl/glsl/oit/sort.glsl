/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2021 Inviwo Foundation
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

/**
 * Fast Single-pass A-Buffer using OpenGL 4.0 V2.0
 * Copyright Cyril Crassin, July 2010
 **/
#ifndef ABUFFERSORT_HGLSL
#define ABUFFERSORT_HGLSL

// Local memory array (probably in L1)
vec4 fragmentList[ABUFFER_SIZE];

// Bubble sort used to sort fragments
void bubbleSort(int array_size);
// Bitonic sort test
void bitonicSort(int n);
// Fill local memory array of fragments
void fillFragmentArray(uint idx, out int numFrag);
// Fill local memory array with lowest values
void fillFragmentArraySortedUntilDepth(uint pixelIdx, float maxDepth, out int numFrag);
// Selection sort, allowing to get the next elements of a larger series.
// Returns the next fragment with lowest depth but behind/at the same depth as the last.
vec4 selectionSortNext(uint headPtr, float lastDepth, inout uint lastPtr);

// Bubble sort used to sort fragments
void bubbleSort(int array_size) {
    for (int i = (array_size - 2); i >= 0; --i) {
        for (int j = 0; j <= i; ++j) {
            if (fragmentList[j].y > fragmentList[j + 1].y) {
                vec4 temp = fragmentList[j + 1];
                fragmentList[j + 1] = fragmentList[j];
                fragmentList[j] = temp;
            }
        }
    }
}

// Swap function
void swapFragArray(int n0, int n1) {
    vec4 temp = fragmentList[n1];
    fragmentList[n1] = fragmentList[n0];
    fragmentList[n0] = temp;
}

//->Same artefact than in L.Bavoil
// Bitonic sort: http://www.tools-of-computing.com/tc/CS/Sorts/bitonic_sort.htm
void bitonicSort(int n) {
    int i, j, k;
    for (k = 2; k <= n; k = 2 * k) {
        for (j = k >> 1; j > 0; j = j >> 1) {
            for (i = 0; i < n; i++) {
                int ixj = i ^ j;
                if ((ixj) > i) {
                    if ((i & k) == 0 && fragmentList[i].y > fragmentList[ixj].y) {
                        swapFragArray(i, ixj);
                    }
                    if ((i & k) != 0 && fragmentList[i].y < fragmentList[ixj].y) {
                        swapFragArray(i, ixj);
                    }
                }
            }
        }
    }
}

void fillFragmentArray(uint pixelIdx, out int numFrag) {
    // Load fragments into a local memory array for sorting
    int ip = 0;
    while (pixelIdx != 0 && ip < ABUFFER_SIZE) {
        vec4 val = readPixelStorage(pixelIdx - 1);
        fragmentList[ip] = val;
        pixelIdx = floatBitsToUint(val.x);
        ip++;
    }
    numFrag = ip;
}

void fillFragmentArraySortedUntilDepth(uint pixelIdx, float maxDepth, out int numFrag) {
    // Load fragments into a local memory array for sorting
    numFrag = 0;
    float depth = 0.0;
    uint lastPtr = 0;

    while (numFrag < ABUFFER_SIZE) {
        vec4 val = selectionSortNext(pixelIdx, depth, lastPtr);
        depth = val.y;
        if (depth < 0.0 || depth > maxDepth) return;
        fragmentList[numFrag] = val;
        numFrag++;
    }
}

vec4 selectionSortNext(uint headPtr, float lastDepth, inout uint lastPtr) {
    // Remember the next fragment closest to the camera.
    vec4 closestVal = vec4(-1.0);
    float minDepth = 1.0;
    uint minPtr = 0;
    // Remember whether the previous element was visited yet to work through fragemtns of same depth in order. 
    bool passedLastPtr = false;
    while (headPtr != 0) {
        vec4 val = readPixelStorage(headPtr - 1);
        if (headPtr == lastPtr) passedLastPtr = true;
        // Require fragment to be both closer than the current nearest one
        // and behind the previous fragment returned in either depth or, iff at the same depth, in order. 
        else if (val.y < minDepth && (val.y > lastDepth || (passedLastPtr && val.y == lastDepth) )) {
            minDepth = val.y;
            closestVal = val;
            minPtr = headPtr;
        }
        headPtr = floatBitsToUint(val.x);
    }
    // Update pointer to selected fragment.
    lastPtr = minPtr;
    return closestVal;
}

#endif