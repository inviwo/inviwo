/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2023 Inviwo Foundation
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

#include <inviwo/core/util/glmmat.h>             // for dmat4
#include <inviwo/core/util/glmvec.h>             // for dvec3, dvec2, size3_t, dvec4
#include <inviwo/core/util/stdextensions.h>      // for overloaded
#include <inviwo/core/util/volumeramutils.h>     // for forEachVoxel
#include <modules/base/algorithm/randomutils.h>  // for randomNumber

#include <cmath>        // for cos, sin, acos, pow
#include <cstddef>      // for size_t
#include <optional>     // for optional, nullopt
#include <random>       // for mt19937_64, random_device
#include <type_traits>  // for remove_reference<>::type
#include <variant>      // for variant, visit

#include <glm/detail/qualifier.hpp>  // for vec
#include <glm/gtc/constants.hpp>     // for two_pi
#include <glm/gtx/transform.hpp>     // for scale
#include <glm/mat4x4.hpp>            // for operator*, mat
#include <glm/vec2.hpp>              // for vec<>::(anonymous)
#include <glm/vec3.hpp>              // for vec<>::(anonymous), operator+

namespace inviwo {

namespace util {

struct Grid3DPointGeneration {
    dmat4 bases = dmat4(1.0);
    size3_t nPoints = {5, 5, 5};
    std::optional<dvec3> jitter = std::nullopt;
    std::optional<size_t> seed = std::nullopt;
};

struct RandomCubicalPointGeneration {
    dmat4 bases = dmat4(1.0);
    size_t nPoints = 25;
    std::optional<size_t> seed = std::nullopt;
};

struct RandomSphericalPointGeneration {
    dvec3 center = dvec3{0.0};
    dvec2 radius = dvec2{0.0, 1.0};
    size_t nPoints = 25;
    std::optional<size_t> seed = std::nullopt;
};

template <typename T, typename OutIt>
void generatePoints(OutIt outIt, std::variant<Grid3DPointGeneration, RandomCubicalPointGeneration,
                                              RandomSphericalPointGeneration>
                                     opts) {

    const auto makeMT = [](const auto& opts) {
        std::mt19937_64 mt;
        if (opts.seed) {
            mt.seed(*opts.seed);
        } else {
            std::random_device rd;
            mt.seed(rd());
        }
        return mt;
    };

    std::visit(
        util::overloaded{
            [&](const Grid3DPointGeneration& opts) {
                const auto bases =
                    opts.bases * glm::scale(dvec3{1.0 / opts.nPoints.x, 1.0 / opts.nPoints.y,
                                                  1.0 / opts.nPoints.z});
                if (opts.jitter) {
                    auto mt = makeMT(opts);
                    util::forEachVoxel(opts.nPoints, [&](const auto& pos) {
                        const auto point =
                            dvec3(bases * dvec4(pos, 1.0)) +
                            dvec3{util::randomNumber<double>(mt, -opts.jitter->x, opts.jitter->x),
                                  util::randomNumber<double>(mt, -opts.jitter->y, opts.jitter->y),
                                  util::randomNumber<double>(mt, -opts.jitter->z, opts.jitter->z)};
                        outIt++ = glm::vec<3, T>{point};
                    });

                } else {
                    util::forEachVoxel(opts.nPoints, [&](const auto& pos) {
                        const auto point = bases * dvec4(pos, 1.0);
                        outIt++ = glm::vec<3, T>{point};
                    });
                }
            },
            [&](const RandomCubicalPointGeneration& opts) {
                auto mt = makeMT(opts);

                for (size_t i = 0; i < opts.nPoints; ++i) {
                    const auto point =
                        opts.bases * dvec4{util::randomNumber<T>(mt, T{0.0}, T{1.0}),
                                           util::randomNumber<T>(mt, T{0.0}, T{1.0}),
                                           util::randomNumber<T>(mt, T{0.0}, T{1.0}), T{1.0}};
                    outIt++ = glm::vec<3, T>{point};
                }
            },
            [&](const RandomSphericalPointGeneration& opts) {
                auto mt = makeMT(opts);

                auto thetaGen = [&]() { return util::randomNumber<T>(mt, 0, glm::two_pi<T>()); };
                auto cosPhiGen = [&]() { return util::randomNumber<T>(mt, T{-1.0}, T{1.0}); };
                auto rGen = [&]() { return util::randomNumber<T>(mt, T{0.0}, T{1.0}); };

                for (size_t i = 0; i < opts.nPoints; i++) {
                    const auto theta = thetaGen();
                    const auto phi = std::acos(cosPhiGen());

                    auto r = std::pow(rGen(), T{1.0} / T{3.0});
                    r = static_cast<T>(opts.radius.x + r * (opts.radius.y - opts.radius.x));

                    const auto ct = std::cos(theta);
                    const auto st = std::sin(theta);
                    const auto sp = std::sin(phi);
                    const auto cp = std::cos(phi);

                    const auto g = glm::vec<3, T>{ct * sp, st * sp, cp};
                    const auto p = g * r + glm::vec<3, T>{opts.center};

                    outIt++ = p;
                }
            }},
        opts);
}

}  // namespace util

}  // namespace inviwo
