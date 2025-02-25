/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2025 Inviwo Foundation
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

#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/numpy.h>

#include <inviwo/core/io/datawriter.h>
#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/transferfunction.h>
#include <inviwo/core/datastructures/isovaluecollection.h>

#include <modules/python3/opaquetypes.h>
#include <modules/python3/polymorphictypehooks.h>

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
class TypedDataWriterFactoryWrapper {
public:
    TypedDataWriterFactoryWrapper() = delete;
    explicit TypedDataWriterFactoryWrapper(DataWriterFactory& rf) : factory{&rf} {}
    TypedDataWriterFactoryWrapper(const TypedDataWriterFactoryWrapper&) = default;
    TypedDataWriterFactoryWrapper(TypedDataWriterFactoryWrapper&&) = default;
    TypedDataWriterFactoryWrapper& operator=(const TypedDataWriterFactoryWrapper&) = default;
    TypedDataWriterFactoryWrapper& operator=(TypedDataWriterFactoryWrapper&&) = default;

    std::vector<FileExtension> getExtensions() const { return factory->getExtensionsForType<T>(); }
    std::unique_ptr<DataWriterType<T>> getWriter(
        const std::filesystem::path& filePathOrExtension) const {
        return factory->getWriterForTypeAndExtension<T>(filePathOrExtension);
    }

    std::unique_ptr<DataWriterType<T>> getWriter(const FileExtension& ext) const {
        return factory->getWriterForTypeAndExtension<T>(ext);
    }

    std::unique_ptr<DataWriterType<T>> getWriter(
        const FileExtension& ext, const std::filesystem::path& fallbackFilePathOrExtension) const {
        return factory->getWriterForTypeAndExtension<T>(ext, fallbackFilePathOrExtension);
    }

    bool hasWriter(const std::filesystem::path& filePathOrExtension) const {
        return factory->hasWriterForTypeAndExtension<T>(filePathOrExtension);
    }

    bool hasWriter(const FileExtension& ext) const {
        return factory->hasWriterForTypeAndExtension<T>(ext);
    }

    bool write(const T* data, const std::filesystem::path& filePath,
               std::optional<FileExtension> ext = std::nullopt) {
        return factory->writeDataForTypeAndExtension<T>(data, filePath, ext);
    }

private:
    DataWriterFactory* factory;
};

template <typename T>
void exposeFactoryWriterType(pybind11::module& m, pybind11::classh<DataWriterFactory>& r,
                             std::string_view type) {
    namespace py = pybind11;

    using TWF = TypedDataWriterFactoryWrapper<T>;
    py::classh<TWF>(m, fmt::format("{}WriterFactory", type).c_str())
        .def("getExtensions", &TWF::getExtensions)
        .def("getWriter",
             static_cast<std::unique_ptr<DataWriterType<T>> (TWF::*)(const std::filesystem::path&)
                             const>(&TWF::getWriter),
             py::arg("path"))
        .def("getWriter",
             static_cast<std::unique_ptr<DataWriterType<T>> (TWF::*)(const FileExtension&) const>(
                 &TWF::getWriter),
             py::arg("fileExtension"))
        .def("getWriter",
             static_cast<std::unique_ptr<DataWriterType<T>> (TWF::*)(
                 const FileExtension&, const std::filesystem::path&) const>(&TWF::getWriter),
             py::arg("fileExtension"), py::arg("fallbackPath"))
        .def("hasWriter",
             static_cast<bool (TWF::*)(const std::filesystem::path&) const>(&TWF::hasWriter),
             py::arg("path"))
        .def("hasWriter", static_cast<bool (TWF::*)(const FileExtension&) const>(&TWF::hasWriter),
             py::arg("fileExtension"))
        .def("write", &TWF::write, py::arg("data"), py::arg("path"),
             py::arg("fileExtension") = std::nullopt);

    r.def_property_readonly(
        toLower(type).c_str(), [](DataWriterFactory& rf) { return TWF(rf); },
        fmt::format("The {} Writer Factory", type).c_str());
}

template <typename T>
void exposeWriterType(pybind11::module& m, std::string_view name) {
    pybind11::classh<DataWriterType<T>, DataWriter, DataWriterTypeTrampoline<T>>(
        m, fmt::format("{}DataWriter", name).c_str())
        .def("writeData", &DataWriterType<T>::writeData);
}

void exposeDataWriters(pybind11::module& m) {
    pybind11::classh<DataWriter>(m, "DataWriter")
        .def("clone", &DataWriter::clone)
        .def_property_readonly("extensions", &DataWriter::getExtensions,
                               pybind11::return_value_policy::reference_internal)
        .def("addExtension", &DataWriter::addExtension)
        .def("getOverwrite", &DataWriter::getOverwrite)
        .def("setOverwrite", &DataWriter::setOverwrite)
        .def("setOption", &DataWriter::setOption)
        .def("getOption", &DataWriter::getOption);

    auto fr = pybind11::classh<DataWriterFactory>(m, "DataWriterFactory");
    fr.def("create", static_cast<std::unique_ptr<DataWriter> (DataWriterFactory::*)(
                         const FileExtension&) const>(&DataWriterFactory::create))
        .def(
            "create",
            static_cast<std::unique_ptr<DataWriter> (DataWriterFactory::*)(std::string_view) const>(
                &DataWriterFactory::create))
        .def("hasKey", static_cast<bool (DataWriterFactory::*)(std::string_view) const>(
                           &DataWriterFactory::hasKey))
        .def("hasKey", static_cast<bool (DataWriterFactory::*)(const FileExtension&) const>(
                           &DataWriterFactory::hasKey));

    // No good way of dealing with template return types so we manually define one for each known
    // type. https://github.com/pybind/pybind11/issues/1667#issuecomment-454348004
    // If your module exposes a new Writer type it will have to bind getXXXWriter.

    exposeWriterType<Layer>(m, "Layer");
    exposeFactoryWriterType<Layer>(m, fr, "Layer");

    exposeWriterType<Image>(m, "Image");
    exposeFactoryWriterType<Image>(m, fr, "Image");

    exposeWriterType<Mesh>(m, "Mesh");
    exposeFactoryWriterType<Mesh>(m, fr, "Mesh");

    exposeWriterType<Volume>(m, "Volume");
    exposeFactoryWriterType<Volume>(m, fr, "Volume");

    exposeWriterType<TransferFunction>(m, "TransferFunction");
    exposeFactoryWriterType<TransferFunction>(m, fr, "TransferFunction");

    exposeWriterType<IsoValueCollection>(m, "IsoValueCollection");
    exposeFactoryWriterType<IsoValueCollection>(m, fr, "IsoValueCollection");
}

}  // namespace inviwo
