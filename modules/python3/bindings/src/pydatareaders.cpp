/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2021 Inviwo Foundation
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

#include <inviwopy/pydatareaders.h>
#include <inviwopy/vectoridentifierwrapper.h>
#include <inviwopy/pypropertytypehook.h>

#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/volume/volume.h>

#include <warn/push>
#include <warn/ignore/shadow>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <warn/pop>

namespace inviwo {


void exposeDataReaders(pybind11::module& m) {
    namespace py = pybind11;

    py::class_<DataReader>(m, "DataReader")
        .def("clone", &DataReader::clone)
        .def("getExtensions", &DataReader::getExtensions)
        .def("addExtension", &DataReader::addExtension)
        .def("setOption", &DataReader::setOption)
        .def("getOption", &DataReader::getOption);

    py::class_<DataReaderType<Image>, DataReader>(m, "ImageDataReader")
        .def("readData", [](DataReaderType<Image>* reader, std::string_view filePath) {
                return reader->readData(filePath);
            },
            py::return_value_policy::reference);
    py::class_<DataReaderType<Mesh>, DataReader>(m, "MeshDataReader")
        .def("readData", [](DataReaderType<Mesh>* reader, std::string_view filePath) {
                return reader->readData(filePath);
            },
            py::return_value_policy::reference);
    py::class_<DataReaderType<Volume>, DataReader>(m, "VolumeDataReader")
        .def("readData", [](DataReaderType<Volume>* reader, std::string_view filePath) {
                return reader->readData(filePath);
            },
            py::return_value_policy::reference);

    py::class_<DataReaderFactory>(m, "DataReaderFactory")
        .def("hasKey", [](DataReaderFactory* f, std::string key) { return f->hasKey(key); })
        .def("hasKey", [](DataReaderFactory* f, FileExtension key) { return f->hasKey(key); })
        .def("create", [](DataReaderFactory* f, std::string key) { return f->create(key); })
        .def("create", [](DataReaderFactory* f, FileExtension key) { return f->create(key); })
        .def("getImageReader", [](DataReaderFactory* f, std::string_view filePathOrExtension) {
            return f->getReaderForTypeAndExtension<Image>(filePathOrExtension);
        })
        .def("getMeshReader", [](DataReaderFactory* f, std::string_view filePathOrExtension) {
                 return f->getReaderForTypeAndExtension<Mesh>(filePathOrExtension);
             })
        .def("getVolumeReader", [](DataReaderFactory* f, std::string_view filePathOrExtension) {
            return f->getReaderForTypeAndExtension<Volume>(filePathOrExtension);
        });


}

}  // namespace inviwo
