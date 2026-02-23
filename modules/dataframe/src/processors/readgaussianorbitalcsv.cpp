/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <inviwo/dataframe/processors/readgaussianorbitalcsv.h>
#include <fstream>
#include <iterator>
#include <string>
namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ReadGaussianOrbitalCSV::processorInfo_{
    "org.inviwo.ReadGaussianOrbitalCSV",  // Class identifier
    "Read Gaussian Orbital CSV",        // Display name
    "Undefined",                   // Category
    CodeState::Experimental,       // Code state
    Tags::None,                    // Tags
    R"(<Explanation of how to use the processor.>)"_unindentHelp,
};

const ProcessorInfo& ReadGaussianOrbitalCSV::getProcessorInfo() const { return processorInfo_; }

ReadGaussianOrbitalCSV::ReadGaussianOrbitalCSV(const std::filesystem::path& file)
    : Processor{}
    , inputFile_{"inputFile_", "CSV File", file, "dataframe"}
    , paddValue_{"paddvalue", "Padd Value",0.0,0.0,100.0}
    , orbitals_("orbitals", "Imported orbitals"_help)
    , minVec_{"minVecPadd", "Padded box"_help}
    , maxVec_{"maxVecPadd", "Padded box"_help}{

    addProperties(inputFile_,paddValue_);
    addPorts(orbitals_,minVec_,maxVec_);

}


std::istream& operator>>(std::istream& is, GaussianOrbital& go) {
        char comma;
        go.p.w = 1;
        is >> go.p.x >> comma >> go.p.y >> comma >> go.p.z >> comma >> go.coefs.x >> comma >>
            go.coefs.y >> comma >> go.coefs.z >> comma >> go.coefs.w >> comma >> go.alpha >>
            comma >> go.c_Coeff >> comma >> go.N_ijk >> comma >> go.m_Coeff;
        return is;
    }

void ReadGaussianOrbitalCSV::process() {
    
    
    
    std::vector<GaussianOrbital> data{};
    auto filename = inputFile_.get();

    std::ifstream ifs{filename};
    vec3 minVec{0};
    vec3 maxVec{1};
    if (ifs.is_open()) {
        std::string header;
        std::getline(ifs, header);
        std::transform(std::istream_iterator<GaussianOrbital>{ifs},
                       std::istream_iterator<GaussianOrbital>{}, std::back_inserter(data),
                       [&minVec, &maxVec](const GaussianOrbital& go) {
                           minVec.x = std::min(minVec.x, go.p.x);
                           minVec.y = std::min(minVec.y, go.p.y);
                           minVec.z = std::min(minVec.z, go.p.z);

                           maxVec.x = std::max(maxVec.x, go.p.x);
                           maxVec.y = std::max(maxVec.y, go.p.y);
                           maxVec.z = std::max(maxVec.z, go.p.z);
                           return go;
                        });

        //std::copy(std::istream_iterator<GaussianOrbital>{ifs},
        //          std::istream_iterator<GaussianOrbital>{}, std::back_inserter(data));
    }
    

    ;

    minVec -= vec3(paddValue_.get()); 
    maxVec += vec3(paddValue_.get());

    vec3 c = 0.5f * (maxVec + minVec);
    vec3 e = maxVec - minVec;
    float L = std::max(e.x, std::max(e.y, e.z));
    minVec = c - 0.5f * vec3(L);
    maxVec = c + 0.5f * vec3(L);

    orbitals_.setData(std::move(data));
    minVec_.setData(std::move(minVec));
    maxVec_.setData(std::move(maxVec));

}

}  // namespace inviwo
