/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2025 Inviwo Foundation
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

#include <modules/base/pythonbindings/io/volumewriting.h>
#include <modules/base/pythonbindings/algorithm/volumeoperations.h>
#include <modules/base/algorithm/tfconstruction.h>

#include <modules/python3/pybindmodule.h>

#include <warn/push>
#include <warn/ignore/shadow>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <pybind11/typing.h>
#include <warn/pop>

namespace py = pybind11;

INVIWO_PYBIND_MODULE(ivwbase, m) {
    m.doc() = R"doc(
        Base Module API
    
        .. rubric:: Modules
        
        .. autosummary::
            :toctree: .
            
            io
            algorithm
        )doc";

    auto ioMod = m.def_submodule("io", "Input and Output functions");
    auto utilMod = m.def_submodule("algorithm", "Algorithms and util functions");

    inviwo::exposeVolumeWriteMethods(ioMod);
    inviwo::exposeVolumeOperations(utilMod);

    utilMod.def(
        "tfSawTooth",
        [](py::typing::Iterable<inviwo::TFPrimitiveData> iterable, glm::dvec2 range, double delta,
           double shift, glm::vec4 low) {
            std::vector<inviwo::TFPrimitiveData> points;
            for (auto i : iterable) {
                points.push_back(i.cast<inviwo::TFPrimitiveData>());
            }
            inviwo::util::SawToothOptions opts{points, range, delta, shift, low};
            return inviwo::util::tfSawTooth(opts);
        },
        py::arg("points"), py::arg("range") = glm::dvec2{0.0, 1.0}, py::arg("delta") = 0.01,
        py::arg("shift") = 0.0, py::arg("low") = glm::vec4(0.0));

    utilMod.def("tfMax", [](py::typing::Iterable<inviwo::TFPrimitiveData> a,
                            py::typing::Iterable<inviwo::TFPrimitiveData> b) {
        std::vector<inviwo::TFPrimitiveData> aPoints;
        for (auto i : a) {
            aPoints.push_back(i.cast<inviwo::TFPrimitiveData>());
        }
        std::vector<inviwo::TFPrimitiveData> bPoints;
        for (auto i : b) {
            bPoints.push_back(i.cast<inviwo::TFPrimitiveData>());
        }

        return inviwo::util::tfMax(aPoints, bPoints);
    });
}
