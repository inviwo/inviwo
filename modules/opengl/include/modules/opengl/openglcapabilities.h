/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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

#pragma once

#include <modules/opengl/openglmoduledefine.h>  // for IVW_MODULE_OPENGL_API

#include <inviwo/core/util/capabilities.h>  // for Capabilities
#include <inviwo/core/util/logcentral.h>    // for LogCentral, LogAudience, LogAudience::User

#include <cstddef>      // for size_t
#include <sstream>      // for stringstream, basic_stringstream<>::strin...
#include <string>       // for string, basic_string
#include <string_view>  // for string_view
#include <vector>       // for vector

namespace inviwo {
class OpenGLSettings;

class IVW_MODULE_OPENGL_API OpenGLCapabilities : public Capabilities {
public:
    class IVW_MODULE_OPENGL_API GLSLShaderVersion {
    public:
        GLSLShaderVersion();
        GLSLShaderVersion(int num);
        GLSLShaderVersion(int num, std::string_view pro);

        const std::string& getProfile() const;
        int getVersion() const;
        std::string getVersionAsString() const;
        std::string getVersionAndProfileAsString() const;

        bool hasProfile() const;

        static bool sortHighestFirst(GLSLShaderVersion i, GLSLShaderVersion j);

    private:
        int number_;
        std::string profile_;
    };

    enum class GlVendor { Nvidia, Amd, Intel, Unknown };

    explicit OpenGLCapabilities(OpenGLSettings* settings);
    virtual ~OpenGLCapabilities();

    virtual void printInfo() override;
    void printDetailedInfo();

    static void initializeGLEW();
    static int getOpenGLVersion();

    // Minimal support is OpenGL 3.3 at the moment
    static bool hasSupportedOpenGLVersion();

    static bool hasOpenGLVersion();

    static bool isExtensionSupported(const char*);
    static bool isSupported(const char*);

    bool isTexturesSupported() const;
    bool is3DTexturesSupported() const;
    bool isTextureArraysSupported() const;
    bool isFboSupported() const;
    bool isShadersSupported() const;
    bool isShadersSupportedARB() const;
    bool isGeometryShadersSupported() const;
    static bool isComputeShadersSupported();
    static bool isShaderStorageBuffersSupported();
    /**
     * The conversion from normalized fixed-point integers to floating-point values and back was
     * changed to use a symmetric range in OpenGL 4.2. That is the range [-127, 127] is used
     * for the normalization of 8bit signed integers instead of [-128, 127].
     *
     * \see DataMapper::SignedNormalization
     * \see OpenGL 4.6 specification, Section 2.3.5 Fixed-Point Data Conversion
     */
    static bool isSignedIntNormalizationSymmetric();

    GLSLShaderVersion getCurrentShaderVersion() const;
    size_t getCurrentShaderIndex() const;
    size_t getNumberOfShaderVersions() const;
    GLSLShaderVersion getShaderVersion(size_t ind) const;

    size_t getCurrentAvailableTextureMem();
    size_t getTotalAvailableTextureMem();

    const std::string& getRenderString() const;
    const std::string& getVendorString() const;
    const std::string& getProfileString() const;
    const std::string& getGLVersionString() const;
    const std::string& getGLSLVersionString() const;
    GlVendor getVendor() const;

    int getMaxProgramLoopCount() const;
    int getMaxTexSize() const;
    int getMax3DTexSize() const;
    int getMaxArrayTexSize() const;
    int getMaxArrayVertexAttribs() const;
    int getMaxColorAttachments() const;
    int getNumTexUnits() const;

protected:
    virtual void retrieveStaticInfo() override;
    virtual void retrieveDynamicInfo() override;

    void addShaderVersion(GLSLShaderVersion);
    void addShaderVersionIfEqualOrLower(GLSLShaderVersion, int);
    void parseAndAddShaderVersion(std::string, int);

    static int parseAndRetrieveVersion(std::string);

private:
    static bool glewInitialized_;
    static int glVersion_;
    static std::string glVersionStr_;

    GlVendor glVendor_;

    std::string glVendorStr_;
    std::string glRenderStr_;
    std::string glProfileStr_;
    std::string glslVersionStr_;

    // GLSL
    bool shadersAreSupported_;
    bool shadersAreSupportedARB_;
    bool geometryShadersAreSupported_;

    int maxProgramLoopCount_;
    int geometryShadersMaxVertices_;
    int geometryShadersMaxOutputComponents_;
    int geometryShadersMaxTotalOutputComponents_;

    size_t currentGlobalGLSLVersionIdx_;
    std::vector<GLSLShaderVersion> supportedShaderVersions_;

    // Texturing
    bool texSupported_;
    bool tex3DSupported_;
    bool texArraySupported_;
    bool fboSupported_;
    int maxTexSize_;
    int max3DTexSize_;
    int maxArrayTexSize_;
    int maxArrayVertexAttribs_;
    int maxColorAttachments_;
    int numTexUnits_;
};

}  // namespace inviwo
