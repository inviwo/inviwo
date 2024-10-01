/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2024 Inviwo Foundation
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

#include <inviwo/core/util/commandlineparser.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/common/inviwocommondefines.h>

namespace inviwo {

namespace {

static constexpr std::string_view info =
    "Inviwo, Interactive Visualization Workshop, a rapid prototyping environment for "
    "interactive visualizations";

}

CommandLineArgHolder::CommandLineArgHolder(InviwoApplication* app, TCLAP::Arg& arg)
    : app_{app}, arg_{arg} {
    app_->getCommandLineParser().add(&arg);
}

CommandLineArgHolder::CommandLineArgHolder(InviwoApplication* app, TCLAP::Arg& arg,
                                           std::function<void()> callback, int priority)
    : app_{app}, arg_{arg} {
    app_->getCommandLineParser().add(&arg, std::move(callback), priority);
}
CommandLineArgHolder::~CommandLineArgHolder() { app_->getCommandLineParser().remove(&arg_); }

CommandLineParser::CommandLineParser() : CommandLineParser(0, nullptr) {}

CommandLineParser::CommandLineParser(int argc, char** argv)
    : args_(argv, argv + argc)
    , ignoredArgs_{}
    , workspace_("w", "workspace", "Specify workspace to open", false, "", "workspace file")
    , outputPath_("o", "output", "Specify output path", false, "", "output path")
    , logfile_("l", "logfile", "Write log messages to file.", false, "", "logfile")
    , moduleSearchPaths_("m", "module-search-path", "Specify additional module search paths", false,
                         "module search path")
    , logConsole_("c", "logconsole", "Write log messages to console (cout)", false)
    , noSplashScreen_("n", "nosplash", "Pass this flag if you do not want to show a splash screen.")
    , quitAfterStartup_("q", "quit", "Pass this flag if you want to close inviwo after startup.")
    , version_{"", "version", "Displays version information and exits.", false}
    , help_{"h", "help", "Displays usage information and exits.", false} {

    args_[0] = std::filesystem::path(args_[0]).filename().string();


    auto it = std::ranges::find(args_, "--");
    if (it != args_.end()) {
        std::copy(std::next(it), args_.end(), std::back_inserter(ignoredArgs_));
    }
    args_.erase(it, args_.end());

    parseInternal(args_, Mode::Quiet);
}

CommandLineParser::~CommandLineParser() = default;

void CommandLineParser::parseInternal(std::vector<std::string> args, Mode mode) {

    TCLAP::CmdLine cmd{std::string{info}, ' ', toString(build::version), false};

    cmd.ignoreUnmatched(mode == Mode::Quiet);

    cmd.add(workspace_);
    cmd.add(outputPath_);
    cmd.add(quitAfterStartup_);
    cmd.add(noSplashScreen_);
    cmd.add(logfile_);
    cmd.add(moduleSearchPaths_);
    cmd.add(logConsole_);
    cmd.add(help_);
    cmd.add(version_);

    for (auto&& [prio, arg, callback] : callbacks_) {
        cmd.add(arg);
    }

    for (auto* arg : cmd.getArgList()) {
        arg->reset();
    }

    try {
        cmd.parse(args);

        if (mode == Mode::Normal && help_.isSet()) {
            cmd.getOutput()->usage(cmd);
            exit(0);
        }

        if (version_.isSet()) {
            cmd.getOutput()->version(cmd);
            exit(0);
        }
    } catch (TCLAP::ArgException& e) {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    }
}

void CommandLineParser::parse(int argc, char** argv) {
    parseInternal(std::vector<std::string>(argv, argv + argc), Mode::Normal);
}

void CommandLineParser::parse() { parseInternal(args_, Mode::Normal); }

std::filesystem::path CommandLineParser::getOutputPath() const {
    if (outputPath_.isSet()) return (outputPath_.getValue());
    return {};
}

std::filesystem::path CommandLineParser::getWorkspacePath() const {
    if (workspace_.isSet()) return (workspace_.getValue());
    return {};
}

std::filesystem::path CommandLineParser::getLogToFileFileName() const {
    if (logfile_.isSet())
        return (logfile_.getValue());
    else
        return {};
}

std::vector<std::filesystem::path> CommandLineParser::getModuleSearchPaths() const {
    if (moduleSearchPaths_.isSet()) {
        std::vector<std::filesystem::path> paths;
        for (auto& path : moduleSearchPaths_.getValue()) {
            paths.emplace_back(path);
        }
        return paths;
    } else {
        return {};
    }
}

bool CommandLineParser::getQuitApplicationAfterStartup() const {
    return quitAfterStartup_.getValue();
}

bool CommandLineParser::getShowSplashScreen() const {
    return !version_.isSet() && !help_.isSet() && !noSplashScreen_.isSet();
}

bool CommandLineParser::getLoadWorkspaceFromArg() const {
    if (workspace_.isSet()) {
        std::string values = workspace_.getValue();
        assert(values.size() != 0);
        return true;
    }

    return false;
}

bool CommandLineParser::getLogToFile() const {
    if (logfile_.isSet()) {
        std::string values = logfile_.getValue();
        assert(values.size() != 0);
        return true;
    }

    return false;
}

bool CommandLineParser::getLogToConsole() const { return logConsole_.isSet(); }

const std::vector<std::string>& CommandLineParser::getArgs() const { return args_; }

const std::vector<std::string>& CommandLineParser::getIgnoredArgs() const {
    return ignoredArgs_;
}

void CommandLineParser::processCallbacks() {
    std::sort(callbacks_.begin(), callbacks_.end(),
              [](const auto& a, const auto& b) { return std::get<0>(a) < std::get<0>(b); });
    for (auto&& [prio, arg, callback] : callbacks_) {
        if (arg->isSet() && callback) {
            callback();
        }
    }
}

void CommandLineParser::add(TCLAP::Arg* arg, std::function<void()> callback, int priority) {
    callbacks_.push_back(std::make_tuple(priority, arg, callback));
}

void CommandLineParser::remove(TCLAP::Arg* arg) {
    auto it = std::find_if(std::begin(callbacks_), std::end(callbacks_),
                           [arg](const auto& callback) { return std::get<1>(callback) == arg; });
    if (it != callbacks_.end()) {
        callbacks_.erase(it);
    }
}

}  // namespace inviwo
