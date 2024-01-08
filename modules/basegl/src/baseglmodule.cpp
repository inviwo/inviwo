/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2024 Inviwo Foundation
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

#include <modules/basegl/baseglmodule.h>

#include <inviwo/core/common/inviwomodule.h>                                        // for Inv...
#include <inviwo/core/common/modulepath.h>                                          // for Mod...
#include <inviwo/core/io/serialization/ticpp.h>                                     // for TxE...
#include <inviwo/core/io/serialization/versionconverter.h>                          // for Kind
#include <inviwo/core/rendering/datavisualizer.h>                                   // for Dat...
#include <modules/basegl/datavisualizer/imagebackgroundvisualizer.h>                // for Ima...
#include <modules/basegl/datavisualizer/imagevisualizer.h>                          // for Ima...
#include <modules/basegl/datavisualizer/layervisualizer.h>                          // for Lay...
#include <modules/basegl/datavisualizer/meshvisualizer.h>                           // for Mes...
#include <modules/basegl/datavisualizer/volumeraycastvisualizer.h>                  // for Vol...
#include <modules/basegl/datavisualizer/volumeslicevisualizer.h>                    // for Vol...
#include <modules/basegl/processors/axisalignedcutplane.h>                          // for Axi...
#include <modules/basegl/processors/background.h>                                   // for Bac...
#include <modules/basegl/processors/cuberenderer.h>                                 // for Cub...
#include <modules/basegl/processors/drawlines.h>                                    // for Dra...
#include <modules/basegl/processors/drawpoints.h>                                   // for Dra...
#include <modules/basegl/processors/embeddedvolumeslice.h>                          // for Emb...
#include <modules/basegl/processors/entryexitpointsprocessor.h>                     // for Ent...
#include <modules/basegl/processors/firstivwprocessor.h>                            // for Fir...
#include <modules/basegl/processors/geometryentryexitpoints.h>                      // for Geo...
#include <modules/basegl/processors/heightfieldprocessor.h>                         // for Hei...
#include <modules/basegl/processors/imageprocessing/columnrowlayout.h>              // for Col...
#include <modules/basegl/processors/imageprocessing/findedges.h>                    // for Fin...
#include <modules/basegl/processors/imageprocessing/imagebinary.h>                  // for Ima...
#include <modules/basegl/processors/imageprocessing/imagechannelcombine.h>          // for Ima...
#include <modules/basegl/processors/imageprocessing/imagechannelselect.h>           // for Ima...
#include <modules/basegl/processors/imageprocessing/imagecompositeprocessorgl.h>    // for Ima...
#include <modules/basegl/processors/imageprocessing/imagegamma.h>                   // for Ima...
#include <modules/basegl/processors/imageprocessing/imagegradient.h>                // for Ima...
#include <modules/basegl/processors/imageprocessing/imagegrayscale.h>               // for Ima...
#include <modules/basegl/processors/imageprocessing/imagehighpass.h>                // for Ima...
#include <modules/basegl/processors/imageprocessing/imageinvert.h>                  // for Ima...
#include <modules/basegl/processors/imageprocessing/imagelayer.h>                   // for Ima...
#include <modules/basegl/processors/imageprocessing/imagelayoutgl.h>                // for Ima...
#include <modules/basegl/processors/imageprocessing/imagelowpass.h>                 // for Ima...
#include <modules/basegl/processors/imageprocessing/imagemapping.h>                 // for Ima...
#include <modules/basegl/processors/imageprocessing/imagemixer.h>                   // for Ima...
#include <modules/basegl/processors/imageprocessing/imagenormalizationprocessor.h>  // for Ima...
#include <modules/basegl/processors/imageprocessing/imageoverlaygl.h>               // for Ima...
#include <modules/basegl/processors/imageprocessing/imageresample.h>                // for Ima...
#include <modules/basegl/processors/imageprocessing/imagescaling.h>                 // for Ima...
#include <modules/basegl/processors/imageprocessing/imagesubsetgl.h>                // for Ima...
#include <modules/basegl/processors/imageprocessing/jacobian2d.h>                   // for Jac...
#include <modules/basegl/processors/imageprocessing/layershader.h>
#include <modules/basegl/processors/instancerenderer.h>  // for Ins...
#include <modules/basegl/processors/isoraycaster.h>      // for ISO...
#include <modules/basegl/processors/layerrenderer.h>
#include <modules/basegl/processors/lightingraycaster.h>                       // for Lig...
#include <modules/basegl/processors/lightvolumegl.h>                           // for Lig...
#include <modules/basegl/processors/linerendererprocessor.h>                   // for Lin...
#include <modules/basegl/processors/mesh2drenderprocessorgl.h>                 // for Mes...
#include <modules/basegl/processors/meshpicking.h>                             // for Mes...
#include <modules/basegl/processors/meshrenderprocessorgl.h>                   // for Mes...
#include <modules/basegl/processors/multichannelraycaster.h>                   // for Mul...
#include <modules/basegl/processors/pointrenderer.h>                           // for Poi...
#include <modules/basegl/processors/raycasting/atlasvolumeraycaster.h>         // for Atl...
#include <modules/basegl/processors/raycasting/multichannelvolumeraycaster.h>  // for Mul...
#include <modules/basegl/processors/raycasting/sphericalvolumeraycaster.h>     // for Sph...
#include <modules/basegl/processors/raycasting/standardvolumeraycaster.h>      // for Sta...
#include <modules/basegl/processors/raycasting/texturedisosurfacerenderer.h>
#include <modules/basegl/processors/redgreenprocessor.h>  // for Red...
#include <modules/basegl/processors/sphererenderer.h>     // for Sph...
#include <modules/basegl/processors/splitimage.h>         // for Spl...
#include <modules/basegl/processors/tuberendering.h>      // for Tub...
#include <modules/basegl/processors/volumemasker.h>
#include <modules/basegl/processors/volumeprocessing/vectormagnitudeprocessor.h>      // for Vec...
#include <modules/basegl/processors/volumeprocessing/volumebinary.h>                  // for Vol...
#include <modules/basegl/processors/volumeprocessing/volumecombiner.h>                // for Vol...
#include <modules/basegl/processors/volumeprocessing/volumediff.h>                    // for Vol...
#include <modules/basegl/processors/volumeprocessing/volumegradientmagnitude.h>       // for Vol...
#include <modules/basegl/processors/volumeprocessing/volumegradientprocessor.h>       // for Vol...
#include <modules/basegl/processors/volumeprocessing/volumelowpass.h>                 // for Vol...
#include <modules/basegl/processors/volumeprocessing/volumemapping.h>                 // for Vol...
#include <modules/basegl/processors/volumeprocessing/volumemerger.h>                  // for Vol...
#include <modules/basegl/processors/volumeprocessing/volumenormalizationprocessor.h>  // for Vol...
#include <modules/basegl/processors/volumeprocessing/volumeregionshrink.h>            // for Vol...
#include <modules/basegl/processors/volumeprocessing/volumeshader.h>                  // for Vol...
#include <modules/basegl/processors/volumeraycaster.h>                                // for Vol...
#include <modules/basegl/processors/volumeslicegl.h>                                  // for Vol...
#include <modules/basegl/properties/linesettingsproperty.h>                           // for Lin...
#include <modules/basegl/properties/splitterproperty.h>                               // for Spl...
#include <modules/basegl/properties/stipplingproperty.h>                              // for Sti...
// Autogenerated
#include <modules/basegl/shader_resources.h>      // for add...
#include <modules/opengl/shader/shadermanager.h>  // for Sha...

#include <string>   // for string
#include <utility>  // for pair
#include <vector>   // for vector

namespace inviwo {
class InviwoApplication;

BaseGLModule::BaseGLModule(InviwoApplication* app) : InviwoModule(app, "BaseGL") {

    basegl::addShaderResources(ShaderManager::getPtr(), {getPath(ModulePath::GLSL)});

    registerProperty<LineSettingsProperty>();
    registerProperty<SplitterProperty>();
    registerProperty<StipplingProperty>();

    registerProcessor<AtlasVolumeRaycaster>();
    registerProcessor<AxisAlignedCutPlane>();
    registerProcessor<Background>();
    registerProcessor<CubeRenderer>();
    registerProcessor<DrawLines>();
    registerProcessor<DrawPoints>();
    registerProcessor<EmbeddedVolumeSlice>();
    registerProcessor<EntryExitPoints>();
    registerProcessor<FirstIvwProcessor>();
    registerProcessor<GeometryEntryExitPoints>();
    registerProcessor<HeightFieldProcessor>();
    registerProcessor<ISORaycaster>();
    registerProcessor<LightingRaycaster>();
    registerProcessor<LightVolumeGL>();
    registerProcessor<LineRendererProcessor>();
    registerProcessor<Mesh2DRenderProcessorGL>();
    registerProcessor<MeshPicking>();
    registerProcessor<MeshRenderProcessorGL>();
    registerProcessor<MultiChannelVolumeRaycaster>();
    registerProcessor<MultichannelRaycaster>();
    registerProcessor<InstanceRenderer>();
    registerProcessor<PointRenderer>();
    registerProcessor<RedGreenProcessor>();
    registerProcessor<SphereRenderer>();
    registerProcessor<SphericalVolumeRaycaster>();
    registerProcessor<SplitImage>();
    registerProcessor<StandardVolumeRaycaster>();
    registerProcessor<TubeRendering>();
    registerProcessor<VolumeRaycaster>();
    registerProcessor<VolumeSliceGL>();

    // image processing
    registerProcessor<ColumnLayout>();
    registerProcessor<FindEdges>();
    registerProcessor<ImageBinary>();
    registerProcessor<ImageChannelCombine>();
    registerProcessor<ImageChannelSelect>();
    registerProcessor<ImageCompositeProcessorGL>();
    registerProcessor<ImageGamma>();
    registerProcessor<ImageGradient>();
    registerProcessor<ImageGrayscale>();
    registerProcessor<ImageHighPass>();
    registerProcessor<ImageInvert>();
    registerProcessor<ImageLayer>();
    registerProcessor<ImageLayoutGL>();
    registerProcessor<ImageLowPass>();
    registerProcessor<ImageMapping>();
    registerProcessor<ImageMixer>();
    registerProcessor<ImageNormalizationProcessor>();
    registerProcessor<ImageOverlayGL>();
    registerProcessor<ImageResample>();
    registerProcessor<ImageScaling>();
    registerProcessor<ImageSubsetGL>();
    registerProcessor<Jacobian2D>();
    registerProcessor<LayerRenderer>();
    registerProcessor<LayerShader>();
    registerProcessor<RowLayout>();

    // volume processing
    registerProcessor<TexturedIsosurfaceRenderer>();
    registerProcessor<VectorMagnitudeProcessor>();
    registerProcessor<VolumeBinary>();
    registerProcessor<VolumeCombiner>();
    registerProcessor<VolumeDiff>();
    registerProcessor<VolumeGradientMagnitude>();
    registerProcessor<VolumeGradientProcessor>();
    registerProcessor<VolumeLowPass>();
    registerProcessor<VolumeMapping>();
    registerProcessor<VolumeMasker>();
    registerProcessor<VolumeMerger>();
    registerProcessor<VolumeNormalizationProcessor>();
    registerProcessor<VolumeRegionShrink>();
    registerProcessor<VolumeShader>();

    registerDataVisualizer(std::make_unique<VolumeRaycastVisualizer>(app));
    registerDataVisualizer(std::make_unique<VolumeSliceVisualizer>(app));
    registerDataVisualizer(std::make_unique<ImageVisualizer>(app));
    registerDataVisualizer(std::make_unique<ImageBackgroundVisualizer>(app));
    registerDataVisualizer(std::make_unique<LayerVisualizer>(app));
    registerDataVisualizer(std::make_unique<MeshVisualizer>(app));
}

int BaseGLModule::getVersion() const { return 6; }

std::unique_ptr<VersionConverter> BaseGLModule::getConverter(int version) const {
    return std::make_unique<Converter>(version);
}

BaseGLModule::Converter::Converter(int version) : version_(version) {}

bool BaseGLModule::Converter::convert(TxElement* root) {
    auto makerulesV1 = []() {
        std::vector<xml::IdentifierReplacement> repl = {
            // MeshRenderProcessorGL
            {{xml::Kind::processor("org.inviwo.GeometryRenderGL"),
              xml::Kind::inport("org.inviwo.MeshFlatMultiInport")},
             "geometry.inport",
             "geometry"},
            {{xml::Kind::processor("org.inviwo.GeometryRenderGL"),
              xml::Kind::outport("org.inviwo.ImageOutport")},
             "image.outport",
             "image"},

            // HeightFieldRenderGL
            {{xml::Kind::processor("org.inviwo.HeightFieldRenderGL"),
              xml::Kind::inport("org.inviwo.MeshFlatMultiInport")},
             "geometry.inport",
             "geometry"},
            {{xml::Kind::processor("org.inviwo.HeightFieldRenderGL"),
              xml::Kind::outport("org.inviwo.ImageOutport")},
             "image.outport",
             "image"},
            {{xml::Kind::processor("org.inviwo.HeightFieldRenderGL"),
              xml::Kind::inport("org.inviwo.ImageInport")},
             "heightfield.inport",
             "heightfield"},
            {{xml::Kind::processor("org.inviwo.HeightFieldRenderGL"),
              xml::Kind::inport("org.inviwo.ImageInport")},
             "texture.inport",
             "texture"},
            {{xml::Kind::processor("org.inviwo.HeightFieldRenderGL"),
              xml::Kind::inport("org.inviwo.ImageInport")},
             "normalmap.inport",
             "normalmap"},

            // HeightFieldMapper
            {{xml::Kind::processor("org.inviwo.HeightFieldMapper"),
              xml::Kind::inport("org.inviwo.ImageInport")},
             "image.inport",
             "image"},
            {{xml::Kind::processor("org.inviwo.HeightFieldMapper"),
              xml::Kind::outport("org.inviwo.ImageOutport")},
             "image.outport",
             "heightfield"},

            // Background
            {{xml::Kind::processor("org.inviwo.Background"),
              xml::Kind::property("org.inviwo.ButtonProperty")},
             "Switch colors",
             "switchColors"},

            // ImageSourceSeries
            {{xml::Kind::processor("org.inviwo.ImageSourceSeries"),
              xml::Kind::outport("org.inviwo.ImageOutport")},
             "image.outport",
             "outputImage"},

            // Mesh2DRenderProcessorGL
            {{xml::Kind::processor("org.inviwo.Mesh2DRenderProcessorGL"),
              xml::Kind::inport("org.inviwo.MeshFlatMultiInport")},
             "geometry.inport",
             "inputMesh"},
            {{xml::Kind::processor("org.inviwo.Mesh2DRenderProcessorGL"),
              xml::Kind::outport("org.inviwo.ImageOutport")},
             "image.outport",
             "outputImage"},

            // DrawPoints
            {{xml::Kind::processor("org.inviwo.DrawPoints"),
              xml::Kind::inport("org.inviwo.ImageInport")},
             "image.inport",
             "inputImage"},
            {{xml::Kind::processor("org.inviwo.DrawPoints"),
              xml::Kind::outport("org.inviwo.ImageOutport")},
             "image.outport",
             "outputImage"},

            // VolumeDiff
            {{xml::Kind::processor("org.inviwo.VolumeDiff"),
              xml::Kind::inport("org.inviwo.VolumeInport")},
             "vol2",
             "volume2"},
        };

        const std::vector<std::pair<std::string, std::string>> imageGLrepl = {
            {"FindEdges", "img_findedges.frag"},    {"ImageBinary", "img_binary.frag"},
            {"ImageGamma", "img_gamma.frag"},       {"ImageGradient", "imagegradient.frag"},
            {"ImageGrayscale", "img_graysc.frag"},  {"ImageLowPass", "img_lowpass.frag"},
            {"ImageHighPass", "img_highpass.frag"}, {"ImageMapping", "img_mapping.frag"},
            {"ImageInvert", "img_invert.frag"},     {"ImageNormalization", "img_normalize.frag"},
            {"ImageResample", "img_resample.frag"}};

        for (const auto& i : imageGLrepl) {
            xml::IdentifierReplacement inport = {{xml::Kind::processor("org.inviwo." + i.first),
                                                  xml::Kind::inport("org.inviwo.ImageInport")},
                                                 i.second + "inport",
                                                 "inputImage"};
            xml::IdentifierReplacement outport = {{xml::Kind::processor("org.inviwo." + i.first),
                                                   xml::Kind::outport("org.inviwo.ImageOutport")},
                                                  i.second + "outport",
                                                  "outputImage"};
            repl.push_back(inport);
            repl.push_back(outport);
        }

        const std::vector<std::pair<std::string, std::string>> volumeGLrepl = {
            {"VolumeGradient", "volume_gradient.frag"},
            {"VolumeBinary", "volume_binary.frag"},
            {"VolumeDiff", "volume_difference.frag"},
            {"VectorMagnitude", "vectormagnitudeprocessor.frag"},
            {"VolumeGradientMagnitude", "volumegradientmagnitude.frag"},
            {"VolumeLowPass", "volume_lowpass.frag"},
            {"VolumeMapping", "volume_mapping.frag"},
            {"VolumeMerger", "volumemerger.frag"}};

        for (const auto& i : volumeGLrepl) {
            xml::IdentifierReplacement inport = {{xml::Kind::processor("org.inviwo." + i.first),
                                                  xml::Kind::inport("org.inviwo.VolumeInport")},
                                                 i.second + "inport",
                                                 "inputVolume"};
            xml::IdentifierReplacement outport = {{xml::Kind::processor("org.inviwo." + i.first),
                                                   xml::Kind::outport("org.inviwo.VolumeOutport")},
                                                  i.second + "outport",
                                                  "outputVolume"};
            repl.push_back(inport);
            repl.push_back(outport);
        }

        return repl;
    };

    bool res = false;
    switch (version_) {
        case 0: {
            auto replV1 = makerulesV1();
            res |= xml::changeIdentifiers(root, replV1);
            [[fallthrough]];
        }
        case 1: {
            res |= xml::changeIdentifier(root,
                                         {{xml::Kind::processor("org.inviwo.Background"),
                                           xml::Kind::property("org.inviwo.FloatVec4Property")}},
                                         "color1", "bgColor1");
            res |= xml::changeIdentifier(root,
                                         {{xml::Kind::processor("org.inviwo.Background"),
                                           xml::Kind::property("org.inviwo.FloatVec4Property")}},
                                         "color2", "bgColor2");
            [[fallthrough]];
        }
        case 2: {
            res |= xml::changeIdentifier(root,
                                         {{xml::Kind::processor("org.inviwo.LineRenderer"),
                                           xml::Kind::property("org.inviwo.FloatProperty")}},
                                         "antialising", "antialiasing");

            [[fallthrough]];
        }
        case 3: {
            std::vector<xml::IdentifierReplacement> repl{
                {{xml::Kind::processor("org.inviwo.SphereRenderer"),
                  xml::Kind::property("org.inviwo.CompositeProperty"),
                  xml::Kind::property("org.inviwo.BoolProperty")},
                 "overrideSphereRadius",
                 "forceRadius"},
                {{xml::Kind::processor("org.inviwo.SphereRenderer"),
                  xml::Kind::property("org.inviwo.CompositeProperty"),
                  xml::Kind::property("org.inviwo.FloatProperty")},
                 "customRadius",
                 "defaultRadius"},
                {{xml::Kind::processor("org.inviwo.SphereRenderer"),
                  xml::Kind::property("org.inviwo.CompositeProperty"),
                  xml::Kind::property("org.inviwo.BoolProperty")},
                 "overrideSphereColor",
                 "forceColor"},
                {{xml::Kind::processor("org.inviwo.SphereRenderer"),
                  xml::Kind::property("org.inviwo.CompositeProperty"),
                  xml::Kind::property("org.inviwo.FloatVec4Property")},
                 "customColor",
                 "defaultColor"},

                {{xml::Kind::processor("org.inviwo.CubeRenderer"),
                  xml::Kind::property("org.inviwo.CompositeProperty"),
                  xml::Kind::property("org.inviwo.BoolProperty")},
                 "overrideCubeSize",
                 "forceSize"},
                {{xml::Kind::processor("org.inviwo.CubeRenderer"),
                  xml::Kind::property("org.inviwo.CompositeProperty"),
                  xml::Kind::property("org.inviwo.FloatProperty")},
                 "customSize",
                 "defaultSize"},
                {{xml::Kind::processor("org.inviwo.CubeRenderer"),
                  xml::Kind::property("org.inviwo.CompositeProperty"),
                  xml::Kind::property("org.inviwo.BoolProperty")},
                 "overrideCubeColor",
                 "forceColor"},
                {{xml::Kind::processor("org.inviwo.CubeRenderer"),
                  xml::Kind::property("org.inviwo.CompositeProperty"),
                  xml::Kind::property("org.inviwo.FloatVec4Property")},
                 "customColor",
                 "defaultColor"},

                {{xml::Kind::processor("org.inviwo.TubeRendering"),
                  xml::Kind::property("org.inviwo.SimpleLightingProperty")},
                 "light",
                 "lighting"},
                {{xml::Kind::processor("org.inviwo.TubeRendering"),
                  xml::Kind::property("org.inviwo.FloatProperty")},
                 "radius",
                 "defaultRadius"}};
            res |= xml::changeIdentifiers(root, repl);
            [[fallthrough]];
        }
        case 4: {
            res |= xml::changeIdentifier(root,
                                         {xml::Kind::processor("org.inviwo.SplitImage"),
                                          xml::Kind::property("org.inviwo.BoolCompositeProperty"),
                                          xml::Kind::property("org.inviwo.FloatVec4Property")},
                                         "triColor", "hoverColor");
            res |= xml::changeAttribute(root,
                                        {{xml::Kind::processor("org.inviwo.SplitImage"),
                                          xml::Kind::property("org.inviwo.BoolCompositeProperty")}},
                                        "type", "org.inviwo.BoolCompositeProperty",
                                        "org.inviwo.SplitterProperty");
            [[fallthrough]];
        }
        case 5: {
            for (auto&& [path, newName] :
                 {std::pair{"sphereProperties.defaultRadius", "radius"},
                  std::pair{"sphereProperties.forceRadius", "overrideRadius"},
                  std::pair{"sphereProperties.defaultColor", "color"},
                  std::pair{"sphereProperties.forceColor", "overrideColor"},
                  std::pair{"clipping.clipMode", "mode"}}) {

                res |=
                    xml::renamePropertyIdentifier(root, "org.inviwo.SphereRenderer", path, newName);
                res |= xml::renamePropertyIdentifier(root, "org.inviwo.SphereRasterizer", path,
                                                     newName);
            }

            xml::visitMatchingNodesRecursive(
                root, {"Processor", {{"type", "org.inviwo.SphereRenderer"}}}, [&](TxElement* prop) {
                    if (auto color =
                            xml::getElement(prop,
                                            "Properties/Property&identifier=sphereProperties/"
                                            "Properties/Property&identifier=color")) {

                        auto alpha = color->Clone();

                        color->SetAttribute("type", "org.inviwo.FloatVec3Property");

                        alpha->ToElement()->SetAttribute("identifier", "alpha");
                        alpha->ToElement()->SetAttribute("type", "org.inviwo.FloatProperty");
                        if (auto* value = alpha->FirstChild(false)) {
                            auto a = value->ToElement()->GetAttribute("w");
                            value->ToElement()->SetAttribute("content", a);
                            color->Parent()->InsertEndChild(*alpha);
                        }

                        res |= true;
                    }
                });

            return res;
        }

        default:
            return false;  // No changes
    }
}

}  // namespace inviwo
