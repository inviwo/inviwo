/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <inviwopy/pydataformat.h>

#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/stdextensions.h>

namespace inviwo {

struct DataFormatHelper {
    template <typename DataFormat>
    auto operator()(pybind11::module &m) {
        namespace py = pybind11;
        m.attr(("Data" + DataFormat::str()).c_str()) =
            py::cast(static_cast<const DataFormatBase *>(DataFormat::get()),
                     py::return_value_policy::reference);
    }
};

void exposeDataFormat(pybind11::module &m) {
    namespace py = pybind11;

    py::enum_<NumericType>(m, "NumericType")
        .value("NotSpecialized", NumericType::NotSpecialized)
        .value("Float", NumericType::Float)
        .value("UnsignedInteger", NumericType::UnsignedInteger)
        .value("SignedInteger", NumericType::SignedInteger);

    py::enum_<DataFormatId> dfid(m, "DataFormatId");
    dfid.value("NotSpecialized", DataFormatId::NotSpecialized);
    for (int i = 1; i < static_cast<int>(DataFormatId::NumberOfFormats); ++i) {
        auto format = DataFormatBase::get(static_cast<DataFormatId>(i));
        dfid.value(format->getString(), static_cast<DataFormatId>(i));
    }
    dfid.value("NumberOfFormats", DataFormatId::NumberOfFormats);

    py::class_<DataFormatBase>(m, "DataFormat")
        .def_property_readonly("size", &DataFormatBase::getSize)
        .def_property_readonly("components", &DataFormatBase::getComponents)
        .def_property_readonly("precision", &DataFormatBase::getPrecision)
        .def_property_readonly("numericType", &DataFormatBase::getNumericType)
        .def_property_readonly("id", &DataFormatBase::getId)
        .def_property_readonly("max", &DataFormatBase::getMax)
        .def_property_readonly("min", &DataFormatBase::getMin)
        .def_property_readonly("lowest", &DataFormatBase::getLowest)
        .def_property_readonly("__repr__", &DataFormatBase::getString);

    util::for_each_type<DefaultDataFormats>{}(DataFormatHelper{}, m);
}

}  // namespace inviwo
