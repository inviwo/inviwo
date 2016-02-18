/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#include <Eigen/Dense>

namespace inviwo {

    namespace util{


       
        template <typename T, typename std::enable_if<util::rank<T>::value == 1, int>::type = 0>
        auto glm2eigen(T& elem) -> Eigen::Matrix<typename T::value_type, util::extent<T, 0>::value, 1> {
            Eigen::Vector<typename T::value_type, util::extent<T, 0>::value> a;
            for (size_t i = 0; i < util::extent<T, 0>::value; i++) {
                a(i) = elem[i];
            }
            return a;
        }
        template <typename T, typename std::enable_if<util::rank<T>::value == 2, int>::type = 0>
        auto glm2eigen(T& elem) -> Eigen::Matrix<typename T::value_type, util::extent<T, 0>::value, util::extent<T, 1>::value> {
            Eigen::Matrix<typename T::value_type, util::extent<T, 0>::value, util::extent<T, 1>::value> a;
            for (size_t i = 0; i < util::extent<T, 0>::value; i++) {
                for (size_t j = 0; j < util::extent<T, 1>::value; j++) {
                    a(i,j) = elem[i][j];
                }
            }
            return a;
        }

    }

} // namespace

#endif // IVW_EIGENUTILS_H

