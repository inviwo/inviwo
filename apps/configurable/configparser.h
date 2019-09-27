#ifndef CONFIGPARSER_H
#define CONFIGPARSER_H

#include <string>
#include <vector>

#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/propertyowner.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/network/processornetwork.h>

/**
 * Represents processor description in config file.
 */
struct ProcDesc {
    ProcDesc(std::string id) : id(id) {}
    std::string id;  // proc identifier
    bool operator==(ProcDesc& other) { return this->id == other.id; }
    bool operator!=(ProcDesc& other) { return this->id != other.id; }
};

/**
 * Represents property description in config file.
 * Has references to form tree structures, where processors are roots,
 * composites are inner nodes and properties are leaf nodes.
 * Note that this is not a tree like you would expect,
 * because properties have a global order, independent of what processors they belong to.
 */
struct PropDesc {
    PropDesc(size_t procIndex, int compositeIndex, std::string id, std::string displayName = "")
        : procIndex(procIndex), compositeIndex(compositeIndex), id(id), displayName(displayName) {}
    size_t procIndex;    // prop must belong to processor
    int compositeIndex;  // prop can belong to composite prop
    std::string id;      // prop identifier
    std::string displayName;
};

/**
 * Represents config file.
 * Defines properties that appear in the GUI, including their order.
 * Points to a workspace file, that it can be applied to.
 */
struct Config {
    bool parseSuccess = true;
    std::vector<ProcDesc> procs;
    std::vector<PropDesc> props;
    std::string workspace;
};

// test config file under: \inviwo\build\apps\minimals\qt

/**
 * Load config from file,
 * where first line contains workspace
 * and all following lines the properties with the following syntax:
 * Processor.Composite.Property:DisplayName
 */
Config loadConfig(std::string file);

/**
 * Searches the property object for the given descriptor in the processor network.
 * Uses parsed config to find property in correct subtree.
 * Returns pointer to the property and optionally pointer to its owner.
 * If property not found, nullptr is returned.
 */
inviwo::Property* getProp(const PropDesc& desc, const Config& config,
                          inviwo::ProcessorNetwork* network,
                          inviwo::PropertyOwner** outOwner = 0);

#endif