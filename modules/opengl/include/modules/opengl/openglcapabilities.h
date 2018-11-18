/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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

        std::string getProfile();
        int getVersion();
        std::string getVersionAsString();
        std::string getVersionAndProfileAsString();

        bool hasProfile();

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
    
    virtual bool canAllocate(glm::u64 dataSize, glm::u8 percentageOfAvailableMemory = 100) override;
    virtual uvec3 calculateOptimalBrickSize(uvec3 dimensions, size_t formatSizeInBytes,
                                            glm::u8 percentageOfAvailableMemory = 100) override;
    static void initializeGLEW();
    static int getOpenGLVersion();

    // Minimal support is OpenGL 3.2 at the moment
    static bool hasSupportedOpenGLVersion();

    static bool hasOpenGLVersion();

    static bool isExtensionSupported(const char*);
    static bool isSupported(const char*);

    bool isTexturesSupported();
    bool is3DTexturesSupported();
    bool isTextureArraysSupported();
    bool isFboSupported();
    bool isShadersSupported();
    bool isShadersSupportedARB();
    bool isGeometryShadersSupported();

    GLSLShaderVersion getCurrentShaderVersion();
    std::string getCurrentGlobalGLSLHeader();
    std::string getCurrentGlobalGLSLVertexDefines();
    std::string getCurrentGlobalGLSLFragmentDefines();

    glm::u64 getCurrentAvailableTextureMem();
    glm::u64 getTotalAvailableTextureMem();

    std::string getRenderString() const;
    std::string getVendorString() const;
    std::string getGLVersionString() const;
    std::string getGLSLVersionString() const;
    GlVendor getVendor() const;

    int getMaxProgramLoopCount();
    int getMaxTexSize();
    int getMax3DTexSize();
    int getMaxArrayTexSize();
    int getMaxArrayVertexAttribs();
    int getMaxColorAttachments();
    int getNumTexUnits();

    static std::string getPreferredProfile();
    bool setPreferredProfile(std::string, bool);

protected:
    virtual void retrieveStaticInfo() override;
    virtual void retrieveDynamicInfo() override;

    void rebuildGLSLHeader();
    void rebuildGLSLVertexDefines();
    void rebuildGLSLFragmentDefines();

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
        LogCentral::getPtr()->log("OpenGLInfo", LogLevel::Info, LogAudience::User, "", "", 0, ss.str());
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
    bool geometryShsadersAreSupported_;
    int maxProgramLoopCount_;
    int geometryShadersMaxVertices_;
    int geometryShadersMaxOutputComponents_;
    int geometryShadersMaxTotalOutputComponents_;

    size_t currentGlobalGLSLVersionIdx_;
    std::string currentGlobalGLSLHeader_;
    std::string currentGlobalGLSLVertexDefines_;
    std::string currentGlobalGLSLFragmentDefines_;
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

}  // namespace

#endif  // IVW_OPENGLCAPABILITIES_H
