/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2021 Inviwo Foundation
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

#include <inviwo/core/common/inviwocore.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/foreacharg.h>

#include <warn/push>
#include <warn/ignore/unused-function>
#include <inviwo/core/properties/optionproperty.h>
#include <warn/pop>

// Cameras
#include <inviwo/core/datastructures/camera/camera.h>
#include <inviwo/core/datastructures/camera/orthographiccamera.h>
#include <inviwo/core/datastructures/camera/perspectivecamera.h>
#include <inviwo/core/datastructures/camera/skewedperspectivecamera.h>

// Meta Data
#include <inviwo/core/metadata/metadata.h>
#include <inviwo/core/metadata/containermetadata.h>
#include <inviwo/core/metadata/processormetadata.h>
#include <inviwo/core/metadata/processorwidgetmetadata.h>

// Utilizes
#include <inviwo/core/util/settings/linksettings.h>
#include <inviwo/core/util/settings/unitsettings.h>

// Io
#include <inviwo/core/io/rawvolumereader.h>

// Others
#include <inviwo/core/processors/canvasprocessor.h>

// Ports
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/ports/bufferport.h>
#include <inviwo/core/datastructures/light/baselightsource.h>

// PortInspectors
#include <inviwo/core/ports/portinspector.h>

// Properties
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttongroupproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/directoryproperty.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/filepatternproperty.h>
#include <inviwo/core/properties/imageeditorproperty.h>
#include <inviwo/core/properties/isovalueproperty.h>
#include <inviwo/core/properties/isotfproperty.h>
#include <inviwo/core/properties/listproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/multifileproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/ordinalrefproperty.h>
#include <inviwo/core/properties/planeproperty.h>
#include <inviwo/core/properties/positionproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/stringsproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/properties/advancedmaterialproperty.h>
#include <inviwo/core/properties/raycastingproperty.h>
#include <inviwo/core/properties/selectioncolorproperty.h>
#include <inviwo/core/properties/simplelightingproperty.h>
#include <inviwo/core/properties/simpleraycastingproperty.h>
#include <inviwo/core/properties/volumeindicatorproperty.h>

#include <inviwo/core/properties/propertyconvertermanager.h>
#include <inviwo/core/properties/propertyconverter.h>

// Processors
#include <inviwo/core/processors/compositeprocessor.h>
#include <inviwo/core/processors/compositesink.h>
#include <inviwo/core/processors/compositesource.h>

#include <inviwo/core/util/stdextensions.h>

#include <inviwo/core/datastructures/representationutil.h>

namespace inviwo {

namespace {

struct OrdinalReghelper {
    template <typename T>
    auto operator()(InviwoModule& qm) {
        using PropertyType = OrdinalProperty<T>;
        qm.registerProperty<PropertyType>();
    }
};

struct MinMaxReghelper {
    template <typename T>
    auto operator()(InviwoModule& qm) {
        using PropertyType = MinMaxProperty<T>;
        qm.registerProperty<PropertyType>();
    }
};

struct OptionReghelper {
    template <typename T>
    auto operator()(InviwoModule& qm) {
        using PropertyType = OptionProperty<T>;
        qm.registerProperty<PropertyType>();
    }
};

// Functors for registration of property converters
// Can't use a regular lambda since we need the explicit template arguments.
// We take a std::function to register the created converter since the registration function is
// protected in the inviwo module
struct ConverterRegFunctor {
    template <typename T, typename U>
    auto operator()(InviwoModule& m) {
        if constexpr (!std::is_same<T, U>::value) {
            m.registerPropertyConverter(
                std::make_unique<
                    OrdinalPropertyConverter<OrdinalProperty<T>, OrdinalProperty<U>>>());

            m.registerPropertyConverter(
                std::make_unique<
                    OrdinalPropertyConverter<OrdinalRefProperty<T>, OrdinalRefProperty<U>>>());
        }
        m.registerPropertyConverter(
            std::make_unique<
                OrdinalPropertyConverter<OrdinalRefProperty<T>, OrdinalProperty<U>>>());
        m.registerPropertyConverter(
            std::make_unique<
                OrdinalPropertyConverter<OrdinalProperty<T>, OrdinalRefProperty<U>>>());
    }
};
struct ScalarStringConverterRegFunctor {
    template <typename T>
    auto operator()(InviwoModule& m) {
        m.registerPropertyConverter(
            std::make_unique<ScalarToStringConverter<OrdinalProperty<T>>>());
        m.registerPropertyConverter(
            std::make_unique<ScalarToStringConverter<OrdinalRefProperty<T>>>());
    }
};
struct VectorStringConverterRegFunctor {
    template <typename T>
    auto operator()(InviwoModule& m) {
        m.registerPropertyConverter(
            std::make_unique<VectorToStringConverter<OrdinalProperty<T>>>());
        m.registerPropertyConverter(
            std::make_unique<VectorToStringConverter<OrdinalRefProperty<T>>>());
    }
};

enum class OptionRegEnumInt : int {};
enum class OptionRegEnumUInt : unsigned int {};

struct OptionStringConverterRegFunctor {
    template <typename T>
    auto operator()(InviwoModule& m) {
        m.registerPropertyConverter(std::make_unique<OptionToStringConverter<OptionProperty<T>>>());
    }
};

struct OptionIntConverterRegFunctor {
    template <typename T>
    auto operator()(InviwoModule& m) {
        m.registerPropertyConverter(std::make_unique<OptionToIntConverter<OptionProperty<T>>>());
    }
};

struct IntOptionConverterRegFunctor {
    template <typename T>
    auto operator()(InviwoModule& m) {
        m.registerPropertyConverter(std::make_unique<IntToOptionConverter<OptionProperty<T>>>());
    }
};

// Functor for registering defaults for datatypes
struct DataTypeRegFunctor {
    template <typename T>
    auto operator()(InviwoModule& m) {
        m.registerDefaultsForDataType<T>();
        m.registerDefaultsForDataType<std::vector<T>>();
    }
};

}  // namespace

template class OptionProperty<OptionRegEnumInt>;
template class OptionProperty<OptionRegEnumUInt>;

InviwoCore::Observer::Observer(InviwoCore& core, InviwoApplication* app)
    : FileObserver(app), core_(core) {}
void InviwoCore::Observer::fileChanged(const std::string& dir) { core_.scanDirForComposites(dir); }

InviwoCore::InviwoCore(InviwoApplication* app)
    : InviwoModule(app, "Core"), compositeDirObserver_{*this, app} {

    util::registerCoreRepresentations(*this);

    // Register MetaData
    registerMetaData(std::make_unique<BoolMetaData>());
    registerMetaData(std::make_unique<IntMetaData>());
    registerMetaData(std::make_unique<FloatMetaData>());
    registerMetaData(std::make_unique<DoubleMetaData>());
    registerMetaData(std::make_unique<StringMetaData>());
    registerMetaData(std::make_unique<SizeMetaData>());
    registerMetaData(std::make_unique<FloatVec2MetaData>());
    registerMetaData(std::make_unique<FloatVec3MetaData>());
    registerMetaData(std::make_unique<FloatVec4MetaData>());
    registerMetaData(std::make_unique<DoubleVec2MetaData>());
    registerMetaData(std::make_unique<DoubleVec3MetaData>());
    registerMetaData(std::make_unique<DoubleVec4MetaData>());
    registerMetaData(std::make_unique<IntVec2MetaData>());
    registerMetaData(std::make_unique<IntVec3MetaData>());
    registerMetaData(std::make_unique<IntVec4MetaData>());
    registerMetaData(std::make_unique<UIntVec2MetaData>());
    registerMetaData(std::make_unique<UIntVec3MetaData>());
    registerMetaData(std::make_unique<UIntVec4MetaData>());
    registerMetaData(std::make_unique<FloatMat2MetaData>());
    registerMetaData(std::make_unique<FloatMat3MetaData>());
    registerMetaData(std::make_unique<FloatMat4MetaData>());
    registerMetaData(std::make_unique<DoubleMat2MetaData>());
    registerMetaData(std::make_unique<DoubleMat4MetaData>());
    registerMetaData(std::make_unique<DoubleMat3MetaData>());
    registerMetaData(std::make_unique<Size2MetaData>());
    registerMetaData(std::make_unique<Size3MetaData>());
    registerMetaData(std::make_unique<Size4MetaData>());
    registerMetaData(std::make_unique<VectorMetaData<2, float>>());
    registerMetaData(std::make_unique<VectorMetaData<3, float>>());
    registerMetaData(std::make_unique<VectorMetaData<4, float>>());
    registerMetaData(std::make_unique<VectorMetaData<2, double>>());
    registerMetaData(std::make_unique<VectorMetaData<3, double>>());
    registerMetaData(std::make_unique<VectorMetaData<4, double>>());
    registerMetaData(std::make_unique<VectorMetaData<2, int>>());
    registerMetaData(std::make_unique<VectorMetaData<3, int>>());
    registerMetaData(std::make_unique<VectorMetaData<4, int>>());
    registerMetaData(std::make_unique<VectorMetaData<2, unsigned int>>());
    registerMetaData(std::make_unique<VectorMetaData<3, unsigned int>>());
    registerMetaData(std::make_unique<VectorMetaData<4, unsigned int>>());
    registerMetaData(std::make_unique<MatrixMetaData<2, float>>());
    registerMetaData(std::make_unique<MatrixMetaData<3, float>>());
    registerMetaData(std::make_unique<MatrixMetaData<4, float>>());
    registerMetaData(std::make_unique<MatrixMetaData<2, double>>());
    registerMetaData(std::make_unique<MatrixMetaData<3, double>>());
    registerMetaData(std::make_unique<MatrixMetaData<4, double>>());
    registerMetaData(std::make_unique<PositionMetaData>());
    registerMetaData(std::make_unique<ProcessorMetaData>());
    registerMetaData(std::make_unique<ProcessorWidgetMetaData>());
    registerMetaData(std::make_unique<StdUnorderedMapMetaData<std::string, std::string>>());
    registerMetaData(std::make_unique<StdVectorMetaData<std::string>>());

    // Register Cameras
    registerCamera<PerspectiveCamera>(PerspectiveCamera::classIdentifier);
    registerCamera<OrthographicCamera>(OrthographicCamera::classIdentifier);
    registerCamera<SkewedPerspectiveCamera>(SkewedPerspectiveCamera::classIdentifier);

    // Register Data readers
    registerDataReader(std::make_unique<RawVolumeReader>());
    // Register Data writers

    // Register Ports
    registerPort<ImageInport>();
    registerPort<ImageMultiInport>();
    registerPort<ImageOutport>();
    registerProcessor<CompositeSource<ImageInport, ImageOutport>>();
    registerProcessor<CompositeSink<ImageInport, ImageOutport>>();
    registerProcessor<CompositeProcessor>();

    registerDefaultsForDataType<Mesh>();
    registerDefaultsForDataType<Volume>();
    registerDefaultsForDataType<VolumeSequence>();
    registerDefaultsForDataType<BufferBase>();
    registerDefaultsForDataType<LightSource>();

    // Register Defaults for Datatypes
    // clang-format off
    using types = std::tuple<
        float, vec2, vec3, vec4,
        double, dvec2, dvec3, dvec4,
        int32_t, ivec2, ivec3, ivec4,
        uint32_t, uvec2, uvec3, uvec4,
        std::int64_t, i64vec2, i64vec3, i64vec4,
        std::uint64_t, u64vec2, u64vec3, u64vec4>;
    // clang-format on
    util::for_each_type<types>{}(DataTypeRegFunctor{}, *this);

    // Register PortInspectors
    registerPortInspector(PortTraits<ImageOutport>::classIdentifier(),
                          app->getPath(PathType::PortInspectors, "/imageportinspector.inv"));
    registerPortInspector(PortTraits<VolumeOutport>::classIdentifier(),
                          app->getPath(PathType::PortInspectors, "/volumeportinspector.inv"));
    registerPortInspector(PortTraits<MeshOutport>::classIdentifier(),
                          app->getPath(PathType::PortInspectors, "/geometryportinspector.inv"));

    registerProperty<CompositeProperty>();
    registerProperty<BoolCompositeProperty>();
    registerProperty<BoolProperty>();
    registerProperty<ButtonGroupProperty>();
    registerProperty<ButtonProperty>();
    registerProperty<CameraProperty>();
    registerProperty<StringProperty>();

    registerProperty<StringsProperty<1>>();
    registerProperty<StringsProperty<2>>();
    registerProperty<StringsProperty<3>>();
    registerProperty<StringsProperty<4>>();

    registerProperty<ImageEditorProperty>();

    registerProperty<FileProperty>();
    registerProperty<MultiFileProperty>();
    registerProperty<FilePatternProperty>();
    registerProperty<DirectoryProperty>();

    // Register ordinal property
    using OrdinalTypes =
        std::tuple<float, vec2, vec3, vec4, mat2, mat3, mat4, double, dvec2, dvec3, dvec4, dmat2,
                   dmat3, dmat4, int, ivec2, ivec3, ivec4, glm::i64, unsigned int, uvec2, uvec3,
                   uvec4, size_t, size2_t, size3_t, size4_t, glm::fquat, glm::dquat>;
    util::for_each_type<OrdinalTypes>{}(OrdinalReghelper{}, *this);

    // Register MinMaxProperty widgets
    using ScalarTypes = std::tuple<float, double, int, glm::i64, size_t>;
    util::for_each_type<ScalarTypes>{}(MinMaxReghelper{}, *this);

    // Register option property widgets
    using OptionTypes = std::tuple<char, unsigned char, unsigned int, int, size_t, float, double,
                                   std::string, FileExtension>;
    util::for_each_type<OptionTypes>{}(OptionReghelper{}, *this);

    registerProperty<IsoValueProperty>();
    registerProperty<IsoTFProperty>();
    registerProperty<TransferFunctionProperty>();

    registerProperty<ListProperty>();

    registerProperty<PlaneProperty>();
    registerProperty<PositionProperty>();
    registerProperty<AdvancedMaterialProperty>();
    registerProperty<RaycastingProperty>();
    registerProperty<SelectionColorProperty>();
    registerProperty<SimpleLightingProperty>();
    registerProperty<SimpleRaycastingProperty>();
    registerProperty<VolumeIndicatorProperty>();

    using Scalars = std::tuple<float, double, int, glm::i64, size_t>;
    using Vec2s = std::tuple<vec2, dvec2, ivec2, size2_t>;
    using Vec3s = std::tuple<vec3, dvec3, ivec3, size3_t>;
    using Vec4s = std::tuple<vec4, dvec4, ivec4, size4_t>;

    registerPropertyConverter(std::make_unique<FileToStringConverter>());
    registerPropertyConverter(std::make_unique<StringToFileConverter>());
    registerPropertyConverter(std::make_unique<DirectoryToStringConverter>());
    registerPropertyConverter(std::make_unique<StringToDirectoryConverter>());

    registerPropertyConverter(std::make_unique<TransferfunctionToIsoTFConverter>());
    registerPropertyConverter(std::make_unique<IsoTFToTransferfunctionConverter>());
    registerPropertyConverter(std::make_unique<IsovalueToIsoTFConverter>());
    registerPropertyConverter(std::make_unique<IsoTFToIsovalueConverter>());

    // for_each_type_pair will call the functor with all permutation of types, and supplied
    // arguments like: ConverterRegFunctor<float, float>(*this), ConverterRegFunctor<float,
    // double>(*this), ...
    util::for_each_type_pair<Scalars, Scalars>{}(ConverterRegFunctor{}, *this);
    util::for_each_type_pair<Vec2s, Vec2s>{}(ConverterRegFunctor{}, *this);
    util::for_each_type_pair<Vec3s, Vec3s>{}(ConverterRegFunctor{}, *this);
    util::for_each_type_pair<Vec4s, Vec4s>{}(ConverterRegFunctor{}, *this);

    util::for_each_type<Scalars>{}(ScalarStringConverterRegFunctor{}, *this);
    util::for_each_type<Vec2s>{}(VectorStringConverterRegFunctor{}, *this);
    util::for_each_type<Vec3s>{}(VectorStringConverterRegFunctor{}, *this);
    util::for_each_type<Vec4s>{}(VectorStringConverterRegFunctor{}, *this);

    util::for_each_type<OptionTypes>{}(OptionStringConverterRegFunctor{}, *this);
    util::for_each_type<OptionTypes>{}(OptionIntConverterRegFunctor{}, *this);
    util::for_each_type<OptionTypes>{}(IntOptionConverterRegFunctor{}, *this);

    using OptionEnumTypes = std::tuple<OptionRegEnumInt, OptionRegEnumUInt>;
    util::for_each_type<OptionEnumTypes>{}(OptionStringConverterRegFunctor{}, *this);
    util::for_each_type<OptionEnumTypes>{}(OptionIntConverterRegFunctor{}, *this);
    util::for_each_type<OptionEnumTypes>{}(IntOptionConverterRegFunctor{}, *this);

    // Observe composite processors
    auto userCompositeDir = app_->getPath(PathType::Settings, "/composites");
    scanDirForComposites(userCompositeDir);
    compositeDirObserver_.startFileObservation(userCompositeDir);

    auto coreCompositeDir = app_->getPath(PathType::Workspaces, "/composites");
    scanDirForComposites(coreCompositeDir);
    compositeDirObserver_.startFileObservation(coreCompositeDir);

    // Register Settings
    // Do this after the property registration since the settings use properties.
    registerSettings(std::make_unique<LinkSettings>("Link Settings", app_->getPropertyFactory()));
    registerSettings(std::make_unique<UnitSettings>());
}

std::string InviwoCore::getPath() const { return filesystem::findBasePath(); }

void InviwoCore::scanDirForComposites(const std::string& dir) {
    for (auto&& file :
         filesystem::getDirectoryContentsRecursively(dir, filesystem::ListMode::Files)) {
        if (filesystem::getFileExtension(file) == "inv") {
            if (addedCompositeFiles_.count(file) == 0) {
                registerCompositeProcessor(file);
                addedCompositeFiles_.insert(file);
            }
        }
    }
}

}  // namespace inviwo
