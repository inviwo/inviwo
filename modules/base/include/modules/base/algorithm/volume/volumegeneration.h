/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2026 Inviwo Foundation
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

#pragma once

#include <inviwo/core/datastructures/volume/volume.h>  // IWYU pragma: keep
#include <inviwo/core/util/glmconvert.h>               // for glm_convert_normalized
#include <inviwo/core/util/glmmat.h>                   // for mat3
#include <inviwo/core/util/glmvec.h>                   // for size3_t, dvec3
#include <inviwo/core/util/indexmapper.h>              // for IndexMapper, IndexMapper3D
#include <inviwo/core/util/volumeramutils.h>           // for forEachVoxelParallel
#include <modules/base/algorithm/algorithmoptions.h>   // for IgnoreSpecialValues, IgnoreSpecialV...
#include <modules/base/algorithm/dataminmax.h>         // for dataMinMax

#include <array>          // for array
#include <bitset>         // for bitset, __bitset<>::reference, bits...
#include <cmath>          // for sin
#include <cstddef>        // for size_t
#include <memory>         // for unique_ptr, make_shared, make_unique
#include <unordered_map>  // for unordered_map

#include <glm/ext/scalar_constants.hpp>  // for pi
#include <glm/geometric.hpp>             // for length
#include <glm/gtx/component_wise.hpp>    // for compMul
#include <glm/gtx/norm.hpp>              // for length2
#include <glm/mat3x3.hpp>                // for mat<>::col_type
#include <glm/vec3.hpp>                  // for operator+, operator-, operator/, vec

namespace inviwo {
template <typename T>
class VolumeRAMPrecision;

namespace util {

/**
 * Convenience function for generating volumes
 * @param dimensions Volume grid dimensions
 * @param basis Volume basis, offset automatically set to center the volume around origo
 * @param function Functor called for each volume voxel. T(const size3_t& ind).
 */
template <typename Functor>
std::unique_ptr<Volume> generateVolume(const size3_t& dimensions, const mat3& basis,
                                       const Functor& function) {
    using T = decltype(function(dimensions));

    auto ram = std::make_shared<VolumeRAMPrecision<T>>(dimensions);
    auto data = ram->getDataTyped();
    IndexMapper3D im(dimensions);

    forEachVoxelParallel(*ram, [&](const size3_t& ind) { data[im(ind)] = function(ind); });

    auto minmax = util::dataMinMax(data, glm::compMul(dimensions), IgnoreSpecialValues::Yes);

    auto volume = std::make_unique<Volume>(ram);
    volume->setBasis(basis);
    volume->setOffset(-0.5f * (basis[0] + basis[1] + basis[2]));
    volume->dataMap.dataRange.x = glm::compMin(minmax.first);
    volume->dataMap.dataRange.y = glm::compMax(minmax.second);
    volume->dataMap.valueRange = volume->dataMap.dataRange;
    return volume;
}

/**
 * Center voxel equal to value all other 0
 */
template <typename T = float>
std::unique_ptr<Volume> makeSingleVoxelVolume(const size3_t& size,
                                              const dvec4& value = dvec4{1.0}) {
    const size3_t mid{(size - size3_t{1u}) / size_t{2}};
    return generateVolume(size, mat3(1.0), [&](const size3_t& ind) {
        if (ind == mid) {
            return glm_convert_normalized<T>(value);
        } else {
            return glm_convert_normalized<T>(0.0);
        }
    });
}

template <typename T = float>
std::unique_ptr<Volume> makeUniformVolume(const size3_t& size, const dvec4& value = dvec4{1.0}) {
    return generateVolume(size, mat3(1.0),
                          [&](const size3_t&) { return glm_convert_normalized<T>(value); });
}

/**
 * Spherically symmetric density centered in the volume decaying radially with the distance from the
 * center
 */
template <typename T = float>
std::unique_ptr<Volume> makeSphericalVolume(const size3_t& size) {
    const dvec3 rsize{size};
    const dvec3 center = (rsize / 2.0);
    const auto r0 = glm::length(rsize);
    return generateVolume(size, mat3(1.0), [&](const size3_t& ind) {
        const auto pos = dvec3(ind) + dvec3{0.5};
        return glm_convert_normalized<T>(r0 / (r0 + glm::length2(center - pos)));
    });
}

/**
 * A quickly oscillating density between 0 and 1
 */
template <typename T = float>
std::unique_ptr<Volume> makeRippleVolume(const size3_t& size) {
    const dvec3 rsize{size};
    const dvec3 center = (rsize / 2.0);
    const double r0 = glm::length(rsize);
    return generateVolume(size, mat3(1.0), [&](const size3_t& ind) {
        const auto pos = dvec3(ind) + dvec3{0.5};
        const auto r = glm::length2(center - pos);
        return glm_convert_normalized<T>(
            0.5 + 0.5 * std::sin(rsize.x * 0.5 * glm::pi<double>() * r / r0));
    });
}

/**
 * A 2x2x2 volume corresponding to a marching cube case
 */
template <typename T = float>
std::unique_ptr<Volume> makeMarchingCubeVolume(const size_t& index) {
    std::bitset<8> corners(index);
    const std::array<size3_t, 8> vertices = {
        {{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}, {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}}};
    std::unordered_map<size3_t, T> map;
    for (int i = 0; i < 8; ++i) {
        map[vertices[i]] = glm_convert_normalized<T>(corners[i] ? 1.0 : 0.0);        
    }
    return generateVolume({2, 2, 2}, mat3(1.0), [&](const size3_t& ind) { return map[ind]; });
}

/* Originally from: https://web.cse.ohio-state.edu/~crawfis.3/Data/Tornado/tornadoSrc.c
 * can be found at:
 * https://web.archive.org/web/20220302130820/https://web.cse.ohio-state.edu/~crawfis.3/Data/Tornado/tornadoSrc.c
 *
 * Adopted and updated for inviwo
 *
 *  Gen_Tornado creates a vector field of dimension [xs,ys,zs,3] from
 *  a procedural function. By passing in different time arguments,
 *  a slightly different and rotating field is created.
 *
 *  The magnitude of the vector field is highest at some funnel shape
 *  and values range from 0.0 to around 0.4 (I think).
 *
 *  I just wrote these comments, 8 years after I wrote the function.
 *
 * Developed by Roger A. Crawfis, The Ohio State University
 */
inline void gen_tornado(const size3_t& size, double time, std::span<vec3> tornado) {
    const double SMALL = 0.00000000001;

    const dvec3 delta = dvec3{1.0} / (dvec3{size} - dvec3{1.0});

    auto it = tornado.begin();

    for (size_t iz = 0; iz < size.z; iz++) {
        const double z = static_cast<double>(iz) * delta.z;  // map z to 0->1

        // For each z-slice, determine the spiral circle.
        const double xc = 0.5 + 0.1 * sin(0.04 * time + 10.0 * z);

        // (xc,yc) determine the center of the circle.
        const double yc = 0.5 + 0.1 * cos(0.03 * time + 3.0 * z);

        // The radius also changes at each z-slice.
        const double r = 0.1 + 0.4 * z * z + 0.1 * z * sin(8.0 * z);

        // r is the center radius, r2 is for damping
        const double r2 = 0.2 + 0.1 * z;

        for (size_t iy = 0; iy < size.y; iy++) {
            const double y = static_cast<double>(iy) * delta.y; // map y to 0->1
            for (size_t ix = 0; ix < size.x; ix++) {
                const double x = static_cast<double>(ix) * delta.x; // map x to 0->1

                double rdist = sqrt((y - yc) * (y - yc) + (x - xc) * (x - xc));
                double scale = fabs(r - rdist);
                /*
                 *  I do not like this next line. It produces a discontinuity
                 *  in the magnitude. Fix it later.
                 */
                if (scale > r2) {
                    scale = 0.8 - scale;
                } else {
                    scale = 1.0;
                }
                const double z0 = std::max(0.1 * (0.1 - rdist * z), 0.0);
                rdist = sqrt(rdist * rdist + z0 * z0);
                scale = (r + r2 - rdist) * scale / (rdist + SMALL);
                scale = scale / (1 + z);

                *it++ = vec3{scale * (y - yc) + 0.1 * (x - xc), scale * -(x - xc) + 0.1 * (y - yc),
                             scale * z0};
            }
        }
    }
}

inline std::unique_ptr<Volume> makeTornadoVolume(const size3_t& dimensions, const size_t& index) {

    auto ram = std::make_shared<VolumeRAMPrecision<vec3>>(dimensions);
    auto data = ram->getView();

    gen_tornado(dimensions, static_cast<double>(index), data);

    auto minmax = util::volumeMinMax(ram.get(), IgnoreSpecialValues::Yes);

    auto volume = std::make_unique<Volume>(ram);
    const auto basis = mat3(1.0);
    volume->setBasis(basis);
    volume->setOffset(-0.5f * (basis[0] + basis[1] + basis[2]));
    volume->dataMap.dataRange.x = glm::compMin(minmax.first);
    volume->dataMap.dataRange.y = glm::compMax(minmax.second);
    volume->dataMap.valueRange = volume->dataMap.dataRange;
    return volume;
}

template <typename T = float>
std::unique_ptr<Volume> makeGaussianVolume(const size3_t& size,const float& sigma) {
    return generateVolume(size, mat3(1.0), [&](const size3_t& ind) {
        dvec3 a{0.1, 0.4, 0.4};
        dvec3 b{0.5, 0.5, 0.5};
        dvec3 c{0.8, 0.3, 0.1};
        std::array<dvec3, 3> points{dvec3(b), dvec3(a), dvec3(c)};
        dvec3 x = dvec3(ind);
        
        x = x/dvec3(size);

        double sum{0.0};
        //double sigma{0.1};
        
        for (auto &p : points) { 
            sum += 1 / (sigma * sqrt(2 * M_PI)) * exp(-0.5 * glm::length2(p - x)/(sigma*sigma));
        }
        return glm_convert_normalized<T>(sum);
    });
}

template <typename T = float>
std::unique_ptr<Volume> makeSingleVoxelSomewhere(const size3_t& size,const int index) {
    return generateVolume(size, mat3(1.0), [&](const size3_t& ind) {
        if (ind == ind)
            return glm_convert_normalized<T>(1.0);
        else
            return glm_convert_normalized<T>(0.0);
        ;

        
    });
}








}  // namespace util

}  // namespace inviwo
