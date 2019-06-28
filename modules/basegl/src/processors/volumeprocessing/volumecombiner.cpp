/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#include <modules/basegl/processors/volumeprocessing/volumecombiner.h>
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/texture/textureunit.h>
#include <inviwo/core/util/shuntingyard.h>
#include <inviwo/core/util/zip.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/volume/volumeutils.h>

namespace {
std::string idToString(const size_t& id) {
    if (id == 0) return "";
    return inviwo::toString(id);
}
}  // namespace

namespace inviwo {

const ProcessorInfo VolumeCombiner::processorInfo_{
    "org.inviwo.VolumeCombiner",  // Class identifier
    "Volume Combiner",            // Display name
    "Volume Operation",           // Category
    CodeState::Experimental,      // Code state
    Tags::GL,                     // Tags
};
const ProcessorInfo VolumeCombiner::getProcessorInfo() const { return processorInfo_; }

VolumeCombiner::VolumeCombiner()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , description_("description", "Volumes")
    , eqn_("eqn", "Equation", "v1")
    , normalizationMode_(
          "normalizationMode", "Normalization Mode",
          {{"normalized", "Normalize volumes", NormalizationMode::Normalized},
           {"signedNormalized", "Normalize volumes with sign", NormalizationMode::SignedNormalized},
           {"noNormalization", "No normalization", NormalizationMode::NotNormalized}},
          0)
    , scales_("scales", "Scale factors")
    , addScale_("addScale", "Add Scale Factor")
    , removeScale_("removeScale", "Remove Scale Factor")
    , useWorldSpace_("useWorldSpaceCoordinateSystem", "World Space", false)
    , borderValue_("borderValue", "Border value", vec4(0.f), vec4(0.f), vec4(1.f), vec4(0.1f))
    , dataRange_("dataRange", "Data Range")
    , rangeMode_("rangeMode", "Mode")
    , outputDataRange_("outputDataRange", "Output Data Range", 0.0, 1.0,
                       std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(),
                       0.01, 0.0, InvalidationLevel::Valid, PropertySemantics::Text)
    , outputValueRange_("outputValueRange", "Output ValueRange", 0.0, 1.0,
                        std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(),
                        0.01, 0.0, InvalidationLevel::Valid, PropertySemantics::Text)
    , customRange_("customRange", "Custom Range")
    , customDataRange_("customDataRange", "Custom Data Range", 0.0, 1.0,
                       std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(),
                       0.01, 0.0, InvalidationLevel::InvalidOutput, PropertySemantics::Text)
    , customValueRange_("customValueRange", "Custom Value Range", 0.0, 1.0,
                        std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(),
                        0.01, 0.0, InvalidationLevel::InvalidOutput, PropertySemantics::Text)
    , fragment_{std::make_shared<StringShaderResource>("volumecombiner.frag", "")}
    , shader_({{ShaderType::Vertex, utilgl::findShaderResource("volume_gpu.vert")},
               {ShaderType::Geometry, utilgl::findShaderResource("volume_gpu.geom")},
               {ShaderType::Fragment, fragment_}},
              Shader::Build::No)
    , fbo_() {

    description_.setSemantics(PropertySemantics::Multiline);
    description_.setReadOnly(true);
    description_.setCurrentStateAsDefault();

    isReady_.setUpdate([this]() { return allInportsAreReady() && (valid_ || dirty_); });

    addPort(inport_);
    addPort(outport_);
    addProperty(description_);
    addProperty(eqn_);
    addProperty(normalizationMode_);
    addProperty(addScale_);
    addProperty(removeScale_);
    addProperty(scales_);
    addProperty(useWorldSpace_);

    useWorldSpace_.addProperty(borderValue_);

    addProperty(dataRange_);
    dataRange_.addProperties(rangeMode_, outputDataRange_, outputValueRange_, customRange_,
                             customDataRange_, customValueRange_);

    outputDataRange_.setReadOnly(true);
    outputValueRange_.setReadOnly(true);

    customRange_.onChange([&]() {
        customDataRange_.setReadOnly(!customRange_.get());
        customValueRange_.setReadOnly(!customRange_.get());
    });
    customDataRange_.setReadOnly(!customRange_.get());
    customValueRange_.setReadOnly(!customRange_.get());

    normalizationMode_.onChange([&]() { dirty_ = true; });

    addScale_.onChange([&]() {
        size_t i = scales_.size();
        auto p = std::make_unique<FloatProperty>("scale" + toString(i), "s" + toString(i + 1), 1.0f,
                                                 -2.f, 2.f, 0.01f);
        p->setSerializationMode(PropertySerializationMode::All);
        scales_.addProperty(p.release());
    });

    removeScale_.onChange([&]() {
        if (scales_.size() > 0) {
            dirty_ = true;
            delete scales_.removeProperty(scales_.getProperties().back());
            isReady_.update();
        }
    });

    eqn_.onChange([&]() {
        dirty_ = true;
        isReady_.update();
    });

    inport_.onConnect([&]() {
        dirty_ = true;
        updateProperties();
        isReady_.update();
    });
    inport_.onDisconnect([&]() {
        dirty_ = true;
        updateProperties();
        isReady_.update();
    });

    useWorldSpace_.onChange([this]() {
        dirty_ = true;
        isReady_.update();
    });
}

std::string VolumeCombiner::buildEquation() const {
    std::map<std::string, double> vars = {};
    std::map<std::string, std::string> symbols;

    for (auto&& i : util::enumerate(inport_)) {
        symbols["s" + toString(i.first() + 1)] = "scale" + toString(i.first());
        symbols["v" + toString(i.first() + 1)] = "vol" + toString(i.first());
    }

    return shuntingyard::Calculator::shaderCode(eqn_.get(), vars, symbols);
}

void VolumeCombiner::buildShader(const std::string& eqn) {
    std::stringstream ss;
    ss << "#include \"utils/structs.glsl\"\n";
    ss << "#include \"utils/sampler3d.glsl\"\n\n";
    ss << "in vec4 texCoord_;\n";

    ss << "uniform vec4 " << borderValue_.getIdentifier() << ";\n";

    for (auto&& i : util::enumerate(inport_)) {
        ss << "uniform sampler3D volume" << idToString(i.first()) << ";\n";
        ss << "uniform VolumeParameters volume" << idToString(i.first()) << "Parameters;\n";
    }
    for (auto prop : scales_.getProperties()) {
        ss << "uniform float " << prop->getIdentifier() << ";\n";
    }

    ss << "\nvoid main() {\n";

    // Determine which normalization mode should be used
    std::string getVoxel;
    auto mode = normalizationMode_.get();
    if (mode == NormalizationMode::Normalized)
        getVoxel = "getNormalizedVoxel";
    else if (mode == NormalizationMode::SignedNormalized)
        getVoxel = "getSignNormalizedVoxel";
    else
        getVoxel = "getVoxel";

    for (auto&& i : util::enumerate(inport_)) {
        const std::string vol = "vol" + toString(i.first());
        const std::string v = "volume" + idToString(i.first());
        const std::string vp = "volume" + idToString(i.first()) + "Parameters";
        if (useWorldSpace_) {
            const std::string coord = "coord" + toString(i.first());
            // Retrieve data from world space and use border value if outside of volume
            ss << "    vec3 " << coord << " = (" << vp << ".worldToTexture * "
               << "volumeParameters.textureToWorld * texCoord_).xyz;\n";
            ss << "    vec4 " << vol << ";\n";
            ss << "    if (all(greaterThanEqual(" << coord << ", vec3(0))) &&"
               << " all(lessThanEqual(" << coord << ", vec3(1)))) {\n";
            ss << "        " << vol << " = " << getVoxel << "(" << v << ", " << vp << ", " << coord
               << ");\n";
            ss << "    } else {\n";
            ss << "        " << vol << " = borderValue;\n";
            ss << "    }\n\n";
        } else {
            ss << "    vec4 " << vol << " = " << getVoxel << "(" << v << ", " << vp
               << ", texCoord_.xyz);\n";
        }
    }

    ss << "    FragData0 = " << eqn << ";\n";
    ss << "    gl_FragDepth = 1.0;\n";
    ss << "}\n";

    fragment_->setSource(ss.str());
    shader_.build();
}

void VolumeCombiner::updateProperties() {
    std::stringstream desc;
    std::vector<OptionPropertyIntOption> options;
    for (const auto& p : util::enumerate(inport_.getConnectedOutports())) {
        const std::string str =
            "v" + toString(p.first() + 1) + ": " + p.second()->getProcessor()->getDisplayName();
        desc << str << "\n";
        options.emplace_back("v" + toString(p.first() + 1), str, static_cast<int>(p.first()));
    }
    options.emplace_back("maxRange", "min/max {v1, v2, ...}", -1);
    description_.set(desc.str());

    rangeMode_.replaceOptions(options);
}

void VolumeCombiner::process() {
    if (inport_.isChanged()) {
        const DataFormatBase* format = inport_.getData()->getDataFormat();
        volume_ = std::make_shared<Volume>(inport_.getData()->getDimensions(), format);
        volume_->setModelMatrix(inport_.getData()->getModelMatrix());
        volume_->setWorldMatrix(inport_.getData()->getWorldMatrix());
        // pass on metadata
        volume_->copyMetaDataFrom(*inport_.getData());
        volume_->dataMap_ = inport_.getData()->dataMap_;
        outport_.setData(volume_);
    }

    updateDataRange();

    if (dirty_) {
        dirty_ = false;
        try {
            buildShader(buildEquation());
            valid_ = true;
        } catch (Exception& e) {
            valid_ = false;
            isReady_.update();
            throw Exception(e.getMessage() + ": " + eqn_.get(), IVW_CONTEXT);
        }
    }

    shader_.activate();

    TextureUnitContainer cont;
    int i = 0;
    for (const auto& vol : inport_) {
        utilgl::bindAndSetUniforms(shader_, cont, *vol, "volume" + idToString(i));
        ++i;
    }

    utilgl::setUniforms(shader_, borderValue_);
    for (auto prop : scales_.getProperties()) {
        utilgl::setUniforms(shader_, *static_cast<FloatProperty*>(prop));
    }

    const size3_t dim{inport_.getData()->getDimensions()};
    fbo_.activate();
    glViewport(0, 0, static_cast<GLsizei>(dim.x), static_cast<GLsizei>(dim.y));

    // We always need to ask for a editable representation
    // this will invalidate any other representations
    VolumeGL* outVolumeGL = volume_->getEditableRepresentation<VolumeGL>();
    if (inport_.isChanged()) {
        fbo_.attachColorTexture(outVolumeGL->getTexture().get(), 0);
    }

    utilgl::multiDrawImagePlaneRect(static_cast<int>(dim.z));

    shader_.deactivate();
    fbo_.deactivate();
    isReady_.update();
}

void VolumeCombiner::updateDataRange() {
    dvec2 dataRange = inport_.getData()->dataMap_.dataRange;
    dvec2 valueRange = inport_.getData()->dataMap_.valueRange;

    if (rangeMode_.getSelectedIdentifier() == "maxRange") {
        auto minmax = [](const dvec2& a, const dvec2& b) {
            return dvec2{std::min(a.x, b.x), std::max(a.y, b.y)};
        };

        for (const auto& vol : inport_) {
            dataRange = minmax(dataRange, vol->dataMap_.dataRange);
            valueRange = minmax(valueRange, vol->dataMap_.valueRange);
        }
    } else {
        dataRange = inport_.getVectorData()[rangeMode_.getSelectedValue()]->dataMap_.dataRange;
        valueRange = inport_.getVectorData()[rangeMode_.getSelectedValue()]->dataMap_.valueRange;
    }
    outputDataRange_.set(dataRange);
    outputValueRange_.set(valueRange);

    if (customRange_) {
        dataRange = customDataRange_;
        valueRange = customValueRange_;
    }

    volume_->dataMap_.dataRange = dataRange;
    volume_->dataMap_.valueRange = valueRange;
}

}  // namespace inviwo
