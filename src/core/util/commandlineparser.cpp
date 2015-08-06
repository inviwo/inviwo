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

namespace inviwo {

CommandLineParser::CommandLineParser() : CommandLineParser(0, nullptr) {}

CommandLineParser::CommandLineParser(int argc, char** argv) try
    : argc_(argc),
      argv_(argv),
      cmd_("Inviwo description...", ' ', IVW_VERSION),
      workspaceValueArg_("w", "workspacePath", "Specify workspace to open", false, "",
                         "Name of workspace"),
      outputValueArg_("o", "outputPath", "Specify output path", false, "", "Output path"),
      snapshotArg_("s", "snapshot",
                   "Specify default name of each snapshot, or empty string for processor name.",
                   false, "", "Snapshot default name: UPN=Use Processor name."),
      screenGrabArg_("g", "screengrab", "Specify default name of each screengrab.", false, "", ""),
      pythonScriptArg_("p", "pythonScript", "Specify a python script to run at startup", false, "",
                       "Path to the file containing the script"),
      logToFileArg_("l", "logtofile", "Write log messages to file.", false, "", ""),
      noSplashScreenArg_("n", "nosplash",
                         "Pass this flag if you do not want to show a splash screen."),
      quitArg_("q", "quit", "Pass this flag if you want to close inviwo after startup.") {
    cmd_.add(workspaceValueArg_);
#if defined(IVW_MODULE_PYTHON3)
    cmd_.add(pythonScriptArg_);
#endif
    cmd_.add(outputValueArg_);
    cmd_.add(snapshotArg_);
    cmd_.add(screenGrabArg_);
    cmd_.add(quitArg_);
    cmd_.add(noSplashScreenArg_);
    cmd_.add(logToFileArg_);

    parse();

} catch (TCLAP::ArgException& e) {
    LogError(e.error() << " for arg " << e.argId());
}

CommandLineParser::~CommandLineParser() {}

const std::string CommandLineParser::getOutputPath() const {
    if (outputValueArg_.isSet()) return (outputValueArg_.getValue());

    return "";
}

const std::string CommandLineParser::getWorkspacePath() const {
    if (workspaceValueArg_.isSet()) return (workspaceValueArg_.getValue());

    return "";
}

void CommandLineParser::parse(int argc, char** argv) {
    try {
        if (argc > 0){
            cmd_.parse(argc, argv);
        }
    } catch (TCLAP::ArgException& e) {
        std::cerr << "error: " << e.error() << " for arg " << e.argId()
                  << std::endl;  // catch exceptions
    }
}

void CommandLineParser::parse() { parse(argc_, argv_); }

bool CommandLineParser::getCaptureAfterStartup() const { return snapshotArg_.isSet(); }

const std::string CommandLineParser::getSnapshotName() const {
    if (snapshotArg_.isSet()) return (snapshotArg_.getValue());

    return "";
}

bool CommandLineParser::getScreenGrabAfterStartup() const { return screenGrabArg_.isSet(); }

const std::string CommandLineParser::getScreenGrabName() const {
    if (screenGrabArg_.isSet()) return (screenGrabArg_.getValue());

    return "";
}

bool CommandLineParser::getRunPythonScriptAfterStartup() const { return pythonScriptArg_.isSet(); }

const std::string CommandLineParser::getPythonScriptName() const {
    if (pythonScriptArg_.isSet()) return (pythonScriptArg_.getValue());

    return "";
}

const std::string CommandLineParser::getLogToFileFileName() const {
    if (logToFileArg_.isSet()) return (logToFileArg_.getValue());

    return "";
}

bool CommandLineParser::getQuitApplicationAfterStartup() const { return quitArg_.getValue(); }

bool CommandLineParser::getShowSplashScreen() const { return !(noSplashScreenArg_.isSet()); }

bool CommandLineParser::getLoadWorkspaceFromArg() const {
    if (workspaceValueArg_.isSet()) {
        std::string values = workspaceValueArg_.getValue();
        assert(values.size() != 0);
        return true;
    }

    return false;
}

bool CommandLineParser::getLogToFile() const {
    if (logToFileArg_.isSet()) {
        std::string values = logToFileArg_.getValue();
        assert(values.size() != 0);
        return true;
    }

    return false;
}

}  // namespace
