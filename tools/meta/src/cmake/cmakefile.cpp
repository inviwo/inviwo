/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2024 Inviwo Foundation
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

#include <inviwo/meta/cmake/cmakefile.hpp>
#include <inviwo/meta/cmake/grammar.hpp>
#include <inviwo/meta/util.hpp>

#include <functional>
#include <iostream>

#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/parse_tree.hpp>
#include <fmt/format.h>

namespace inviwo::meta::cmake {

namespace {

void convert(std::vector<ArgElement>& args, const p::parse_tree::node& n) {
    for (const auto& child : n.children) {
        if (child->is_type<argument>()) {
            args.emplace_back(Argument{child->string()});
        } else if (child->is_type<space>()) {
            args.emplace_back(Space{child->string()});
        } else if (child->is_type<newline>()) {
            args.emplace_back(LineEnding{});
        } else if (child->is_type<line_comment>() || child->is_type<bracket_comment>()) {
            args.emplace_back(Comment{child->string()});
        } else if (child->is_type<begin_brace>()) {
            args.emplace_back(BeginBrace{});
        } else if (child->is_type<end_brace>()) {
            args.emplace_back(EndBrace{});
        } else if (child->is_type<arguments>()) {
            convert(args, *child);
        } else {
            throw util::makeError("Error: Unhandled node '{}' while parsing CMakeLists: '{}'",
                                  child->type, n.source);
        }
    }
}

void convert(Command& command, const p::parse_tree::node& n) {
    const auto size = n.children.size();
    size_t ind = 0;
    if (size > ind && n.children[ind]->is_type<space>()) {
        command.space1 = {n.children[ind]->string()};
        ++ind;
    }
    if (size > ind && n.children[ind]->is_type<identifier>()) {
        command.identifier = n.children[ind]->string();
        ++ind;
    } else {
        throw std::runtime_error("Error: Missing identifier: " + n.string());
    }

    if (size > ind && n.children[ind]->is_type<space>()) {
        command.space2 = {n.children[ind]->string()};
        ++ind;
    }

    if (size > ind && n.children[ind]->is_type<begin_brace>()) {
        ++ind;
    } else {
        throw std::runtime_error("Error: Missing begin brace: " + n.string());
    }

    if (size > ind && n.children[ind]->is_type<arguments>()) {
        convert(command.arguments, *n.children[ind]);
        ++ind;
    }

    if (size > ind && n.children[ind]->is_type<end_brace>()) {
        ++ind;
    } else {
        throw std::runtime_error("Error: Missing end brace: " + n.string());
    }
}

void convert(CMakeFile& cmakefile, const p::parse_tree::node& n) {
    for (auto& child : n.children) {
        if (child->is_type<line_comment>() || child->is_type<bracket_comment>()) {
            cmakefile.items.emplace_back(Comment{child->string()});
        } else if (child->is_type<newline>()) {
            cmakefile.items.emplace_back(LineEnding{});
        } else if (child->is_type<space>()) {
            cmakefile.items.emplace_back(Space{child->string()});
        } else if (child->is_type<command>()) {
            Command cmd{};
            convert(cmd, *child);
            cmakefile.items.emplace_back(std::move(cmd));
        } else {
            throw util::makeError("Error: Unhandled node '{}' while parsing CMakeLists: '{}'",
                                  child->type, n.source);
        }
    }
}

template <typename Input>
void parse(CMakeFile& file, Input&& input) {
    namespace p = tao::pegtl;
    std::string bracket_id;
    try {
        const auto root = p::parse_tree::parse<cmake::file, cmake::selector, cmake::action>(
            std::forward<Input>(input), bracket_id);
        convert(file, *(root->children[0]));
    } catch (const p::parse_error& e) {
        auto& pos0 = e.positions().front();
        throw util::makeError("Error: Parsing CMakeFile: '{}' at line: {} col: {}", pos0.source,
                              pos0.line, pos0.column);
    }
}

template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

}  // namespace

CMakeFile::CMakeFile(const std::filesystem::path& path) {
    tao::pegtl::file_input<> in(path);
    parse(*this, in);
}

CMakeFile::CMakeFile(const std::string_view string) {
    tao::pegtl::memory_input in(string.data(), string.size(), "");
    parse(*this, in);
}

std::ostream& CMakeFile::print(std::ostream& os) const {
    for (const auto& item : items) {
        std::visit(overloaded{[&os](const Space& arg) { os << arg.value; },
                              [&os](const LineEnding&) { os << '\n'; },
                              [&os](const Comment& arg) { os << arg.value; },
                              [&os](const Command& arg) { arg.print(os); }},
                   item);
    }
    return os;
}

std::ostream& Command::print(std::ostream& os) const {
    os << space1.value << identifier << space2.value << "(";
    for (const auto& item : arguments) {
        std::visit(overloaded{[&os](const Space& arg) { os << arg.value; },
                              [&os](const LineEnding&) { os << '\n'; },
                              [&os](const BeginBrace&) { os << '('; },
                              [&os](const EndBrace&) { os << ')'; },
                              [&os](const Argument& arg) { os << arg.value; },
                              [&os](const Comment& arg) { os << arg.value; }},
                   item);
    }
    os << ")";
    return os;
}

CMakeFile::iterator CMakeFile::begin() {
    return TransformIterator{transform, FilterIterator(filter, items.begin(), items.end())};
}
CMakeFile::iterator CMakeFile::end() {
    return TransformIterator{transform, FilterIterator(filter, items.end(), items.end())};
}

CMakeFile::const_iterator CMakeFile::begin() const {
    return TransformIterator{const_transform, FilterIterator(filter, items.begin(), items.end())};
}
CMakeFile::const_iterator CMakeFile::end() const {
    return TransformIterator{const_transform, FilterIterator(filter, items.end(), items.end())};
}

Range<CMakeFile::iterator> CMakeFile::commands() { return Range{begin(), end()}; }
Range<CMakeFile::const_iterator> CMakeFile::commands() const { return Range{begin(), end()}; }

Command::iterator Command::begin() {
    return TransformIterator{transform, FilterIterator(filter, arguments.begin(), arguments.end())};
}
Command::iterator Command::end() {
    return TransformIterator{transform, FilterIterator(filter, arguments.end(), arguments.end())};
}
Command::const_iterator Command::begin() const {
    return TransformIterator{const_transform,
                             FilterIterator(filter, arguments.begin(), arguments.end())};
}
Command::const_iterator Command::end() const {
    return TransformIterator{const_transform,
                             FilterIterator(filter, arguments.end(), arguments.end())};
}

Range<Command::iterator> Command::args() { return Range{begin(), end()}; }
Range<Command::const_iterator> Command::args() const { return Range{begin(), end()}; }

}  // namespace inviwo::meta::cmake
