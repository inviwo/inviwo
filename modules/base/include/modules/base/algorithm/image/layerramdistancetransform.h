/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#ifndef IVW_LAYERRAMDISTANCETRANSFORM_H
#define IVW_LAYERRAMDISTANCETRANSFORM_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/indexmapper.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>

#ifndef __clang__
#include <omp.h>
#endif

namespace inviwo {

namespace util {

/**
 *	Implementation of Euclidean Distance Transform according to Saito's algorithm:
 *  T. Saito and J.I. Toriwaki. New algorithms for Euclidean distance transformations
 *  of an n-dimensional digitized picture with applications. Pattern Recognition, 27(11).
 *  pp. 1551-1565, 1994.
 *  http://www.cs.jhu.edu/~misha/ReadingSeminar/Papers/Saito94.pdf
 *
 * Calculates the distance in base mat space
 *     * Predicate is a function of type (const T &value) -> bool to deside if a value in the input
 *       is a "feature".
 *     * ValueTransform is a function of type (const U& squaredDist) -> U that is appiled to all
 *       squared distance values at the end of the calculation.
 *     * ProcessCallback is a function of type (double progress) -> void that is called with a value
 *       from 0 to 1 to indicate the progress of the calculation.
 */
template <typename T, typename U, typename Predicate, typename ValueTransform,
          typename ProgressCallback>
void layerRAMDistanceTransform(const LayerRAMPrecision<T> *inLayer,
                               LayerRAMPrecision<U> *outDistanceField, const Matrix<2, U> basis,
                               const size2_t upsample, Predicate predicate,
                               ValueTransform valueTransform, ProgressCallback callback);

template <typename T, typename U>
void layerRAMDistanceTransform(const LayerRAMPrecision<T> *inVolume,
                               LayerRAMPrecision<U> *outDistanceField, const Matrix<2, U> basis,
                               const size2_t upsample);

template <typename U, typename Predicate, typename ValueTransform, typename ProgressCallback>
void layerDistanceTransform(const Layer *inLayer, LayerRAMPrecision<U> *outDistanceField,
                            const size2_t upsample, Predicate predicate,
                            ValueTransform valueTransform, ProgressCallback callback);

template <typename U, typename ProgressCallback>
void layerDistanceTransform(const Layer *inLayer, LayerRAMPrecision<U> *outDistanceField,
                            const size2_t upsample, double threshold, bool normalize, bool flip,
                            bool square, double scale, ProgressCallback callback);

template <typename U>
void layerDistanceTransform(const Layer *inLayer, LayerRAMPrecision<U> *outDistanceField,
                            const size2_t upsample, double threshold, bool normalize, bool flip,
                            bool square, double scale);

}  // namespace util

template <typename T, typename U, typename Predicate, typename ValueTransform,
          typename ProgressCallback>
void util::layerRAMDistanceTransform(const LayerRAMPrecision<T> *inLayer,
                                     LayerRAMPrecision<U> *outDistanceField,
                                     const Matrix<2, U> basis, const size2_t upsample,
                                     Predicate predicate, ValueTransform valueTransform,
                                     ProgressCallback callback) {

#ifndef __clang__
    omp_set_num_threads(std::thread::hardware_concurrency());
#endif

    using int64 = glm::int64;
    using i64vec2 = glm::tvec2<int64>;

    auto square = [](auto a) { return a * a; };

    callback(0.0);

    const T *src = inLayer->getDataTyped();
    U *dst = outDistanceField->getDataTyped();

    const i64vec2 srcDim{inLayer->getDimensions()};
    const i64vec2 dstDim{outDistanceField->getDimensions()};
    const i64vec2 sm{upsample};

    const auto squareBasis = glm::transpose(basis) * basis;
    const Vector<2, U> squareBasisDiag{squareBasis[0][0], squareBasis[1][1]};
    const Vector<2, U> squareVoxelSize{squareBasisDiag / Vector<2, U>{dstDim * dstDim}};
    const Vector<2, U> invSquareVoxelSize{Vector<2, U>{1.0f} / squareVoxelSize};

    {
        const auto maxdist = glm::compMax(squareBasisDiag);
        bool orthogonal = true;
        for (size_t i = 0; i < squareBasis.length(); i++) {
            for (size_t j = 0; j < squareBasis.length(); j++) {
                if (i != j) {
                    if (std::abs(squareBasis[i][j]) > 10.0e-8 * maxdist) {
                        orthogonal = false;
                        break;
                    }
                }
            }
        }
        if (!orthogonal) {
            LogWarnCustom(
                "layerRAMDistanceTransform",
                "Calculating the distance transform on a non-orthogonal layer will not give "
                "correct values");
        }
    }

    if (srcDim * sm != dstDim) {
        throw Exception(
            "DistanceTransformRAM: Dimensions does not match src = " + toString(srcDim) +
                " dst = " + toString(dstDim) + " scaling = " + toString(sm),
            IVW_CONTEXT_CUSTOM("layerRAMDistanceTransform"));
    }

    util::IndexMapper<2, int64> srcInd(srcDim);
    util::IndexMapper<2, int64> dstInd(dstDim);

    auto is_feature = [&](const int64 x, const int64 y) {
        return predicate(src[srcInd(x / sm.x, y / sm.y)]);
    };

// first pass, forward and backward scan along x
// result: min distance in x direction
#pragma omp parallel for
    for (int64 y = 0; y < dstDim.y; ++y) {
        // forward
        U dist = static_cast<U>(dstDim.x);
        for (int64 x = 0; x < dstDim.x; ++x) {
            if (!is_feature(x, y)) {
                ++dist;
            } else {
                dist = U(0);
            }
            dst[dstInd(x, y)] = squareVoxelSize.x * square(dist);
        }

        // backward
        dist = static_cast<U>(dstDim.x);
        for (int64 x = dstDim.x - 1; x >= 0; --x) {
            if (!is_feature(x, y)) {
                ++dist;
            } else {
                dist = U(0);
            }
            dst[dstInd(x, y)] = std::min<U>(dst[dstInd(x, y)], squareVoxelSize.x * square(dist));
        }
    }

    // second pass, scan y direction
    // for each voxel v(x,y,z) find min_i(data(x,i,z) + (y - i)^2), 0 <= i < dimY
    // result: min distance in x and y direction
    callback(0.45);
#pragma omp parallel
    {
        std::vector<U> buff;
        buff.resize(dstDim.y);
#pragma omp for
        for (int64 x = 0; x < dstDim.x; ++x) {

            // cache column data into temporary buffer
            for (int64 y = 0; y < dstDim.y; ++y) {
                buff[y] = dst[dstInd(x, y)];
            }

            for (int64 y = 0; y < dstDim.y; ++y) {
                auto d = buff[y];
                if (d != U(0)) {
                    const auto rMax = static_cast<int64>(std::sqrt(d * invSquareVoxelSize.y)) + 1;
                    const auto rStart = std::min(rMax, y - 1);
                    const auto rEnd = std::min(rMax, dstDim.y - y);
                    for (int64 n = -rStart; n < rEnd; ++n) {
                        const auto w = buff[y + n] + squareVoxelSize.y * square(n);
                        if (w < d) d = w;
                    }
                }
                dst[dstInd(x, y)] = d;
            }
        }
    }

    // scale data
    callback(0.9);
    const int64 layerSize = dstDim.x * dstDim.y;
#pragma omp parallel for
    for (int64 i = 0; i < layerSize; ++i) {
        dst[i] = valueTransform(dst[i]);
    }
    callback(1.0);
}

template <typename T, typename U>
void util::layerRAMDistanceTransform(const LayerRAMPrecision<T> *inLayer,
                                     LayerRAMPrecision<U> *outDistanceField,
                                     const Matrix<2, U> basis, const size2_t upsample) {

    util::layerRAMDistanceTransform(
        inLayer, outDistanceField, basis, upsample,
        [](const T &val) { return util::glm_convert_normalized<double>(val) > 0.5; },
        [](const U &squareDist) {
            return static_cast<U>(std::sqrt(static_cast<double>(squareDist)));
        },
        [](double f) {});
}

template <typename U, typename Predicate, typename ValueTransform, typename ProgressCallback>
void util::layerDistanceTransform(const Layer *inLayer, LayerRAMPrecision<U> *outDistanceField,
                                  const size2_t upsample, Predicate predicate,
                                  ValueTransform valueTransform, ProgressCallback callback) {

    const auto inputLayerRep = inLayer->getRepresentation<LayerRAM>();
    inputLayerRep->dispatch<void, dispatching::filter::Scalars>([&](const auto lrprecision) {
        layerRAMDistanceTransform(lrprecision, outDistanceField, inLayer->getBasis(), upsample,
                                  predicate, valueTransform, callback);
    });
}

template <typename U, typename ProgressCallback>
void util::layerDistanceTransform(const Layer *inLayer, LayerRAMPrecision<U> *outDistanceField,
                                  const size2_t upsample, double threshold, bool normalize,
                                  bool flip, bool square, double scale, ProgressCallback progress) {

    const auto inputLayerRep = inLayer->getRepresentation<LayerRAM>();
    inputLayerRep->dispatch<void, dispatching::filter::Scalars>([&](const auto lrprecision) {
        using ValueType = util::PrecisionValueType<decltype(lrprecision)>;

        const auto predicateIn = [threshold](const ValueType &val) { return val < threshold; };
        const auto predicateOut = [threshold](const ValueType &val) { return val > threshold; };

        const auto normPredicateIn = [threshold](const ValueType &val) {
            return util::glm_convert_normalized<double>(val) < threshold;
        };
        const auto normPredicateOut = [threshold](const ValueType &val) {
            return util::glm_convert_normalized<double>(val) > threshold;
        };

        const auto valTransIdent = [scale](const float &squareDist) {
            return static_cast<float>(scale * squareDist);
        };
        const auto valTransSqrt = [scale](const float &squareDist) {
            return static_cast<float>(scale * std::sqrt(squareDist));
        };

        if (normalize && square && flip) {
            util::layerRAMDistanceTransform(lrprecision, outDistanceField, inLayer->getBasis(),
                                            upsample, normPredicateIn, valTransIdent, progress);
        } else if (normalize && square && !flip) {
            util::layerRAMDistanceTransform(lrprecision, outDistanceField, inLayer->getBasis(),
                                            upsample, normPredicateOut, valTransIdent, progress);
        } else if (normalize && !square && flip) {
            util::layerRAMDistanceTransform(lrprecision, outDistanceField, inLayer->getBasis(),
                                            upsample, normPredicateIn, valTransSqrt, progress);
        } else if (normalize && !square && !flip) {
            util::layerRAMDistanceTransform(lrprecision, outDistanceField, inLayer->getBasis(),
                                            upsample, normPredicateOut, valTransSqrt, progress);
        } else if (!normalize && square && flip) {
            util::layerRAMDistanceTransform(lrprecision, outDistanceField, inLayer->getBasis(),
                                            upsample, predicateIn, valTransIdent, progress);
        } else if (!normalize && square && !flip) {
            util::layerRAMDistanceTransform(lrprecision, outDistanceField, inLayer->getBasis(),
                                            upsample, predicateOut, valTransIdent, progress);
        } else if (!normalize && !square && flip) {
            util::layerRAMDistanceTransform(lrprecision, outDistanceField, inLayer->getBasis(),
                                            upsample, predicateIn, valTransSqrt, progress);
        } else if (!normalize && !square && !flip) {
            util::layerRAMDistanceTransform(lrprecision, outDistanceField, inLayer->getBasis(),
                                            upsample, predicateOut, valTransSqrt, progress);
        }
    });
}

template <typename U>
void util::layerDistanceTransform(const Layer *inLayer, LayerRAMPrecision<U> *outDistanceField,
                                  const size2_t upsample, double threshold, bool normalize,
                                  bool flip, bool square, double scale) {
    util::layerDistanceTransform(inLayer, outDistanceField, upsample, threshold, normalize, flip,
                                 square, scale, [](double) {});
}

}  // namespace inviwo

#endif  // IVW_LAYERRAMDISTANCETRANSFORM_H
