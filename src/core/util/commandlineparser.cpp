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

#include <inviwo/core/util/commandlineparser.h>
#include <inviwo/core/common/inviwoapplication.h>

namespace inviwo {

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
    : argc_(argc)
    , argv_(argv)
    , cmdQuiet_(
          "Inviwo, Interactive Visualization Workshop, a rapid prototyping environment for "
          "interactive visualizations",
          ' ', IVW_VERSION, false)
    , cmd_(
          "Inviwo, Interactive Visualization Workshop, a rapid prototyping environment for "
          "interactive visualizations",
          ' ', IVW_VERSION)
    , workspace_("w", "workspace", "Specify workspace to open", false, "", "workspace file")
    , outputPath_("o", "output", "Specify output path", false, "", "output path")
    , logfile_("l", "logfile", "Write log messages to file.", false, "", "logfile")
    , logConsole_("c", "logconsole", "Write log messages to console (cout)", false)
    , noSplashScreen_("n", "nosplash", "Pass this flag if you do not want to show a splash screen.")
    , quitAfterStartup_("q", "quit", "Pass this flag if you want to close inviwo after startup.")
    , wildcard_()
    , helpQuiet_("h", "help", "")
    , versionQuiet_("v", "version", "")
    , disableResourceManager_("", "no-resource-manager",
                              "Pass this flag to disable the resource manager") {
    cmdQuiet_.add(workspace_);
    cmdQuiet_.add(outputPath_);
    cmdQuiet_.add(quitAfterStartup_);
    cmdQuiet_.add(noSplashScreen_);
    cmdQuiet_.add(logfile_);
    cmdQuiet_.add(logConsole_);
    cmdQuiet_.add(helpQuiet_);
    cmdQuiet_.add(versionQuiet_);
    cmdQuiet_.add(disableResourceManager_);
    cmdQuiet_.add(wildcard_);

    cmd_.add(workspace_);
    cmd_.add(outputPath_);
    cmd_.add(quitAfterStartup_);
    cmd_.add(noSplashScreen_);
    cmd_.add(logfile_);
    cmd_.add(logConsole_);
    cmd_.add(disableResourceManager_);

    parse(Mode::Quiet);
}

CommandLineParser::~CommandLineParser() = default;

void CommandLineParser::parse(int argc, char** argv, Mode mode) {
    switch (mode) {
        case CommandLineParser::Mode::Normal: {
            helpQuiet_.reset();
            versionQuiet_.reset();
            try {
                if (argc > 0) {
                    cmd_.reset();
                    cmd_.parse(argc, argv);
                }
            } catch (TCLAP::ArgException& e) {
                std::cerr << "error: " << e.error() << " for arg " << e.argId()
                          << std::endl;  // catch exceptions
            }
            break;
        }

        case CommandLineParser::Mode::Quiet: {
            try {
                if (argc > 0) {
                    cmdQuiet_.reset();
                    cmdQuiet_.parse(argc, argv);
                }
            } catch (...) {
            }
            break;
        }
        default:
            break;
    }
}

void CommandLineParser::parse(Mode mode) { parse(argc_, argv_, mode); }

void CommandLineParser::setArgc(int argc) { argc_ = argc; }

void CommandLineParser::setArgv(char** argv) { argv_ = argv; }

const std::string CommandLineParser::getOutputPath() const {
    if (outputPath_.isSet()) return (outputPath_.getValue());
    return "";
}

const std::string CommandLineParser::getWorkspacePath() const {
    if (workspace_.isSet()) return (workspace_.getValue());
    return "";
}

const std::string CommandLineParser::getLogToFileFileName() const {
    if (logfile_.isSet())
        return (logfile_.getValue());
    else
        return "";
}

bool CommandLineParser::getQuitApplicationAfterStartup() const {
    return quitAfterStartup_.getValue();
}

bool CommandLineParser::getShowSplashScreen() const {
    if (versionQuiet_.isSet() || helpQuiet_.isSet())
        return false;
    else
        return !(noSplashScreen_.isSet());
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

bool CommandLineParser::getDisableResourceManager() const {
    return disableResourceManager_.isSet();
}

int CommandLineParser::getARGC() const { return argc_; }

char** CommandLineParser::getARGV() const { return argv_; }

void CommandLineParser::processCallbacks() {
    std::sort(
        callbacks_.begin(), callbacks_.end(),
        [](const decltype(callbacks_)::value_type& a, const decltype(callbacks_)::value_type& b) {
            return std::get<0>(a) < std::get<0>(b);
        });
    for (auto& elem : callbacks_) {
        if (std::get<1>(elem)->isSet()) {
            std::get<2>(elem)();
        }
    }
}

void CommandLineParser::add(TCLAP::Arg* arg) { cmd_.add(arg); }

void CommandLineParser::add(TCLAP::Arg* arg, std::function<void()> callback, int priority) {
    cmd_.add(arg);
    callbacks_.push_back(std::make_tuple(priority, arg, callback));
}

void CommandLineParser::xorAdd(TCLAP::Arg* a, std::function<void()> callbackA, int priorityA,
                               TCLAP::Arg* b, std::function<void()> callbackB, int priorityB) {
    auto args = std::vector<TCLAP::Arg*>{a, b};
    cmd_.xorAdd(args);
    callbacks_.push_back(std::make_tuple(priorityA, a, callbackA));
    callbacks_.push_back(std::make_tuple(priorityB, b, callbackB));
}

void CommandLineParser::remove(TCLAP::Arg* arg) {
    auto& args = cmd_.getArgList();
    auto argIt = std::find_if(std::begin(args), std::end(args),
                              [arg](const auto& arg_) { return arg_ == arg; });
    if (argIt != args.end()) {
        args.erase(argIt);
    }
    auto it = std::find_if(std::begin(callbacks_), std::end(callbacks_),
                           [arg](const auto& callback) { return std::get<1>(callback) == arg; });
    if (it != callbacks_.end()) {
        callbacks_.erase(it);
    }
}

}  // namespace inviwo
