/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
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

#include <inviwo/core/util/formatdispatching.h>
#include <inviwo/core/util/stringconversion.h>

#include <inviwo/core/datastructures/volume/volumeram.h>

namespace inviwo {

std::string dispatching::detail::predicateNameHelper(const char* name) {
    auto str = util::demangle(name);

    // Example name "inviwo::dispatching::filter::Vec3s<inviwo::DataFormat<float>>"

    constexpr std::string_view prefix = "inviwo::dispatching::filter::";
    constexpr std::string_view postfix = "<inviwo::DataFormat<float>>";

    if (str.ends_with(postfix)) {
        str.erase(str.end() - postfix.size(), str.end());
    }

    if (str.starts_with(prefix)) {
        str.erase(str.begin(), str.begin() + prefix.size());
    }
    return str;
}

#include <warn/push>
#include <warn/ignore/unused-function>

namespace {

//! [Format singleDispatch example]
std::shared_ptr<VolumeRAM> create(DataFormatId dataFormatId) {
    return dispatching::singleDispatch<std::shared_ptr<VolumeRAM>, dispatching::filter::All>(
        dataFormatId, []<typename T>() {
            return std::make_shared<VolumeRAMPrecision<T>>(size3_t{128, 128, 128});
        });
}
//! [Format singleDispatch example]

//! [Format doubleDispatch example]
std::shared_ptr<VolumeRAMPrecision<double>> sum(const VolumeRAM& v1, const VolumeRAM& v2) {
    auto sum = std::make_shared<VolumeRAMPrecision<double>>(v1.getDimensions());

    dispatching::doubleDispatch<void, dispatching::filter::Scalars, dispatching::filter::Scalars>(
        v1.getDataFormat()->getId(), v2.getDataFormat()->getId(), [&]<typename T1, typename T2>() {
            const auto& ram1 = static_cast<const VolumeRAMPrecision<T1>&>(v1);
            const auto& ram2 = static_cast<const VolumeRAMPrecision<T2>&>(v2);

            const auto data1 = ram1.getView();
            const auto data2 = ram2.getView();
            auto dest = sum->getView();

            for (size_t i = 0; data1.size(); ++i) {
                dest[i] = static_cast<double>(data1[i]) + static_cast<double>(data2[i]);
            }
        });
    return sum;
}
//! [Format doubleDispatch example]

//! [Format tripleDispatch example]
std::shared_ptr<VolumeRAM> sum(const VolumeRAM& v1, const VolumeRAM& v2, DataFormatId sumType) {
    return dispatching::tripleDispatch<std::shared_ptr<VolumeRAM>, dispatching::filter::Scalars,
                                       dispatching::filter::Scalars, dispatching::filter::Scalars>(
        v1.getDataFormat()->getId(), v2.getDataFormat()->getId(), sumType,
        [&]<typename T1, typename T2, typename T3>() {
            auto sum = std::make_shared<VolumeRAMPrecision<T3>>(v1.getDimensions());
            const auto& ram1 = static_cast<const VolumeRAMPrecision<T1>&>(v1);
            const auto& ram2 = static_cast<const VolumeRAMPrecision<T2>&>(v2);

            const auto data1 = ram1.getView();
            const auto data2 = ram2.getView();
            auto dest = sum->getView();

            for (size_t i = 0; data1.size(); ++i) {
                dest[i] = static_cast<T3>(data1[i]) + static_cast<T3>(data2[i]);
            }
            return sum;
        });
}
//! [Format tripleDispatch example]

}  // namespace

#include <warn/pop>

}  // namespace inviwo
