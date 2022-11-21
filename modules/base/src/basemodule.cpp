/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2022 Inviwo Foundation
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

#include <modules/base/basemodule.h>

#include <inviwo/core/common/inviwomodule.h>                            // for InviwoModule
#include <inviwo/core/datastructures/buffer/buffer.h>                   // for Buffer
#include <inviwo/core/datastructures/buffer/bufferram.h>                // for BufferRAMPre...
#include <inviwo/core/datastructures/geometry/mesh.h>                   // for Mesh
#include <inviwo/core/datastructures/representationconverter.h>         // for Representati...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for Representati...
#include <inviwo/core/datastructures/volume/volume.h>                   // for Volume, Data...
#include <inviwo/core/io/datareader.h>                                  // for DataReader
#include <inviwo/core/io/datawriter.h>                                  // for DataWriter
#include <inviwo/core/io/serialization/ticpp.h>                         // for TxElement, Node
#include <inviwo/core/io/serialization/versionconverter.h>              // for Kind, Identi...
#include <inviwo/core/ports/datainport.h>                               // for MultiDataInport
#include <inviwo/core/ports/imageport.h>                                // for ImageMultiIn...
#include <inviwo/core/ports/meshport.h>                                 // for MeshOutport
#include <inviwo/core/ports/outportiterable.h>                          // for OutportIterable
#include <inviwo/core/ports/volumeport.h>                               // for VolumeOutport
#include <inviwo/core/properties/optionproperty.h>                      // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>                     // for OrdinalProperty
#include <inviwo/core/rendering/datavisualizer.h>                       // for DataVisualizer
#include <inviwo/core/util/foreacharg.h>                                // for for_each_type
#include <inviwo/core/util/glmvec.h>                                    // for vec3, vec4
#include <inviwo/core/util/staticstring.h>                              // for operator+
#include <inviwo/core/util/stringconversion.h>                          // for dotSeperated...
#include <modules/base/datavisualizer/imageinformationvisualizer.h>     // for ImageInforma...
#include <modules/base/datavisualizer/meshinformationvisualizer.h>      // for MeshInformat...
#include <modules/base/datavisualizer/volumeinformationvisualizer.h>    // for VolumeInform...
// Io
#include <modules/base/io/binarystlwriter.h>          // for BinarySTLWriter
#include <modules/base/io/datvolumesequencereader.h>  // for DatVolumeSeq...
#include <modules/base/io/datvolumewriter.h>          // for DatVolumeWriter
#include <modules/base/io/ivfsequencevolumereader.h>  // for IvfSequenceV...
#include <modules/base/io/ivfvolumereader.h>          // for IvfVolumeReader
#include <modules/base/io/ivfvolumewriter.h>          // for IvfVolumeWriter
#include <modules/base/io/stlwriter.h>                // for StlWriter
#include <modules/base/io/wavefrontwriter.h>          // for WaveFrontWriter
// Processors
#include <modules/base/processors/buffertomeshprocessor.h>                   // for BufferToMesh...
#include <modules/base/processors/camerafrustum.h>                           // for CameraFrustum
#include <modules/base/processors/convexhull2dprocessor.h>                   // for ConvexHull2D...
#include <modules/base/processors/cubeproxygeometryprocessor.h>              // for CubeProxyGeo...
#include <modules/base/processors/diffuselightsourceprocessor.h>             // for DiffuseLight...
#include <modules/base/processors/directionallightsourceprocessor.h>         // for DirectionalL...
#include <modules/base/processors/distancetransformram.h>                    // for DistanceTran...
#include <modules/base/processors/gridplanes.h>                              // for GridPlanes
#include <modules/base/processors/heightfieldmapper.h>                       // for HeightFieldM...
#include <modules/base/processors/imagecontourprocessor.h>                   // for ImageContour...
#include <modules/base/processors/imageexport.h>                             // for ImageExport
#include <modules/base/processors/imageinformation.h>                        // for ImageInforma...
#include <modules/base/processors/imagesequenceelementselectorprocessor.h>   // for ImageSequenc...
#include <modules/base/processors/imagesnapshot.h>                           // for ImageSnapshot
#include <modules/base/processors/imagesource.h>                             // for ImageSource
#include <modules/base/processors/imagesourceseries.h>                       // for ImageSourceS...
#include <modules/base/processors/imagestackvolumesource.h>                  // for ImageStackVo...
#include <modules/base/processors/imagetospatialsampler.h>                   // for ImageToSpati...
#include <modules/base/processors/inputselector.h>                           // for InputSelector
#include <modules/base/processors/layerdistancetransformram.h>               // for LayerDistanc...
#include <modules/base/processors/meshclipping.h>                            // for MeshClipping
#include <modules/base/processors/meshcolorfromnormals.h>                    // for MeshColorFro...
#include <modules/base/processors/meshconverterprocessor.h>                  // for MeshConverte...
#include <modules/base/processors/meshcreator.h>                             // for MeshCreator
#include <modules/base/processors/meshexport.h>                              // for MeshExport
#include <modules/base/processors/meshinformation.h>                         // for MeshInformation
#include <modules/base/processors/meshmapping.h>                             // for MeshMapping
#include <modules/base/processors/meshplaneclipping.h>                       // for MeshPlaneCli...
#include <modules/base/processors/meshsequenceelementselectorprocessor.h>    // for MeshSequence...
#include <modules/base/processors/meshsource.h>                              // for MeshSource
#include <modules/base/processors/noiseprocessor.h>                          // for NoiseProcessor
#include <modules/base/processors/noisevolumeprocessor.h>                    // for NoiseVolumeP...
#include <modules/base/processors/ordinalpropertyanimator.h>                 // for OrdinalPrope...
#include <modules/base/processors/orientationindicator.h>                    // for OrientationI...
#include <modules/base/processors/pixeltobufferprocessor.h>                  // for PixelToBuffe...
#include <modules/base/processors/pixelvalue.h>                              // for PixelValue
#include <modules/base/processors/pointgenerationprocessor.h>                // for Point3DGener...
#include <modules/base/processors/pointlightsourceprocessor.h>               // for PointLightSo...
#include <modules/base/processors/randommeshgenerator.h>                     // for RandomMeshGe...
#include <modules/base/processors/randomspheregenerator.h>                   // for RandomSphere...
#include <modules/base/processors/singlevoxel.h>                             // for SingleVoxel
#include <modules/base/processors/spotlightsourceprocessor.h>                // for SpotLightSou...
#include <modules/base/processors/stereocamerasyncer.h>                      // for StereoCamera...
#include <modules/base/processors/surfaceextractionprocessor.h>              // for SurfaceExtra...
#include <modules/base/processors/tfselector.h>                              // for TFSelector
#include <modules/base/processors/transform.h>                               // for Transform
#include <modules/base/processors/trianglestowireframe.h>                    // for TrianglesToW...
#include <modules/base/processors/vectortobuffer.h>                          // for VectorToBuffer
#include <modules/base/processors/volumebasistransformer.h>                  // for BasisTransform
#include <modules/base/processors/volumeboundaryplanes.h>                    // for VolumeBounda...
#include <modules/base/processors/volumeboundingbox.h>                       // for VolumeBoundi...
#include <modules/base/processors/volumeconverter.h>                         // for VolumeConverter
#include <modules/base/processors/volumecreator.h>                           // for VolumeCreator
#include <modules/base/processors/volumecurlcpuprocessor.h>                  // for VolumeCurlCP...
#include <modules/base/processors/volumedivergencecpuprocessor.h>            // for VolumeDiverg...
#include <modules/base/processors/volumeexport.h>                            // for VolumeExport
#include <modules/base/processors/volumegradientcpuprocessor.h>              // for VolumeGradie...
#include <modules/base/processors/volumeinformation.h>                       // for VolumeInform...
#include <modules/base/processors/volumelaplacianprocessor.h>                // for VolumeLaplac...
#include <modules/base/processors/volumesequenceelementselectorprocessor.h>  // for VolumeSequen...
#include <modules/base/processors/volumesequencesingletimestepsampler.h>     // for VolumeSequen...
#include <modules/base/processors/volumesequencesource.h>                    // for VolumeSequen...
#include <modules/base/processors/volumesequencetospatial4dsampler.h>        // for VolumeSequen...
#include <modules/base/processors/volumeshifter.h>                           // for VolumeShifter
#include <modules/base/processors/volumesliceextractor.h>                    // for VolumeSliceE...
#include <modules/base/processors/volumesource.h>                            // for VolumeSource
#include <modules/base/processors/volumesubsample.h>                         // for VolumeSubsample
#include <modules/base/processors/volumesubset.h>                            // for VolumeSubset
#include <modules/base/processors/volumetospatialsampler.h>                  // for VolumeToSpat...
#include <modules/base/processors/worldtransformdeprecated.h>                // for WorldTransfo...
// Properties
#include <modules/base/properties/basisproperty.h>              // for BasisProperty
#include <modules/base/properties/bufferinformationproperty.h>  // for BufferInform...
#include <modules/base/properties/datarangeproperty.h>          // for DataRangePro...
#include <modules/base/properties/gaussianproperty.h>           // for Gaussian1DPr...
#include <modules/base/properties/imageinformationproperty.h>   // for ImageInforma...
#include <modules/base/properties/layerinformationproperty.h>   // for LayerInforma...
#include <modules/base/properties/meshinformationproperty.h>    // for MeshInformat...
#include <modules/base/properties/sequencetimerproperty.h>      // for SequenceTime...
#include <modules/base/properties/transformlistproperty.h>      // for CustomTransf...
#include <modules/base/properties/volumeinformationproperty.h>  // for VolumeInform...

#include <functional>        // for __base, func...
#include <initializer_list>  // for initializer_...
#include <string>            // for string, oper...
#include <tuple>             // for make_tuple, get
#include <unordered_map>     // for unordered_map
#include <unordered_set>     // for unordered_set
#include <vector>            // for vector

#include <fmt/core.h>                    // for basic_string...
#include <fmt/format.h>                  // for formatbuf<>:...
#include <glm/common.hpp>                // for clamp
#include <glm/ext/scalar_constants.hpp>  // for pi
#include <glm/ext/vector_uint3.hpp>      // for uvec3
#include <glm/gtx/io.hpp>                // for operator<<
#include <glm/mat4x4.hpp>                // for operator*
#include <glm/vec2.hpp>                  // for operator*
#include <glm/vec3.hpp>                  // for operator*
#include <glm/vec4.hpp>                  // for operator*
#include <glm/vector_relational.hpp>     // for greaterThanE...

namespace inviwo {
class InviwoApplication;

using BasisTransformMesh = BasisTransform<Mesh>;
using BasisTransformVolume = BasisTransform<Volume>;

using TransformMesh = Transform<Mesh>;
using TransformVolume = Transform<Volume>;

using WorldTransformMeshDeprecated = WorldTransformDeprecated<Mesh>;
using WorldTransformVolumeDeprecated = WorldTransformDeprecated<Volume>;

BaseModule::BaseModule(InviwoApplication* app) : InviwoModule(app, "Base") {
    registerProcessor<ConvexHull2DProcessor>();
    registerProcessor<CubeProxyGeometry>();
    registerProcessor<DiffuseLightSourceProcessor>();
    registerProcessor<DirectionalLightSourceProcessor>();
    registerProcessor<DistanceTransformRAM>();
    registerProcessor<GridPlanes>();
    registerProcessor<MeshSource>();
    registerProcessor<HeightFieldMapper>();
    registerProcessor<ImageExport>();
    registerProcessor<ImageInformation>();
    registerProcessor<ImageSnapshot>();
    registerProcessor<ImageSource>();
    registerProcessor<ImageSourceSeries>();
    registerProcessor<ImageStackVolumeSource>();
    registerProcessor<LayerDistanceTransformRAM>();
    registerProcessor<MeshClipping>();
    registerProcessor<MeshColorFromNormals>();
    registerProcessor<MeshCreator>();
    registerProcessor<MeshInformation>();
    registerProcessor<MeshMapping>();
    registerProcessor<MeshPlaneClipping>();
    registerProcessor<NoiseProcessor>();
    registerProcessor<PixelToBufferProcessor>();
    registerProcessor<Point3DGenerationProcessor>();
    registerProcessor<PointLightSourceProcessor>();
    registerProcessor<OrdinalPropertyAnimator>();
    registerProcessor<SpotLightSourceProcessor>();
    registerProcessor<SurfaceExtraction>();
    registerProcessor<VolumeBoundaryPlanes>();
    registerProcessor<VolumeSource>();
    registerProcessor<VolumeExport>();
    registerProcessor<BasisTransformMesh>();
    registerProcessor<BasisTransformVolume>();
    registerProcessor<TrianglesToWireframe>();
    registerProcessor<TransformMesh>();
    registerProcessor<TransformVolume>();
    registerProcessor<VectorToBuffer<unsigned int>>();
    registerProcessor<VectorToBuffer<float>>();
    registerProcessor<VectorToBuffer<vec2>>();
    registerProcessor<VectorToBuffer<vec3>>();
    registerProcessor<VectorToBuffer<vec4>>();
    registerProcessor<VolumeConverter>();
    registerProcessor<WorldTransformMeshDeprecated>();
    registerProcessor<WorldTransformVolumeDeprecated>();
    registerProcessor<VolumeSliceExtractor>();
    registerProcessor<VolumeSubsample>();
    registerProcessor<VolumeSubset>();
    registerProcessor<ImageContourProcessor>();
    registerProcessor<VolumeSequenceSource>();
    registerProcessor<VolumeSequenceElementSelectorProcessor>();
    registerProcessor<ImageSequenceElementSelectorProcessor>();
    registerProcessor<MeshSequenceElementSelectorProcessor>();

    registerProcessor<VolumeBoundingBox>();
    registerProcessor<SingleVoxel>();
    registerProcessor<StereoCameraSyncer>();

    registerProcessor<VolumeToSpatialSampler>();
    registerProcessor<OrientationIndicator>();
    registerProcessor<PixelValue>();
    registerProcessor<VolumeSequenceToSpatial4DSampler>();
    registerProcessor<VolumeGradientCPUProcessor>();
    registerProcessor<VolumeCurlCPUProcessor>();
    registerProcessor<VolumeDivergenceCPUProcessor>();
    registerProcessor<VolumeLaplacianProcessor>();
    registerProcessor<MeshExport>();
    registerProcessor<RandomMeshGenerator>();
    registerProcessor<RandomSphereGenerator>();
    registerProcessor<NoiseVolumeProcessor>();
    registerProcessor<BufferToMeshProcessor>();
    registerProcessor<ImageToSpatialSampler>();
    registerProcessor<CameraFrustum>();
    registerProcessor<VolumeSequenceSingleTimestepSamplerProcessor>();
    registerProcessor<VolumeCreator>();
    registerProcessor<MeshConverterProcessor>();
    registerProcessor<VolumeInformation>();
    registerProcessor<TFSelector>();
    registerProcessor<VolumeShifter>();

    // input selectors
    registerProcessor<InputSelector<MultiDataInport<Volume>, VolumeOutport>>();
    registerProcessor<InputSelector<MultiDataInport<Mesh>, MeshOutport>>();
    registerProcessor<InputSelector<ImageMultiInport, ImageOutport>>();

    registerProperty<BasisProperty>();
    registerProperty<BufferInformationProperty>();
    registerProperty<DataRangeProperty>();
    registerProperty<ImageInformationProperty>();
    registerProperty<IndexBufferInformationProperty>();
    registerProperty<LayerInformationProperty>();
    registerProperty<MeshBufferInformationProperty>();
    registerProperty<MeshInformationProperty>();
    registerProperty<SequenceTimerProperty>();
    registerProperty<VolumeInformationProperty>();

    registerProperty<Gaussian1DProperty>();
    registerProperty<Gaussian2DProperty>();

    registerProperty<transform::TranslateProperty>();
    registerProperty<transform::RotateProperty>();
    registerProperty<transform::ScaleProperty>();
    registerProperty<transform::CustomTransformProperty>();
    registerProperty<TransformListProperty>();

    // Register Data readers
    registerDataReader(std::make_unique<DatVolumeSequenceReader>());
    registerDataReader(std::make_unique<IvfVolumeReader>());
    registerDataReader(std::make_unique<IvfSequenceVolumeReader>());
    // Register Data writers
    registerDataWriter(std::make_unique<DatVolumeWriter>());
    registerDataWriter(std::make_unique<IvfVolumeWriter>());
    registerDataWriter(std::make_unique<StlWriter>());
    registerDataWriter(std::make_unique<BinarySTLWriter>());
    registerDataWriter(std::make_unique<WaveFrontWriter>());

    registerDataVisualizer(std::make_unique<ImageInformationVisualizer>(app));
    registerDataVisualizer(std::make_unique<MeshInformationVisualizer>(app));
    registerDataVisualizer(std::make_unique<VolumeInformationVisualizer>(app));

    util::for_each_type<OrdinalPropertyAnimator::Types>{}(RegHelper{}, *this);
}

int BaseModule::getVersion() const { return 5; }

std::unique_ptr<VersionConverter> BaseModule::getConverter(int version) const {
    return std::make_unique<Converter>(version);
}

BaseModule::Converter::Converter(int version) : version_(version) {}

bool BaseModule::Converter::convert(TxElement* root) {
    std::vector<xml::IdentifierReplacement> repl = {
        // CubeProxyGeometry
        {{xml::Kind::processor("org.inviwo.CubeProxyGeometry"),
          xml::Kind::inport("org.inviwo.VolumeInport")},
         "volume.inport",
         "volume"},
        {{xml::Kind::processor("org.inviwo.CubeProxyGeometry"),
          xml::Kind::outport("org.inviwo.MeshOutport")},
         "geometry.outport",
         "proxyGeometry"},
        // ImageSource
        {{xml::Kind::processor("org.inviwo.ImageSource"),
          xml::Kind::outport("org.inviwo.ImageOutport")},
         "image.outport",
         "image"},
        // MeshClipping
        {{xml::Kind::processor("org.inviwo.MeshClipping"),
          xml::Kind::inport("org.inviwo.MeshInport")},
         "geometry.input",
         "inputMesh"},
        {{xml::Kind::processor("org.inviwo.MeshClipping"),
          xml::Kind::outport("org.inviwo.MeshOutport")},
         "geometry.output",
         "clippedMesh"},

        // VolumeSlice
        {{xml::Kind::processor("org.inviwo.VolumeSlice"),
          xml::Kind::inport("org.inviwo.VolumeInport")},
         "volume.inport",
         "inputVolume"},
        {{xml::Kind::processor("org.inviwo.VolumeSlice"),
          xml::Kind::outport("org.inviwo.ImageOutport")},
         "image.outport",
         "outputImage"},

        // VolumeSubsample
        {{xml::Kind::processor("org.inviwo.VolumeSubsample"),
          xml::Kind::inport("org.inviwo.VolumeInport")},
         "volume.inport",
         "inputVolume"},
        {{xml::Kind::processor("org.inviwo.VolumeSubsample"),
          xml::Kind::outport("org.inviwo.VolumeOutport")},
         "volume.outport",
         "outputVolume"},

        // DistanceTransformRAM
        {{xml::Kind::processor("org.inviwo.DistanceTransformRAM"),
          xml::Kind::inport("org.inviwo.VolumeInport")},
         "volume.inport",
         "inputVolume"},
        {{xml::Kind::processor("org.inviwo.DistanceTransformRAM"),
          xml::Kind::outport("org.inviwo.VolumeOutport")},
         "volume.outport",
         "outputVolume"},

        // VolumeSubset
        {{xml::Kind::processor("org.inviwo.VolumeSubset"),
          xml::Kind::inport("org.inviwo.VolumeInport")},
         "volume.inport",
         "inputVolume"},
        {{xml::Kind::processor("org.inviwo.VolumeSubset"),
          xml::Kind::outport("org.inviwo.VolumeOutport")},
         "volume.outport",
         "outputVolume"}};

    for (const auto& id : {"org.inviwo.FloatProperty", "org.inviwo.FloatVec2Property",
                           "org.inviwo.FloatVec3Property", "org.inviwo.FloatVec4Property",
                           "org.inviwo.DoubleProperty", "org.inviwo.DoubleVec2Property",
                           "org.inviwo.DoubleVec3Property", "org.inviwo.DoubleVec4Property",
                           "org.inviwo.IntProperty", "org.inviwo.IntVec2Property",
                           "org.inviwo.IntVec3Property", "org.inviwo.IntVec4Property"}) {
        xml::IdentifierReplacement ir1 = {
            {xml::Kind::processor("org.inviwo.OrdinalPropertyAnimator"), xml::Kind::property(id)},
            id,
            dotSeperatedToPascalCase(id)};
        repl.push_back(ir1);
        xml::IdentifierReplacement ir2 = {
            {xml::Kind::processor("org.inviwo.OrdinalPropertyAnimator"), xml::Kind::property(id)},
            std::string(id) + "-Delta",
            dotSeperatedToPascalCase(id) + "-Delta"};
        repl.push_back(ir2);
    }

    std::vector<xml::IdentifierReplacement> replV3 = {
        // WorldTransform (Deprecated)
        {{xml::Kind::processor("org.inviwo.WorldTransformMeshDeprecated")},
         "World Transform Mesh",
         "World Transform Mesh (Deprecated)"},
        {{xml::Kind::processor("org.inviwo.WorldTransformVolumeDeprecated")},
         "World Transform Volume",
         "World Transform Volume (Deprecated)"}};

    bool res = false;
    switch (version_) {
        case 0: {
            res |= xml::changeIdentifiers(root, repl);
            [[fallthrough]];
        }
        case 1: {
            res |= xml::changeAttribute(
                root, {{xml::Kind::processor("org.inviwo.GeometeryGenerator")}}, "type",
                "org.inviwo.GeometeryGenerator", "org.inviwo.RandomMeshGenerator");
            [[fallthrough]];
        }
        case 2: {
            TraversingVersionConverter conv{[&](TxElement* node) -> bool {
                std::string key;
                node->GetValue(&key);
                if (key != "Property") return true;
                const auto type = node->GetAttributeOrDefault("type", "");
                if (type != "org.inviwo.VolumeBasisProperty") return true;

                TxElement newNode{"overrideModel"};
                for (const auto& item :
                     {std::make_tuple(0, "A", 0.0), std::make_tuple(1, "B", 0.0),
                      std::make_tuple(2, "C", 0.0), std::make_tuple(3, "Offset", 1.0)}) {
                    const auto path = fmt::format("Properties/Property&identifier=override{}/value",
                                                  std::get<1>(item));
                    if (auto elem = xml::getElement(node, path)) {
                        auto data = elem->Clone();
                        data->SetValue(fmt::format("col{}", std::get<0>(item)));
                        data->ToElement()->SetAttribute("w",
                                                        fmt::format("{:f}", std::get<2>(item)));
                        newNode.InsertEndChild(*data);
                    }
                }

                res = true;
                node->InsertEndChild(newNode);

                return true;
            }};

            conv.convert(root);
            [[fallthrough]];
        }
        case 3: {
            res |= xml::changeAttribute(
                root, {{xml::Kind::processor("org.inviwo.WorldTransformGeometry")}}, "type",
                "org.inviwo.WorldTransformGeometry", "org.inviwo.WorldTransformMeshDeprecated");
            res |= xml::changeAttribute(
                root, {{xml::Kind::processor("org.inviwo.WorldTransformVolume")}}, "type",
                "org.inviwo.WorldTransformVolume", "org.inviwo.WorldTransformVolumeDeprecated");
            res |= xml::changeIdentifiers(root, replV3);

            [[fallthrough]];
        }
        case 4: {
            res |=
                xml::changeAttribute(root, {xml::Kind::processor("org.inviwo.VolumeSlice")}, "type",
                                     "org.inviwo.VolumeSlice", "org.inviwo.VolumeSliceExtractor");
            return res;
        }

        default:
            return false;  // No changes
    }
}

}  // namespace inviwo
