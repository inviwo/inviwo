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

#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/volume/volume.h>

namespace inviwo {

// Allow overriding virtual functions from Python
class DataReaderTrampoline : public DataReader,
                                               public pybind11::trampoline_self_life_support {
public:
    using DataReader::DataReader; // Inherit constructors

    virtual DataReader* clone() {
        PYBIND11_OVERRIDE_PURE(DataReader*, /* Return type */
                               DataReader,  /* Parent class */
                               clone,        /* Name of function in C++ (must match Python name) */
        );
    }
    virtual bool setOption(std::string_view key, std::any value) {
        PYBIND11_OVERRIDE(bool,              /* Return type */
                          DataReader, /* Parent class */
                          setOption,         /* Name of function in C++ (must match Python name) */
                          key, value         /* Argument(s) */
        );
    }
};

template <typename T>
class DataReaderTypeTrampoline : public DataReaderType<T>,
                                 public pybind11::trampoline_self_life_support {
public:
    using DataReaderType<T>::DataReaderType;  // Inherit constructors

    virtual DataReaderType<T>* clone() {
        PYBIND11_OVERRIDE_PURE(DataReaderType<T>*, /* Return type */
                               DataReaderType<T>,  /* Parent class */
                               clone,               /* Name of function in C++ (must match Python name) */
        );
    }
    virtual bool setOption(std::string_view key, std::any value) {
        PYBIND11_OVERRIDE(bool,              /* Return type */
                               DataReaderType<T>,  /* Parent class */
                               setOption, /* Name of function in C++ (must match Python name) */
                               key, value       /* Argument(s) */
        );
    }
    virtual std::shared_ptr<T> readData(std::string_view filePath) {
        PYBIND11_OVERRIDE_PURE(std::shared_ptr<T>, /* Return type */
                               DataReaderType<T>,  /* Parent class */
                               readData,           /* Name of function in C++ (must match Python name) */
                               filePath            /* Argument(s) */
        );
    }
    virtual std::shared_ptr<T> readData(std::string_view filePath, MetaDataOwner* owner) {
        PYBIND11_OVERRIDE(std::shared_ptr<T>, /* Return type */
                               DataReaderType<T>,  /* Parent class */
                               readData, /* Name of function in C++ (must match Python name) */
                               filePath, owner  /* Argument(s) */
        );
    }
};

void exposeDataReaders(pybind11::module& m) {
    namespace py = pybind11;

    py::class_<DataReader, DataReaderTrampoline>(m, "DataReader")
        .def("clone", &DataReader::clone)
        .def_property_readonly("extensions", &DataReader::getExtensions,
                               py::return_value_policy::reference_internal)
        .def("addExtension", &DataReader::addExtension)
        .def("setOption", &DataReader::setOption)
        .def("getOption", &DataReader::getOption);

    py::class_<DataReaderType<Image>, DataReader, DataReaderTypeTrampoline<Image>>(
        m, "ImageDataReader")
        .def("readData", py::overload_cast<std::string_view>(&DataReaderType<Image>::readData))
        .def("readData",
             py::overload_cast<std::string_view, MetaDataOwner*>(&DataReaderType<Image>::readData));
    py::class_<DataReaderType<Mesh>, DataReader, DataReaderTypeTrampoline<Mesh>>(
        m, "MeshDataReader")
        .def("readData", py::overload_cast<std::string_view>(&DataReaderType<Mesh>::readData))
        .def("readData",
             py::overload_cast<std::string_view, MetaDataOwner*>(&DataReaderType<Mesh>::readData));
    py::class_<DataReaderType<Volume>, DataReader, DataReaderTypeTrampoline<Volume>>(
        m, "VolumeDataReader")
        .def("readData", py::overload_cast<std::string_view>(&DataReaderType<Volume>::readData))
        .def("readData", py::overload_cast<std::string_view, MetaDataOwner*>(
                             &DataReaderType<Volume>::readData));

    py::class_<DataReaderFactory>(m, "DataReaderFactory")
        .def(py::init<>())
        .def("hasKey", [](DataReaderFactory* f, std::string key) { return f->hasKey(key); })
        .def("hasKey", [](DataReaderFactory* f, FileExtension key) { return f->hasKey(key); })
        .def("create", [](DataReaderFactory* f, std::string key) { return f->create(key); })
        .def("create", [](DataReaderFactory* f, FileExtension key) { return f->create(key); })
        // No good way of dealing with template return types so we manually define one for each known type. 
        // https://github.com/pybind/pybind11/issues/1667#issuecomment-454348004
        // If your module exposes a new reader type it will have to bind getXXXReader.
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
