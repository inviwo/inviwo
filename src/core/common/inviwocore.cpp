/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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
#include <inviwo/core/datastructures/datasequence.h>

// Cameras
#include <inviwo/core/datastructures/camera/camera.h>
#include <inviwo/core/datastructures/camera/orthographiccamera.h>
#include <inviwo/core/datastructures/camera/plotcamera.h>
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
#include <inviwo/core/io/transferfunctionitfreader.h>
#include <inviwo/core/io/transferfunctionitfwriter.h>
#include <inviwo/core/io/transferfunctionxmlreader.h>
#include <inviwo/core/io/transferfunctionxmlwriter.h>
#include <inviwo/core/io/transferfunctionlayerreader.h>
#include <inviwo/core/io/transferfunctionlayerwriter.h>
#include <inviwo/core/io/isovaluecollectioniivreader.h>
#include <inviwo/core/io/isovaluecollectioniivwriter.h>

// Others
#include <inviwo/core/processors/canvasprocessor.h>

// Ports
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/layerport.h>
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
#include <inviwo/core/interaction/trackball.h>
#include <inviwo/core/interaction/cameratrackball.h>
#include <inviwo/core/properties/directoryproperty.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/filepatternproperty.h>
#include <inviwo/core/properties/imageeditorproperty.h>
#include <inviwo/core/properties/isovalueproperty.h>
#include <inviwo/core/properties/isotfproperty.h>
#include <inviwo/core/properties/listproperty.h>
#include <inviwo/core/properties/marginproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/multifileproperty.h>
#include <inviwo/core/properties/optionproperty.h>
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
#include <inviwo/core/processors/sequenceprocessor.h>
#include <inviwo/core/processors/sequencecompositesink.h>
#include <inviwo/core/processors/sequencecompositesource.h>

#include <inviwo/core/util/stdextensions.h>

#include <inviwo/core/datastructures/representationutil.h>

namespace inviwo {

namespace {

enum class OptionRegEnumInt : int {};
enum class OptionRegEnumUInt : unsigned int {};

}  // namespace

InviwoCore::Observer::Observer(InviwoCore& core, InviwoApplication* app)
    : FileObserver(app), core_(core) {}
void InviwoCore::Observer::fileChanged(const std::filesystem::path& dir) {
    core_.scanDirForComposites(dir);
}

InviwoCore::InviwoCore(InviwoApplication* app)
    : InviwoModule(app, "Core")
    , compositeDirObserver_{*this, app}
    , tfLayerWriters_{app->getDataWriterFactory()}
    , tfLayerReaders_{app->getDataReaderFactory()} {

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
    registerMetaData(std::make_unique<PositionMetaData>());
    registerMetaData(std::make_unique<ProcessorMetaData>());
    registerMetaData(std::make_unique<ProcessorWidgetMetaData>());
    registerMetaData(std::make_unique<StdUnorderedMapMetaData<std::string, std::string>>());
    registerMetaData(std::make_unique<StdVectorMetaData<std::string>>());

    // Register Cameras
    registerCamera<PerspectiveCamera>(PerspectiveCamera::classIdentifier);
    registerCamera<OrthographicCamera>(OrthographicCamera::classIdentifier);
    registerCamera<PlotCamera>(PlotCamera::classIdentifier);
    registerCamera<SkewedPerspectiveCamera>(SkewedPerspectiveCamera::classIdentifier);

    // Register Data readers
    registerDataReader(std::make_unique<RawVolumeReader>());
    registerDataReader(std::make_unique<TransferFunctionITFReader>());
    registerDataReader(std::make_unique<TransferFunctionXMLReader>());
    registerDataReader(std::make_unique<IsoValueCollectionIIVReader>());

    // Register Data writers
    registerDataWriter(std::make_unique<TransferFunctionITFWriter>());
    registerDataWriter(std::make_unique<TransferFunctionXMLWriter>());
    registerDataWriter(std::make_unique<IsoValueCollectionIIVWriter>());

    // Register Ports
    registerPort<ImageInport>();
    registerPort<ImageMultiInport>();
    registerPort<ImageOutport>();

    registerProcessor<CompositeProcessor>();
    registerProcessor<CompositeSink<ImageInport, ImageOutport>>();
    registerProcessor<CompositeSource<ImageInport, ImageOutport>>();

    registerProcessor<SequenceProcessor>();
    registerProcessor<SequenceCompositeSink<ImageInport, DataOutport<DataSequence<Image>>>>();
    registerProcessor<SequenceCompositeSource<DataInport<DataSequence<Image>>, ImageOutport>>();

    registerDefaultsForDataType<Mesh>();
    registerDefaultsForDataType<Volume>();
    registerDefaultsForScalarDataType<VolumeSequence>();
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

    // Functor for registering defaults for datatypes
    util::for_each_type<types>{}([&]<typename T>() {
        registerDefaultsForScalarDataType<T>();
        registerDefaultsForScalarDataType<std::vector<T>>();
    });




    // Register PortInspectors
    registerPortInspector(PortTraits<ImageOutport>::classIdentifier(),
                          filesystem::getPath(PathType::PortInspectors, "/imageportinspector.inv"));
    registerPortInspector(PortTraits<LayerOutport>::classIdentifier(),
                          filesystem::getPath(PathType::PortInspectors, "/layerportinspector.inv"));
    registerPortInspector(
        PortTraits<VolumeOutport>::classIdentifier(),
        filesystem::getPath(PathType::PortInspectors, "/volumeportinspector.inv"));
    registerPortInspector(
        PortTraits<MeshOutport>::classIdentifier(),
        filesystem::getPath(PathType::PortInspectors, "/geometryportinspector.inv"));

    registerProperty<CompositeProperty>();
    registerProperty<BoolCompositeProperty>();
    registerProperty<BoolProperty>();
    registerProperty<ButtonGroupProperty>();
    registerProperty<ButtonProperty>();
    registerProperty<CameraProperty>();
    registerProperty<StringProperty>();
    registerProperty<Trackball>();
    registerProperty<CameraTrackball>();

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
    util::for_each_type<OrdinalTypes>{}([&]<typename T>() {
        registerProperty<OrdinalProperty<T>>();
        registerProperty<OrdinalRefProperty<T>>();
    });

    // Register MinMaxProperty widgets
    using ScalarTypes = std::tuple<float, double, int, glm::i64, size_t>;
    util::for_each_type<ScalarTypes>{}(
        [&]<typename T>() { registerProperty<MinMaxProperty<T>>(); });

    // Register option property widgets
    using OptionTypes = std::tuple<char, unsigned char, unsigned int, int, size_t, float, double,
                                   std::string, FileExtension>;
    util::for_each_type<OptionTypes>{}(
        [&]<typename T>() { registerProperty<OptionProperty<T>>(); });

    registerProperty<IsoValueProperty>();
    registerProperty<IsoTFProperty>();
    registerProperty<TransferFunctionProperty>();

    registerProperty<ListProperty>();

    registerProperty<MarginProperty>();
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

    using Mat2s = std::tuple<mat2, dmat2>;
    using Mat3s = std::tuple<mat3, dmat3>;
    using Mat4s = std::tuple<mat4, dmat4>;

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

    const auto ordinalLikeConverters = [&]<typename T, typename U>() {
        if constexpr (!std::is_same<T, U>::value) {
            registerPropertyConverter(
                std::make_unique<
                    OrdinalPropertyConverter<OrdinalProperty<T>, OrdinalProperty<U>>>());

            registerPropertyConverter(
                std::make_unique<
                    OrdinalPropertyConverter<OrdinalRefProperty<T>, OrdinalRefProperty<U>>>());
        }
        registerPropertyConverter(
            std::make_unique<
                OrdinalPropertyConverter<OrdinalRefProperty<T>, OrdinalProperty<U>>>());
        registerPropertyConverter(
            std::make_unique<
                OrdinalPropertyConverter<OrdinalProperty<T>, OrdinalRefProperty<U>>>());
    };

    util::for_each_type_pair<Scalars, Scalars>{}(ordinalLikeConverters);
    util::for_each_type_pair<Vec2s, Vec2s>{}(ordinalLikeConverters);
    util::for_each_type_pair<Vec3s, Vec3s>{}(ordinalLikeConverters);
    util::for_each_type_pair<Vec4s, Vec4s>{}(ordinalLikeConverters);

    util::for_each_type_pair<Mat2s, Mat2s>{}(ordinalLikeConverters);
    util::for_each_type_pair<Mat3s, Mat3s>{}(ordinalLikeConverters);
    util::for_each_type_pair<Mat4s, Mat4s>{}(ordinalLikeConverters);

    const auto scalarStringConverter = [&]<typename T>() {
        registerPropertyConverter(std::make_unique<ScalarToStringConverter<OrdinalProperty<T>>>());
        registerPropertyConverter(
            std::make_unique<ScalarToStringConverter<OrdinalRefProperty<T>>>());
    };
    const auto vectorStringConverter = [&]<typename T>() {
        registerPropertyConverter(std::make_unique<VectorToStringConverter<OrdinalProperty<T>>>());
        registerPropertyConverter(
            std::make_unique<VectorToStringConverter<OrdinalRefProperty<T>>>());
    };
    util::for_each_type<Scalars>{}(scalarStringConverter);
    util::for_each_type<Vec2s>{}(vectorStringConverter);
    util::for_each_type<Vec3s>{}(vectorStringConverter);
    util::for_each_type<Vec4s>{}(vectorStringConverter);

    const auto optionStringConverter = [&]<typename T>() {
        registerPropertyConverter(std::make_unique<OptionToStringConverter<OptionProperty<T>>>());
    };

    const auto optionIntConverter = [&]<typename T>() {
        registerPropertyConverter(std::make_unique<OptionToIntConverter<OptionProperty<T>>>());
    };

    const auto intOptionConverter = [&]<typename T>() {
        registerPropertyConverter(std::make_unique<IntToOptionConverter<OptionProperty<T>>>());
    };

    util::for_each_type<OptionTypes>{}(optionStringConverter);
    util::for_each_type<OptionTypes>{}(optionIntConverter);
    util::for_each_type<OptionTypes>{}(intOptionConverter);

    using OptionEnumTypes = std::tuple<OptionRegEnumInt, OptionRegEnumUInt>;
    util::for_each_type<OptionEnumTypes>{}(optionStringConverter);
    util::for_each_type<OptionEnumTypes>{}(optionIntConverter);
    util::for_each_type<OptionEnumTypes>{}(intOptionConverter);

    // Observe composite processors
    auto userCompositeDir = filesystem::getPath(PathType::Settings, "/composites");
    scanDirForComposites(userCompositeDir);
    compositeDirObserver_.startFileObservation(userCompositeDir);

    auto coreCompositeDir = filesystem::getPath(PathType::Workspaces, "/composites");
    scanDirForComposites(coreCompositeDir);
    compositeDirObserver_.startFileObservation(coreCompositeDir);

    // Register Settings
    // Do this after the property registration since the settings use properties.
    registerSettings(std::make_unique<LinkSettings>("Link Settings", app_->getPropertyFactory()));
    registerSettings(std::make_unique<UnitSettings>());
}

const std::filesystem::path& InviwoCore::getPath() const { return filesystem::findBasePath(); }

void InviwoCore::scanDirForComposites(const std::filesystem::path& dir) {
    if (!std::filesystem::is_directory(dir)) {
        return;
    }

    for (auto&& item : std::filesystem::recursive_directory_iterator{dir}) {
        if (item.is_regular_file() && item.path().extension() == ".inv") {
            if (!addedCompositeFiles_.contains(item)) {
                registerCompositeProcessor(item);
                addedCompositeFiles_.insert(item);
            }
        }
    }
}

}  // namespace inviwo
