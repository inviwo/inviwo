/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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
#include <inviwo/core/io/serialization/versionconverter.h>
#include <modules/basegl/processors/axisalignedcutplane.h>
#include <modules/basegl/processors/background.h>
#include <modules/basegl/processors/cuberenderer.h>
#include <modules/basegl/processors/drawlines.h>
#include <modules/basegl/processors/drawpoints.h>
#include <modules/basegl/processors/embeddedvolumeslice.h>
#include <modules/basegl/processors/entryexitpointsprocessor.h>
#include <modules/basegl/processors/firstivwprocessor.h>
#include <modules/basegl/processors/geometryentryexitpoints.h>
#include <modules/basegl/processors/heightfieldprocessor.h>
#include <modules/basegl/processors/imageprocessing/findedges.h>
#include <modules/basegl/processors/imageprocessing/imagebinary.h>
#include <modules/basegl/processors/imageprocessing/imagechannelcombine.h>
#include <modules/basegl/processors/imageprocessing/imagechannelselect.h>
#include <modules/basegl/processors/imageprocessing/imagecompositeprocessorgl.h>
#include <modules/basegl/processors/imageprocessing/imagegamma.h>
#include <modules/basegl/processors/imageprocessing/imageglprocessor.h>
#include <modules/basegl/processors/imageprocessing/imagegradient.h>
#include <modules/basegl/processors/imageprocessing/imagegrayscale.h>
#include <modules/basegl/processors/imageprocessing/imagehighpass.h>
#include <modules/basegl/processors/imageprocessing/imageinvert.h>
#include <modules/basegl/processors/imageprocessing/imagelayer.h>
#include <modules/basegl/processors/imageprocessing/imagelayoutgl.h>
#include <modules/basegl/processors/imageprocessing/imagelowpass.h>
#include <modules/basegl/processors/imageprocessing/imagemapping.h>
#include <modules/basegl/processors/imageprocessing/imagemixer.h>
#include <modules/basegl/processors/imageprocessing/imagenormalizationprocessor.h>
#include <modules/basegl/processors/imageprocessing/imageoverlaygl.h>
#include <modules/basegl/processors/imageprocessing/imageresample.h>
#include <modules/basegl/processors/imageprocessing/imagescaling.h>
#include <modules/basegl/processors/imageprocessing/imagesubsetgl.h>
#include <modules/basegl/processors/imageprocessing/jacobian2d.h>
#include <modules/basegl/processors/isoraycaster.h>
#include <modules/basegl/processors/lightingraycaster.h>
#include <modules/basegl/processors/lightvolumegl.h>
#include <modules/basegl/processors/linerenderer.h>
#include <modules/basegl/processors/mesh2drenderprocessorgl.h>
#include <modules/basegl/processors/meshpicking.h>
#include <modules/basegl/processors/meshrenderprocessorgl.h>
#include <modules/basegl/processors/multichannelraycaster.h>
#include <modules/basegl/processors/pointrenderer.h>
#include <modules/basegl/processors/redgreenprocessor.h>
#include <modules/basegl/processors/sphererenderer.h>
#include <modules/basegl/processors/splitimage.h>
#include <modules/basegl/processors/tuberendering.h>
#include <modules/basegl/processors/volumeprocessing/vectormagnitudeprocessor.h>
#include <modules/basegl/processors/volumeprocessing/volumebinary.h>
#include <modules/basegl/processors/volumeprocessing/volumecombiner.h>
#include <modules/basegl/processors/volumeprocessing/volumediff.h>
#include <modules/basegl/processors/volumeprocessing/volumegradientmagnitude.h>
#include <modules/basegl/processors/volumeprocessing/volumegradientprocessor.h>
#include <modules/basegl/processors/volumeprocessing/volumelowpass.h>
#include <modules/basegl/processors/volumeprocessing/volumemapping.h>
#include <modules/basegl/processors/volumeprocessing/volumemerger.h>
#include <modules/basegl/processors/volumeraycaster.h>
#include <modules/basegl/processors/volumeslicegl.h>
#include <modules/basegl/processors/volumeprocessing/volumeshader.h>
#include <modules/basegl/datavisualizer/volumeraycastvisualizer.h>
#include <modules/basegl/datavisualizer/volumeslicevisualizer.h>
#include <modules/basegl/datavisualizer/imagevisualizer.h>
#include <modules/basegl/datavisualizer/imagebackgroundvisualizer.h>
#include <modules/basegl/datavisualizer/meshvisualizer.h>

#include <modules/opengl/shader/shadermanager.h>

// Autogenerated
#include <modules/basegl/shader_resources.h>

namespace inviwo {

BaseGLModule::BaseGLModule(InviwoApplication* app) : InviwoModule(app, "BaseGL") {

    basegl::addShaderResources(ShaderManager::getPtr(), {getPath(ModulePath::GLSL)});

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
    registerProcessor<ImageCompositeProcessorGL>();
    registerProcessor<ImageLayoutGL>();
    registerProcessor<ImageMixer>();
    registerProcessor<ImageOverlayGL>();
    registerProcessor<ISORaycaster>();
    registerProcessor<Jacobian2D>();
    registerProcessor<LightingRaycaster>();
    registerProcessor<LightVolumeGL>();
    registerProcessor<LineRenderer>();
    registerProcessor<Mesh2DRenderProcessorGL>();
    registerProcessor<MeshPicking>();
    registerProcessor<MeshRenderProcessorGL>();
    registerProcessor<MultichannelRaycaster>();
    registerProcessor<PointRenderer>();
    registerProcessor<RedGreenProcessor>();
    registerProcessor<SphereRenderer>();
    registerProcessor<TubeRendering>();
    registerProcessor<VolumeRaycaster>();
    registerProcessor<VolumeSliceGL>();

    // image processing
    registerProcessor<FindEdges>();
    registerProcessor<ImageBinary>();
    registerProcessor<ImageChannelCombine>();
    registerProcessor<ImageChannelSelect>();
    registerProcessor<ImageGamma>();
    registerProcessor<ImageGradient>();
    registerProcessor<ImageGrayscale>();
    registerProcessor<ImageHighPass>();
    registerProcessor<ImageInvert>();
    registerProcessor<ImageLayer>();
    registerProcessor<ImageLowPass>();
    registerProcessor<ImageMapping>();
    registerProcessor<ImageNormalizationProcessor>();
    registerProcessor<ImageResample>();
    registerProcessor<ImageScaling>();
    registerProcessor<ImageSubsetGL>();
    registerProcessor<SplitImage>();

    // volume processing
    registerProcessor<VectorMagnitudeProcessor>();
    registerProcessor<VolumeBinary>();
    registerProcessor<VolumeCombiner>();
    registerProcessor<VolumeDiff>();
    registerProcessor<VolumeGradientMagnitude>();
    registerProcessor<VolumeGradientProcessor>();
    registerProcessor<VolumeLowPass>();
    registerProcessor<VolumeMapping>();
    registerProcessor<VolumeMerger>();
    registerProcessor<VolumeShader>();

    registerDataVisualizer(std::make_unique<VolumeRaycastVisualizer>(app));
    registerDataVisualizer(std::make_unique<VolumeSliceVisualizer>(app));
    registerDataVisualizer(std::make_unique<ImageVisualizer>(app));
    registerDataVisualizer(std::make_unique<ImageBackgroundVisualizer>(app));
    registerDataVisualizer(std::make_unique<MeshVisualizer>(app));
}

int BaseGLModule::getVersion() const { return 4; }

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

    auto replV1 = makerulesV1();

    bool res = false;
    switch (version_) {
        case 0: {
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

            return res;
        }

        default:
            return false;  // No changes
    }
    return true;
}

}  // namespace inviwo
