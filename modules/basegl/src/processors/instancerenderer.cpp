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

#include <modules/basegl/processors/instancerenderer.h>

#include <inviwo/core/algorithm/boundingbox.h>         // for boundingBox
#include <inviwo/core/algorithm/markdown.h>            // for operator""_help, operator""_un...
#include <inviwo/core/datastructures/geometry/mesh.h>  // for Mesh
#include <inviwo/core/ports/imageport.h>               // for BaseImageInport, ImageInport
#include <inviwo/core/ports/inport.h>                  // for Inport
#include <inviwo/core/ports/meshport.h>                // for MeshInport
#include <inviwo/core/ports/outportiterable.h>         // for OutportIterable
#include <inviwo/core/processors/processor.h>          // for Processor
#include <inviwo/core/processors/processorinfo.h>      // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>     // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>      // for Tags, Tags::GL
#include <inviwo/core/properties/cameraproperty.h>     // for CameraProperty
#include <inviwo/core/properties/compositeproperty.h>  // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>  // for InvalidationLevel, Invalidatio...
#include <inviwo/core/properties/listproperty.h>       // for ListProperty
#include <inviwo/core/properties/ordinalproperty.h>    // for IntProperty, OrdinalProperty
#include <inviwo/core/properties/property.h>           // for Property
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/properties/propertyownerobserver.h>  // for PropertyOwnerObservable
#include <inviwo/core/properties/propertysemantics.h>      // for PropertySemantics, PropertySem...
#include <inviwo/core/properties/stringproperty.h>         // for StringProperty
#include <inviwo/core/util/assertion.h>                    // for IVW_ASSERT
#include <inviwo/core/util/glmvec.h>                       // for vec3, vec2, vec4, uvec3
#include <inviwo/core/util/staticstring.h>                 // for operator+
#include <inviwo/core/util/stringconversion.h>             // for trim, htmlEncode
#include <inviwo/core/util/utilities.h>                    // for findUniqueIdentifier
#include <inviwo/core/util/raiiutils.h>                    // for OnScopeExit
#include <inviwo/core/util/rendercontext.h>                // for RenderContext
#include <inviwo/core/util/stdextensions.h>                // for overloaded
#include <inviwo/core/util/foreacharg.h>                   // for forEachArg
#include <modules/opengl/geometry/meshgl.h>                // for MeshGL
#include <modules/opengl/inviwoopengl.h>                   // for GL_DEPTH_TEST, GL_ONE_MINUS_SR...
#include <modules/opengl/openglutils.h>                    // for BlendModeState, GlBoolState
#include <modules/opengl/rendering/meshdrawergl.h>         // for MeshDrawerGL::DrawObject, Mesh...
#include <modules/opengl/shader/shader.h>                  // for Shader, Shader::Build
#include <modules/opengl/shader/shaderobject.h>            // for ShaderObject
#include <modules/opengl/shader/shaderresource.h>          // for StringShaderResource
#include <modules/opengl/shader/shadersegment.h>           // for ShaderSegment, ShaderSegment::...
#include <modules/opengl/shader/shadertype.h>              // for ShaderType, ShaderType::Fragment
#include <modules/opengl/shader/shaderutils.h>             // for addShaderDefines, setShaderUni...
#include <modules/opengl/texture/textureutils.h>           // for activateTargetAndClearOrCopySo...
#include <modules/opengl/buffer/buffergl.h>

#include <algorithm>    // for for_each, min_element
#include <string>       // for basic_string, operator==, string
#include <string_view>  // for string_view, operator==
#include <utility>      // for move, pair, swap
#include <ranges>
#include <numeric>
#include <tuple>

#include <fmt/core.h>      // for format, basic_string_view, for...
#include <fmt/format.h>    // for formatbuf<>::int_type, formatb...
#include <glm/gtx/io.hpp>  // for operator<<

namespace inviwo {

const ProcessorInfo InstanceRenderer::processorInfo_{
    "org.inviwo.InstanceRenderer",  // Class identifier
    "Instance Renderer",            // Display name
    "Mesh Rendering",               // Category
    CodeState::Stable,              // Code state
    Tags::GL,                       // Tags
    R"(
        Renders multiple instances of a mesh.
        Each instance can be modified using uniform data provided from a set of dynamic inports
        holding vectors of data. The the number of inport and types can be controlled using
        a List Property.
        
        The rendering will happen along these lines:
        
            for (auto uniforms : zip(dynamic vector data ports)) {
                for(uniform : uniforms) {
                    shader.setUniform(uniform.name, uniform.value);
                }
                shader.draw();
            }
        
        How the uniforms are applied in the shader can be specified in a set of properties.
        
        Example network:
        [basegl/instance_renderer.inv](file:~modulePath~/data/workspaces/instance_renderer.inv)
    )"_unindentHelp};

const ProcessorInfo& InstanceRenderer::getProcessorInfo() const { return processorInfo_; }

namespace irplaceholder {

constexpr ShaderSegment::Placeholder uniform{"#pragma IVW_SHADER_SEGMENT_PLACEHOLDER_UNIFORM",
                                             "uniform"};
constexpr ShaderSegment::Placeholder transforms{"#pragma IVW_SHADER_SEGMENT_PLACEHOLDER_TRANSFORMS",
                                                "transforms"};
constexpr ShaderSegment::Placeholder commonCode{"#pragma IVW_SHADER_SEGMENT_PLACEHOLDER_COMMONCODE",
                                                "commoncode"};
constexpr ShaderSegment::Placeholder setupVert{"#pragma IVW_SHADER_SEGMENT_PLACEHOLDER_SETUP",
                                               "setup"};
}  // namespace irplaceholder

namespace {

constexpr std::string_view vertexShader = util::trim(R"(
#include "utils/pickingutils.glsl"
#include "utils/structs.glsl"

mat4 rotate(vec3 axis, float angle) {
  axis = normalize(axis);
  float s = sin(angle);
  float c = cos(angle);
  float oc = 1.0 - c;

  return mat4(
    oc * axis.x * axis.x + c,           oc * axis.y * axis.x + axis.z * s,  oc * axis.z * axis.x - axis.y * s,  0.0,
    oc * axis.x * axis.y - axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.z * axis.y + axis.x * s,  0.0,
    oc * axis.x * axis.z + axis.y * s,  oc * axis.y * axis.z - axis.x * s,  oc * axis.z * axis.z + c,           0.0,
    0.0,                                0.0,                                0.0,                                1.0
  );
}

mat4 scale(vec3 scale) {
  return mat4(
    scale.x, 0.0, 0.0, 0.0,
    0.0, scale.y, 0.0, 0.0,
    0.0, 0.0, scale.z, 0.0,
    0.0, 0.0, 0.0,     1.0
  );
}

layout(location = 7) in uint in_PickId;

uniform GeometryParameters geometry;
uniform CameraParameters camera;

#pragma IVW_SHADER_SEGMENT_PLACEHOLDER_UNIFORM

out vec4 color;
out vec3 normal;
out vec3 texCoord;
out vec4 worldPosition;
flat out vec4 picking;
 
#pragma IVW_SHADER_SEGMENT_PLACEHOLDER_SETUP
 
void main() {
#pragma IVW_SHADER_SEGMENT_PLACEHOLDER_COMMONCODE

#pragma IVW_SHADER_SEGMENT_PLACEHOLDER_TRANSFORMS

    gl_Position = camera.worldToClip * worldPosition;
}
)");

constexpr std::string_view fragmentShader = util::trim(R"(
#include "utils/shading.glsl"

uniform LightParameters lighting;
uniform CameraParameters camera;

in vec4 color;
in vec3 normal;
in vec3 texCoord;
in vec4 worldPosition;
flat in vec4 picking;
 
void main() {
    // Prevent invisible fragments from blocking other objects (e.g., depth/picking)
    if (color.a == 0) { discard; }

    vec4 fragColor = color;
    vec3 toCameraDir = camera.position - worldPosition.xyz;

    fragColor.rgb = APPLY_LIGHTING(lighting, color.rgb, color.rgb, vec3(1.0f), worldPosition.xyz,
                                   normalize(normal), normalize(toCameraDir));
    FragData0 = fragColor;
    PickingData = picking;
}
)");

}  // namespace

namespace detail {

DynPort::DynPort(InstanceRenderer* theRenderer, std::unique_ptr<Inport> aPort,
                 std::function<std::optional<size_t>()> aSize,
                 std::function<void(Shader&, size_t)> aSet,
                 std::function<void(ShaderObject&)> aAddUniform)
    : renderer{theRenderer}
    , port{std::move(aPort)}
    , size{aSize}
    , set{aSet}
    , addUniform{aAddUniform} {

    renderer->addPort(*port);
}

DynPort::DynPort(DynPort&& rhs)
    : renderer{nullptr}
    , port{nullptr}
    , size{std::move(rhs.size)}
    , set{std::move(rhs.set)}
    , addUniform{std::move(rhs.addUniform)} {
    std::swap(rhs.renderer, renderer);
    std::swap(rhs.port, port);
}

DynPort& DynPort::operator=(DynPort&& that) {
    if (this != &that) {
        std::swap(that.renderer, renderer);
        std::swap(that.port, port);
        std::swap(that.size, size);
        std::swap(that.set, set);
        std::swap(that.addUniform, addUniform);
    }
    return *this;
}
DynPort::~DynPort() {
    if (port) renderer->removePort(port.get());
}

template <typename T>
DynPort createDynPortManager(InstanceRenderer* theRenderer, std::string_view identifier,
                             StringProperty* uniform) {

    auto port = std::make_unique<DataInport<std::vector<T>>>(identifier);
    port->setOptional(true);
    auto size = [p = port.get()]() -> std::optional<size_t> {
        if (p->hasData()) {
            return p->getData()->size();
        } else {
            return std::nullopt;
        }
    };
    auto set = [p = port.get(), u = uniform](Shader& shader, size_t index) {
        if (p->hasData()) {
            shader.setUniform(u->get(), (*p->getData())[index]);
        }
    };
    auto addUniform = [u = uniform](ShaderObject& so) {
        so.addSegment(ShaderSegment{
            irplaceholder::uniform, u->get(),
            fmt::format("uniform {0} {1} = {0}(0);", utilgl::glslTypeName<T>(), u->get())});
    };

    return DynPort{theRenderer, std::move(port), std::move(size), std::move(set),
                   std::move(addUniform)};
}

DynUniform::DynUniform(std::function<void(Shader&, TextureUnitContainer&)> aSetAndBind,
                       std::function<void(ShaderObject&)> aAddUniform)
    : setAndBind{aSetAndBind}, addUniform{aAddUniform} {}

DynUniform::DynUniform(DynUniform&& rhs)
    : setAndBind{std::move(rhs.setAndBind)}, addUniform{std::move(rhs.addUniform)} {}

DynUniform::~DynUniform() = default;

DynUniform& DynUniform::operator=(DynUniform&& that) {
    if (this != &that) {
        std::swap(that.setAndBind, setAndBind);
        std::swap(that.addUniform, addUniform);
    }
    return *this;
}

}  // namespace detail

InstanceRenderer::InstanceRenderer()
    : Processor()
    , inport_("mesh", "Mesh to be drawn multiple times"_help)
    , background_("background", "Background image (optional)"_help)
    , outport_("image", "The rendered image"_help)
    , uniforms_{"uniforms",
                "Uniforms",
                R"(
        Add and remove uniforms for the instance rendering. Uniforms of different
        types can be added. The property displayname will be used as uniform name.
        The display name can be changed from the context menu or by doubleclicking
        the label.)"_unindentHelp,
                uniformPrefabs(),
                0,
                ListPropertyUIFlag::Add | ListPropertyUIFlag::Remove,
                InvalidationLevel::InvalidResources}
    , dynUniforms_{}
    , ports_{"ports", "Ports", R"(
        Add and remove vector ports for the instance rendering. The rendering will render as many
        instances as there are elements in the port with the least elements. Ports of different
        types can be added.)"_unindentHelp,
             portPrefabs()}
    , vecPorts_{}
    , camera_("camera", "Camera",
              [this]() -> std::optional<mat4> {
                  rendercontext::activateDefault();
                  return render(true);
              })
    , trackball_(&camera_)
    , lightingProperty_("lighting", "Lighting", &camera_)

    , setupVert_{"setupVert",
                 "Setup Vertex Shader",
                 "Extra vertex shader code can be added here, for example to include other "
                 "files, or to add short functions."_help,
                 "",
                 InvalidationLevel::InvalidResources,
                 PropertySemantics::Multiline}
    , commonCode_{"commonCode",
                  "Common code",
                  "Extra vertex shader code can be added here."_help,
                  "",
                  InvalidationLevel::InvalidResources,
                  PropertySemantics::Multiline}
    , transforms_{{{"color", "Color",
                    "Equation for the vertex color, defaults to the mesh color i.e. "
                    "'in_Color'. Expects a vec4.)"_help,
                    "in_Color", InvalidationLevel::InvalidResources, PropertySemantics::Multiline},
                   {"texCoord", "Texture Coordinate",
                    "Equation for the vertex texture coordinate, defaults to the mesh "
                    "texture coordinate i.e. 'in_TexCoord'. Expects a vec3."_help,
                    "in_TexCoord", InvalidationLevel::InvalidResources,
                    PropertySemantics::Multiline},
                   {"worldPosition", "World Position",
                    "Equation for the vertex world position, defaults to ´geometry.dataToWorld "
                    "* in_Vertex'. Apply a translation based on a data from a port here to move "
                    "each instance to a unique position for example. Expects a vec4."_help,
                    "geometry.dataToWorld * in_Vertex", InvalidationLevel::InvalidResources,
                    PropertySemantics::Multiline},
                   {"normal", "Normal",
                    "Equation for the vertex normal, defaults to "
                    "'geometry.dataToWorldNormalMatrix * in_Normal'. Expects a vec3."_help,
                    "geometry.dataToWorldNormalMatrix * in_Normal",
                    InvalidationLevel::InvalidResources, PropertySemantics::Multiline},
                   {"picking", "Picking",
                    "Equation for the vertex picking color, defaults to "
                    "vec4(pickingIndexToColor(in_PickId), 1.0). Expects a vec4."_help,
                    "vec4(pickingIndexToColor(in_PickId), 1.0)",
                    InvalidationLevel::InvalidResources, PropertySemantics::Multiline}}}
    , vert_{std::make_shared<StringShaderResource>("InstanceRenderer.vert", vertexShader)}
    , frag_{std::make_shared<StringShaderResource>("InstanceRenderer.frag", fragmentShader)}
    , shader_{{{ShaderType::Vertex, vert_}, {ShaderType::Fragment, frag_}}, Shader::Build::No} {

    addPorts(inport_);
    addPort(background_).setOptional(true);
    addPort(outport_);

    addProperties(uniforms_, ports_, setupVert_, commonCode_);
    for (auto& transform : transforms_) {
        addProperty(transform);
    }
    addProperties(camera_, lightingProperty_, trackball_);
    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });

    uniforms_.PropertyOwnerObservable::addObserver(this);
    ports_.PropertyOwnerObservable::addObserver(this);
}

std::vector<std::unique_ptr<Property>> InstanceRenderer::uniformPrefabs() {
    std::vector<std::unique_ptr<Property>> res;

    res.push_back(std::make_unique<IntProperty>("int", "intValue"));
    res.push_back(std::make_unique<IntVec2Property>("ivec2", "ivec2Value"));
    res.push_back(std::make_unique<IntVec3Property>("ivec3", "ivec3Value"));
    res.push_back(std::make_unique<IntVec4Property>("ivec4", "ivec4Value"));

    res.push_back(std::make_unique<FloatProperty>("float", "floatValue"));
    res.push_back(std::make_unique<FloatVec2Property>("vec2", "vec2Value"));
    res.push_back(std::make_unique<FloatVec3Property>("vec3", "vec3Value"));
    res.push_back(std::make_unique<FloatVec4Property>("vec4", "vec4Value"));

    res.push_back(std::make_unique<FloatMat2Property>("mat2", "mat2Value"));
    res.push_back(std::make_unique<FloatMat3Property>("mat3", "mat3Value"));
    res.push_back(std::make_unique<FloatMat4Property>("mat4", "mat4Value"));

    res.push_back(std::make_unique<TransferFunctionProperty>("tf", "tf"));

    return res;
}

std::vector<std::unique_ptr<Property>> InstanceRenderer::portPrefabs() {
    std::vector<std::unique_ptr<Property>> res;

    for (int i = 1; i <= 4; ++i) {
        auto comp = std::make_unique<CompositeProperty>(fmt::format("floatVec{}Port", i),
                                                        fmt::format("Float Vec{} Port", i));
        comp->addProperty(std::make_unique<IntProperty>("components", "Components", i))
            ->setReadOnly(true)
            .setSemantics(PropertySemantics::Text);
        comp->addProperty(std::make_unique<StringProperty>("format", "Format", "Float"))
            ->setReadOnly(true);
        comp->addProperty(std::make_unique<StringProperty>("uniform", "Uniform", "value",
                                                           InvalidationLevel::InvalidResources));

        res.push_back(std::move(comp));
    }

    for (int i = 2; i <= 4; ++i) {
        auto comp = std::make_unique<CompositeProperty>(fmt::format("floatMat{}Port", i),
                                                        fmt::format("Float Mat{} Port", i));
        comp->addProperty(
                std::make_unique<IntVec2Property>("components", "Components", ivec2{i, i}))
            ->setReadOnly(true)
            .setSemantics(PropertySemantics::Text);
        comp->addProperty(std::make_unique<StringProperty>("format", "Format", "Float"))
            ->setReadOnly(true);
        comp->addProperty(std::make_unique<StringProperty>("uniform", "Uniform", "value",
                                                           InvalidationLevel::InvalidResources));

        res.push_back(std::move(comp));
    }

    return res;
}

InstanceRenderer::~InstanceRenderer() { ports_.PropertyOwnerObservable::removeObserver(this); }

void InstanceRenderer::onSetDisplayName(Property*, const std::string&) {
    invalidate(InvalidationLevel::InvalidResources);
}

void InstanceRenderer::onDidAddProperty(Property* property, size_t) {
    if (property->getOwner() == &ports_) {
        onDidAddPort(property);
    } else if (property->getOwner() == &uniforms_) {
        onDidAddUniform(property);
    }
}

void InstanceRenderer::onDidAddUniform(Property* property) {
    using types = std::tuple<IntProperty, IntVec2Property, IntVec3Property, IntVec4Property,
                             FloatProperty, FloatVec2Property, FloatVec3Property, FloatVec4Property,
                             FloatMat2Property, FloatMat3Property, FloatMat4Property>;
    util::for_each_type<types>{}([&]<typename T>() {
        if (auto prop = dynamic_cast<T*>(property)) {
            dynUniforms_.emplace_back(
                [prop](Shader& shader, TextureUnitContainer&) {
                    auto name = util::stripIdentifier(prop->getDisplayName());
                    shader.setUniform(name, prop->get());
                },
                [prop](ShaderObject& so) {
                    const auto type = utilgl::glslTypeName<typename T::value_type>();
                    auto name = util::stripIdentifier(prop->getDisplayName());
                    so.addSegment(ShaderSegment{irplaceholder::uniform, name,
                                                fmt::format("uniform {} {};", type, name)});
                });
        }
    });

    if (auto tfProp = dynamic_cast<TransferFunctionProperty*>(property)) {
        dynUniforms_.emplace_back(
            [tfProp](Shader& shader, TextureUnitContainer& units) {
                auto name = util::stripIdentifier(tfProp->getDisplayName());
                auto& unit = units.emplace_back();
                utilgl::bindTexture(*tfProp, unit);
                shader.setUniform(name, unit);
            },
            [tfProp](ShaderObject& so) {
                auto name = util::stripIdentifier(tfProp->getDisplayName());
                so.addSegment(ShaderSegment{irplaceholder::uniform, name,
                                            fmt::format("uniform sampler2D {};", name)});
            });
    }

    property->addObserver(this);

    invalidate(InvalidationLevel::InvalidResources);
}

void InstanceRenderer::onDidAddPort(Property* property) {
    auto comp = dynamic_cast<CompositeProperty*>(property);
    IVW_ASSERT(comp, "should always exist");

    auto uniform = dynamic_cast<StringProperty*>(comp->getPropertyByIdentifier("uniform"));
    IVW_ASSERT(uniform, "should always exist");

    const auto uniqueID = util::findUniqueIdentifier(
        uniform->get(),
        [&](std::string_view id) {
            for (auto prop : ports_) {
                if (prop == property) continue;

                auto comp = dynamic_cast<CompositeProperty*>(prop);
                auto uniform =
                    dynamic_cast<StringProperty*>(comp->getPropertyByIdentifier("uniform"));
                if (uniform->get() == id) return false;
            }
            return true;
        },
        "");
    uniform->set(uniqueID);

    auto components1D = dynamic_cast<IntProperty*>(comp->getPropertyByIdentifier("components"));
    auto components2D = dynamic_cast<IntVec2Property*>(comp->getPropertyByIdentifier("components"));
    IVW_ASSERT(components1D || components2D, "should always exist");

    const auto& id = property->getIdentifier();
    if (components1D) {
        switch (components1D->get()) {
            case 1:
                vecPorts_.push_back(detail::createDynPortManager<float>(this, id, uniform));
                break;
            case 2:
                vecPorts_.push_back(detail::createDynPortManager<vec2>(this, id, uniform));
                break;
            case 3:
                vecPorts_.push_back(detail::createDynPortManager<vec3>(this, id, uniform));
                break;
            case 4:
                vecPorts_.push_back(detail::createDynPortManager<vec4>(this, id, uniform));
                break;
        }
    } else if (components2D) {
        switch (components2D->get().x) {
            case 2:
                vecPorts_.push_back(detail::createDynPortManager<mat2>(this, id, uniform));
                break;
            case 3:
                vecPorts_.push_back(detail::createDynPortManager<mat3>(this, id, uniform));
                break;
            case 4:
                vecPorts_.push_back(detail::createDynPortManager<mat4>(this, id, uniform));
                break;
        }
    }

    invalidate(InvalidationLevel::InvalidResources);
}

void InstanceRenderer::onWillRemoveProperty(Property* property, size_t index) {
    if (property->getOwner() == &ports_) {
        onDidRemovePort(property, index);
    } else if (property->getOwner() == &uniforms_) {
        onDidRemoveUniform(property, index);
    }
}

void InstanceRenderer::onDidRemoveUniform(Property* /*property*/, size_t index) {
    if (index < dynUniforms_.size()) {
        dynUniforms_.erase(dynUniforms_.begin() + index);
        invalidate(InvalidationLevel::InvalidResources);
    }
}

void InstanceRenderer::onDidRemovePort(Property* /*property*/, size_t index) {
    if (index < vecPorts_.size()) {
        vecPorts_.erase(vecPorts_.begin() + index);
        invalidate(InvalidationLevel::InvalidResources);
    }
}

void InstanceRenderer::initializeResources() {
    utilgl::addShaderDefines(shader_, lightingProperty_);

    auto* vso = shader_.getVertexShaderObject();
    vso->clearSegments();

    vso->addSegment(
        ShaderSegment{irplaceholder::setupVert, setupVert_.getIdentifier(), setupVert_.get()});

    vso->addSegment(
        ShaderSegment{irplaceholder::commonCode, commonCode_.getIdentifier(), commonCode_.get()});

    std::for_each(vecPorts_.begin(), vecPorts_.end(), [&](auto& port) { port.addUniform(*vso); });

    std::for_each(dynUniforms_.begin(), dynUniforms_.end(),
                  [&](auto& uniform) { uniform.addUniform(*vso); });

    size_t prio = 100;
    for (auto& transform : transforms_) {
        vso->addSegment(ShaderSegment{
            irplaceholder::transforms, transform.getIdentifier(),
            fmt::format("{} = {};", transform.getIdentifier(), transform.get()), prio});
        prio += 100;
    }

    std::array<const GLchar*, 1> feedbackVaryings = {"worldPosition"};
    shader_.setTransformFeedbackVaryings(feedbackVaryings, GL_INTERLEAVED_ATTRIBS);

    shader_.build();
}

namespace {

size_t numberOfVertices(DrawType dt, ConnectivityType ct, size_t nIndices) {
    return util::numberOfVerticesForPrimitive(dt) * util::numberOfPrimitives(dt, ct, nIndices);
}
size_t numberOfVertices(Mesh::MeshInfo mi, size_t nIndices) {
    return numberOfVertices(mi.dt, mi.ct, nIndices);
}
size_t numberOfVertices(const MeshGL& meshGL, size_t index) {
    auto mi = meshGL.getMeshInfoForIndexBuffer(index);
    auto nIndices = meshGL.getIndexBuffer(index)->getSize();
    return numberOfVertices(mi, nIndices);
}
size_t numberOfVertices(const MeshGL& meshGL) {
    const std::size_t numIndexBuffers = meshGL.getIndexBufferCount();
    if (numIndexBuffers > 0) {
        auto indices = std::ranges::iota_view{size_t{0}, numIndexBuffers};
        return std::transform_reduce(indices.begin(), indices.end(), size_t{0}, std::plus<>{},
                                     [&](size_t ib) { return numberOfVertices(meshGL, ib); });
    } else {
        auto nIndices = meshGL.getBufferGL(0)->getSize();
        return numberOfVertices(meshGL.getDefaultMeshInfo(), nIndices);
    }
}

auto primitiveMode(DrawType dt) {
    switch (dt) {
        case DrawType::Points:
            return GL_POINTS;
        case DrawType::Lines:
            return GL_LINES;
        case DrawType::Triangles:
            return GL_TRIANGLES;
        default:
            return GL_TRIANGLES;
    }
}

size_t commonNumberOfElements(auto& ports) {
    auto valid = ports | std::views::filter(
                             [](const auto& port) -> bool { return port.size().has_value(); });
    if (valid.empty()) return 0;

    const auto minIt = std::ranges::min_element(valid, std::less<>{},
                                                [](const auto& port) { return *port.size(); });
    return *minIt->size();
}

void forEach(const MeshGL& meshGL, Shader& shader, auto& ports, size_t nInstances,
             auto&& callback) {
    if (meshGL.empty()) return;

    const std::size_t numIndexBuffers = meshGL.getIndexBufferCount();

    for (size_t i = 0; i < nInstances; ++i) {
        std::for_each(ports.begin(), ports.end(), [&](auto& port) { port.set(shader, i); });
        for (std::size_t ib = 0; ib < numIndexBuffers; ++ib) {
            const auto& indexBuffer = *meshGL.getIndexBuffer(ib);
            if (indexBuffer.getSize() == 0) continue;
            const auto meshInfo = meshGL.getMeshInfoForIndexBuffer(ib);
            indexBuffer.bind();
            callback(indexBuffer.getSize(), meshInfo, [&]() {
                glDrawElements(MeshDrawerGL::getGLDrawMode(meshInfo),
                               static_cast<GLsizei>(indexBuffer.getSize()),
                               indexBuffer.getFormatType(), nullptr);
            });
        }
        if (numIndexBuffers == 0) {
            auto size = meshGL.getBufferGL(0)->getSize();
            const auto meshInfo = meshGL.getDefaultMeshInfo();
            callback(size, meshInfo, [&]() {
                glDrawArrays(MeshDrawerGL::getGLDrawMode(meshInfo), 0, static_cast<GLsizei>(size));
            });
        }
    }
}

mat4 calculateBoundingBox(std::span<const vec4> data) {
    using Pair = std::pair<vec4, vec4>;
    const auto minMax = std::accumulate(data.begin(), data.end(),
                                        Pair{DataFormat<vec4>::max(), DataFormat<vec4>::lowest()},
                                        [](const Pair& mm, const vec4& v) -> Pair {
                                            return {glm::min(mm.first, v), glm::max(mm.second, v)};
                                        });
    auto boundingBox = glm::scale(vec3{minMax.second} - vec3{minMax.first});
    boundingBox[3] = vec4(vec3{minMax.first}, 1.0f);
    return boundingBox;
}

}  // namespace

void InstanceRenderer::process() { render(false); }

std::optional<mat4> InstanceRenderer::render(bool enableBoundingBoxCalc) {
    utilgl::activateTargetAndClearOrCopySource(outport_, background_);
    util::OnScopeExit deactivate(utilgl::deactivateCurrentTarget);

    const auto nInstances = commonNumberOfElements(vecPorts_);
    if (nInstances == 0) return std::nullopt;

    const auto& mesh = *inport_.getData();
    const auto& meshGL = *mesh.getRepresentation<MeshGL>();

    utilgl::Activate activate{&shader_};
    utilgl::GlBoolState depthTest(GL_DEPTH_TEST, true);
    utilgl::BlendModeState blendModeStateGL(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    utilgl::setUniforms(shader_, camera_, lightingProperty_);
    utilgl::setShaderUniforms(shader_, mesh, "geometry");
    utilgl::Enable<MeshGL> enable{&meshGL};

    TextureUnitContainer units;
    for (auto& uniform : dynUniforms_) {
        uniform.setAndBind(shader_, units);
    }

    if (enableBoundingBoxCalc) {
        const auto nVertices = nInstances * numberOfVertices(meshGL);
        BufferObject feedbackBuffer{nVertices * sizeof(vec4), GLFormats::get(DataVec4Float32::id()),
                                    GL_STATIC_READ, GL_TRANSFORM_FEEDBACK_BUFFER};
        size_t byteOffset = 0;
        forEach(meshGL, shader_, vecPorts_, nInstances,
                [&](size_t size, Mesh::MeshInfo mi, const auto& draw) {
                    const auto bufferByteSize = sizeof(vec4) * numberOfVertices(mi, size);
                    feedbackBuffer.bindRange(0, byteOffset, bufferByteSize);
                    glBeginTransformFeedback(primitiveMode(mi.dt));
                    draw();
                    glEndTransformFeedback();
                    byteOffset += bufferByteSize;
                });

        feedbackBuffer.bindBase(0);
        if (auto buffer = glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, GL_READ_ONLY)) {
            auto boundingBox =
                calculateBoundingBox(std::span{static_cast<const vec4*>(buffer), nVertices});
            glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);
            return boundingBox;
        }
    } else {
        forEach(meshGL, shader_, vecPorts_, nInstances,
                [&](size_t, Mesh::MeshInfo, const auto& draw) { draw(); });
    }

    return std::nullopt;
}

}  // namespace inviwo
