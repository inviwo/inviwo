/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef IVW_GLMOPERATORS_H
#define IVW_GLMOPERATORS_H

template<typename T>
std::ostream& operator << (std::ostream& s, glm::detail::tvec2<T, glm::defaultp> const& v) {
    s << glm::to_string(v);
    return s;
}

template<typename T>
std::ostream& operator << (std::ostream& s, glm::detail::tvec3<T, glm::defaultp> const& v) {
    s << glm::to_string(v);
    return s;
}

template<typename T>
std::ostream& operator << (std::ostream& s, glm::detail::tvec4<T, glm::defaultp> const& v) {
    s << glm::to_string(v);
    return s;
}

template<typename T>
std::ostream& operator << (std::ostream& s, glm::detail::tmat2x2<T, glm::defaultp> const& m) {
    s << glm::to_string(m);
    return s;
}

template<typename T>
std::ostream& operator << (std::ostream& s, glm::detail::tmat3x3<T, glm::defaultp> const& m) {
    s << glm::to_string(m);
    return s;
}

template<typename T>
std::ostream& operator << (std::ostream& s, glm::detail::tmat4x4<T, glm::defaultp> const& m) {
    s << glm::to_string(m);
    return s;
}

#endif
