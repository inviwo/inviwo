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

#ifndef IVW_OPENGLCAPABILITIES_H
#define IVW_OPENGLCAPABILITIES_H

#include <modules/opengl/openglmoduledefine.h>
#include <inviwo/core/util/capabilities.h>

namespace inviwo {
class OpenGLSettings;

class IVW_MODULE_OPENGL_API OpenGLCapabilities : public Capabilities {
public:
    class IVW_MODULE_OPENGL_API GLSLShaderVersion {
    public:
        GLSLShaderVersion();
        GLSLShaderVersion(int num);
        GLSLShaderVersion(int num, std::string pro);

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

    OpenGLCapabilities(OpenGLSettings* settings);
    virtual ~OpenGLCapabilities();

    virtual void printInfo() override;
    void printDetailedInfo();

    static void initializeGLEW();
    static int getOpenGLVersion();

    // Minimal support is OpenGL 3.2 at the moment
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
    bool isComputeShadersSupported() const;

    GLSLShaderVersion getCurrentShaderVersion();
    size_t getCurrentShaderIndex() const;
    size_t getNumberOfShaderVersions() const;
    GLSLShaderVersion getShaderVersion(size_t ind) const;

    glm::u64 getCurrentAvailableTextureMem();
    glm::u64 getTotalAvailableTextureMem();

    const std::string& getRenderString() const;
    const std::string& getVendorString() const;
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

    static std::string getPreferredProfile();
    bool setPreferredProfile(std::string, bool);

protected:
    virtual void retrieveStaticInfo() override;
    virtual void retrieveDynamicInfo() override;

    void addShaderVersion(GLSLShaderVersion);
    void addShaderVersionIfEqualOrLower(GLSLShaderVersion, int);
    void parseAndAddShaderVersion(std::string, int);

    static int parseAndRetrieveVersion(std::string);

private:
    template <typename T, typename... Ts>
    void printHelper(std::stringstream& ss, T& message, const Ts&... messages) const {
        ss << message;
        printHelper(ss, messages...);
    }
    template <typename T>
    void printHelper(std::stringstream& ss, const T& message) const {
        ss << message;
    }

    template <typename... Ts>
    void print(const Ts&... messages) const {
        std::stringstream ss;
        printHelper(ss, messages...);
        LogCentral::getPtr()->log("OpenGLInfo", LogLevel::Info, LogAudience::User, "", "", 0,
                                  ss.str());
    }

    static bool glewInitialized_;
    static std::string preferredProfile_;
    static int glVersion_;
    static std::string glVersionStr_;

    GlVendor glVendor_;

    std::string glVendorStr_;
    std::string glRenderStr_;
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

#endif  // IVW_OPENGLCAPABILITIES_H
