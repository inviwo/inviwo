/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2018 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 *FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#ifndef IVW_HDF5PATH_H
#define IVW_HDF5PATH_H

#include <modules/hdf5/hdf5moduledefine.h>
#include <inviwo/core/common/inviwo.h>

namespace inviwo {

namespace hdf5 {

class IVW_MODULE_HDF5_API Path {
public:
    Path();
    Path(const std::string &path);
    Path(const Path &rhs);
    Path &operator=(const Path &that);
    Path(Path &&rhs);
    Path &operator=(Path &&that);

    Path &push(const std::string &path);
    Path &push(const Path &path);
    Path &pop();

    Path &operator+=(const Path &path);
    Path &operator+=(const std::string &path);

    operator std::string() const;

    std::string toString() const;

private:
    void splitString(const std::string &string);
    std::vector<std::string> path_;
};

IVW_MODULE_HDF5_API Path operator+(const Path &lhs, const Path &rhs);
IVW_MODULE_HDF5_API Path operator+(const Path &lhs, const std::string &rhs);

template <typename CTy, typename CTr>
IVW_MODULE_HDF5_API std::basic_ostream<CTy, CTr> &operator<<(std::basic_ostream<CTy, CTr> &os,
                                                             const Path &path) {
    return os << path.toString();
}

}  // namespace hdf5

}  // namespace inviwo

#endif  // IVW_HDF5PATH_H
