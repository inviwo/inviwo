/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
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

#include <modules/basegl/algorithm/entryexitpoints.h>

#include <inviwo/core/datastructures/camera/camera.h>          // for mat4, Camera
#include <inviwo/core/datastructures/coordinatetransformer.h>  // for SpatialCameraCoordinateTra...
#include <inviwo/core/datastructures/geometry/mesh.h>          // for Mesh
#include <inviwo/core/datastructures/image/image.h>            // for Image
#include <inviwo/core/datastructures/image/imagetypes.h>       // for ImageType, ImageType::AllL...
#include <inviwo/core/datastructures/image/layer.h>            // for Layer
#include <inviwo/core/datastructures/volume/volume.h>          // for Volume
#include <inviwo/core/util/dispatcher.h>                       // for Dispatcher
#include <inviwo/core/util/formats.h>                          // for DataVec3Float32
#include <inviwo/core/util/glmvec.h>                           // for vec3, size2_t
#include <modules/opengl/image/imagegl.h>                      // for ImageGL
#include <modules/opengl/image/layergl.h>                      // for LayerGL
#include <modules/opengl/inviwoopengl.h>                       // for GL_BACK, GL_FRONT, GL_GREATER
#include <modules/opengl/openglutils.h>                        // for CullFaceState, DepthFuncState
#include <modules/opengl/rendering/meshdrawergl.h>             // for MeshDrawerGL::DrawObject
#include <modules/opengl/shader/shader.h>                      // for Shader, Shader::Build
#include <modules/opengl/shader/shadermanager.h>               // for ShaderManager
#include <modules/opengl/shader/shaderobject.h>                // for ShaderObject
#include <modules/opengl/shader/stringshaderresource.h>        // for StringShaderResource, Shad...
#include <modules/opengl/shader/shadertype.h>                  // for ShaderType, ShaderType::Fr...
#include <modules/opengl/texture/textureunit.h>                // for TextureUnit
#include <modules/opengl/texture/textureutils.h>               // for clearCurrentTarget, deacti...

#include <array>        // for array, array<>::value_type
#include <chrono>       // for literals
#include <cstddef>      // for size_t
#include <string_view>  // for string_view
#include <type_traits>  // for remove_extent_t
#include <utility>      // for pair
#include <vector>       // for vector

#include <glm/geometric.hpp>                    // for cross, dot, normalize
#include <glm/gtx/handed_coordinate_space.hpp>  // for rightHanded
#include <glm/mat4x4.hpp>                       // for operator*, mat
#include <glm/vec2.hpp>                         // for operator!=
#include <glm/vec3.hpp>                         // for operator*, operator-, vec
#include <glm/vec4.hpp>                         // for operator*, operator+

namespace inviwo {

namespace algorithm {

EntryExitPointsHelper::EntryExitPointsHelper() = default;

void EntryExitPointsHelper::operator()(Image& entryPoints, Image& exitPoints, const Camera& camera,
                                       const Mesh& mesh, CapNearClip capNearClip,
                                       IncludeNormals includeNormals) {
    if (capNearClip == CapNearClip::Yes) {
        createCappedEntryExitPoints(*entryPoints.getEditableRepresentation<ImageGL>(),
                                    *exitPoints.getEditableRepresentation<ImageGL>(), camera, mesh,
                                    includeNormals, ApplyTransformation::No);
    } else {
        createEntryExitPoints(*entryPoints.getEditableRepresentation<ImageGL>(),
                              *exitPoints.getEditableRepresentation<ImageGL>(), camera, mesh,
                              includeNormals, ApplyTransformation::No);
    }
}

void EntryExitPointsHelper::operator()(ImageGL& entryPoints, ImageGL& exitPoints,
                                       const Camera& camera, const Mesh& mesh,
                                       CapNearClip capNearClip, IncludeNormals includeNormals) {
    if (capNearClip == CapNearClip::Yes) {
        createCappedEntryExitPoints(entryPoints, exitPoints, camera, mesh, includeNormals,
                                    ApplyTransformation::No);
    } else {
        createEntryExitPoints(entryPoints, exitPoints, camera, mesh, includeNormals,
                              ApplyTransformation::No);
    }
}

void EntryExitPointsHelper::operator()(Image& entryPoints, Image& exitPoints, const Camera& camera,
                                       const Volume& volume, const Mesh& mesh,
                                       CapNearClip capNearClip, IncludeNormals includeNormals) {
    const mat4 meshDataToVolumeData =
        volume.getCoordinateTransformer(camera).getWorldToDataMatrix() *
        mesh.getCoordinateTransformer().getDataToWorldMatrix();

    if (capNearClip == CapNearClip::Yes) {
        createCappedEntryExitPoints(*entryPoints.getEditableRepresentation<ImageGL>(),
                                    *exitPoints.getEditableRepresentation<ImageGL>(), camera, mesh,
                                    includeNormals, ApplyTransformation::Yes, meshDataToVolumeData);
    } else {
        createEntryExitPoints(*entryPoints.getEditableRepresentation<ImageGL>(),
                              *exitPoints.getEditableRepresentation<ImageGL>(), camera, mesh,
                              includeNormals, ApplyTransformation::Yes, meshDataToVolumeData);
    }
}

void EntryExitPointsHelper::operator()(ImageGL& entryPoints, ImageGL& exitPoints,
                                       const Camera& camera, const Volume& volume, const Mesh& mesh,
                                       CapNearClip capNearClip, IncludeNormals includeNormals) {

    const mat4 meshDataToVolumeData =
        volume.getCoordinateTransformer(camera).getWorldToDataMatrix() *
        mesh.getCoordinateTransformer().getDataToWorldMatrix();

    if (capNearClip == CapNearClip::Yes) {
        createCappedEntryExitPoints(entryPoints, exitPoints, camera, mesh, includeNormals,
                                    ApplyTransformation::Yes, meshDataToVolumeData);
    } else {
        createEntryExitPoints(entryPoints, exitPoints, camera, mesh, includeNormals,
                              ApplyTransformation::Yes, meshDataToVolumeData);
    }
}

std::shared_ptr<std::function<void()>> EntryExitPointsHelper::onReload(
    std::function<void()> callback) {
    return onReloadCallback_.add(callback);
}

void EntryExitPointsHelper::createEntryExitPoints(ImageGL& entryPoints, ImageGL& exitPoints,
                                                  const Camera& camera, const Mesh& mesh,
                                                  IncludeNormals includeNormals,
                                                  ApplyTransformation applyTrafo,
                                                  const mat4& meshDataToVolumeData) {

    auto& entryExitShader = getEntryExitShader(includeNormals, applyTrafo);

    entryExitShader.activate();
    const mat4 dataToClipMatrix = mesh.getCoordinateTransformer(camera).getDataToClipMatrix();
    entryExitShader.setUniform("dataToClip", dataToClipMatrix);
    entryExitShader.setUniform("meshDataToVolData", meshDataToVolumeData);

    const bool righthanded = glm::rightHanded(vec3(dataToClipMatrix[0]), vec3(dataToClipMatrix[1]),
                                              vec3(dataToClipMatrix[2]));

    auto drawer = MeshDrawerGL::getDrawObject(&mesh);

    {
        // generate exit points
        utilgl::DepthFuncState depthfunc(GL_GREATER);
        utilgl::ClearDepth clearDepth(0.0f);
        exitPoints.activateBuffer(ImageType::AllLayers);
        utilgl::clearCurrentTarget();
        utilgl::CullFaceState cull(righthanded ? GL_BACK : GL_FRONT);
        drawer.draw();
    }

    {
        // generate entry points
        utilgl::DepthFuncState depthfunc(GL_LESS);
        entryPoints.activateBuffer(ImageType::AllLayers);
        utilgl::clearCurrentTarget();

        utilgl::CullFaceState cull(righthanded ? GL_FRONT : GL_BACK);
        drawer.draw();
    }
    utilgl::deactivateCurrentTarget();
    entryExitShader.deactivate();
}

void EntryExitPointsHelper::createCappedEntryExitPoints(ImageGL& entryPoints, ImageGL& exitPoints,
                                                        const Camera& camera, const Mesh& mesh,
                                                        IncludeNormals includeNormals,
                                                        ApplyTransformation applyTrafo,
                                                        const mat4& meshDataToVolumeData) {

    auto& entryExitShader = getEntryExitShader(includeNormals, applyTrafo);

    entryExitShader.activate();
    const mat4 dataToClipMatrix = mesh.getCoordinateTransformer(camera).getDataToClipMatrix();
    entryExitShader.setUniform("dataToClip", dataToClipMatrix);
    entryExitShader.setUniform("meshDataToVolData", meshDataToVolumeData);

    const bool righthanded = glm::rightHanded(vec3(dataToClipMatrix[0]), vec3(dataToClipMatrix[1]),
                                              vec3(dataToClipMatrix[2]));

    auto drawer = MeshDrawerGL::getDrawObject(&mesh);

    {
        // generate exit points
        utilgl::DepthFuncState depthfunc(GL_GREATER);
        utilgl::ClearDepth clearDepth(0.0f);
        exitPoints.activateBuffer(ImageType::AllLayers);
        utilgl::clearCurrentTarget();
        utilgl::CullFaceState cull(righthanded ? GL_BACK : GL_FRONT);
        drawer.draw();
    }

    {
        // generate entry points
        utilgl::DepthFuncState depthfunc(GL_LESS);
        if (!tmpEntry_ ||
            tmpEntry_->getDataFormat() != entryPoints.getColorLayerGL()->getDataFormat()) {
            tmpEntry_.reset(new Image(entryPoints.getDimensions(),
                                      entryPoints.getColorLayerGL()->getDataFormat()));
            tmpEntry_->addColorLayer(std::make_shared<Layer>(
                entryPoints.getDimensions(), DataVec3Float32::get(), LayerType::Color));

            tmpEntryGL_ = tmpEntry_->getEditableRepresentation<ImageGL>();

            tmpEntry_->updateResource(ResourceMeta{.source = "EntryExitPointsHelper"});
        }

        if (tmpEntry_->getDimensions() != entryPoints.getDimensions()) {
            tmpEntry_->setDimensions(entryPoints.getDimensions());
            tmpEntryGL_ = tmpEntry_->getEditableRepresentation<ImageGL>();
        }

        tmpEntryGL_->activateBuffer(ImageType::AllLayers);
        utilgl::clearCurrentTarget();

        utilgl::CullFaceState cull(righthanded ? GL_FRONT : GL_BACK);
        drawer.draw();
    }

    // render an image plane aligned quad to cap the proxy geometry
    entryPoints.activateBuffer(ImageType::AllLayers);
    utilgl::clearCurrentTarget();

    auto& nearClipShader = getNearClipShader(includeNormals);
    nearClipShader.activate();

    TextureUnit entryColorUnit, entryDepthUnit, entryNormalUnit;
    tmpEntryGL_->getColorLayerGL()->bindTexture(entryColorUnit);
    tmpEntryGL_->getDepthLayerGL()->bindTexture(entryDepthUnit);
    nearClipShader.setUniform("entryColor", entryColorUnit);
    nearClipShader.setUniform("entryDepth", entryDepthUnit);
    if (includeNormals == IncludeNormals::Yes) {
        tmpEntryGL_->getColorLayerGL(1)->bindTexture(entryNormalUnit);
        nearClipShader.setUniform("entryNormal", entryNormalUnit);
    }

    TextureUnit exitColorUnit, exitDepthUnit;
    exitPoints.getColorLayerGL()->bindTexture(exitColorUnit);
    exitPoints.getDepthLayerGL()->bindTexture(exitDepthUnit);
    nearClipShader.setUniform("exitColor", exitColorUnit);
    nearClipShader.setUniform("exitDepth", exitDepthUnit);

    // the rendered plane is specified in camera coordinates
    // thus we must transform from camera to world to texture coordinates
    mat4 clipToTexMat = mesh.getCoordinateTransformer(camera).getClipToDataMatrix();
    nearClipShader.setUniform("NDCToTextureMat", clipToTexMat);
    nearClipShader.setUniform("nearDist", camera.getNearPlaneDist());
    nearClipShader.setUniform("mViewDir", -glm::normalize(camera.getDirection()));

    utilgl::singleDrawImagePlaneRect();
    nearClipShader.deactivate();
    utilgl::deactivateCurrentTarget();
}

Shader& EntryExitPointsHelper::getEntryExitShader(IncludeNormals includeNormals,
                                                  ApplyTransformation applyTrafo) {
    if (!ees_.shader || includeNormals != ees_.includeNormals || applyTrafo != ees_.applyTrafo) {
        ees_.shader = createEntryExitShader(includeNormals, applyTrafo);
        ees_.includeNormals = includeNormals;
        ees_.applyTrafo = applyTrafo;
        ees_.reloadCallback = ees_.shader->onReloadScoped([this]() { onReloadCallback_.invoke(); });
    }
    return *ees_.shader;
}

Shader& EntryExitPointsHelper::getNearClipShader(IncludeNormals includeNormals) {
    if (!ncs_.shader || includeNormals != ncs_.includeNormals) {
        ncs_.shader = createNearClipShader(includeNormals);
        ncs_.includeNormals = includeNormals;
        ncs_.reloadCallback = ncs_.shader->onReloadScoped([this]() { onReloadCallback_.invoke(); });
    }
    return *ncs_.shader;
}

namespace {

// Standard
constexpr std::string_view entryExitVert = R"(
uniform mat4 dataToClip = mat4(1);
out vec4 color;
void main() {
    color = in_Color;
    gl_Position = dataToClip * in_Vertex;
}
)";
constexpr std::string_view entryExitFrag = R"(
in vec4 color;
void main() {
    FragData0 = color;
    PickingData = vec4(0);
}
)";

// Normals
constexpr std::string_view entryExitNormalVert = R"(
uniform mat4 dataToClip = mat4(1);
out vec4 color;
out vec3 normal;
void main() {
    color = in_Color;
    normal = in_Normal;
    gl_Position = dataToClip * in_Vertex;
}
)";
constexpr std::string_view entryExitNormalFrag = R"(
in vec4 color;
in vec3 normal;
void main() {
    FragData0 = color;
    PickingData = vec4(0);
    NormalData = normal;
}
)";

// Mesh
constexpr std::string_view meshEntryExitVert = R"(
uniform mat4 dataToClip = mat4(1);
uniform mat4 meshDataToVolData = mat4(1);
out vec4 color;
void main() {
    color = meshDataToVolData * in_Vertex;
    gl_Position = dataToClip * in_Vertex;
}
)";
constexpr std::string_view meshEntryExitFrag = entryExitFrag;

// Mesh + Normals
constexpr std::string_view meshEntryExitNormalVert = R"(
uniform mat4 dataToClip = mat4(1);
uniform mat4 meshDataToVolData = mat4(1);
out vec4 color;
out vec3 normal;
void main() {
    color = meshDataToVolData * in_Vertex;
    normal = in_Normal;
    gl_Position = dataToClip * in_Vertex;
}
)";
constexpr std::string_view meshEntryExitNormalFrag = entryExitNormalFrag;

}  // namespace

std::shared_ptr<Shader> EntryExitPointsHelper::createEntryExitShader(
    IncludeNormals includeNormals, ApplyTransformation applyTrafo) {
    using namespace std::literals;
    size_t index = 0;
    if (includeNormals == IncludeNormals::Yes) index += 1;
    if (applyTrafo == ApplyTransformation::Yes) index += 2;

    constexpr std::array<std::pair<std::string_view, std::string_view>, 4> keys = {{
        {"entryExit.vert", "entryExit.frag"},                     // default
        {"entryExitNormal.vert", "entryExitNormal.frag"},         // normals
        {"meshEntryExit.vert", "meshEntryExit.frag"},             // mesh
        {"meshEntryExitNormal.vert", "meshEntryExitNormal.frag"}  // mesh + normals
    }};

    constexpr std::array<std::pair<std::string_view, std::string_view>, 4> sources = {{
        {entryExitVert, entryExitFrag},                     // default
        {entryExitNormalVert, entryExitNormalFrag},         // normals
        {meshEntryExitVert, meshEntryExitFrag},             // mesh
        {meshEntryExitNormalVert, meshEntryExitNormalFrag}  // mesh + normals
    }};

    static std::array<std::weak_ptr<Shader>, 4> shaders;
    if (auto shader = shaders[index].lock()) {
        return shader;
    } else {
        using Res = std::vector<std::pair<ShaderType, std::shared_ptr<const ShaderResource>>>;
        auto vert = std::make_shared<StringShaderResource>(keys[index].first, sources[index].first);
        auto frag =
            std::make_shared<StringShaderResource>(keys[index].second, sources[index].second);
        shader = std::make_shared<Shader>(
            Res{{ShaderType::Vertex, vert}, {ShaderType::Fragment, frag}}, Shader::Build::No);

        ShaderManager::getPtr()->addShaderResource(vert);
        ShaderManager::getPtr()->addShaderResource(frag);

        if (includeNormals == IncludeNormals::Yes) {
            shader->getFragmentShaderObject()->addOutDeclaration("NormalData", 2, "vec3");
        }
        shader->build();

        shaders[index] = shader;
        return shader;
    }
}

constexpr std::string_view capnNearClippingVert = R"(
out vec3 texCoord;
void main() {
    texCoord = in_TexCoord;
    gl_Position = in_Vertex;
}
)";

constexpr std::string_view capnNearClippingFrag = R"(
uniform sampler2D entryColor;
uniform sampler2D entryDepth;
uniform sampler2D exitColor;
uniform sampler2D exitDepth;

uniform mat4 NDCToTextureMat; // Normalized device coordinates to volume texture coordinates
uniform float nearDist;
in vec3 texCoord;

void main() {
    float entry = texture(entryDepth, texCoord.xy).r;
    float exit = texture(exitDepth, texCoord.xy).r;
    vec4 color;

    if ((entry > exit) && (exit > 0.0)) {
        // entry points are clipped by near plane
        // Convert texture coordinates to normalized device coordinates (ndc).
        // The z value will always be -1 on the clipping plane
        vec4 cameraCoordinates = vec4(2.0f * texCoord.xy - 1.0f, -1.0f, 1.0f);
        // convert the ndc back to the volume texture coordinates
        color = NDCToTextureMat * cameraCoordinates * nearDist;
        entry = 0.0f;
    } else {
        color = texture(entryColor, texCoord.xy);
    }

    FragData0 = color;
    gl_FragDepth = entry;
}
)";

constexpr std::string_view capnNearClippingNormalFrag = R"(
uniform sampler2D entryColor;
uniform sampler2D entryDepth;
uniform sampler2D entryNormal;
uniform sampler2D exitColor;
uniform sampler2D exitDepth;

uniform mat4 NDCToTextureMat; // Normalized device coordinates to volume texture coordinates
uniform float nearDist;
uniform vec3 mViewDir;

in vec3 texCoord;

void main() {
    float entry = texture(entryDepth, texCoord.xy).r;
    float exit = texture(exitDepth, texCoord.xy).r;
    vec4 color;
    vec3 normal;

    if ((entry > exit) && (exit > 0.0)) {
        // entry points are clipped by near plane
        // Convert texture coordinates to normalized device coordinates (ndc).
        // The z value will always be -1 on the clipping plane
        vec4 cameraCoordinates = vec4(2.0f * texCoord.xy - 1.0f, -1.0f, 1.0f);
        // convert the ndc back to the volume texture coordinates
        color = NDCToTextureMat * cameraCoordinates * nearDist;
        entry = 0.0f;
        normal = mViewDir;
    } else {
        color = texture(entryColor, texCoord.xy);
        normal = texture(entryNormal, texCoord.xy).xyz;
    }

    FragData0 = color;
    gl_FragDepth = entry;
    NormalData = normal;
}
)";

std::shared_ptr<Shader> EntryExitPointsHelper::createNearClipShader(IncludeNormals includeNormals) {
    const size_t index = includeNormals == IncludeNormals::Yes ? 1 : 0;

    constexpr std::array<std::pair<std::string_view, std::string_view>, 2> keys = {{
        {"capnNearClipping.vert", "capnNearClipping.frag"},       // default
        {"capnNearClipping.vert", "capnNearClippingNormal.frag"}  // normals
    }};

    constexpr std::array<std::pair<std::string_view, std::string_view>, 2> sources = {{
        {capnNearClippingVert, capnNearClippingFrag},       // default
        {capnNearClippingVert, capnNearClippingNormalFrag}  // normals
    }};

    static std::array<std::weak_ptr<Shader>, 4> shaders;
    if (auto shader = shaders[index].lock()) {
        return shader;
    } else {
        using Res = std::vector<std::pair<ShaderType, std::shared_ptr<const ShaderResource>>>;
        auto vert = std::make_shared<StringShaderResource>(keys[index].first, sources[index].first);
        auto frag =
            std::make_shared<StringShaderResource>(keys[index].second, sources[index].second);
        shader = std::make_shared<Shader>(
            Res{{ShaderType::Vertex, vert}, {ShaderType::Fragment, frag}}, Shader::Build::No);

        ShaderManager::getPtr()->addShaderResource(vert);
        ShaderManager::getPtr()->addShaderResource(frag);

        if (includeNormals == IncludeNormals::Yes) {
            shader->getFragmentShaderObject()->addOutDeclaration("NormalData", 2, "vec3");
        }
        shader->build();

        shaders[index] = shader;
        return shader;
    }
}

}  // namespace algorithm

}  // namespace inviwo
