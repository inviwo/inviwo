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

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>

#include <string>
#include <vector>
#include <tuple>
#include <functional>
#include <filesystem>

#include <tclap/CmdLine.h>

namespace inviwo {

class InviwoApplication;

class WildCardArg : public TCLAP::Arg {
public:
    WildCardArg();
    virtual bool processArg(int* i, std::vector<std::string>& args) override;

    const std::vector<std::string>& getMatches() const;

    virtual void addToList(std::list<Arg*>& argList) const override;

private:
    std::vector<std::string> matches_;
};

class IVW_CORE_API CommandLineArgHolder {
public:
    CommandLineArgHolder(InviwoApplication* app, TCLAP::Arg& arg);
    CommandLineArgHolder(InviwoApplication* app, TCLAP::Arg& arg, std::function<void()> callback,
                         int priority = 0);
    CommandLineArgHolder(const CommandLineArgHolder&) = delete;
    CommandLineArgHolder(CommandLineArgHolder&&) = delete;
    CommandLineArgHolder& operator=(const CommandLineArgHolder&) = delete;
    CommandLineArgHolder& operator=(CommandLineArgHolder&&) = delete;

    ~CommandLineArgHolder();

private:
    InviwoApplication* app_;
    TCLAP::Arg& arg_;
};

/** \brief Wrapper class to handle command line parsing.
 *
 *  Wraps around TCLAP to provide command line argument parsing.
 */
class IVW_CORE_API CommandLineParser {
public:
    CommandLineParser();
    CommandLineParser(int argc, char** argv);
    ~CommandLineParser();

    void parse(int argc, char** argv);
    void parse();

    std::filesystem::path getOutputPath() const;
    std::filesystem::path getWorkspacePath() const;
    std::filesystem::path getLogToFileFileName() const;
    std::vector<std::filesystem::path> getModuleSearchPaths() const;
    bool getQuitApplicationAfterStartup() const;
    bool getLoadWorkspaceFromArg() const;
    bool getShowSplashScreen() const;
    bool getLogToFile() const;
    bool getLogToConsole() const;

    const std::vector<std::string>& getArgs() const;

    const std::vector<std::string>& getIgnoredArgs() const;

    void processCallbacks();

    void add(TCLAP::Arg* arg, std::function<void()> callback = nullptr, int priority = 0);
    void remove(TCLAP::Arg* arg);

private:
    enum class Mode { Normal, Quiet };
    void parseInternal(std::vector<std::string> args, Mode mode = Mode::Normal);

    std::vector<std::string> args_;
    std::vector<std::string> ignoredArgs_;  // Args after a "--"

    TCLAP::ValueArg<std::string> workspace_;
    TCLAP::ValueArg<std::string> outputPath_;
    TCLAP::ValueArg<std::string> logfile_;
    TCLAP::MultiArg<std::string> moduleSearchPaths_;
    TCLAP::SwitchArg logConsole_;
    TCLAP::SwitchArg noSplashScreen_;
    TCLAP::SwitchArg quitAfterStartup_;
    TCLAP::SwitchArg version_;
    TCLAP::SwitchArg help_;

    std::vector<std::tuple<int, TCLAP::Arg*, std::function<void()>>> callbacks_;
};

inline bool WildCardArg::processArg(int* i, std::vector<std::string>& args) {
    matches_.push_back(args[*i]);
    return true;
}

inline WildCardArg::WildCardArg()
    : TCLAP::Arg("*", "wildcard", "matches everything", false, false, nullptr) {}

inline const std::vector<std::string>& WildCardArg::getMatches() const { return matches_; }

// make sure we add the wildcard at the back.
inline void WildCardArg::addToList(std::list<TCLAP::Arg*>& argList) const {
    argList.push_back(const_cast<WildCardArg*>(this));
}

}  // namespace inviwo
