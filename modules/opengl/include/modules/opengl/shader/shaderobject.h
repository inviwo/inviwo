/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#ifndef IVW_SHADEROBJECT_H
#define IVW_SHADEROBJECT_H

#include <inviwo/core/common/inviwo.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/openglmoduledefine.h>
#include <modules/opengl/shader/shaderresource.h>
#include <modules/opengl/shader/shadertype.h>
#include <inviwo/core/util/dispatcher.h>

namespace inviwo {

class OpenGLCapabilities;

/**
 * A wrapper for an OpenGL shader object.
 * Handles loading sources from shader resources, either files or stings.
 * Pre-processes the sources resolving all include with help of the shader manager.
 * Keeps a lookup table of from which include each line originates.
 * Handles a list of defines, that can be added or removed @see addShaderDefine
 * Handles a list of shader extensions @see addShaderExtension
 * Handles a list of input declarations @see addInDeclaration
 * Handles a list of output declarations @see addOutDeclaration
 *
 * A fragment shader will by default have the following out declarations:
 *     out vec4 FragData0;   (location 0)
 *     out vev4 PickingData; (location 1)
 * and a vertex shader will by default have the following in declarations:
 *     in vec4 in_Vertex;   (location 0)
 *     in vec3 in_Normal;   (location 1)
 *     in vec4 in_Color;    (location 2)
 *     in vec3 in_TexCoord; (location 3)
 * The defaults can be removed by calling clearOutDeclarations or clearInDeclarations
 * respectively.
 */
class IVW_MODULE_OPENGL_API ShaderObject {
public:
    struct InDeclaration {
        std::string name;
        int location = -1;
        std::string type = "vec4";
        std::string decl = "in {type} {name};";
        std::string toString() const;
    };
    struct OutDeclaration {
        std::string name;
        int location = -1;
        std::string type = "vec4";
        std::string decl = "out {type} {name};";
        std::string toString() const;
    };

    using Callback = std::function<void(ShaderObject*)>;

    ShaderObject(ShaderType shaderType, std::shared_ptr<const ShaderResource> resource);
    ShaderObject(std::shared_ptr<const ShaderResource> resource);
    ShaderObject(ShaderType shaderType, std::string fileName);
    ShaderObject(std::string fileName);
    ShaderObject(GLenum shaderType, std::string fileName);

    ShaderObject(const ShaderObject& rhs);
    ShaderObject(ShaderObject&& rhs) noexcept;
    ShaderObject& operator=(const ShaderObject& that);
    ShaderObject& operator=(ShaderObject&& that) noexcept;

    ~ShaderObject();

    GLuint getID() const;
    std::string getFileName() const;
    std::shared_ptr<const ShaderResource> getResource() const;
    const std::vector<std::shared_ptr<const ShaderResource>>& getResources() const;
    ShaderType getShaderType() const;

    void preprocess();
    void upload();
    void compile();
    void build();
    bool isReady() const;

    /**
     * Add a define to the shader as
     *     #define name value
     */
    void addShaderDefine(const std::string& name, const std::string& value = "");
    /**
     * Remove a previously added define
     */
    void removeShaderDefine(const std::string& name);

    /**
     * Adds or removed a define with name 'name'
     * @param name Name of the definition.
     * @param shouldAdd If true the define is added with name and value, otherwise the define is
     * removed
     * @param value Value of definition.
     */
    void setShaderDefine(const std::string& name, bool shouldAdd, const std::string& value = "");
    bool hasShaderDefine(const std::string& name) const;
    void clearShaderDefines();

    void addShaderExtension(const std::string& extName, bool enabled);
    void removeShaderExtension(const std::string& extName);
    bool hasShaderExtension(const std::string& extName) const;
    void clearShaderExtensions();

    /**
     * \brief adds an additional output specifier to the shader
     * The given name will be added as
     *
     *     out __type__ __name__;
     *
     * The shader will call glBindFragDataLocation for each of the output declarations
     * before linking the shader
     *
     * @param name      identifier of the output specifier
     * @param location  index location of the output (< MAX_RENDER_TARGETS)
     * @param type      type used for the output specifier
     */
    void addOutDeclaration(const std::string& name, int location = -1,
                           const std::string& type = "vec4");
    void addOutDeclaration(const OutDeclaration& decl);
    void clearOutDeclarations();
    const std::vector<OutDeclaration>& getOutDeclarations() const;

    /**
     * \brief adds an additional input specifier to the shader
     * The given name will be added as
     *
     *     in __type__ __name__;
     *
     * The shader will call glBindAttribLocation for each of the input declarations
     * before linking the shader
     *
     * @param name      identifier of the output specifier
     * @param location  index location of the output (< MAX_RENDER_TARGETS)
     * @param type      type used for the output specifier
     */
    void addInDeclaration(const std::string& name, int location = -1,
                          const std::string& type = "vec4");
    void addInDeclaration(const InDeclaration& decl);
    void clearInDeclarations();
    const std::vector<InDeclaration>& getInDeclarations() const;

    /**
     * Adds the default fragment out declarations to the list of out declarations.
     * This function is automatically called in the constructor for a fragment shader.
     * If clearOutDeclarations is called then they will be removed and one would have to
     * manually call this function to re-add them if needed.
     * The defaults are:
     *     out vec4 FragData0;   (location 0)
     *     out vev4 PickingData; (location 1)
     */
    void addStandardFragmentOutDeclarations();

    /**
     * Adds the default vertex in declarations to the list of in declarations.
     * This function is automatically called in the constructor for a vertex shader.
     * If clearInDeclarations is called then they will be removed and one would have to
     * manually call this function to re-add them if needed.
     * The defaults are:
     *     in vec4 in_Vertex;   (location 0)
     *     in vec3 in_Normal;   (location 1)
     *     in vec4 in_Color;    (location 2)
     *     in vec3 in_TexCoord; (location 3)
     */
    void addStandardVertexInDeclarations();

    std::pair<std::string, size_t> resolveLine(size_t line) const;
    std::string print(bool showSource = false, bool preprocess = true) const;

    template <typename T>
    std::shared_ptr<Callback> onChange(T&& callback);

private:
    struct LineNumberResolver {
        LineNumberResolver() = default;
        void addLine(const std::string& file, size_t line);
        std::pair<std::string, size_t> resolveLine(size_t line) const;
        void clear();
        size_t size() { return lines_.size(); }
        std::string resolveLog(const std::string& compileLog) const;
        auto begin() const { return lines_.cbegin(); }
        auto end() const { return lines_.cend(); }

    private:
        std::vector<std::pair<std::string, size_t>> lines_;
    };

    static std::shared_ptr<const ShaderResource> loadResource(std::string fileName);
    void addDefines(std::ostringstream& source);
    void addIncludes(std::ostringstream& source, std::shared_ptr<const ShaderResource> resource);

    // state variables
    ShaderType shaderType_;
    GLuint id_;
    std::shared_ptr<const ShaderResource> resource_;

    std::vector<InDeclaration> inDeclarations_;
    std::vector<OutDeclaration> outDeclarations_;

    using ShaderDefines = std::map<std::string, std::string>;
    ShaderDefines shaderDefines_;

    using ShaderExtensions = std::map<std::string, bool>;  // extension name, enable flag
    ShaderExtensions shaderExtensions_;

    // derived variables
    std::string sourceProcessed_;
    std::vector<std::shared_ptr<const ShaderResource>> includeResources_;
    LineNumberResolver lnr_;

    Dispatcher<void(ShaderObject*)> callbacks_;
    std::vector<std::shared_ptr<ShaderResource::Callback>> resourceCallbacks_;
};

template <typename T>
std::shared_ptr<ShaderObject::Callback> ShaderObject::onChange(T&& callback) {
    return callbacks_.add(std::forward<T>(callback));
}

}  // namespace inviwo

#endif  // IVW_SHADEROBJECT_H
