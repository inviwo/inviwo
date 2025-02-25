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

#include <inviwopy/pydatareaders.h>

#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/numpy.h>

#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/transferfunction.h>
#include <inviwo/core/datastructures/isovaluecollection.h>
#include <inviwo/core/util/stringconversion.h>

#include <modules/python3/opaquetypes.h>
#include <modules/python3/polymorphictypehooks.h>

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
class TypedDataReaderFactoryWrapper {
public:
    TypedDataReaderFactoryWrapper() = delete;
    explicit explicitTypedDataReaderFactoryWrapper(DataReaderFactory& rf) : factory{&rf} {}
    TypedDataReaderFactoryWrapper(const TypedDataReaderFactoryWrapper&) = default;
    TypedDataReaderFactoryWrapper(TypedDataReaderFactoryWrapper&&) = default;
    TypedDataReaderFactoryWrapper& operator=(const TypedDataReaderFactoryWrapper&) = default;
    TypedDataReaderFactoryWrapper& operator=(TypedDataReaderFactoryWrapper&&) = default;

    std::vector<FileExtension> getExtensions() const { return factory->getExtensionsForType<T>(); }
    std::unique_ptr<DataReaderType<T>> getReader(
        const std::filesystem::path& filePathOrExtension) const {
        return factory->getReaderForTypeAndExtension<T>(filePathOrExtension);
    }

    std::unique_ptr<DataReaderType<T>> getReader(const FileExtension& ext) const {
        return factory->getReaderForTypeAndExtension<T>(ext);
    }

    std::unique_ptr<DataReaderType<T>> getReader(
        const FileExtension& ext, const std::filesystem::path& fallbackFilePathOrExtension) const {
        return factory->getReaderForTypeAndExtension<T>(ext, fallbackFilePathOrExtension);
    }

    bool hasReader(const std::filesystem::path& filePathOrExtension) const {
        return factory->hasReaderForTypeAndExtension<T>(filePathOrExtension);
    }

    bool hasReader(const FileExtension& ext) const {
        return factory->hasReaderForTypeAndExtension<T>(ext);
    }

    std::shared_ptr<T> read(const std::filesystem::path& filePath,
                            std::optional<FileExtension> ext = std::nullopt) {
        return factory->readDataForTypeAndExtension<T>(filePath, ext);
    }

private:
    DataReaderFactory* factory;
};

template <typename T>
void exposeFactoryReaderType(pybind11::module& m, pybind11::classh<DataReaderFactory>& r,
                             std::string_view type) {
    namespace py = pybind11;

    using TRF = TypedDataReaderFactoryWrapper<T>;
    py::classh<TRF>(m, fmt::format("{}ReaderFactory", type).c_str())
        .def("getExtensions", &TRF::getExtensions)
        .def("getReader",
             static_cast<std::unique_ptr<DataReaderType<T>> (TRF::*)(const std::filesystem::path&)
                             const>(&TRF::getReader),
             py::arg("path"))
        .def("getReader",
             static_cast<std::unique_ptr<DataReaderType<T>> (TRF::*)(const FileExtension&) const>(
                 &TRF::getReader),
             py::arg("fileExtension"))
        .def("getReader",
             static_cast<std::unique_ptr<DataReaderType<T>> (TRF::*)(
                 const FileExtension&, const std::filesystem::path&) const>(&TRF::getReader),
             py::arg("fileExtension"), py::arg("fallbackPath"))
        .def("hasReader",
             static_cast<bool (TRF::*)(const std::filesystem::path&) const>(&TRF::hasReader),
             py::arg("path"))
        .def("hasReader", static_cast<bool (TRF::*)(const FileExtension&) const>(&TRF::hasReader))
        .def("read", &TRF::read, py::arg("path"), py::arg("fileExtension") = std::nullopt);

    r.def_property_readonly(
        toLower(type).c_str(), [](DataReaderFactory& rf) { return TRF(rf); },
        fmt::format("The {} Reader Factory", type).c_str());
}

template <typename T>
void exposeReaderType(pybind11::module& m, std::string_view name) {
    pybind11::classh<DataReaderType<T>, DataReader, DataReaderTypeTrampoline<T>>(
        m, fmt::format("{}DataReader", name).c_str())
        .def("readData",
             pybind11::overload_cast<const std::filesystem::path&>(&DataReaderType<T>::readData))
        .def("readData", pybind11::overload_cast<const std::filesystem::path&, MetaDataOwner*>(
                             &DataReaderType<T>::readData));
}

void exposeDataReaders(pybind11::module& m) {
    pybind11::classh<DataReader>(m, "DataReader")
        .def("clone", &DataReader::clone)
        .def_property_readonly("extensions", &DataReader::getExtensions,
                               pybind11::return_value_policy::reference_internal)
        .def("addExtension", &DataReader::addExtension)
        .def("setOption", &DataReader::setOption)
        .def("getOption", &DataReader::getOption);

    auto fr = pybind11::classh<DataReaderFactory>(m, "DataReaderFactory");
    fr.def("create", static_cast<std::unique_ptr<DataReader> (DataReaderFactory::*)(
                         const FileExtension&) const>(&DataReaderFactory::create))
        .def(
            "create",
            static_cast<std::unique_ptr<DataReader> (DataReaderFactory::*)(std::string_view) const>(
                &DataReaderFactory::create))
        .def("hasKey", static_cast<bool (DataReaderFactory::*)(std::string_view) const>(
                           &DataReaderFactory::hasKey))
        .def("hasKey", static_cast<bool (DataReaderFactory::*)(const FileExtension&) const>(
                           &DataReaderFactory::hasKey));

    // No good way of dealing with template return types so we manually define one for each
    // known type. https://github.com/pybind/pybind11/issues/1667#issuecomment-454348004 If your
    // module exposes a new reader type it will have to bind getXXXReader.

    exposeReaderType<Layer>(m, "Layer");
    exposeFactoryReaderType<Layer>(m, fr, "Layer");

    exposeReaderType<Image>(m, "Image");
    exposeFactoryReaderType<Image>(m, fr, "Image");

    exposeReaderType<Mesh>(m, "Mesh");
    exposeFactoryReaderType<Mesh>(m, fr, "Mesh");

    exposeReaderType<Volume>(m, "Volume");
    exposeFactoryReaderType<Volume>(m, fr, "Volume");

    exposeReaderType<TransferFunction>(m, "TransferFunction");
    exposeFactoryReaderType<TransferFunction>(m, fr, "TransferFunction");

    exposeReaderType<IsoValueCollection>(m, "IsoValueCollection");
    exposeFactoryReaderType<IsoValueCollection>(m, fr, "IsoValueCollection");
}

}  // namespace inviwo
