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

#include <inviwo/core/util/commandlineparser.h>

#undef HAVE_CONFIG_H
#include <tclap/CmdLine.h>

namespace inviwo {

CommandLineParser::CommandLineParser() {
    cmd_ = new TCLAP::CmdLine("Inviwo description...", ' ', "0.0.0");
}

CommandLineParser::CommandLineParser(int argc, char** argv) : argc_(argc), argv_(argv)
{
    cmd_ = new TCLAP::CmdLine("Inviwo description...", ' ', "0.0.0");
}

CommandLineParser::~CommandLineParser() {
    delete cmd_;
    delete workspaceValueArg_;
    delete outputValueArg_;
    delete snapshotArg_;
    delete screenGrabArg_;
    delete pythonScriptArg_;
    delete noSplashScreenArg_;
    delete quitArg_;
    delete logToFileArg_;
}

void CommandLineParser::initialize() {
    // Set up available arguments and flags
    try {
        workspaceValueArg_ = new TCLAP::ValueArg<std::string>("w",
                "workspacePath",
                "Specify workspace to open",
                false,
                "",
                "Name of workspace");
        outputValueArg_ = new TCLAP::ValueArg<std::string>("o",
                "outputPath",
                "Specify output path",
                false,
                "",
                "Output path");
        pythonScriptArg_ = new TCLAP::ValueArg<std::string>("p",
                "pythonScript",
                "Specify a python script to run at startup",
                false,
                "",
                "Path to the file containing the script");
        snapshotArg_ = new TCLAP::ValueArg<std::string>("s",
                "snapshot",
                "Specify default name of each snapshot, or empty string for processor name.",
                false,
                "",
                "Snapshot default name: UPN=Use Processor name.");
        screenGrabArg_ = new TCLAP::ValueArg<std::string>("g",
                "screengrab",
                "Specify default name of each screengrab.",
                false,
                "",
                "");
        logToFileArg_ = new TCLAP::ValueArg<std::string>("l",
                "logtofile",
                "Write log messages to file.",
                false,
                "",
                "");
        quitArg_ = new TCLAP::SwitchArg("q", "quit",
                                        "Pass this flag if you want to close inviwo after startup.");
        noSplashScreenArg_ = new TCLAP::SwitchArg("n", "nosplash",
                "Pass this flag if you do not want to show a splash screen.");
        cmd_->add(*workspaceValueArg_);
        cmd_->add(*pythonScriptArg_);
        cmd_->add(*outputValueArg_);
        cmd_->add(*snapshotArg_);
        cmd_->add(*screenGrabArg_);
        cmd_->add(*quitArg_);
        cmd_->add(*noSplashScreenArg_);
        cmd_->add(*logToFileArg_);
    } catch (TCLAP::ArgException& e) {
        LogError(e.error() << " for arg " << e.argId());
    }
}

const std::string CommandLineParser::getOutputPath() const {
    if (outputValueArg_->isSet())
        return (outputValueArg_->getValue());

    return "";
}

const std::string CommandLineParser::getWorkspacePath() const {
    if (workspaceValueArg_->isSet())
        return (workspaceValueArg_->getValue());

    return "";
}

void CommandLineParser::parse(int argc, char** argv) {
    try {
        cmd_->parse(argc, argv);
    } catch (TCLAP::ArgException& e) {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; // catch exceptions
    }
}

void CommandLineParser::parse() {
    parse(argc_, argv_);
}

bool CommandLineParser::getCaptureAfterStartup() const {
    return snapshotArg_->isSet();
}

const std::string CommandLineParser::getSnapshotName() const {
    if (snapshotArg_->isSet())
        return (snapshotArg_->getValue());

    return "";
}


bool CommandLineParser::getScreenGrabAfterStartup() const {
    return screenGrabArg_->isSet();
}

const std::string CommandLineParser::getScreenGrabName() const {
    if (screenGrabArg_->isSet())
        return (screenGrabArg_->getValue());

    return "";
}


bool CommandLineParser::getRunPythonScriptAfterStartup() const {
    return pythonScriptArg_->isSet();
}

const std::string CommandLineParser::getPythonScriptName() const {
    if (pythonScriptArg_->isSet())
        return (pythonScriptArg_->getValue());

    return "";
}

const std::string CommandLineParser::getLogToFileFileName() const {
    if (logToFileArg_->isSet())
        return (logToFileArg_->getValue());

    return "";
}

bool CommandLineParser::getQuitApplicationAfterStartup() const {
    return quitArg_->getValue();
}

bool CommandLineParser::getShowSplashScreen() const {
    return !(noSplashScreenArg_->isSet());
}

bool CommandLineParser::getLoadWorkspaceFromArg() const {
    if (workspaceValueArg_->isSet()) {
        std::string values = workspaceValueArg_->getValue();
        assert(values.size() != 0);
        return true;
    }

    return false;
}

bool CommandLineParser::getLogToFile() const {
    if (logToFileArg_->isSet()) {
        std::string values = logToFileArg_->getValue();
        assert(values.size() != 0);
        return true;
    }

    return false;
}

void CommandLineParser::deinitialize() {
}

} // namespace
