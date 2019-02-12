/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <inviwo/meta/inviwometadefine.hpp>

// Workaround for "combaseapi.h(229): error C2187: syntax error: 'identifier' was unexpected here"
// when using /permissive-
struct IUnknown;
#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/parse_tree.hpp>
#include <string>

namespace inviwo::meta {
namespace cmake {

namespace p = tao::pegtl;

// Grammer
struct INVIWO_META_API space : p::plus<p::blank> {};
struct INVIWO_META_API newline : p::sor<p::string<'\r', '\n'>, p::one<'\n'>> {};
struct INVIWO_META_API line_comment : p::seq<p::one<'#'>, p::star<p::seq<p::not_at<newline>, p::any>>> {};
struct INVIWO_META_API line_ending : p::seq<p::opt<line_comment>, newline> {};
struct INVIWO_META_API begin_brace : p::one<'('> {};
struct INVIWO_META_API end_brace : p::one<')'> {};
struct INVIWO_META_API identifier : p::identifier {};
struct INVIWO_META_API seperation : p::sor<space, line_ending> {};

struct INVIWO_META_API bracket_id : p::star<p::one<'='>> {};
struct INVIWO_META_API bracket_open : p::seq<p::one<'['>, bracket_id, p::one<'['>> {};
struct INVIWO_META_API bracket_mark {
    using analyze_t = p::analysis::generic<p::analysis::rule_type::SEQ>;

    template <p::apply_mode, p::rewind_mode, template <typename...> class Action,
              template <typename...> class Control, typename Input, typename... ExtraStates>
    static bool match(Input& in, const std::string& id, ExtraStates&&...) {
        const auto size = id.size();
        if (in.size(size) >= size) {
            if (std::memcmp(in.current(), id.data(), size) == 0) {
                in.bump(size);
                return true;
            }
        }
        return false;
    }
};
struct INVIWO_META_API bracket_close : p::seq<p::one<']'>, bracket_mark, p::one<']'>> {};
struct INVIWO_META_API bracket_body : p::any {};
struct INVIWO_META_API bracket_argument : p::seq<bracket_open, p::until<bracket_close, bracket_body>> {};
struct INVIWO_META_API bracket_comment : p::seq<p::one<'#'>, bracket_argument> {};

struct INVIWO_META_API escape_identity
    : p::seq<p::one<'\\'>,
             p::seq<p::not_at<p::ranges<'A', 'Z', 'a', 'z', '0', '9', ';'>>, p::any>> {};
struct INVIWO_META_API escape_encoded : p::sor<p::string<'\\', 't'>, p::string<'\\', 'r'>, p::string<'\\', 'n'>> {};
struct INVIWO_META_API escape_semicolon : p::string<'\\', ';'> {};
struct INVIWO_META_API escape_sequence : p::sor<escape_identity, escape_encoded, escape_semicolon> {};

struct INVIWO_META_API unquoted_element
    : p::sor<p::not_one<'(', ')', '#', '"', '\\', ' ', '\t', '\n', '\r'>, escape_sequence> {};

struct INVIWO_META_API quoted_continuation : p::seq<p::one<'\\'>, newline> {};
struct INVIWO_META_API quoted_element : p::sor<p::not_one<'\\', '"'>, escape_sequence, quoted_continuation> {};

struct INVIWO_META_API quoted_argument : p::seq<p::one<'"'>, p::star<quoted_element>, p::one<'"'>> {};
struct INVIWO_META_API unquoted_argument : p::plus<unquoted_element> {};  // TODO unquoted_legacy

struct INVIWO_META_API argument : p::sor<bracket_argument, quoted_argument, unquoted_argument> {};

struct arguments;
struct INVIWO_META_API seperated_arguments
    : p::sor<p::seq<p::plus<seperation>, p::opt<argument>>,
             p::seq<p::star<seperation>, begin_brace, arguments, end_brace>> {};

struct INVIWO_META_API arguments : p::seq<p::opt<argument>, p::star<seperated_arguments>> {};

struct INVIWO_META_API command
    : p::seq<p::star<space>, identifier, p::star<space>, begin_brace, arguments, end_brace> {};

struct INVIWO_META_API file_element : p::sor<p::seq<command, p::opt<space>, line_ending>,
                             p::seq<p::star<p::sor<bracket_comment, space>>, line_ending>> {};

struct INVIWO_META_API file : p::must<p::seq<p::bof, p::star<file_element>, p::eof>> {};

template <typename Rule>
struct action : p::nothing<Rule> {};

template <>
struct action<bracket_id> {
    template <typename Input, typename... ExtraStates>
    static void apply(const Input& in, std::string& id, ExtraStates&&...) {
        id = in.string();
    }
};

// clang-format off
template <typename Rule>
using selector =
    p::parse_tree::selector<Rule, 
        p::parse_tree::apply_store_content::to<
            identifier,
            argument,
            line_comment,
            bracket_comment,
            space,
            newline,
            begin_brace,
            end_brace
        >,
        p::parse_tree::apply_discard_empty::to<
            arguments
        >,
        p::parse_tree::apply_remove_content::to<
            file,  
            command
        >
    >;
// clang-format on

}  // namespace cmake
}  // namespace inviwo::meta
