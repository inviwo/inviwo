/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <inviwo/core/algorithm/samplevolume.h>

namespace inviwo::sample {

/*
void sample(const State& state, std::span<const dvec3> positions, std::span<double> output) {
    state.volumeRAM.dispatch<void>([&]<typename T>(const VolumeRAMPrecision<T>* vr) {
        sample<1, T>(state, *vr, positions, output);
    });
}
void sample(const State& state, std::span<const dvec3> positions, std::span<dvec2> output) {
    state.volumeRAM.dispatch<void>([&]<typename T>(const VolumeRAMPrecision<T>* vr) {
        sample<2, T>(state, *vr, positions, output);
    });
}
void sample(const State& state, std::span<const dvec3> positions, std::span<dvec3> output) {
    state.volumeRAM.dispatch<void>([&]<typename T>(const VolumeRAMPrecision<T>* vr) {
        sample<3, T>(state, *vr, positions, output);
    });
}
void sample(const State& state, std::span<const dvec3> positions, std::span<dvec4> output) {
    state.volumeRAM.dispatch<void>([&]<typename T>(const VolumeRAMPrecision<T>* vr) {
        sample<4, T>(state, *vr, positions, output);
    });
}

void sample(const Volume& volume, std::span<const dvec3> positions, std::span<double> output,
            CoordinateSpace positionSpace, DataMapper::Space outputSpace) {

    const auto state = createState(volume, positionSpace, outputSpace);
    sample(state, positions, output);
}
void sample(const Volume& volume, std::span<const dvec3> positions, std::span<dvec2> output,
            CoordinateSpace positionSpace, DataMapper::Space outputSpace) {

    const auto state = createState(volume, positionSpace, outputSpace);
    sample(state, positions, output);
}
void sample(const Volume& volume, std::span<const dvec3> positions, std::span<dvec3> output,
            CoordinateSpace positionSpace, DataMapper::Space outputSpace) {

    const auto state = createState(volume, positionSpace, outputSpace);
    sample(state, positions, output);
}
void sample(const Volume& volume, std::span<const dvec3> positions, std::span<dvec4> output,
            CoordinateSpace positionSpace, DataMapper::Space outputSpace) {

    const auto state = createState(volume, positionSpace, outputSpace);
    sample(state, positions, output);
}
*/

auto createVec1Functor(const Volume& volume, CoordinateSpace positionSpace, DataMapper::Space space)
    -> SampleFunctor<1> {
    const auto state = createState(volume, positionSpace, space);
    return state.volumeRAM.dispatch<SampleFunctor<1>>(
        [&]<typename T>(const VolumeRAMPrecision<T>* vr) {
            return createFunctor<1, T>(state, *vr);
        });
}
auto createVec2Functor(const Volume& volume, CoordinateSpace positionSpace, DataMapper::Space space)
    -> SampleFunctor<2> {
    const auto state = createState(volume, positionSpace, space);
    return state.volumeRAM.dispatch<SampleFunctor<2>>(
        [&]<typename T>(const VolumeRAMPrecision<T>* vr) {
            return createFunctor<2, T>(state, *vr);
        });
}
auto createVec3Functor(const Volume& volume, CoordinateSpace positionSpace, DataMapper::Space space)
    -> SampleFunctor<3> {
    const auto state = createState(volume, positionSpace, space);
    return state.volumeRAM.dispatch<SampleFunctor<3>>(
        [&]<typename T>(const VolumeRAMPrecision<T>* vr) {
            return createFunctor<3, T>(state, *vr);
        });
}
auto createVec4Functor(const Volume& volume, CoordinateSpace positionSpace, DataMapper::Space space)
    -> SampleFunctor<4> {
    const auto state = createState(volume, positionSpace, space);
    return state.volumeRAM.dispatch<SampleFunctor<4>>(
        [&]<typename T>(const VolumeRAMPrecision<T>* vr) {
            return createFunctor<4, T>(state, *vr);
        });
}

}  // namespace inviwo::sample
