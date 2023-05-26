/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2023 Inviwo Foundation
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

#include <inviwopy/pydatawriters.h>

#include <inviwo/core/io/datawriter.h>
#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/transferfunction.h>
#include <inviwo/core/datastructures/isovaluecollection.h>

#include <pybind11/stl/filesystem.h>

namespace inviwo {

#include <warn/push>
#include <warn/ignore/attributes>
// Allow overriding virtual functions from Python
template <typename T>
class DataWriterTypeTrampoline : public DataWriterType<T>,
                                 public pybind11::trampoline_self_life_support {
public:
    using DataWriterType<T>::DataWriterType;  // Inherit constructors

    virtual DataWriterType<T>* clone() const override {
        PYBIND11_OVERRIDE_PURE(DataWriterType<T>*, /* Return type */
                               DataWriterType<T>,  /* Parent class */
                               clone, /* Name of function in C++ (must match Python name) */
        );
    }
    virtual bool setOption(std::string_view key, std::any value) override {
        PYBIND11_OVERRIDE(bool,              /* Return type */
                          DataWriterType<T>, /* Parent class */
                          setOption,         /* Name of function in C++ (must match Python name) */
                          key, value         /* Argument(s) */
        );
    }
    virtual std::any getOption(std::string_view key) const override {
        PYBIND11_OVERRIDE(std::any,          /* Return type */
                          DataWriterType<T>, /* Parent class */
                          getOption,         /* Name of function in C++ (must match Python name) */
                          key                /* Argument(s) */
        );
    }
    virtual void writeData(const T* data, const std::filesystem::path& filePath) const override {
        PYBIND11_OVERRIDE_PURE(void,              /* Return type */
                               DataWriterType<T>, /* Parent class */
                               writeData,     /* Name of function in C++ (must match Python name) */
                               data, filePath /* Argument(s) */
        );
    }
};
#include <warn/pop>

template <typename T>
void exposeFactoryWriterType(pybind11::class_<DataWriterFactory>& r, std::string_view type) {
    namespace py = pybind11;
    r.def(fmt::format("getExtensionsFor{}", type).c_str(),
          (std::vector<FileExtension>(DataWriterFactory::*)() const) &
              DataWriterFactory::getExtensionsForType<T>)
        .def(fmt::format("get{}Writer", type).c_str(),
             (std::unique_ptr<DataWriterType<T>>(DataWriterFactory::*)(const std::filesystem::path&)
                  const) &
                 DataWriterFactory::getWriterForTypeAndExtension<T>)
        .def(
            fmt::format("get{}Writer", type).c_str(),
            (std::unique_ptr<DataWriterType<T>>(DataWriterFactory::*)(const FileExtension&) const) &
                DataWriterFactory::getWriterForTypeAndExtension<T>)
        .def(fmt::format("get{}Writer", type).c_str(),
             (std::unique_ptr<DataWriterType<T>>(DataWriterFactory::*)(
                 const FileExtension&, const std::filesystem::path&) const) &
                 DataWriterFactory::getWriterForTypeAndExtension<T>)
        .def(fmt::format("has{}Writer", type).c_str(),
             (bool(DataWriterFactory::*)(const std::filesystem::path&) const) &
                 DataWriterFactory::hasWriterForTypeAndExtension<T>)
        .def(fmt::format("has{}Writer", type).c_str(),
             (bool(DataWriterFactory::*)(const FileExtension&) const) &
                 DataWriterFactory::hasWriterForTypeAndExtension<T>)
        .def(fmt::format("write{}", type).c_str(),
             &DataWriterFactory::writeDataForTypeAndExtension<T>);
}

template <typename T>
void exposeWriterType(pybind11::module& m, std::string_view name) {
    namespace py = pybind11;
    py::class_<DataWriterType<T>, DataWriter, DataWriterTypeTrampoline<T>>(
        m, fmt::format("{}DataWriter", name).c_str())
        .def("writeData", &DataWriterType<T>::writeData);
}

void exposeDataWriters(pybind11::module& m) {
    namespace py = pybind11;

    py::class_<DataWriter>(m, "DataWriter")
        .def("clone", &DataWriter::clone)
        .def_property_readonly("extensions", &DataWriter::getExtensions,
                               py::return_value_policy::reference_internal)
        .def("addExtension", &DataWriter::addExtension)
        .def("getOverwrite", &DataWriter::getOverwrite)
        .def("setOverwrite", &DataWriter::setOverwrite)
        .def("setOption", &DataWriter::setOption)
        .def("getOption", &DataWriter::getOption);

    auto fr =
        py::class_<DataWriterFactory>(m, "DataWriterFactory")
            .def("create",
                 (std::unique_ptr<DataWriter>(DataWriterFactory::*)(const FileExtension&) const) &
                     DataWriterFactory::create)
            .def("create",
                 (std::unique_ptr<DataWriter>(DataWriterFactory::*)(std::string_view) const) &
                     DataWriterFactory::create)
            .def("hasKey",
                 (bool(DataWriterFactory::*)(std::string_view) const) & DataWriterFactory::hasKey)
            .def("hasKey", (bool(DataWriterFactory::*)(const FileExtension&) const) &
                               DataWriterFactory::hasKey);

    // No good way of dealing with template return types so we manually define one for each known
    // type. https://github.com/pybind/pybind11/issues/1667#issuecomment-454348004
    // If your module exposes a new Writer type it will have to bind getXXXWriter.

    exposeWriterType<Image>(m, "Image");
    exposeFactoryWriterType<Image>(fr, "Image");

    exposeWriterType<Mesh>(m, "Mesh");
    exposeFactoryWriterType<Mesh>(fr, "Mesh");

    exposeWriterType<Volume>(m, "Volume");
    exposeFactoryWriterType<Volume>(fr, "Volume");

    exposeWriterType<TransferFunction>(m, "TransferFunction");
    exposeFactoryWriterType<TransferFunction>(fr, "TransferFunction");

    exposeWriterType<IsoValueCollection>(m, "IsoValueCollection");
    exposeFactoryWriterType<IsoValueCollection>(fr, "IsoValueCollection");
}

}  // namespace inviwo
