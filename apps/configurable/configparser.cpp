#include "configparser.h"

#include <fstream>

#include <inviwo/core/util/filesystem.h>

Config loadConfig(std::string file) {
    Config config;
    std::ifstream in(file);

    if (!in.good()) {
        config.parseSuccess = false;
        return config;
    }

    std::string line;
    for (size_t lineIndex = 0; std::getline(in, line); lineIndex++) {

        // Trim empty lines
        if (line.empty()) continue;
        // Do not trim other whitespace since it may be part of identifiers

        // First line contains workspace path (relative to config file path)
        if (lineIndex == 0) {
            std::string dir = inviwo::filesystem::getFileDirectory(file);
            config.workspace = dir + "/" + line;
            continue;
        }

        size_t pos, last_pos;
        int procIndex;

        // The path to a property is separated by dots
        if ((pos = line.find('.')) != std::string::npos) {

            // There is one processor in each line...
            ProcDesc proc(line.substr(0, pos));

            // ... which maybe already appeared in previous lines...
            procIndex = -1;
            for (int i = 0; i < config.procs.size(); i++)
                if (config.procs[i] == proc) procIndex = i;

            // ... or not.
            if (procIndex < 0) {
                procIndex = (int)config.procs.size();
                config.procs.push_back(proc);
            }

            // At this position follow the properties...
            last_pos = pos;

            // ... possibly multiple composited properties...
            int last_propIndex = -1;  // points to parent composite if existing
            while ((pos = line.find('.', last_pos + 1)) != std::string::npos) {
                PropDesc prop(procIndex, last_propIndex,
                              line.substr(last_pos + 1, pos - last_pos - 1));
                last_propIndex = (int)config.props.size();
                config.props.push_back(prop);
                last_pos = pos;
            }

            // ... but at least one.
            // At the end may be an optional display name.
            const auto propStr = line.substr(last_pos + 1);
            const size_t colonPos = propStr.find(':');
            std::string propIdentifier, propDisplayName;
            if (colonPos != std::string::npos) {
                propIdentifier = propStr.substr(0, colonPos);
                propDisplayName = propStr.substr(colonPos + 1);
            } else {
                propIdentifier = propStr;
            }
            PropDesc prop(procIndex, last_propIndex, propIdentifier, propDisplayName);
            config.props.push_back(prop);
        }
    }

    // Useful config has at least one processor and at least one property per processor...
    if (config.procs.size() < 1 || config.props.size() < config.procs.size()) {
        config.parseSuccess = false;
    }
    // ... and all ids are non-empty strings
    for (const auto& proc : config.procs) {
        if (proc.id.empty()) {
            config.parseSuccess = false;
            break;
        }
    }
    for (const auto& prop : config.props) {
        if (prop.id.empty()) {
            config.parseSuccess = false;
            break;
        }
    }

    return config;
}

inviwo::Property* getProp(const PropDesc& desc, const Config& config,
                          inviwo::ProcessorNetwork* network, inviwo::PropertyOwner** outOwner) {
    if (desc.compositeIndex < 0) {
        inviwo::Processor* proc =
            network->getProcessorByIdentifier(config.procs[desc.procIndex].id);
        if (!proc) return nullptr;
        if (outOwner) *outOwner = proc;
        return proc->getPropertyByIdentifier(desc.id);
    }
    inviwo::CompositeProperty* composite = dynamic_cast<inviwo::CompositeProperty*>(
        getProp(config.props[desc.compositeIndex], config, network, outOwner));
    if (!composite) return nullptr;
    if (outOwner) *outOwner = composite;
    return composite->getPropertyByIdentifier(desc.id);
}