/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include "volumecombiner.h"
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/texture/textureunit.h>
#include <inviwo/core/util/shuntingyard.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/volume/volumeutils.h>

namespace inviwo {

const ProcessorInfo VolumeCombiner::processorInfo_{
    "org.inviwo.VolumeCombiner",  // Class identifier
    "Volume Combiner",            // Display name
    "Volume Operation",           // Category
    CodeState::Experimental,      // Code state
    Tags::GL,                     // Tags
};
const ProcessorInfo VolumeCombiner::getProcessorInfo() const {
    return processorInfo_;
}

VolumeCombiner::VolumeCombiner()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , description_("description", "Volumes")
    , eqn_("eqn", "Equation", "s1*v1")
    , scales_("scales", "Scale factors")
    , useWorldSpaceCoordinateSystem_("useWorldSpaceCoordinateSystem", "World space", false)
    , borderValue_("borderValue", "Border value", vec4(0.f), vec4(0.f), vec4(1.f), vec4(0.1f))
    , fragment_{std::make_shared<StringShaderResource>("volumecombiler.frag", "")}
    , shader_({{ShaderType::Vertex, utilgl::findShaderResource("volume_gpu.vert")},
               {ShaderType::Geometry, utilgl::findShaderResource("volume_gpu.geom")},
               {ShaderType::Fragment, fragment_}},
              Shader::Build::No)
    , fbo_() {

    description_.setSemantics(PropertySemantics::Multiline);
    description_.setReadOnly(true);
    description_.setCurrentStateAsDefault();

    borderValue_.setVisible(useWorldSpaceCoordinateSystem_);

    addPort(inport_);
    addPort(outport_);
    addProperty(description_);
    addProperty(eqn_);
    addProperty(scales_);
    addProperty(useWorldSpaceCoordinateSystem_);
    addProperty(borderValue_);

    eqn_.onChange([&]() { 
        dirty_ = true; 
        testEquation();
    });

    inport_.onConnect([&]() {
        dirty_ = true;
        updateProperties();
        testEquation();
    });
    inport_.onDisconnect([&]() {
        dirty_ = true;
        updateProperties();
        testEquation();
    });

    useWorldSpaceCoordinateSystem_.onChange([this]() {
        dirty_ = true;
        borderValue_.setVisible(useWorldSpaceCoordinateSystem_);
    });
}

bool VolumeCombiner::isReady() const {
    if (!Processor::isReady()) return false;
    try {
        buildEquation();
        return true;
    } catch (Exception&) {
        return false;
    }
}

#include <warn/push>
#include <warn/ignore/unused-variable>
std::string VolumeCombiner::buildEquation() const {
    std::map<std::string, double> vars = {};
    std::map<std::string, std::string> symbols;

    size_t i = 0;
    for (const auto& dummy : inport_) {
        symbols["s" + toString(i + 1)] = "scale" + toString(i);
        symbols["v" + toString(i + 1)] = "vol" + toString(i);
        ++i;
    }

    return shuntingyard::Calculator::shaderCode(eqn_.get(), vars, symbols);
}

void VolumeCombiner::buildShader(const std::string& eqn) {
    std::stringstream ss;
    ss << "#include \"utils/structs.glsl\"\n";
    ss << "#include \"utils/sampler3d.glsl\"\n\n";
    ss << "in vec4 texCoord_;\n";

    ss << "uniform vec4 " << borderValue_.getIdentifier() << ";\n";
    
    size_t id = 0;
    for (const auto& dummy : inport_) {
        ss << "uniform sampler3D volume" << id << ";\n";
        ss << "uniform VolumeParameters volume" << id << "Parameters;\n";
        ss << "uniform float scale" << id << ";\n";
        ++id;
    }

    ss << "\nvoid main() {\n";

    id = 0;
    for (const auto& dummy : inport_) {
        const std::string vol = "vol" + toString(id);
        const std::string v = "volume" + toString(id);
        const std::string vp = "volume" + toString(id) + "Parameters";
        if (useWorldSpaceCoordinateSystem_) {
            const std::string coord = "coord" + toString(id);
            // Retrieve data from world space and use border value if outside of volume
            ss << "    vec3 " << coord << " = (" << vp << ".worldToTexture * "
               << "volume0Parameters.textureToWorld * texCoord_).xyz;\n";
            ss << "    vec4 " << vol << ";\n";
            ss << "    if (all(greaterThanEqual(" << coord << ", vec3(0))) &&"
               << " all(lessThanEqual(" << coord << ", vec3(1)))) {\n";
            ss << "        " << vol << " = getNormalizedVoxel(" << v << ", " << vp << ", " << coord
               << ");\n";
            ss << "    } else {\n";
            ss << "        " << vol << " = borderValue;\n";
            ss << "    }\n\n";
        } else {
            ss << "    vec4 " << vol << " = getNormalizedVoxel(" << v << ", " << vp
               << ", texCoord_.xyz);\n";
        }
        ++id;
    }

    ss << "    FragData0 = " << eqn << ";\n";
    ss << "    gl_FragDepth = 1.0;\n";
    ss << "}\n";

    fragment_->setSource(ss.str());
    shader_.build();
}

void VolumeCombiner::updateProperties() {
    auto format = [](const std::string& type, size_t i,
                     const std::pair<Outport*, std::shared_ptr<const inviwo::Volume>>& item) {
        return type + toString(i + 1) + ", " + item.first->getProcessor()->getIdentifier();
    };

    size_t i = 0;
    std::stringstream desc;
    for (const auto& item : inport_.getSourceVectorData()) {
        desc << format("v", i, item) << "\n";

        if (scales_.getProperties().size() <= i) {
            auto p = new FloatProperty("scale" + toString(i), "s"+toString(i + 1), 1.0f, -2.f,
                                       2.f, 0.01f);
            p->setSerializationMode(PropertySerializationMode::ALL);
            scales_.addProperty(p);
        } else {
            scales_.getProperties()[i]->setDisplayName("s"+toString(i + 1));
        }
        i++;
    }
    while (scales_.getProperties().size() > i) {
        delete scales_.removeProperty(scales_.getProperties().back());
    }

    description_.set(desc.str());
}

void VolumeCombiner::testEquation() {
    if (InviwoApplication::getPtr()->getProcessorNetwork()->isDeserializing()) return;
    try {
        buildEquation();
    } catch (Exception& e) {
        LogError(e.getMessage() + ": " + eqn_.get());
    }
}

#include <warn/pop>

void VolumeCombiner::process() {
    if (inport_.isChanged()) {
        updateProperties();

        const DataFormatBase* format = inport_.getData()->getDataFormat();
        volume_ = std::make_shared<Volume>(inport_.getData()->getDimensions(), format);
        volume_->setModelMatrix(inport_.getData()->getModelMatrix());
        volume_->setWorldMatrix(inport_.getData()->getWorldMatrix());
        // pass on metadata
        volume_->copyMetaDataFrom(*inport_.getData());
        volume_->dataMap_ = inport_.getData()->dataMap_;
        outport_.setData(volume_);
    }

    if (dirty_) {
        dirty_ = false;
        try {
            buildShader(buildEquation());
        } catch (Exception& e) {
            throw Exception(e.getMessage() + ": " + eqn_.get(), IvwContext);
        }     
    }

    shader_.activate();

    TextureUnitContainer cont;
    int i = 0;
    for (const auto& vol : inport_) {
        utilgl::bindAndSetUniforms(shader_, cont, *vol, "volume" + toString(i));
        ++i;
    }

    for (auto prop : scales_.getProperties()) {
        utilgl::setUniforms(shader_, *static_cast<FloatProperty*>(prop));
    }
    utilgl::setUniforms(shader_, borderValue_);

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
}



}  // namespace

