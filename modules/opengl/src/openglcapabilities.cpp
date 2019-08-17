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

#include <modules/opengl/openglcapabilities.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/texture/textureunit.h>
#include <inviwo/core/util/formatconversion.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/properties/optionproperty.h>
#include <modules/opengl/openglsettings.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <modules/opengl/shader/shadermanager.h>

namespace inviwo {

// Or minimal opengl version is 3.3 and glsl version 330

bool OpenGLCapabilities::glewInitialized_ = false;
std::string OpenGLCapabilities::preferredProfile_ = "core";
int OpenGLCapabilities::glVersion_ = 0;
std::string OpenGLCapabilities::glVersionStr_ = "0";

OpenGLCapabilities::GLSLShaderVersion::GLSLShaderVersion() : number_(0), profile_("") {}

OpenGLCapabilities::GLSLShaderVersion::GLSLShaderVersion(int num) : number_(num), profile_("") {}

OpenGLCapabilities::GLSLShaderVersion::GLSLShaderVersion(int num, std::string pro)
    : number_(num), profile_(pro) {}

const std::string& OpenGLCapabilities::GLSLShaderVersion::getProfile() const { return profile_; }

int OpenGLCapabilities::GLSLShaderVersion::getVersion() const { return number_; }

std::string OpenGLCapabilities::GLSLShaderVersion::getVersionAsString() const {
    return toString<int>(number_);
}

std::string OpenGLCapabilities::GLSLShaderVersion::getVersionAndProfileAsString() const {
    return (hasProfile() ? getVersionAsString() + " " + profile_ : getVersionAsString());
}

bool OpenGLCapabilities::GLSLShaderVersion::hasProfile() const { return (profile_ != ""); }

bool OpenGLCapabilities::GLSLShaderVersion::sortHighestFirst(GLSLShaderVersion i,
                                                             GLSLShaderVersion j) {
    return (i.getVersion() > j.getVersion());
}

OpenGLCapabilities::OpenGLCapabilities(OpenGLSettings* settings)
    : shadersAreSupported_(false)
    , shadersAreSupportedARB_(false)
    , geometryShadersAreSupported_(false)
    , maxProgramLoopCount_(-1)
    , geometryShadersMaxVertices_(-1)
    , geometryShadersMaxOutputComponents_(-1)
    , geometryShadersMaxTotalOutputComponents_(-1)
    , texSupported_(false)
    , tex3DSupported_(false)
    , texArraySupported_(false)
    , fboSupported_(false)
    , maxTexSize_(-1)
    , max3DTexSize_(-1)
    , maxArrayTexSize_(-1)
    , maxArrayVertexAttribs_(-1)
    , maxColorAttachments_(-1)
    , numTexUnits_(-1) {

    supportedShaderVersions_.clear();

    preferredProfile_ = settings->selectedOpenGLProfile_.getSelectedIdentifier();

    settings->btnOpenGLInfo_.onChange([this]() { printDetailedInfo(); });

    bool hasOutputedGLSLVersionOnce = false;
    settings->selectedOpenGLProfile_.onChange(
        [this, settings, hasOutputedGLSLVersionOnce]() mutable {
            if (setPreferredProfile(settings->selectedOpenGLProfile_.getSelectedValue(),
                                    !hasOutputedGLSLVersionOnce) &&
                hasOutputedGLSLVersionOnce) {
                ShaderManager::getPtr()->rebuildAllShaders();
                if (preferredProfile_ != settings->selectedOpenGLProfile_.getSelectedValue()) {
                    print("Restart application to enable " +
                          settings->selectedOpenGLProfile_.getSelectedValue() + " mode.");
                }
            }
            hasOutputedGLSLVersionOnce = true;
        });
}

OpenGLCapabilities::~OpenGLCapabilities() {
    TextureUnit::deinitialize();

    // reset stuff.
    glewInitialized_ = false;
    preferredProfile_ = "core";
    glVersion_ = 0;
    glVersionStr_ = "0";
}

void OpenGLCapabilities::printInfo() {
    // OpenGL General Info
    print("GPU Vendor: ", glVendorStr_);
    print("GPU Renderer: ", glRenderStr_);
    if (isTexturesSupported()) {
        auto totalMem = getTotalAvailableTextureMem();
        print("Dedicated video memory: ",
              (totalMem > 0 ? util::formatBytesToString(totalMem) : "UNKNOWN"));
    }
    print("OpenGL Version: ", glVersionStr_);
    std::string profile = preferredProfile_;
    profile[0] = static_cast<char>(toupper(profile[0]));
    print("OpenGL Profile: ", profile);

    if (isShadersSupported()) {
        print("GLSL version: ", glslVersionStr_);
    }
}

void OpenGLCapabilities::printDetailedInfo() {
    // OpenGL General Info
    print("GPU Vendor: " + glVendorStr_);
    print("GPU Renderer: " + glRenderStr_);
    print("OpenGL Version: " + glVersionStr_);
    std::string profile = preferredProfile_;
    profile[0] = static_cast<char>(toupper(profile[0]));
    print("OpenGL Profile: " + profile);

    // GLSL
    if (isShadersSupported()) {
        print("GLSL version: ", glslVersionStr_);
        print("Current set global GLSL version: ",
              getCurrentShaderVersion().getVersionAndProfileAsString());
        print("Shaders supported: YES");
    } else if (isShadersSupportedARB()) {
        print("GLSL version: ", glslVersionStr_);
        print("Current set global GLSL version: ",
              getCurrentShaderVersion().getVersionAndProfileAsString());
        print("Shaders supported: YES(ARB)");
    } else {
        print("Shaders supported: NO");
    }

    if (isGeometryShadersSupported()) {
        print("Geometry shaders supported: YES");
        print("Geometry shaders: Max output vertices : ", geometryShadersMaxVertices_);
        print("Geometry shaders: Max output components: ", geometryShadersMaxOutputComponents_);
        print("Geometry shaders: Max total output components: ",
              geometryShadersMaxTotalOutputComponents_);
    } else {
        print("Geometry shaders supported: NO");
    }

    print("Framebuffer objects supported: ", (isFboSupported() ? "YES" : "NO "));
    // Texturing
    print("1D/2D textures supported: ", (isTexturesSupported() ? "YES" : "NO "));
    print("3D textures supported: ", (is3DTexturesSupported() ? "YES" : "NO "));
    print("Array textures supported: ", (isTextureArraysSupported() ? "YES" : "NO "));

    if (isTexturesSupported()) print("Max 1D/2D texture size: ", getMaxTexSize());
    if (is3DTexturesSupported()) print("Max 3D texture size: ", getMax3DTexSize());
    if (isTextureArraysSupported()) print("Max array texture size: ", getMaxArrayTexSize());
    if (isFboSupported()) print("Max color attachments: ", getMaxColorAttachments());

    if (isTexturesSupported()) {
        print("Max number of texture units: ", getNumTexUnits());
        auto totalMem = getTotalAvailableTextureMem();
        print("Total available texture memory: ",
              (totalMem > 0 ? util::formatBytesToString(totalMem) : "UNKNOWN"));
        auto curMem = getCurrentAvailableTextureMem();
        print("Current available texture memory: ",
              (curMem > 0 ? util::formatBytesToString(curMem) : "UNKNOWN"));
    }
}

int OpenGLCapabilities::getOpenGLVersion() { return glVersion_; }

bool OpenGLCapabilities::hasSupportedOpenGLVersion() { return (glVersion_ >= 330); }

bool OpenGLCapabilities::hasOpenGLVersion() { return (glVersion_ > 0); }

void OpenGLCapabilities::initializeGLEW() {
    if (!hasSupportedOpenGLVersion()) {
        std::string preferProfile = getPreferredProfile();
        if (preferProfile == "core") glewExperimental = GL_TRUE;
        GLenum glewError = glewInit();
        if (GLEW_OK == glewError) {
            const GLubyte* glversion = glGetString(GL_VERSION);
            if (glversion == 0) {
                // There was an error retrieving the version. Executing further OpenGl calls may
                // crash the application
                std::stringstream ss;
                ss << "Initialized GLEW but failed to retrieve OpenGL Version, glError"
                   << getGLErrorString(glGetError());
                throw OpenGLInitException(ss.str(), IVW_CONTEXT_CUSTOM("OpenGLCapabilities"));
            }
            glVersionStr_ = std::string(
                (glversion != nullptr ? reinterpret_cast<const char*>(glversion) : "INVALID"));
            glVersion_ = parseAndRetrieveVersion(glVersionStr_);
        } else {
            std::stringstream ss;
            ss << "Failed to initialize GLEW: " << glewGetErrorString(glewError);
            throw OpenGLInitException(ss.str(), IVW_CONTEXT_CUSTOM("OpenGLCapabilities"));
        }
        LGL_ERROR_SUPPRESS;
        glewInitialized_ = true;
    }
}

OpenGLCapabilities::GLSLShaderVersion OpenGLCapabilities::getCurrentShaderVersion() {
    if (supportedShaderVersions_.size() > currentGlobalGLSLVersionIdx_) {
        return supportedShaderVersions_[currentGlobalGLSLVersionIdx_];
    } else {
        return GLSLShaderVersion(0);
    }
}

size_t OpenGLCapabilities::getNumberOfShaderVersions() const {
    return supportedShaderVersions_.size();
}
OpenGLCapabilities::GLSLShaderVersion OpenGLCapabilities::getShaderVersion(size_t ind) const {
    return supportedShaderVersions_[ind];
}

size_t OpenGLCapabilities::getCurrentShaderIndex() const { return currentGlobalGLSLVersionIdx_; }

bool OpenGLCapabilities::isExtensionSupported(const char* name) {
    return (glewIsExtensionSupported(name) != '0');
}

bool OpenGLCapabilities::isSupported(const char* name) { return (glewIsSupported(name) != '0'); }
bool OpenGLCapabilities::isTexturesSupported() const { return texSupported_; }
bool OpenGLCapabilities::isTextureArraysSupported() const { return texArraySupported_; }
bool OpenGLCapabilities::is3DTexturesSupported() const { return tex3DSupported_; }
bool OpenGLCapabilities::isFboSupported() const { return fboSupported_; }
bool OpenGLCapabilities::isShadersSupported() const { return shadersAreSupported_; }
bool OpenGLCapabilities::isShadersSupportedARB() const { return shadersAreSupportedARB_; }
bool OpenGLCapabilities::isGeometryShadersSupported() const { return geometryShadersAreSupported_; }
bool OpenGLCapabilities::isComputeShadersSupported() const {
    return isExtensionSupported("GL_ARB_compute_shader");
}

int OpenGLCapabilities::getMaxProgramLoopCount() const { return maxProgramLoopCount_; }
int OpenGLCapabilities::getNumTexUnits() const { return numTexUnits_; }
int OpenGLCapabilities::getMaxTexSize() const { return maxTexSize_; }
int OpenGLCapabilities::getMax3DTexSize() const { return max3DTexSize_; }
int OpenGLCapabilities::getMaxArrayTexSize() const { return maxArrayTexSize_; }
int OpenGLCapabilities::getMaxArrayVertexAttribs() const { return maxArrayVertexAttribs_; }
int OpenGLCapabilities::getMaxColorAttachments() const { return maxColorAttachments_; }

const std::string& OpenGLCapabilities::getRenderString() const { return glRenderStr_; }
const std::string& OpenGLCapabilities::getVendorString() const { return glVendorStr_; }
const std::string& OpenGLCapabilities::getGLVersionString() const { return glVersionStr_; }
const std::string& OpenGLCapabilities::getGLSLVersionString() const { return glslVersionStr_; }
OpenGLCapabilities::GlVendor OpenGLCapabilities::getVendor() const { return glVendor_; }

glm::u64 OpenGLCapabilities::getCurrentAvailableTextureMem() {
    glm::u64 currentAvailableTexMeminBytes = 0;

    if (!OpenGLCapabilities::hasOpenGLVersion()) return currentAvailableTexMeminBytes;

    try {
        GLint nCurAvailMemoryInKB[4] = {0};

        if (glVendor_ == GlVendor::Nvidia) {
#ifdef GL_NVX_gpu_memory_info
            glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, nCurAvailMemoryInKB);
#endif
        } else if (glVendor_ == GlVendor::Amd) {
#ifdef GL_ATI_meminfo
            glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, nCurAvailMemoryInKB);
#endif
        }

        currentAvailableTexMeminBytes =
            util::kilobytes_to_bytes(static_cast<glm::u64>(nCurAvailMemoryInKB[0]));
    } catch (const Exception& e) {
        LogWarn("Failed to fetch current available texture memory: " << e.what());
    }

    return currentAvailableTexMeminBytes;
}

glm::u64 OpenGLCapabilities::getTotalAvailableTextureMem() {
    glm::u64 totalAvailableTexMemInBytes = 0;

    try {
        if (glVendor_ == GlVendor::Nvidia) {
#ifdef GL_NVX_gpu_memory_info
            GLint nTotalAvailMemoryInKB[4] = {0};
            glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, nTotalAvailMemoryInKB);
            totalAvailableTexMemInBytes =
                util::kilobytes_to_bytes(static_cast<glm::u64>(nTotalAvailMemoryInKB[0]));
#endif
        } else if (glVendor_ == GlVendor::Amd) {
#if defined(WGL_AMD_gpu_association)
            UINT n = wglGetGPUIDsAMD(0, 0);
            UINT* ids = new UINT[n];
            size_t total_mem_mb = 0;
            wglGetGPUIDsAMD(n, ids);
            wglGetGPUInfoAMD(ids[0], WGL_GPU_RAM_AMD, GL_UNSIGNED_INT, sizeof(size_t),
                             &total_mem_mb);
            totalAvailableTexMemInBytes =
                util::megabytes_to_bytes(static_cast<glm::u64>(total_mem_mb));
#elif defined(GLX_AMD_gpu_association)
            UINT n = glXGetGPUIDsAMD(0, 0);
            UINT* ids = new UINT[n];
            size_t total_mem_mb = 0;
            glXGetGPUIDsAMD(n, ids);
            glXGetGPUInfoAMD(ids[0], GLX_GPU_RAM_AMD, GL_UNSIGNED_INT, sizeof(size_t),
                             &total_mem_mb);
            totalAvailableTexMemInBytes =
                util::megabytes_to_bytes(static_cast<glm::u64>(total_mem_mb));
#endif
        }
    } catch (const Exception& e) {
        LogWarn("Failed to fetch total available texture memory: " << e.what());
    }

    return totalAvailableTexMemInBytes;
}

std::string OpenGLCapabilities::getPreferredProfile() { return preferredProfile_; }

bool OpenGLCapabilities::setPreferredProfile(std::string profile, bool showMessage) {
    preferredProfile_ = profile;

    size_t i = 0;
    while (i < supportedShaderVersions_.size() &&
           (supportedShaderVersions_[i].hasProfile() &&
            supportedShaderVersions_[i].getProfile() != preferredProfile_)) {
        i++;
    }

    bool changed = false;
    if (i != currentGlobalGLSLVersionIdx_) {
        currentGlobalGLSLVersionIdx_ = i;
        changed = true;
    }

    if (changed || showMessage) {
        print("Current set global GLSL version: ",
              getCurrentShaderVersion().getVersionAndProfileAsString());
    }

    return changed;
}

void OpenGLCapabilities::retrieveStaticInfo() {
    if (!OpenGLCapabilities::hasOpenGLVersion()) return;

    const GLubyte* vendor = glGetString(GL_VENDOR);
    glVendorStr_ =
        std::string((vendor != nullptr ? reinterpret_cast<const char*>(vendor) : "INVALID"));

    if (glVendorStr_.find("NVIDIA") != std::string::npos) {
        glVendor_ = GlVendor::Nvidia;
    } else if (glVendorStr_.find("AMD") != std::string::npos ||
               glVendorStr_.find("ATI") != std::string::npos) {
        glVendor_ = GlVendor::Amd;
    } else if (glVendorStr_.find("INTEL") != std::string::npos ||
               glVendorStr_.find("Intel") != std::string::npos) {
        glVendor_ = GlVendor::Intel;
    } else {
        glVendor_ = GlVendor::Unknown;
    }

    const GLubyte* glrender = glGetString(GL_RENDERER);
    glRenderStr_ =
        std::string((glrender != nullptr ? reinterpret_cast<const char*>(glrender) : "INVALID"));
    // GLSL
    shadersAreSupported_ = (glVersion_ >= 200);
    shadersAreSupportedARB_ = isExtensionSupported("GL_EXT_ARB_fragment_program");
    geometryShadersAreSupported_ = isExtensionSupported("GL_EXT_ARB_geometry_shader4");

    GLint numberOfSupportedVersions = 0;
    const GLubyte* glslStrByte = nullptr;
#ifdef GL_VERSION_4_3
    if (glVersion_ >= 430) {
        glslStrByte = glGetString(GL_SHADING_LANGUAGE_VERSION);
        glslVersionStr_ = std::string(
            (glslStrByte != nullptr ? reinterpret_cast<const char*>(glslStrByte) : "000"));
        glGetIntegerv(GL_NUM_SHADING_LANGUAGE_VERSIONS, &numberOfSupportedVersions);
        int glslVersion = parseAndRetrieveVersion(glslVersionStr_);

        for (int i = 0; i < numberOfSupportedVersions; i++) {
            parseAndAddShaderVersion(
                toString<const GLubyte*>(glGetStringi(GL_SHADING_LANGUAGE_VERSION, i)),
                glslVersion);
        }

        std::sort(supportedShaderVersions_.begin(), supportedShaderVersions_.end(),
                  &GLSLShaderVersion::sortHighestFirst);
    }
#endif

    if (numberOfSupportedVersions == 0) {
        if (isShadersSupported())
            glslStrByte = glGetString(GL_SHADING_LANGUAGE_VERSION);
        else if (isShadersSupportedARB())
            glslStrByte = glGetString(GL_SHADING_LANGUAGE_VERSION_ARB);

        glslVersionStr_ = std::string(
            (glslStrByte != nullptr ? reinterpret_cast<const char*>(glslStrByte) : "000"));
        int glslVersion = parseAndRetrieveVersion(glslVersionStr_);

        if (glslVersion != 0) {
#ifdef GL_VERSION_4_4
            addShaderVersionIfEqualOrLower(GLSLShaderVersion(440, "core"), glslVersion);
            addShaderVersionIfEqualOrLower(GLSLShaderVersion(440, "compatibility"), glslVersion);
#endif
#ifdef GL_VERSION_4_3
            addShaderVersionIfEqualOrLower(GLSLShaderVersion(430, "core"), glslVersion);
            addShaderVersionIfEqualOrLower(GLSLShaderVersion(430, "compatibility"), glslVersion);
#endif
#ifdef GL_VERSION_4_2
            addShaderVersionIfEqualOrLower(GLSLShaderVersion(420, "core"), glslVersion);
            addShaderVersionIfEqualOrLower(GLSLShaderVersion(420, "compatibility"), glslVersion);
#endif
#ifdef GL_VERSION_4_1
            addShaderVersionIfEqualOrLower(GLSLShaderVersion(410, "core"), glslVersion);
            addShaderVersionIfEqualOrLower(GLSLShaderVersion(410, "compatibility"), glslVersion);
#endif
#ifdef GL_VERSION_4_0
            addShaderVersionIfEqualOrLower(GLSLShaderVersion(400, "core"), glslVersion);
            addShaderVersionIfEqualOrLower(GLSLShaderVersion(400, "compatibility"), glslVersion);
#endif
#ifdef GL_VERSION_3_3
            addShaderVersionIfEqualOrLower(GLSLShaderVersion(330, "core"), glslVersion);
            addShaderVersionIfEqualOrLower(GLSLShaderVersion(330, "compatibility"), glslVersion);
#endif
#ifdef GL_VERSION_3_2
            addShaderVersionIfEqualOrLower(GLSLShaderVersion(150, "core"), glslVersion);
            addShaderVersionIfEqualOrLower(GLSLShaderVersion(150, "compatibility"), glslVersion);
#endif
#ifdef GL_VERSION_3_1
            addShaderVersionIfEqualOrLower(GLSLShaderVersion(140), glslVersion);
#endif
#ifdef GL_VERSION_3_0
            addShaderVersionIfEqualOrLower(GLSLShaderVersion(130), glslVersion);
#endif
#ifdef GL_VERSION_2_1
            addShaderVersionIfEqualOrLower(GLSLShaderVersion(120), glslVersion);
#endif
#ifdef GL_VERSION_2_0
            addShaderVersionIfEqualOrLower(GLSLShaderVersion(110), glslVersion);
#endif
        }
    }

    // Set current used GLSL version to highest(i.e. 1st in vector) with preferred profile (or no
    // profile)
    if (isShadersSupported() || isShadersSupportedARB()) {
        size_t i = 0;

        while (i < supportedShaderVersions_.size() &&
               (supportedShaderVersions_[i].hasProfile() &&
                supportedShaderVersions_[i].getProfile() != preferredProfile_))
            i++;

        currentGlobalGLSLVersionIdx_ = i;
    }

    maxProgramLoopCount_ = -1;

    if (GLEW_NV_fragment_program2) {
        GLint i = -1;
        glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_MAX_PROGRAM_LOOP_COUNT_NV, &i);

        if (i > 0) {
            // Restrict cycles to realistic samplingRate*maximumDimension,
            // 20*(10 000) slices = 200000
            maxProgramLoopCount_ = std::min<int>(static_cast<int>(i), 200000);
        }
    }

    if (isGeometryShadersSupported()) {
        glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES,
                      static_cast<GLint*>(&geometryShadersMaxVertices_));
        glGetIntegerv(GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS,
                      static_cast<GLint*>(&geometryShadersMaxOutputComponents_));
        glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_COMPONENTS,
                      static_cast<GLint*>(&geometryShadersMaxTotalOutputComponents_));
    }

// Texturing
#ifdef GL_VERSION_1_1
    texSupported_ = true;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*)&maxTexSize_);
#else
    texSupported_ = isExtensionSupported("GL_EXT_texture");
    maxTexSize_ = 0;
#endif
#ifdef GL_VERSION_1_2
    tex3DSupported_ = true;
    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, (GLint*)&max3DTexSize_);
#else
    tex3DSupported_ = isExtensionSupported("GL_EXT_texture3D");

    if (is3DTexturesSupported())
        glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, (GLint*)&max3DTexSize_);
    else
        max3DTexSize_ = 0;

#endif
#ifdef GL_VERSION_2_0
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, (GLint*)&maxArrayVertexAttribs_);
#endif
#ifdef GL_VERSION_3_0
    if (glVersion_ >= 300) {
        texArraySupported_ = true;
        glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, (GLint*)&maxArrayTexSize_);
    } else {
#endif
        texArraySupported_ = isExtensionSupported("GL_EXT_texture_array");

        if (isTextureArraysSupported())
            glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, (GLint*)&maxArrayTexSize_);
        else
            maxArrayTexSize_ = 0;
#ifdef GL_VERSION_3_0
    }
#endif
    numTexUnits_ = -1;

    if (isShadersSupported()) glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS_ARB, (GLint*)&numTexUnits_);

    if (getNumTexUnits() < 0) glGetIntegerv(GL_MAX_TEXTURE_UNITS, (GLint*)&numTexUnits_);

    if (numTexUnits_ < 0) numTexUnits_ = 0;

    TextureUnit::initialize(numTexUnits_);
    // FBO
    fboSupported_ = isExtensionSupported("GL_EXT_framebuffer_object");
    maxColorAttachments_ = 0;

    if (isFboSupported()) glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAttachments_);
}

void OpenGLCapabilities::retrieveDynamicInfo() {}

void OpenGLCapabilities::addShaderVersion(GLSLShaderVersion version) {
    supportedShaderVersions_.push_back(version);
}

void OpenGLCapabilities::addShaderVersionIfEqualOrLower(GLSLShaderVersion version,
                                                        int compVersion) {
    if (version.getVersion() <= compVersion) addShaderVersion(version);
}

void OpenGLCapabilities::parseAndAddShaderVersion(std::string versionStr, int compVersion) {
    // Assumes <version><space><profile> or <version>, example 420 core or 140
    if (!versionStr.empty()) {
        // Remove all non-alphanumeric characters, but keep spaces
        versionStr.erase(
            std::remove_if(versionStr.begin(), versionStr.end(),
                           [](char c) { return !(std::isspace(c) || std::isalnum(c)); }),
            versionStr.end());

        auto versionSplit = splitString(versionStr);
        if (versionSplit.size() > 1 && (versionSplit[1].compare("core") == 0 ||
                                        versionSplit[1].compare("compatibility") == 0)) {
            addShaderVersionIfEqualOrLower(
                GLSLShaderVersion(stringTo<int>(versionSplit[0]), versionSplit[1]), compVersion);
        } else {
            addShaderVersionIfEqualOrLower(GLSLShaderVersion(stringTo<int>(versionSplit[0])),
                                           compVersion);
        }
    }
}

int OpenGLCapabilities::parseAndRetrieveVersion(std::string versionStr) {
    // Assumes format <version><space><desc> example "4.1 ATI-1.20.11"
    // Version to int mapping, 4.1 => 410, 4.40 => 440
    if (!versionStr.empty()) {
        auto versionSplit = splitString(versionStr, ' ');
        auto numberSplit = splitString(versionSplit[0], '.');
        int factor = 1;
        if (numberSplit.size() > 1) {
            if (numberSplit[1].size() == 1) {
                factor = 10;
            }
            return stringTo<int>(numberSplit[0]) * 100 + stringTo<int>(numberSplit[1]) * factor;
        }
    }

    return 0;
}

}  // namespace inviwo
