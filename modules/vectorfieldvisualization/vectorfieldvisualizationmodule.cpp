/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2017 Inviwo Foundation
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

#include <modules/vectorfieldvisualization/vectorfieldvisualizationmodule.h>
#include <modules/vectorfieldvisualization/processors/datageneration/rbfvectorfieldgenerator2d.h>
#include <modules/vectorfieldvisualization/processors/datageneration/rbfvectorfieldgenerator3d.h>
#include <modules/vectorfieldvisualization/processors/datageneration/seedpointgenerator.h>
#include <modules/vectorfieldvisualization/processors/datageneration/seedpointsfrommask.h>

#include <modules/vectorfieldvisualization/processors/3d/streamlines.h>
#include <modules/vectorfieldvisualization/processors/3d/pathlines.h>
#include <modules/vectorfieldvisualization/processors/3d/streamribbons.h>
#include <modules/vectorfieldvisualization/processors/integrallinevectortomesh.h>

#include <modules/vectorfieldvisualization/ports/seedpointsport.h>

#include <modules/vectorfieldvisualization/properties/streamlineproperties.h>
#include <modules/vectorfieldvisualization/properties/pathlineproperties.h>
#include <modules/vectorfieldvisualization/processors/seed3dto4d.h>
#include <modules/vectorfieldvisualization/processors/seedsfrommasksequence.h>
#include <modules/vectorfieldvisualization/processors/discardshortlines.h>

namespace inviwo {

VectorFieldVisualizationModule::VectorFieldVisualizationModule(InviwoApplication* app)
    : InviwoModule(app, "VectorFieldVisualization") {
    registerProcessor<RBFVectorFieldGenerator2D>();
    registerProcessor<RBFVectorFieldGenerator3D>();
    registerProcessor<SeedPointGenerator>();
    registerProcessor<SeedPointsFromMask>();

    registerProcessor<StreamLines>();
    registerProcessor<PathLines>();
    registerProcessor<StreamRibbons>();

    registerProcessor<IntegralLineVectorToMesh>();
    registerProcessor<Seed3Dto4D>();
    registerProcessor<SeedsFromMaskSequence>();
    registerProcessor<DiscardShortLines>();

    registerPort < SeedPoints3DOutport>("SeedPointsOutport");
    registerPort < SeedPoints3DInport>("SeedPointsInport");
    registerPort < SeedPoints4DOutport>("SeedPoints4DOutport");
    registerPort < SeedPoints4DInport>("SeedPoints4DInport");

    registerProperty<StreamLineProperties>();
    registerProperty<PathLineProperties>();
}


int VectorFieldVisualizationModule::getVersion() const { return 1; }

std::unique_ptr<VersionConverter> VectorFieldVisualizationModule::getConverter(int version) const {
    return util::make_unique<Converter>(version);
}

bool VectorFieldVisualizationModule::Converter::traverseNodes(TxElement* node, updateType update) {
    (this->*update)(node);
    ticpp::Iterator<ticpp::Element> child;
    bool res = false;
    for (child = child.begin(node); child != child.end(); child++) {
        res |= traverseNodes(child.Get(), update);
    }

    return res;
}

bool VectorFieldVisualizationModule::Converter::updateAllowLooping(TxElement* node)
{
    std::string key;
    node->GetValue(&key);


    if (key == "Processor") {
        std::string type = node->GetAttributeOrDefault("type", "");
        if (type == "org.inviwo.PathLines") {

            TxElement *propertiesNode = nullptr;
            TxElement *pathLinePropertiesNode = nullptr;

            TxElement *pathLinePropertiesPropertiesNode = nullptr;
            TxElement *allowLoopingNode = nullptr;

            ticpp::Iterator<ticpp::Element> child;
            for (child = child.begin(node); child != child.end(); child++) {
                std::string childkey;
                child->GetValue(&childkey);
                if (childkey == "Properties") {
                    propertiesNode = child.Get();
                    break;
                }
            }
            if (propertiesNode == nullptr) {
                return false;
            }

            for (child = child.begin(propertiesNode); child != child.end(); child++) {
                std::string childkey;
                child->GetValue(&childkey);
                if (childkey == "Property") {
                    std::string childType = child->GetAttributeOrDefault("type", "");

                    if (childType == "org.inviwo.PathLineProperties") {
                        pathLinePropertiesNode = child.Get();
                        break;
                    }
                }
            }

            if (pathLinePropertiesNode == nullptr) {
                return false;
            }
            for (child = child.begin(pathLinePropertiesNode); child != child.end(); child++) {
                std::string childkey;
                child->GetValue(&childkey);
                if (childkey == "Properties") {
                    pathLinePropertiesPropertiesNode = child.Get();
                    break;
                }
            }
            if (pathLinePropertiesPropertiesNode == nullptr) {
                return false;
            }



            for (child = child.begin(pathLinePropertiesPropertiesNode); child != child.end(); child++) {
                std::string childkey;
                child->GetValue(&childkey);
                if (childkey == "Property") {
                    std::string childType = child->GetAttributeOrDefault("type", "");
                    std::string childIdentifier = child->GetAttributeOrDefault("identifier", "");

                    if (childType == "org.inviwo.BoolProperty" && childIdentifier == "allowLooping") {
                        allowLoopingNode = child.Get();
                        break;
                    }
                }
            }
            if (allowLoopingNode == nullptr) {
                return false;
            }

            propertiesNode->InsertEndChild(*allowLoopingNode);
            pathLinePropertiesPropertiesNode->RemoveChild(allowLoopingNode);
            return true;
        }

    }
    return false;
}

VectorFieldVisualizationModule::Converter::Converter(int version) : version_(version) {}

bool VectorFieldVisualizationModule::Converter::convert(TxElement* root) {
    std::vector<xml::IdentifierReplacement> repl = {
        { { xml::Kind::processor("org.inviwo.StreamRibbons")
        , xml::Kind::inport("Inport") },
        "vectorVolume",
        "sampler" },
        { { xml::Kind::processor("org.inviwo.StreamRibbons")
        , xml::Kind::inport("Inport") },
        "vorticityVolume",
        "vorticitySampler" }
    };

    bool res = false;
    switch (version_) {
    case 0: {
        res |= traverseNodes(root, &VectorFieldVisualizationModule::Converter::updateAllowLooping);
        res |= xml::changeIdentifiers(root, repl);
        return res;
    }

    default:
        return false;  // No changes
    }
    return true;


}


}  // namespace
