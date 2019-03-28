/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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

#include <inviwo/core/network/processornetworkconverter.h>
#include <inviwo/core/util/utilities.h>

namespace inviwo {

ProcessorNetworkConverter::ProcessorNetworkConverter(int from) : VersionConverter(), from_(from) {}

#include <warn/push>
#include <warn/ignore/implicit-fallthrough>
bool ProcessorNetworkConverter::convert(TxElement* root) {
    switch (from_) {
        case 0:
            traverseNodes(root, &ProcessorNetworkConverter::updateProcessorType);
            [[fallthrough]];
        case 1:
            traverseNodes(root, &ProcessorNetworkConverter::updateMetaDataTree);
            [[fallthrough]];
        case 2:
            traverseNodes(root, &ProcessorNetworkConverter::updatePropertType);
            [[fallthrough]];
        case 3:
            traverseNodes(root, &ProcessorNetworkConverter::updateShadingMode);
            [[fallthrough]];
        case 4:
            traverseNodes(root, &ProcessorNetworkConverter::updateCameraToComposite);
            [[fallthrough]];
        case 5:
            traverseNodes(root, &ProcessorNetworkConverter::updateMetaDataType);
            [[fallthrough]];
        case 6:
            traverseNodes(root, &ProcessorNetworkConverter::updateMetaDataKeys);
            [[fallthrough]];
        case 7:
            traverseNodes(root, &ProcessorNetworkConverter::updateDimensionTag);
            [[fallthrough]];
        case 8:
            traverseNodes(root, &ProcessorNetworkConverter::updatePropertyLinks);
            [[fallthrough]];
        case 9:
            ProcessorNetworkConverter::updatePortsInProcessors(root);
            [[fallthrough]];
        case 10:
            traverseNodes(root,
                          &ProcessorNetworkConverter::updateNoSpaceInProcessorClassIdentifers);
            [[fallthrough]];
        case 11:
            traverseNodes(root, &ProcessorNetworkConverter::updateDisplayName);
            [[fallthrough]];
        case 12:
            traverseNodes(root, &ProcessorNetworkConverter::updateProcessorIdentifiers);
            [[fallthrough]];
        case 13:
            traverseNodes(root, &ProcessorNetworkConverter::updateTransferfunctions);
            [[fallthrough]];
        case 14:
            usedIdentifier_.clear();
            traverseNodes(root, &ProcessorNetworkConverter::updateProcessorIdentifiersStriped);
            [[fallthrough]];
        case 15:
            traverseNodes(root, &ProcessorNetworkConverter::updatePropertyEditorMetadata);
            return true;  // Changes has been made.
        default:
            return false;  // No changes
    }
}

#include <warn/pop>

void ProcessorNetworkConverter::traverseNodes(TxElement* node, updateType update) {
    (this->*update)(node);
    ticpp::Iterator<ticpp::Element> child;
    for (child = child.begin(node); child != child.end(); child++) {
        traverseNodes(child.Get(), update);
    }
}

void ProcessorNetworkConverter::updateProcessorType(TxElement* node) {
    std::string key;
    node->GetValue(&key);

    if (key == "Processor") {
        std::string type = node->GetAttributeOrDefault("type", "");
        if (splitString(type, '.').size() < 3) {
            node->SetAttribute("type", "org.inviwo." + type);
        }
    }
}

void ProcessorNetworkConverter::updateMetaDataTree(TxElement* node) {
    std::string key;
    node->GetValue(&key);

    if (key == "MetaDataList") {
        node->SetValue("MetaDataMap");
    }
    if (key == "MetaData") {
        node->SetValue("MetaDataItem");
        node->SetAttribute("key", node->GetAttribute("type"));
    }
}

void ProcessorNetworkConverter::updatePropertType(TxElement* node) {
    std::string renamed[] = {"undefined",
                             "BoolProperty",
                             "AdvancedMaterialProperty",
                             "BaseOptionProperty",
                             "OptionPropertyFloat",
                             "OptionPropertyDouble",
                             "OptionPropertyInt",
                             "OptionPropertyInt64",
                             "OptionPropertyString",
                             "OptionPropertyFloatVec2",
                             "OptionPropertyFloatVec3",
                             "OptionPropertyFloatVec4",
                             "OptionPropertyDoubleVec2",
                             "OptionPropertyDoubleVec3",
                             "OptionPropertyDoubleVec4",
                             "OptionPropertyIntVec2",
                             "OptionPropertyIntVec3",
                             "OptionPropertyIntVec4",
                             "OptionPropertyFloatMat2",
                             "OptionPropertyFloatMat3",
                             "OptionPropertyFloatMat4",
                             "OptionPropertyDoubleMat2",
                             "OptionPropertyDoubleMat3",
                             "OptionPropertyDoubleMat4",
                             "ButtonProperty",
                             "CameraProperty",
                             "CompositeProperty",
                             "DirectoryProperty",
                             "EventProperty",
                             "FileProperty",
                             "ImageEditorProperty",
                             "FloatMinMaxProperty",
                             "DoubleMinMaxProperty",
                             "IntMinMaxProperty",
                             "FloatProperty",
                             "DoubleProperty",
                             "IntProperty",
                             "Int64Property",
                             "FloatVec2Property",
                             "FloatVec3Property",
                             "FloatVec4Property",
                             "DoubleVec2Property",
                             "DoubleVec3Property",
                             "DoubleVec4Property",
                             "IntVec2Property",
                             "IntVec3Property",
                             "IntVec4Property",
                             "FloatMat2Property",
                             "FloatMat3Property",
                             "FloatMat4Property",
                             "DoubleMat2Property",
                             "DoubleMat3Property",
                             "DoubleMat4Property",
                             "SimpleLightingProperty",
                             "SimpleRaycastingProperty",
                             "StringProperty",
                             "TransferFunctionProperty"};

    std::string key;
    node->GetValue(&key);

    if (key == "Property") {
        std::string type = node->GetAttributeOrDefault("type", "");
        int size = sizeof(renamed) / sizeof(std::string);
        if (std::find(renamed, renamed + size, type) != renamed + size) {
            node->SetAttribute("type", "org.inviwo." + type);
        }
    }
}

void ProcessorNetworkConverter::updateMetaDataType(TxElement* node) {
    std::string renamed[] = {"BoolMetaData",
                             "IntMetaData",
                             "FloatMetaData",
                             "DoubleMetaData",
                             "StringMetaData",
                             "FloatVec2MetaData",
                             "FloatVec3MetaData",
                             "FloatVec4MetaData",
                             "DoubleVec2MetaData",
                             "DoubleVec3MetaData",
                             "DoubleVec4MetaData",
                             "IntVec2MetaData",
                             "IntVec3MetaData",
                             "IntVec4MetaData",
                             "UIntVec2MetaData",
                             "UIntVec3MetaData",
                             "UIntVec4MetaData",
                             "FloatMat2MetaData",
                             "FloatMat3MetaData",
                             "FloatMat4MetaData",
                             "DoubleMat2MetaData",
                             "DoubleMat4MetaData",
                             "DoubleMat3MetaData",
                             "VectorMetaData<2, Float>",
                             "VectorMetaData<3, Float>",
                             "VectorMetaData<4, Float>",
                             "VectorMetaData<2, Int>",
                             "VectorMetaData<3, Int>",
                             "VectorMetaData<4, Int>",
                             "VectorMetaData<2, Uint",
                             "VectorMetaData<3, UInt>",
                             "VectorMetaData<4, UInt>",
                             "MatrixMetaData<2, Float>",
                             "MatrixMetaData<3, Float>",
                             "MatrixMetaData<4, Float>",
                             "PositionMetaData",
                             "ProcessorMetaData",
                             "ProcessorWidgetMetaData",
                             "PropertyEditorWidgetMetaData"

    };
    std::string key;
    node->GetValue(&key);

    if (key == "MetaDataItem") {
        std::string type = node->GetAttributeOrDefault("type", "");
        int size = sizeof(renamed) / sizeof(std::string);
        if (std::find(renamed, renamed + size, type) != renamed + size) {
            node->SetAttribute("type", "org.inviwo." + type);
        }
    }
}

void ProcessorNetworkConverter::updateShadingMode(TxElement* node) {
    std::string key;
    node->GetValue(&key);

    if (key == "Property") {
        std::string type = node->GetAttributeOrDefault("type", "");
        std::string identifier = node->GetAttributeOrDefault("identifier", "");
        if (type == "org.inviwo.OptionPropertyString" && identifier == "shadingMode") {
            node->SetAttribute("type", "org.inviwo.OptionPropertyInt");
        }
    }
}

void ProcessorNetworkConverter::updateCameraToComposite(TxElement* node) {
    std::string key;
    node->GetValue(&key);

    if (key == "Property") {
        std::string type = node->GetAttributeOrDefault("type", "");
        if (type == "org.inviwo.CameraProperty") {
            // create
            TxElement newNode;
            newNode.SetValue("Properties");

            // temp list
            std::vector<TxElement*> toBeDeleted;

            // copy and remove children
            ticpp::Iterator<TxElement> child;
            for (child = child.begin(node); child != child.end(); child++) {
                std::string propKey;
                TxElement* subNode = child.Get();
                subNode->GetValue(&propKey);
                if (propKey == "lookFrom" || propKey == "lookTo" || propKey == "lookUp" ||
                    propKey == "fovy" || propKey == "aspectRatio" || propKey == "nearPlane" ||
                    propKey == "farPlane") {
                    subNode->SetValue("Property");
                    newNode.InsertEndChild(*subNode->Clone());
                    toBeDeleted.push_back(subNode);
                }
            }

            for (auto& elem : toBeDeleted) {
                node->RemoveChild(elem);
            }

            // insert new node
            node->InsertEndChild(newNode);

            LogNetworkWarn(
                "Camera property updated to composite property. Workspace requires resave")
        }
    }
}

void ProcessorNetworkConverter::updateMetaDataKeys(TxElement* node) {
    std::string renamed[] = {"PositionMetaData", "ProcessorMetaData", "ProcessorWidgetMetaData",
                             "PropertyEditorWidgetMetaData"};

    std::string key;
    node->GetValue(&key);

    if (key == "MetaDataItem") {
        std::string keyname = node->GetAttributeOrDefault("key", "");
        int size = sizeof(renamed) / sizeof(std::string);
        if (std::find(renamed, renamed + size, keyname) != renamed + size) {
            node->SetAttribute("key", "org.inviwo." + keyname);
        }
    }
}

void ProcessorNetworkConverter::updateDimensionTag(TxElement* node) {
    std::string key;
    node->GetValue(&key);

    if (key == "dimension") {
        node->SetValue("dimensions");
    }
}

void ProcessorNetworkConverter::updatePropertyLinks(TxElement* node) {
    std::string key;
    node->GetValue(&key);

    if (key == "PropertyLink") {
        TxElement* properties = node->FirstChildElement(false);
        if (properties) {
            TxElement* src = properties->FirstChild()->ToElement();
            TxElement* dest = properties->LastChild()->ToElement();
            src->SetValue("SourceProperty");
            dest->SetValue("DestinationProperty");

            node->InsertEndChild(*src);
            node->InsertEndChild(*dest);

            node->RemoveChild(properties);
        }
    }
}

void ProcessorNetworkConverter::updatePortsInProcessors(TxElement* root) {
    struct RefManager : public ticpp::Visitor {
        virtual bool VisitEnter(const TxElement& node, const TxAttribute*) override {
            std::string id = node.GetAttributeOrDefault("id", "");
            if (!id.empty()) {
                ids_.push_back(id);
                std::sort(ids_.begin(), ids_.end());
            }
            return true;
        };
        virtual bool VisitEnter(const TxDocument& doc) override {
            return ticpp::Visitor::VisitEnter(doc);
        }
        virtual bool VisitEnter(const TiXmlElement& element,
                                const TiXmlAttribute* firstAttribute) override {
            return ticpp::Visitor::VisitEnter(element, firstAttribute);
        }
        virtual bool VisitEnter(const TiXmlDocument& doc) override {
            return ticpp::Visitor::VisitEnter(doc);
        }

        std::string getNewRef() {
            std::string ref("ref0");
            for (int i = 1; std::find(ids_.begin(), ids_.end(), ref) != ids_.end(); ++i) {
                ref = "ref" + toString(i);
            }
            ids_.push_back(ref);
            return ref;
        };

        std::vector<std::string> ids_;
    };

    RefManager refs;

    root->Accept(&refs);

    TxNode* processorlist = root->FirstChild("Processors");
    std::map<std::string, TxElement*> processorsOutports;
    std::map<std::string, TxElement*> processorsInports;

    ticpp::Iterator<TxElement> child;
    for (child = child.begin(processorlist); child != child.end(); child++) {
        // create

        TxElement* outports = new TxElement("OutPorts");
        child->LinkEndChild(outports);
        processorsOutports[child->GetAttributeOrDefault("identifier", "")] = outports;

        TxElement* inports = new TxElement("InPorts");
        child->LinkEndChild(inports);
        processorsInports[child->GetAttributeOrDefault("identifier", "")] = inports;
    }

    TxNode* connectionlist = root->FirstChild("Connections");
    for (child = child.begin(connectionlist); child != child.end(); child++) {
        TxElement* outport = child->FirstChild("OutPort")->ToElement();
        if (outport->GetAttributeOrDefault("reference", "").empty()) {
            std::string pid = outport->FirstChild("Processor")
                                  ->ToElement()
                                  ->GetAttributeOrDefault("identifier", "");
            outport->RemoveChild(outport->FirstChild());

            TxElement* outclone = outport->Clone()->ToElement();

            std::string id = outport->GetAttributeOrDefault("id", "");
            if (id.empty()) id = refs.getNewRef();

            outport->SetAttribute("reference", id);
            outport->RemoveAttribute("id");

            outclone->SetAttribute("id", id);
            processorsOutports[pid]->LinkEndChild(outclone);
        }

        TxElement* inport = child->FirstChild("InPort")->ToElement();
        if (inport->GetAttributeOrDefault("reference", "").empty()) {
            std::string pid = inport->FirstChild("Processor")
                                  ->ToElement()
                                  ->GetAttributeOrDefault("identifier", "");
            inport->RemoveChild(inport->FirstChild());

            TxElement* inclone = inport->Clone()->ToElement();

            std::string id = inport->GetAttributeOrDefault("id", "");
            if (id.empty()) id = refs.getNewRef();

            inport->SetAttribute("reference", id);
            inport->RemoveAttribute("id");

            inclone->SetAttribute("id", id);
            processorsInports[pid]->LinkEndChild(inclone);
        }
    }

    for (auto& processorsOutport : processorsOutports) {
        delete processorsOutport.second;
    }

    for (auto& processorsInport : processorsInports) {
        delete processorsInport.second;
    }
}

void ProcessorNetworkConverter::updateNoSpaceInProcessorClassIdentifers(TxElement* node) {
    std::string renamed[] = {"org.inviwo.Diffuse light source",
                             "org.inviwo.Directional light source",
                             "org.inviwo.Ordinal Property Animator",
                             "org.inviwo.Point light source",
                             "org.inviwo.Spot light source",
                             "org.inviwo.3D Model Reader",
                             "org.inviwo.Cones Test",
                             "org.inviwo.Edge Processor",
                             "org.inviwo.Final Composition",
                             "org.inviwo.Bader Info",
                             "Cone Liver test",
                             "El. vis. Raycaster",
                             "org.inviwo.Point Cloud Generator",
                             "org.inviwo.Surface Extraction",
                             "org.inviwo.GromacsDynamicTrajectoryData source",
                             "org.inviwo.Gromacs source",
                             "org.inviwo.Gromacs to geometry",
                             "org.inviwo.Gromacs to stream line geometry",
                             "org.inviwo.Occlusion Density Volume",
                             "org.inviwo.Point Cloud Mesh Extraction",
                             "org.inviwo.Point Cloud Mesh Extraction2",
                             "org.inviwo.Camera Test",
                             "org.inviwo.Composite Property Test",
                             "org.inviwo.Volume Laplacian",
                             "org.inviwo.Bader Partition"};

    std::string key;
    node->GetValue(&key);

    if (key == "Processor") {
        std::string type = node->GetAttributeOrDefault("type", "");
        int size = sizeof(renamed) / sizeof(std::string);
        if (std::find(renamed, renamed + size, type) != renamed + size) {
            std::string newtype = removeFromString(type, ' ');
            if (splitString(type, '.').size() < 3) {
                node->SetAttribute("type", "org.inviwo." + newtype);
            } else {
                node->SetAttribute("type", newtype);
            }
        }
    }
}

void ProcessorNetworkConverter::updateDisplayName(TxElement* node) {
    std::string key;
    node->GetValue(&key);

    if (key == "Property") {
        if (node->HasAttribute("displayName")) {
            std::string displayName = node->GetAttributeOrDefault("displayName", "");

            TxElement newNode;
            newNode.SetValue("displayName");
            newNode.SetAttribute("content", displayName);
            node->InsertEndChild(newNode);
        }
    }
}

void ProcessorNetworkConverter::updateProcessorIdentifiers(TxElement* node) {
    std::string key;
    node->GetValue(&key);

    if (key == "Processor") {
        std::string identifier = node->GetAttributeOrDefault("identifier", "");
        if (identifier != "") {
            std::transform(identifier.begin(), identifier.end(), identifier.begin(), [](char c) {
                if (!util::isValidIdentifierCharacter(c, " ()=&")) {
                    return ' ';
                }
                return c;
            });

            node->SetAttribute("identifier", identifier);
        }
    }
}

void ProcessorNetworkConverter::updateTransferfunctions(TxElement* node) {
    std::string key;
    node->GetValue(&key);

    if (key == "transferFunction") {
        node->SetValue("TransferFunction");

        xml::visitMatchingNodes(node, {{"dataPoints", {}}},
                                [](TxElement* node) { node->SetValue("Points"); });

        xml::visitMatchingNodes(node, {{"Points", {}}, {"point", {}}},
                                [](TxElement* node) { node->SetValue("Point"); });

        xml::visitMatchingNodes(node, {{"Points", {}}, {"Point", {}}, {"pos", {}}},
                                [](TxElement* posNode) {
                                    auto pos = posNode->GetAttributeOrDefault("x", "0.0");
                                    posNode->SetAttribute("content", pos);
                                });
    }
}

void ProcessorNetworkConverter::updateProcessorIdentifiersStriped(TxElement* node) {
    std::string key;
    node->GetValue(&key);

    if (key == "Processor") {
        std::string identifier = node->GetAttributeOrDefault("identifier", "");
        if (identifier != "") {

            std::string baseIdentifier = identifier;
            std::string newIdentifier = identifier;
            int i = 2;

            auto parts = splitString(identifier, ' ');
            if (parts.size() > 1 &&
                util::all_of(parts.back(), [](const char& c) { return std::isdigit(c); })) {
                i = std::stoi(parts.back());
                baseIdentifier = joinString(parts.begin(), parts.end() - 1, " ");
                newIdentifier = baseIdentifier + " " + toString(i);
            }

            std::string stripedIdentifier = util::stripIdentifier(newIdentifier);

            while (usedIdentifier_.find(stripedIdentifier) != usedIdentifier_.end()) {
                newIdentifier = baseIdentifier + " " + toString(i++);
                stripedIdentifier = util::stripIdentifier(newIdentifier);
            }

            usedIdentifier_.insert(stripedIdentifier);

            node->SetAttribute("identifier", newIdentifier);
        }
    }
}
// Transform
//<MetaDataMap>
//    <MetaDataItem type="org.inviwo.PropertyEditorWidgetMetaData"
//    key="org.inviwo.PropertyEditorWidgetMetaData">
//        <position x="1407" y="1205" />
//        <dimensions x="935" y="409" />
//        <visibility content="0" />
//        <dockstatus content="Floating" />
//        <stickyflag content="0" />
//    </MetaDataItem>
//</MetaDataMap>
// to
//<MetaDataMap>
//    <MetaDataItem type="org.inviwo.IntMetaData" key="PropertyEditorWidgetDockStatus">
//        <MetaData content="2" />
//    </MetaDataItem>
//    <MetaDataItem type="org.inviwo.BoolMetaData" key="PropertyEditorWidgetFloating">
//        <MetaData content="1" />
//    </MetaDataItem>
//    <MetaDataItem type="org.inviwo.IntVec2MetaData" key="PropertyEditorWidgetPosition">
//        <MetaData x="510" y="209" />
//    </MetaDataItem>
//    <MetaDataItem type="org.inviwo.IntVec2MetaData" key="PropertyEditorWidgetSize">
//        <MetaData x="500" y="324" />
//    </MetaDataItem>
//    <MetaDataItem type="org.inviwo.BoolMetaData" key="PropertyEditorWidgetSticky">
//        <MetaData content="0" />
//    </MetaDataItem>
//    <MetaDataItem type="org.inviwo.BoolMetaData" key="PropertyEditorWidgetVisible">
//        <MetaData content="1" />
//    </MetaDataItem>
//</MetaDataMap>
void ProcessorNetworkConverter::updatePropertyEditorMetadata(TxElement* parent) {
    using R = std::tuple<std::string, std::string, std::string>;
    const std::vector<R> replacements = {
        R{"position", "org.inviwo.IntVec2MetaData", "PropertyEditorWidgetPosition"},
        R{"dimensions", "org.inviwo.IntVec2MetaData", "PropertyEditorWidgetSize"},
        R{"visibility", "org.inviwo.BoolMetaData", "PropertyEditorWidgetVisible"},
        R{"stickyflag", "org.inviwo.BoolMetaData", "PropertyEditorWidgetSticky"}};

    std::string key;
    parent->GetValue(&key);

    if (key == "MetaDataMap") {
        std::vector<TxElement*> toRemove;
        ticpp::Iterator<TxElement> child;
        for (child = child.begin(parent); child != child.end(); child++) {
            std::string childKey;
            TxElement* node = child.Get();
            node->GetValue(&childKey);
            if (childKey == "MetaDataItem") {
                std::string nodeKey = node->GetAttributeOrDefault("key", "");
                if (nodeKey == "org.inviwo.PropertyEditorWidgetMetaData") {
                    for (const auto& item : replacements) {
                        if (auto n = node->FirstChild(std::get<0>(item), false)) {
                            TxElement newNode;
                            newNode.SetValue("MetaDataItem");
                            newNode.SetAttribute("type", std::get<1>(item));
                            newNode.SetAttribute("key", std::get<2>(item));
                            auto data = n->Clone();
                            data->SetValue("MetaData");
                            newNode.InsertEndChild(*data);
                            parent->InsertEndChild(newNode);
                        }
                    }
                    toRemove.push_back(node);
                }
            }
        }
        for (auto item : toRemove) {
            parent->RemoveChild(item);
        }
    }
}

}  // namespace inviwo
