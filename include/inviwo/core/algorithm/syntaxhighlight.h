/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024-2026 Inviwo Foundation
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

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/document.h>

#include <string_view>

namespace inviwo::util {

/**
 * Applies syntax highlighting to source code and appends the resulting HTML tokens as
 * child nodes (plain text nodes and colored `<span>` elements) to @p handle.
 *
 * Supported languages (case-sensitive lang tag):
 *   C++  : "cpp", "c++", "cxx", "cc", "c"
 *   Python: "python", "py"
 *   GLSL  : "glsl", "frag", "vert", "comp"
 *
 * For unrecognised language tags the code is appended as HTML-escaped plain text
 * without any colour markup.
 *
 * The colour mapping mirrors the GitHub `.pl-*` CSS classes used by the Linguist
 * highlighter:
 *   keywords           → #66d9ef  (.pl-k)
 *   string literals    → #e6db74  (.pl-s)
 *   numeric literals   → #f92672  (.pl-c1)
 *   comments           → #f6f8fa  (.pl-c)
 *   function names     → #a6e22e  (.pl-en)
 *
 * @param handle  DocumentHandle for the element that will receive the tokens as children.
 * @param code    Raw (unescaped) source-code text.
 * @param lang    Fenced-code-block language tag (e.g. "cpp", "python", "glsl").
 */
IVW_CORE_API void highlightCode(Document::DocumentHandle handle, std::string_view code,
                                std::string_view lang);

}  // namespace inviwo::util
