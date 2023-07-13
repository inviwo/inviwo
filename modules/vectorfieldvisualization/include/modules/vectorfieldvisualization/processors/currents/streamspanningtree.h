/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <modules/vectorfieldvisualization/vectorfieldvisualizationmoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/meshport.h>
#include <modules/vectorfieldvisualization/integrallinetracer.h>
#include <modules/vectorfieldvisualization/algorithms/integrallineoperations.h>
#include <modules/vectorfieldvisualization/ports/seedpointsport.h>
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>

namespace inviwo {

/** \docpage{org.inviwo.StreamSpanningTree, Stream Spanning Tree}
 * ![](org.inviwo.StreamSpanningTree.png?classIdentifier=org.inviwo.StreamSpanningTree)
 * Find a path from one point to another using streamlines while allowing for a little diffusion.
 * Given a vector field, a seed mask, a beginning and a destination point.
 * Integrate streamlines from the mask.
 * Start at beginning point.
 * Dijkstra, while
 * - Nodes are start and end points of streamlines
 * - Edges are streamlines and 'diffusion edges' from end to start points
 *   - stream line weights are their length, diffusion weights are the squared distance
 * - Break if destination point is reached
 */
class IVW_MODULE_VECTORFIELDVISUALIZATION_API StreamSpanningTree : public Processor {
public:
    StreamSpanningTree();
    virtual ~StreamSpanningTree() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    using Sampler = SpatialSampler<2, 2, double>;
    using Tracer = IntegralLineTracer<Sampler>;

    // Inports
    DataInport<Sampler> velocitySampler_;
    ImageInport seedMask_;
    BrushingAndLinkingInport lineSelection_;

    // Properties
    IntSize2Property beginning_, destination_;
    BoolProperty beginAtMaxValue_;
    FloatProperty maxDestinationDistance_;
    IntegralLineProperties integrationProperties_;
    FloatProperty maxDiffusionRadius_;
    IntSizeTProperty samplingStride_, maxNumNodes_;
    BoolProperty markJumps_;
    BoolProperty integrate_;

    // Outports
    MeshOutport pointMesh_, lineMesh_;
    IntegralLineSetOutport integralLines_;
    SeedPoints2DOutport seedPoints_;

    struct Node {
        const size2_t startPoint;
        IntegralLine streamline;
        Node* shortestPathPreviousNode;
        float shortestPathLength;

        Node(const size2_t& start)  //, IntegralLine&& line)
            : startPoint(start)
            , streamline()
            // , streamline(std::move(line))
            , shortestPathPreviousNode(nullptr)
            , shortestPathLength(std::numeric_limits<float>::infinity()) {}
        // ~Node() { delete streamline; }
    };

    struct QueueNode {
        size_t nodeIndex, prevIndex;
        float shortestDistance;
        QueueNode(size_t idx, size_t prevIdx, float dist)
            : nodeIndex(idx), prevIndex(prevIdx), shortestDistance(dist) {}
    };

    std::unique_ptr<std::vector<Node>> nodes_;
    std::shared_ptr<IntegralLineSet> allLines_;
    std::unique_ptr<size_t[]> seedIndexMask_;
    bool integrationRequired_;
};

}  // namespace inviwo
