/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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

#ifndef IVW_EIGENUTILS_H
#define IVW_EIGENUTILS_H

#include <modules/eigenutils/eigenutilsmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>

#include <warn/push>
#include <warn/ignore/all>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <warn/pop>

namespace inviwo {

class IVW_MODULE_EIGENUTILS_API EigenException : public Exception {
public:
    EigenException(const std::string& message = "", ExceptionContext context = ExceptionContext());
    virtual ~EigenException() noexcept = default;
};

namespace util {

template <typename T, typename std::enable_if<util::rank<T>::value == 1, int>::type = 0>
auto glm2eigen(const T& elem)
    -> Eigen::Matrix<typename T::value_type, util::extent<T, 0>::value, 1> {
    Eigen::Matrix<typename T::value_type, util::extent<T, 0>::value, 1> a;
    for (size_t i = 0; i < util::extent<T, 0>::value; i++) {
        a(i) = elem[i];
    }
    return a;
}

template <typename T, typename std::enable_if<util::rank<T>::value == 2, int>::type = 0>
auto glm2eigen(const T& elem)
    -> Eigen::Matrix<typename T::value_type, util::extent<T, 0>::value, util::extent<T, 1>::value> {
    Eigen::Matrix<typename T::value_type, util::extent<T, 0>::value, util::extent<T, 1>::value> a;
    for (size_t row = 0; row < util::extent<T, 0>::value; row++) {
        for (size_t col = 0; col < util::extent<T, 1>::value; col++) {
            a(row, col) = elem[col][row];
        }
    }
    return a;
}

template <
    typename T, unsigned Rows, unsigned Cols,
    typename std::enable_if<(Rows >= 2 && Rows <= 4 && Cols >= 2 && Cols <= 4), int>::type = 0>
auto eigen2glm(const Eigen::Matrix<T, Rows, Cols>& m) {
    using GlmMatrix = typename util::glmtype<T, Cols, Rows>::type;
    GlmMatrix outm;
    for (size_t row = 0; row < Rows; row++) {
        for (size_t col = 0; col < Cols; col++) {
            outm[col][row] = m(row, col);
        }
    }
    return outm;
}

template <typename T, unsigned Rows, unsigned Cols,
          typename std::enable_if<(Rows >= 2 && Rows <= 4 && Cols == 1), int>::type = 0>
auto eigen2glm(const Eigen::Matrix<T, Rows, Cols>& m) {
    using GlmVector = typename util::glmtype<T, Rows, 1>::type;
    GlmVector outv;
    for (size_t row = 0; row < Rows; row++) {
        outv[row] = m(row);
    }
    return outv;
}

template <typename T, unsigned Rows, unsigned Cols,
          typename std::enable_if<(Cols >= 2 && Cols <= 4 && Rows == 1), int>::type = 0>
auto eigen2glm(const Eigen::Matrix<T, Cols, Cols>& m) {
    using GlmVector = typename util::glmtype<T, Cols, 1>::type;
    GlmVector outv;
    for (size_t row = 0; row < Cols; row++) {
        outv[row] = m(row);
    }
    return outv;
}

template <typename T>
std::shared_ptr<Image> eigenMatToImage(const T& m, bool flipY = false, std::string name = "") {
    using Type = typename T::value_type;
    if (!T::IsRowMajor) {
        using RowMajorM =
            Eigen::Matrix<Type, T::RowsAtCompileTime, T::ColsAtCompileTime, Eigen::RowMajor>;
        return eigenMatToImage(RowMajorM(m), flipY, name);
    }

    auto img = std::make_shared<Image>(size2_t(m.cols(), m.rows()), DataFormat<Type>::get());
    img->getColorLayer()->setSwizzleMask(swizzlemasks::luminance);
    if (name != "") {
        img->template setMetaData<StringMetaData>("name", name);
    }

    auto rep = dynamic_cast<LayerRAMPrecision<Type>*>(
        img->getColorLayer(0)->template getEditableRepresentation<LayerRAM>());
    auto data = rep->getDataTyped();
    if (flipY) {
        std::copy_n(m.colwise().reverse().eval().data(), m.rows() * m.cols(), data);
    } else {
        std::copy_n(m.data(), m.rows() * m.cols(), data);
    }

    return img;
}

enum class DownsampleAggreationMode {
    First
    //,Min,Max,Avg
};

template <typename T>
Eigen::Matrix<T, -1, -1> downsample(
    const Eigen::Matrix<T, -1, -1>& src, Eigen::Index rows, Eigen::Index cols,
    DownsampleAggreationMode mode = DownsampleAggreationMode::First) {
    if (rows == src.rows() && cols == src.cols()) return src;
    if (rows > src.rows() || cols > src.cols()) {
        throw EigenException("Size of output matrix is larger than input matrix, can't downsample",
                             IvwContextCustom("eigen::downsample"));
    }


    Eigen::Matrix<T, -1, -1> dst(rows, cols);

    double dx = static_cast<double>(src.cols()) /  static_cast<double>(cols); 
    double dy = static_cast<double>(src.rows()) /  static_cast<double>(rows); 

    switch (mode)
    {
    case inviwo::util::DownsampleAggreationMode::First:
    {
        // eigen matrices are col major
        for(int col = 0; col<cols;col++ ){
            auto readCol = static_cast<int>(col*dx);
            for(int row = 0; row<rows;row++ ){
                auto readRow = static_cast<int>(row*dy);
                dst(row,col) = src(readRow,readCol);
            }}
    }
        break;
    default:
        break;
    }

    return dst;


}

}  // namespace util

}  // namespace inviwo

#endif  // IVW_EIGENUTILS_H
