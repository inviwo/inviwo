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

void sample(const VolumeRAM& volume, std::span<const dvec3> positions, std::span<double> output,
            const dmat4& posToTexture, const dmat4& textureToIndex, const DataMapper& dataMap,
            DataMapper::Space space) {

    volume.dispatch<void>([&]<typename T>(const VolumeRAMPrecision<T>* vr) {
        sample<1, T>(*vr, positions, output, posToTexture, textureToIndex, dataMap, space);
    });
}
void sample(const VolumeRAM& volume, std::span<const dvec3> positions, std::span<dvec2> output,
            const dmat4& posToTexture, const dmat4& textureToIndex, const DataMapper& dataMap,
            DataMapper::Space space) {

    volume.dispatch<void>([&]<typename T>(const VolumeRAMPrecision<T>* vr) {
        sample<2, T>(*vr, positions, output, posToTexture, textureToIndex, dataMap, space);
    });
}
void sample(const VolumeRAM& volume, std::span<const dvec3> positions, std::span<dvec3> output,
            const dmat4& posToTexture, const dmat4& textureToIndex, const DataMapper& dataMap,
            DataMapper::Space space) {

    volume.dispatch<void>([&]<typename T>(const VolumeRAMPrecision<T>* vr) {
        sample<3, T>(*vr, positions, output, posToTexture, textureToIndex, dataMap, space);
    });
}
void sample(const VolumeRAM& volume, std::span<const dvec3> positions, std::span<dvec4> output,
            const dmat4& posToTexture, const dmat4& textureToIndex, const DataMapper& dataMap,
            DataMapper::Space space) {

    volume.dispatch<void>([&]<typename T>(const VolumeRAMPrecision<T>* vr) {
        sample<4, T>(*vr, positions, output, posToTexture, textureToIndex, dataMap, space);
    });
}

void sample(const Volume& volume, std::span<const dvec3> positions, std::span<double> output,
            CoordinateSpace positionSpace, DataMapper::Space outputSpace) {

    const auto& cm = volume.getCoordinateTransformer();
    auto ram = volume.getRepresentation<VolumeRAM>();
    sample(*ram, positions, output, cm.getMatrix(positionSpace, CoordinateSpace::Data),
           cm.getDataToIndexMatrix(), volume.dataMap, outputSpace);
}
void sample(const Volume& volume, std::span<const dvec3> positions, std::span<dvec2> output,
            CoordinateSpace positionSpace, DataMapper::Space outputSpace) {

    const auto& cm = volume.getCoordinateTransformer();
    auto ram = volume.getRepresentation<VolumeRAM>();
    sample(*ram, positions, output, cm.getMatrix(positionSpace, CoordinateSpace::Data),
           cm.getDataToIndexMatrix(), volume.dataMap, outputSpace);
}
void sample(const Volume& volume, std::span<const dvec3> positions, std::span<dvec3> output,
            CoordinateSpace positionSpace, DataMapper::Space outputSpace) {

    const auto& cm = volume.getCoordinateTransformer();
    auto ram = volume.getRepresentation<VolumeRAM>();
    sample(*ram, positions, output, cm.getMatrix(positionSpace, CoordinateSpace::Data),
           cm.getDataToIndexMatrix(), volume.dataMap, outputSpace);
}
void sample(const Volume& volume, std::span<const dvec3> positions, std::span<dvec4> output,
            CoordinateSpace positionSpace, DataMapper::Space outputSpace) {

    const auto& cm = volume.getCoordinateTransformer();
    auto ram = volume.getRepresentation<VolumeRAM>();
    sample(*ram, positions, output, cm.getMatrix(positionSpace, CoordinateSpace::Data),
           cm.getDataToIndexMatrix(), volume.dataMap, outputSpace);
}

}  // namespace inviwo::sample
