/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
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

#ifndef IVW_COMMANDLINEPARSER_H
#define IVW_COMMANDLINEPARSER_H

#include <inviwo/core/common/inviwo.h>

#include <string>
#include <vector>

#if defined(HAVE_CONFIG_H)
#  define HAVE_CONFIG_H_ENABLED
#  undef HAVE_CONFIG_H
#endif // HAVE_CONFIG_H

#include <tclap/CmdLine.h>


#if defined(HAVE_CONFIG_H_ENABLED)
#  define HAVE_CONFIG_H
#endif


namespace inviwo {

/** \brief Wrapper class to handle command line parsing.
 *
 *  Wraps around TCLAP to provide command line argument parsing.
 *
 *  TODO: Change into an abstract, general, class and then a derived subclass,
 *  which contains the TCLAP specific parts.
 *
 *  TODO2: Not yet fully implemented
 */
class IVW_CORE_API CommandLineParser {

public:
    CommandLineParser();
    CommandLineParser(int argc, char** argv);
    ~CommandLineParser();

    void parse(int argc, char** argv);

    void parse();

    void setArgc(int argc) {
        argc_ = argc;
    }

    void setArgv(char** argv) {
        argv_ = argv;
    }
    const std::string getOutputPath() const;
    const std::string getWorkspacePath() const;
    bool getCaptureAfterStartup() const;
    const std::string getSnapshotName() const;
    bool getScreenGrabAfterStartup() const;
    const std::string getScreenGrabName() const;
    bool getRunPythonScriptAfterStartup() const;
    const std::string getPythonScriptName() const;
    const std::string getLogToFileFileName() const;
    bool getQuitApplicationAfterStartup() const;
    bool getLoadWorkspaceFromArg() const;
    bool getShowSplashScreen() const;
    bool getLogToFile() const;

    int getARGC()const {return argc_;}
    char** getARGV()const {return argv_;}
    
private:
    int argc_;
    char** argv_;
    TCLAP::CmdLine cmd_;
    TCLAP::ValueArg<std::string> workspaceValueArg_;
    TCLAP::ValueArg<std::string> outputValueArg_;
    TCLAP::ValueArg<std::string> snapshotArg_;
    TCLAP::ValueArg<std::string> screenGrabArg_;
    TCLAP::ValueArg<std::string> pythonScriptArg_;
    TCLAP::ValueArg<std::string> logToFileArg_;
    TCLAP::SwitchArg noSplashScreenArg_;
    TCLAP::SwitchArg quitArg_;
    std::string workspaceName_;
};

} // namespace

#endif // IVW_COMMANDLINEPARSER_H
