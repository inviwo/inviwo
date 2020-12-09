/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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

#include <modules/openglqt/glslsyntaxhighlight.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QFontDatabase>
#include <warn/pop>

#include <array>
#include <string_view>

using namespace std::literals;

namespace inviwo {

GLSLSyntaxHighlight::GLSLSyntaxHighlight()
    : Settings("GLSL Syntax Highlighting")
    , font("font", "Font", utilqt::getMonoSpaceFonts(), utilqt::getDefaultFontIndex())
    , fontSize("fontSize", "Size", syntax::fontSize, 1, 72)

    , textColor("text", "Text", util::ordinalColor(syntax::text))
    , backgroundColor("background", "Background", util::ordinalColor(syntax::background))
    , highLightColor("highLight", "HighLight", util::ordinalColor(syntax::highLight))
    , keywordColor("keyword", "Keyword", util::ordinalColor(syntax::keyword))
    , builtinVarColor("bultinVar", "Built-in Variable", util::ordinalColor(syntax::builtinVars))
    , typeColor("type", "Type", util::ordinalColor(syntax::type))
    , builtinFuncColor("bultinFunc", "Built-in Function", util::ordinalColor(syntax::builtinFuncs))
    , commentColor("comment", "Comment", util::ordinalColor(syntax::comment))
    , preProcessorColor("preProcessor", "Pre-Processor", util::ordinalColor(syntax::preProcessor))
    , litteralColor("literal", "String Literal", util::ordinalColor(syntax::literal))
    , constantColor("constants", "Constant", util::ordinalColor(syntax::constant))
    , mainColor("main", "Main Function", util::ordinalColor(syntax::main)) {

    addProperties(font, fontSize, textColor, backgroundColor, highLightColor, commentColor,
                  typeColor, keywordColor, builtinVarColor, builtinFuncColor, preProcessorColor,
                  litteralColor, constantColor, mainColor);

    load();
}

namespace {

// define GLSL types and keywords using regular expressions. \b indicates word boundaries.
constexpr std::array types = {"float"sv,         "[bi]?vec[2-4]"sv, "int"sv,
                              "bool"sv,          "mat[2-4]"sv,      "void"sv,
                              "sampler[1-3]D"sv, "samplerCube"sv,   "sampler[1-2]DShadow"sv};

constexpr std::array qualifiers = {"struct"sv, "uniform"sv, "attribute"sv, "varying"sv, "in"sv,
                                   "out"sv,    "inout"sv,   "const"sv,     "discard"sv, "if"sv,
                                   "const"sv,  "while"sv,   "continue"sv,  "break"sv,   "return"sv,
                                   "layout"sv, "flat"sv};

constexpr std::array builtinsVars = {"gl_ModelViewMatrix"sv,
                                     "gl_ModelViewProjectionMatrix"sv,
                                     "gl_ProjectionMatrix"sv,
                                     "gl_TextureMatrix"sv,
                                     "gl_ModelViewMatrixInverse"sv,
                                     "gl_ModelViewProjectionMatrixInverse"sv,
                                     "gl_ProjectionMatrixInverse"sv,
                                     "gl_TextureMatrixInverse"sv,
                                     "gl_ModelViewMatrixTranspose"sv,
                                     "gl_ModelViewProjectionMatrixTranspose"sv,
                                     "gl_ProjectionMatrixTranspose"sv,
                                     "gl_TextureMatrixTranspose"sv,
                                     "gl_ModelViewMatrixInverseTranspose"sv,
                                     "gl_ModelViewProjectionMatrixInverseTranspose"sv,
                                     "gl_ProjectionMatrixInverseTranspose"sv,
                                     "gl_TextureMatrixInverseTranspose"sv,
                                     "gl_NormalMatrix"sv,
                                     "gl_NormalScale"sv,
                                     "gl_DepthRangeParameters"sv,
                                     "gl_DepthRangeParameters"sv,
                                     "gl_DepthRange"sv,
                                     "gl_FogParameters"sv,
                                     "gl_Fog"sv,
                                     "gl_LightSourceParameters"sv,
                                     "gl_LightSource"sv,
                                     "gl_LightModelParameters"sv,
                                     "gl_LightModel"sv,
                                     "gl_LightModelProducts"sv,
                                     "gl_FrontLightModelProduct"sv,
                                     "gl_BackLightModelProduct"sv,
                                     "gl_LightProducts"sv,
                                     "gl_FrontLightProduct"sv,
                                     "gl_BackLightProduct"sv,
                                     "gl_MaterialParameters"sv,
                                     "gl_FrontMaterial"sv,
                                     "gl_BackMaterial"sv,
                                     "gl_PointParameters"sv,
                                     "gl_Point"sv,
                                     "gl_TextureEnvColor"sv,
                                     "gl_ClipPlane"sv,
                                     "gl_EyePlaneS"sv,
                                     "gl_EyePlaneT"sv,
                                     "gl_EyePlaneR"sv,
                                     "gl_EyePlaneQ"sv,
                                     "gl_ObjectPlaneS"sv,
                                     "gl_ObjectPlaneT"sv,
                                     "gl_ObjectPlaneR"sv,
                                     "gl_ObjectPlaneQ"sv,
                                     "gl_Position"sv,
                                     "gl_PointSize"sv,
                                     "gl_ClipVertex"sv,
                                     "gl_Vertex"sv,
                                     "gl_Normal"sv,
                                     "gl_Color"sv,
                                     "gl_SecondaryColor"sv,
                                     "gl_MultiTexCoord[0-7]"sv,
                                     "gl_FogCoord"sv,
                                     "gl_FrontColor"sv,
                                     "gl_BackColor"sv,
                                     "gl_FrontSecondaryColor"sv,
                                     "gl_BackSecondaryColor"sv,
                                     "gl_TexCoord"sv,
                                     "gl_FogFragCoord"sv,
                                     "gl_FragData"sv,
                                     "gl_FragDepth"sv,
                                     "gl_FragColor"sv,
                                     "gl_FragCoord"sv,
                                     "gl_FrontFacing"sv,
                                     "gl_MaxVertexUniformComponents"sv,
                                     "gl_MaxFragmentUniformComponents"sv,
                                     "gl_MaxVertexAttribs"sv,
                                     "gl_MaxVaryingFloats"sv,
                                     "gl_MaxDrawBuffers "sv,
                                     "gl_MaxTextureCoords"sv,
                                     "gl_MaxTextureUnits"sv,
                                     "gl_MaxTextureImageUnits"sv,
                                     "gl_MaxVertexTextureImageUnits"sv,
                                     "gl_MaxCombinedTextureImageUnits"sv,
                                     "gl_MaxLights"sv,
                                     "gl_MaxClipPlanes"sv};
constexpr std::array builtinsFuncs = {"sin"sv,
                                      "cos"sv,
                                      "tab"sv,
                                      "asin"sv,
                                      "acos"sv,
                                      "atan"sv,
                                      "radians"sv,
                                      "degrees"sv,
                                      "pow"sv,
                                      "exp"sv,
                                      "log"sv,
                                      "exp2"sv,
                                      "log2"sv,
                                      "sqrt"sv,
                                      "inversesqrt"sv,
                                      "abs"sv,
                                      "ceil"sv,
                                      "clamp"sv,
                                      "floor"sv,
                                      "fract"sv,
                                      "max"sv,
                                      "min"sv,
                                      "mix"sv,
                                      "mod"sv,
                                      "sign"sv,
                                      "smoothstep"sv,
                                      "step"sv,
                                      "matrixCompMult"sv,
                                      "ftransform"sv,
                                      "cross"sv,
                                      "distance"sv,
                                      "dot"sv,
                                      "faceforward"sv,
                                      "length"sv,
                                      "normalize"sv,
                                      "reflect"sv,
                                      "refract"sv,
                                      "dFdx"sv,
                                      "dFdy"sv,
                                      "fwidth"sv,
                                      "all"sv,
                                      "any"sv,
                                      "equal"sv,
                                      "greaterThan"sv,
                                      "greaterThanEqual"sv,
                                      "lessThan"sv,
                                      "lessThanEqual"sv,
                                      "not"sv,
                                      "notEqual"sv,
                                      "texture[1-3]D"sv,
                                      "texture1DProj"sv,
                                      "texture[1-3]DProj"sv,
                                      "textureCube"sv,
                                      "shadow[1-2]D"sv,
                                      "shadow[1-2]DProj"sv,
                                      "texture[1-3]DProjLod"sv,
                                      "texture[1-2]DLod"sv,
                                      "textureCubeLod"sv,
                                      "shadow[1-2]DLod"sv,
                                      "shadow1DProjLod"sv,
                                      "shadow2DProjLod"sv,
                                      "noise[1-4]"sv};

constexpr std::array voidmain = {"main"sv};
constexpr std::array preprocessor = {
    "define"sv, "include"sv, "if"sv,    "ifdef"sv,  "ifndef"sv, "else"sv,
    "elif"sv,   "endif"sv,   "error"sv, "pragma"sv, "line"sv,   "version"sv,
};

constexpr std::array preprocessorAdditional = {"__LINE__"sv, "__FILE__"sv, "__VERSION__"sv};

constexpr std::array operators = {"\\+"sv, "-"sv, "\\*"sv, "\\/"sv, "<"sv, ">"sv,   "="sv,
                                  "!"sv,   "&"sv, "\\|"sv, "\\^"sv, "%"sv, "\\?"sv, ":"sv};

}  // namespace

std::vector<std::shared_ptr<std::function<void()>>> utilqt::setGLSLSyntaxHighlight(
    SyntaxHighligther& sh, GLSLSyntaxHighlight& settings) {

    QColor bgColor = utilqt::toQColor(settings.backgroundColor);

    QTextCharFormat defaultFormat;
    defaultFormat.setBackground(bgColor);
    defaultFormat.setForeground(utilqt::toQColor(settings.textColor));

    QTextCharFormat typeformat;
    typeformat.setBackground(bgColor);
    typeformat.setForeground(utilqt::toQColor(settings.typeColor));

    QTextCharFormat qualifiersformat;
    qualifiersformat.setBackground(bgColor);
    qualifiersformat.setForeground(utilqt::toQColor(settings.keywordColor));

    QTextCharFormat builtinsVarformat;
    builtinsVarformat.setBackground(bgColor);
    builtinsVarformat.setForeground(utilqt::toQColor(settings.builtinVarColor));

    QTextCharFormat builtinsFuncformat;
    builtinsFuncformat.setBackground(bgColor);
    builtinsFuncformat.setForeground(utilqt::toQColor(settings.builtinFuncColor));

    QTextCharFormat commentformat;
    commentformat.setBackground(bgColor);
    commentformat.setForeground(utilqt::toQColor(settings.commentColor));

    QTextCharFormat preprocessorformat;
    preprocessorformat.setBackground(bgColor);
    preprocessorformat.setForeground(utilqt::toQColor(settings.preProcessorColor));

    QTextCharFormat constantsformat;
    constantsformat.setBackground(bgColor);
    constantsformat.setForeground(utilqt::toQColor(settings.constantColor));

    QTextCharFormat mainformat;
    mainformat.setBackground(bgColor);
    mainformat.setForeground(utilqt::toQColor(settings.mainColor));

    QTextCharFormat litteralFormat;
    litteralFormat.setBackground(bgColor);
    litteralFormat.setForeground(utilqt::toQColor(settings.litteralColor));

    sh.clear();

    sh.setFont(settings.font);
    sh.setFontSize(settings.fontSize);
    sh.setHighlight(settings.highLightColor);
    sh.setDefaultFormat(defaultFormat);

    sh.addWordBoundaryPattern(typeformat, types);
    sh.addWordBoundaryPattern(qualifiersformat, qualifiers);
    sh.addWordBoundaryPattern(builtinsVarformat, builtinsVars);
    sh.addWordBoundaryPattern(builtinsFuncformat, builtinsFuncs);
    sh.addWordBoundaryPattern(mainformat, voidmain);

    sh.addPatternWithFormatStr(preprocessorformat, preprocessor, "#[ \t]*{}");
    sh.addWordBoundaryPattern(preprocessorformat, preprocessorAdditional);

    sh.addPatterns(preprocessorformat, operators);

    sh.addPattern(constantsformat, "\\b([0-9]+\\.)?[0-9]+([eE][+-]?[0-9]+)?");

    sh.addPattern(litteralFormat, R"("([^"\\]|\\.)*")");

    sh.addPattern(commentformat, "\\/\\/.*$");
    sh.addMultBlockPattern(commentformat, R"(/\*)"sv, R"(\*/)"sv);

    sh.update();

    std::vector<std::shared_ptr<std::function<void()>>> callbacks;
    for (auto p : settings) {
        callbacks.emplace_back(p->onChangeScoped(
            [psh = &sh, psettings = &settings]() { setGLSLSyntaxHighlight(*psh, *psettings); }));
    }
    return callbacks;
}

}  // namespace inviwo
