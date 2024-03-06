/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2024 Inviwo Foundation
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
#include <inviwo/core/datastructures/transferfunction.h>
#include <inviwo/core/datastructures/isovaluecollection.h>

#include <modules/python3/opaquetypes.h>

#include <pybind11/stl/filesystem.h>

namespace inviwo {

#include <warn/push>
#include <warn/ignore/attributes>
// Allow overriding virtual functions from Python
template <typename T>
class DataReaderTypeTrampoline : public DataReaderType<T>,
                                 public pybind11::trampoline_self_life_support {
public:
    using DataReaderType<T>::DataReaderType;  // Inherit constructors

    virtual DataReaderType<T>* clone() const override {
        PYBIND11_OVERRIDE_PURE(DataReaderType<T>*, /* Return type */
                               DataReaderType<T>,  /* Parent class */
                               clone, /* Name of function in C++ (must match Python name) */
        );
    }
    virtual bool setOption(std::string_view key, std::any value) override {
        PYBIND11_OVERRIDE(bool,              /* Return type */
                          DataReaderType<T>, /* Parent class */
                          setOption,         /* Name of function in C++ (must match Python name) */
                          key, value         /* Argument(s) */
        );
    }
    virtual std::any getOption(std::string_view key) override {
        PYBIND11_OVERRIDE(std::any,          /* Return type */
                          DataReaderType<T>, /* Parent class */
                          getOption,         /* Name of function in C++ (must match Python name) */
                          key                /* Argument(s) */
        );
    }
    virtual std::shared_ptr<T> readData(const std::filesystem::path& filePath) override {
        PYBIND11_OVERRIDE_PURE(std::shared_ptr<T>, /* Return type */
                               DataReaderType<T>,  /* Parent class */
                               readData, /* Name of function in C++ (must match Python name) */
                               filePath  /* Argument(s) */
        );
    }
    virtual std::shared_ptr<T> readData(const std::filesystem::path& filePath,
                                        MetaDataOwner* owner) override {
        PYBIND11_OVERRIDE(std::shared_ptr<T>, /* Return type */
                          DataReaderType<T>,  /* Parent class */
                          readData,           /* Name of function in C++ (must match Python name) */
                          filePath, owner     /* Argument(s) */
        );
    }
};
#include <warn/pop>

template <typename T>
void exposeFactoryReaderType(pybind11::class_<DataReaderFactory>& r, std::string_view type) {
    namespace py = pybind11;
    r.def(fmt::format("getExtensionsFor{}", type).c_str(),
          (std::vector<FileExtension>(DataReaderFactory::*)() const) &
              DataReaderFactory::getExtensionsForType<T>)
        .def(fmt::format("get{}Reader", type).c_str(),
             (std::unique_ptr<DataReaderType<T>>(DataReaderFactory::*)(const std::filesystem::path&)
                  const) &
                 DataReaderFactory::getReaderForTypeAndExtension<T>)
        .def(
            fmt::format("get{}Reader", type).c_str(),
            (std::unique_ptr<DataReaderType<T>>(DataReaderFactory::*)(const FileExtension&) const) &
                DataReaderFactory::getReaderForTypeAndExtension<T>)
        .def(fmt::format("get{}Reader", type).c_str(),
             (std::unique_ptr<DataReaderType<T>>(DataReaderFactory::*)(
                 const FileExtension&, const std::filesystem::path&) const) &
                 DataReaderFactory::getReaderForTypeAndExtension<T>)
        .def(fmt::format("has{}Reader", type).c_str(),
             (bool(DataReaderFactory::*)(const std::filesystem::path&) const) &
                 DataReaderFactory::hasReaderForTypeAndExtension<T>)
        .def(fmt::format("has{}Reader", type).c_str(),
             (bool(DataReaderFactory::*)(const FileExtension&) const) &
                 DataReaderFactory::hasReaderForTypeAndExtension<T>)
        .def(fmt::format("read{}", type).c_str(),
             &DataReaderFactory::readDataForTypeAndExtension<T>);
}

template <typename T>
void exposeReaderType(pybind11::module& m, std::string_view name) {
    namespace py = pybind11;
    py::class_<DataReaderType<T>, DataReader, DataReaderTypeTrampoline<T>>(
        m, fmt::format("{}DataReader", name).c_str())
        .def("readData",
             py::overload_cast<const std::filesystem::path&>(&DataReaderType<T>::readData))
        .def("readData", py::overload_cast<const std::filesystem::path&, MetaDataOwner*>(
                             &DataReaderType<T>::readData));
}

void exposeDataReaders(pybind11::module& m) {
    namespace py = pybind11;

    py::class_<DataReader>(m, "DataReader")
        .def("clone", &DataReader::clone)
        .def_property_readonly("extensions", &DataReader::getExtensions,
                               py::return_value_policy::reference_internal)
        .def("addExtension", &DataReader::addExtension)
        .def("setOption", &DataReader::setOption)
        .def("getOption", &DataReader::getOption);

    auto fr =
        py::class_<DataReaderFactory>(m, "DataReaderFactory")
            .def("create",
                 (std::unique_ptr<DataReader>(DataReaderFactory::*)(const FileExtension&) const) &
                     DataReaderFactory::create)
            .def("create",
                 (std::unique_ptr<DataReader>(DataReaderFactory::*)(std::string_view) const) &
                     DataReaderFactory::create)
            .def("hasKey",
                 (bool(DataReaderFactory::*)(std::string_view) const) & DataReaderFactory::hasKey)
            .def("hasKey", (bool(DataReaderFactory::*)(const FileExtension&) const) &
                               DataReaderFactory::hasKey);

    // No good way of dealing with template return types so we manually define one for each known
    // type. https://github.com/pybind/pybind11/issues/1667#issuecomment-454348004
    // If your module exposes a new reader type it will have to bind getXXXReader.

    exposeReaderType<Image>(m, "Image");
    exposeFactoryReaderType<Image>(fr, "Image");

    exposeReaderType<Mesh>(m, "Mesh");
    exposeFactoryReaderType<Mesh>(fr, "Mesh");

    exposeReaderType<Volume>(m, "Volume");
    exposeFactoryReaderType<Volume>(fr, "Volume");

    exposeReaderType<TransferFunction>(m, "TransferFunction");
    exposeFactoryReaderType<TransferFunction>(fr, "TransferFunction");

    exposeReaderType<IsoValueCollection>(m, "IsoValueCollection");
    exposeFactoryReaderType<IsoValueCollection>(fr, "IsoValueCollection");
}

}  // namespace inviwo
