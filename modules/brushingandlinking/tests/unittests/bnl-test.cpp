/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/network/processornetwork.h>

#include <modules/brushingandlinking/brushingandlinkingmanager.h>
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>

#include <gtest/gtest.h>

namespace inviwo {

namespace {

struct BnlProcessor : Processor {
    explicit BnlProcessor(std::string_view identifier)
        : Processor{identifier, identifier}, inport("in"), outport("out") {

        outport.getManager().setParent(&inport.getManager());
    }

    virtual const ProcessorInfo& getProcessorInfo() const override { return processorInfo; }

    static const ProcessorInfo processorInfo;

    BrushingAndLinkingInport inport;
    BrushingAndLinkingOutport outport;
};

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo BnlProcessor::processorInfo{
    "org.inviwo.BnlProcessor",  // Class identifier
    "BnlProcessor",             // Display name
    "Testing",                  // Category
    CodeState::Stable,          // Code state
    Tags::CPU,                  // Tags
};

auto createSource(std::string_view identifier) {
    auto source = std::make_unique<BnlProcessor>(identifier);
    source->addPort(source->outport);
    return source;
};

auto createSink(std::string_view identifier) {
    auto sink = std::make_unique<BnlProcessor>(identifier);
    sink->addPort(sink->inport);
    return sink;
};

auto createProcessor(std::string_view identifier) {
    auto sink = std::make_unique<BnlProcessor>(identifier);
    sink->addPort(sink->inport);
    sink->addPort(sink->outport);
    return sink;
};

}  // namespace

TEST(BrushingManager, SelectRows) {
    BrushingAndLinkingInport inport{"in"};
    BrushingAndLinkingManager bnlManager{&inport};

    EXPECT_TRUE(bnlManager.getSelectedIndices(BrushingTarget::Row).empty());

    const BitSet selection{1, 2, 4};

    bnlManager.brush(BrushingAction::Select, BrushingTarget::Row, selection);
    const auto brushed = bnlManager.getSelectedIndices(BrushingTarget::Row);
    EXPECT_EQ(brushed.size(), selection.size());
    for (auto index : selection) {
        EXPECT_TRUE(brushed.contains(index));
    }

    EXPECT_TRUE(bnlManager.isModified());
    EXPECT_TRUE(bnlManager.isSelectionModified());
    EXPECT_FALSE(bnlManager.isFilteringModified());
    EXPECT_FALSE(bnlManager.isHighlightModified());

    bnlManager.clearSelected(BrushingTarget::Row);
    EXPECT_TRUE(bnlManager.getSelectedIndices(BrushingTarget::Row).empty());

    // check convenience function
    bnlManager.select(selection, BrushingTarget::Row);
    const auto selected = bnlManager.getSelectedIndices(BrushingTarget::Row);
    EXPECT_EQ(selected.size(), selection.size());
    for (auto index : selection) {
        EXPECT_TRUE(selected.contains(index));
    }
}
TEST(BrushingManager, SelectModification) {
    BrushingAndLinkingInport inport{"in"};
    BrushingAndLinkingManager bnlManager{&inport};

    const BitSet selection{1, 2, 4};

    bnlManager.brush(BrushingAction::Select, BrushingTarget::Row, selection);

    EXPECT_EQ(bnlManager.getModifiedActions(), BrushingModification::Selected);

    EXPECT_TRUE(bnlManager.isModified());
    EXPECT_TRUE(bnlManager.isTargetModified(BrushingTarget::Row, BrushingAction::Select));
    EXPECT_FALSE(bnlManager.isTargetModified(BrushingTarget::Column, BrushingAction::Select));
    EXPECT_TRUE(bnlManager.isSelectionModified());
    EXPECT_FALSE(bnlManager.isFilteringModified());
    EXPECT_FALSE(bnlManager.isHighlightModified());
}

TEST(BrushingManager, SelectColumns) {
    BrushingAndLinkingInport inport{"in"};
    BrushingAndLinkingManager bnlManager{&inport};

    EXPECT_TRUE(bnlManager.getSelectedIndices(BrushingTarget::Column).empty());

    const BitSet selection{1, 2, 4};

    bnlManager.brush(BrushingAction::Select, BrushingTarget::Column, selection);
    const auto brushed = bnlManager.getSelectedIndices(BrushingTarget::Column);
    EXPECT_EQ(brushed.size(), selection.size());
    for (auto index : selection) {
        EXPECT_TRUE(brushed.contains(index));
    }

    bnlManager.clearSelected(BrushingTarget::Column);
    EXPECT_TRUE(bnlManager.getSelectedIndices(BrushingTarget::Column).empty());
}

TEST(BnlPropagation, SelectUp) {
    ProcessorNetwork network{InviwoApplication::getPtr()};

    auto source = createProcessor("source");
    auto sink = createSink("sink");

    auto& src = *source;
    auto& snk = *sink;

    network.addProcessor(std::move(source));
    network.addProcessor(std::move(sink));
    network.addConnection(&src.outport, &snk.inport);

    const BitSet selection{1, 2, 4};

    const auto& sourceManager = src.outport.getManager();
    auto& sinkManager = snk.inport.getManager();

    sinkManager.select(selection, BrushingTarget::Row);

    EXPECT_TRUE(sourceManager.isSelectionModified());
    EXPECT_TRUE(sinkManager.isSelectionModified());

    EXPECT_EQ(sinkManager.getSelectedIndices(), selection);
    EXPECT_EQ(sourceManager.getSelectedIndices(), selection);
}

TEST(BnlPropagation, SelectDown) {
    ProcessorNetwork network{InviwoApplication::getPtr()};

    auto source = createProcessor("source");
    auto sink = createSink("sink");

    auto& src = *source;
    auto& snk = *sink;

    network.addProcessor(std::move(source));
    network.addProcessor(std::move(sink));
    network.addConnection(&src.outport, &snk.inport);

    const BitSet selection{1, 2, 4};

    auto& sourceManager = src.outport.getManager();
    const auto& sinkManager = snk.inport.getManager();

    sourceManager.select(selection, BrushingTarget::Row);

    EXPECT_TRUE(sourceManager.isSelectionModified());
    EXPECT_TRUE(sinkManager.isSelectionModified());

    EXPECT_EQ(sinkManager.getSelectedIndices(), selection);
    EXPECT_EQ(sourceManager.getSelectedIndices(), selection);
}

TEST(BnlPropagationTree, SelectFromRoot) {
    ProcessorNetwork network{InviwoApplication::getPtr()};

    auto source = createProcessor("source");
    auto sink1 = createSink("sink1");
    auto sink2 = createSink("sink2");

    auto& src = *source;
    auto& snk1 = *sink1;
    auto& snk2 = *sink2;

    network.addProcessor(std::move(source));
    network.addProcessor(std::move(sink1));
    network.addProcessor(std::move(sink2));
    network.addConnection(&src.outport, &snk1.inport);
    network.addConnection(&src.outport, &snk2.inport);

    const BitSet selection{1, 2, 4};

    src.outport.getManager().select(selection, BrushingTarget::Row);

    EXPECT_TRUE(src.outport.getManager().isSelectionModified());
    EXPECT_TRUE(snk1.inport.getManager().isSelectionModified());
    EXPECT_TRUE(snk2.inport.getManager().isSelectionModified());

    EXPECT_EQ(snk1.inport.getManager().getSelectedIndices(), selection);
    EXPECT_EQ(snk2.inport.getManager().getSelectedIndices(), selection);

    // clear selection
    src.outport.getManager().clearSelected(BrushingTarget::Row);

    EXPECT_TRUE(src.outport.getManager().getSelectedIndices().empty());
    EXPECT_TRUE(snk1.inport.getManager().getSelectedIndices().empty());
    EXPECT_TRUE(snk2.inport.getManager().getSelectedIndices().empty());
}

TEST(BnlPropagationTree, SelectFromRootClearFromChild) {
    ProcessorNetwork network{InviwoApplication::getPtr()};

    auto source = createProcessor("source");
    auto sink1 = createSink("sink1");
    auto sink2 = createSink("sink2");

    auto& src = *source;
    auto& snk1 = *sink1;
    auto& snk2 = *sink2;

    network.addProcessor(std::move(source));
    network.addProcessor(std::move(sink1));
    network.addProcessor(std::move(sink2));
    network.addConnection(&src.outport, &snk1.inport);
    network.addConnection(&src.outport, &snk2.inport);

    const BitSet selection{1, 2, 4};

    src.outport.getManager().select(selection, BrushingTarget::Row);

    EXPECT_TRUE(src.inport.getManager().isSelectionModified());
    EXPECT_TRUE(src.outport.getManager().isSelectionModified());
    EXPECT_TRUE(snk1.inport.getManager().isSelectionModified());
    EXPECT_TRUE(snk2.inport.getManager().isSelectionModified());

    EXPECT_EQ(snk1.inport.getManager().getSelectedIndices(), selection);
    EXPECT_EQ(snk2.inport.getManager().getSelectedIndices(), selection);

    // clear selection from child
    snk1.outport.getManager().clearSelected(BrushingTarget::Row);

    EXPECT_TRUE(src.inport.getManager().getSelectedIndices().empty());
    EXPECT_TRUE(src.outport.getManager().getSelectedIndices().empty());
    EXPECT_TRUE(snk1.inport.getManager().getSelectedIndices().empty());
    EXPECT_TRUE(snk2.inport.getManager().getSelectedIndices().empty());
}

TEST(BnlPropagationTree, SelectFromRootSelectEmptyFromChild) {
    ProcessorNetwork network{InviwoApplication::getPtr()};

    auto source = createProcessor("source");
    auto sink1 = createSink("sink1");
    auto sink2 = createSink("sink2");

    auto& src = *source;
    auto& snk1 = *sink1;
    auto& snk2 = *sink2;

    network.addProcessor(std::move(source));
    network.addProcessor(std::move(sink1));
    network.addProcessor(std::move(sink2));
    network.addConnection(&src.outport, &snk1.inport);
    network.addConnection(&src.outport, &snk2.inport);

    const BitSet selection{1, 2, 4};

    src.outport.getManager().select(selection, BrushingTarget::Row);

    EXPECT_TRUE(src.inport.getManager().isSelectionModified());
    EXPECT_TRUE(src.outport.getManager().isSelectionModified());
    EXPECT_TRUE(snk1.inport.getManager().isSelectionModified());
    EXPECT_TRUE(snk2.inport.getManager().isSelectionModified());

    EXPECT_EQ(snk1.inport.getManager().getSelectedIndices(), selection);
    EXPECT_EQ(snk2.inport.getManager().getSelectedIndices(), selection);

    // clear selection from child
    snk1.outport.getManager().select(BitSet{}, BrushingTarget::Row);

    EXPECT_TRUE(src.inport.getManager().getSelectedIndices().empty());
    EXPECT_TRUE(src.outport.getManager().getSelectedIndices().empty());
    EXPECT_TRUE(snk1.inport.getManager().getSelectedIndices().empty());
    EXPECT_TRUE(snk2.inport.getManager().getSelectedIndices().empty());
}

TEST(BnlPropagationTree, SelectFromChild) {
    ProcessorNetwork network{InviwoApplication::getPtr()};

    auto source = createProcessor("source");
    auto sink1 = createSink("sink1");
    auto sink2 = createSink("sink2");

    auto& src = *source;
    auto& snk1 = *sink1;
    auto& snk2 = *sink2;

    network.addProcessor(std::move(source));
    network.addProcessor(std::move(sink1));
    network.addProcessor(std::move(sink2));
    network.addConnection(&src.outport, &snk1.inport);
    network.addConnection(&src.outport, &snk2.inport);

    const BitSet selection{1, 2, 4};
    snk1.outport.getManager().select(selection, BrushingTarget::Row);

    EXPECT_EQ(src.inport.getManager().getSelectedIndices(), selection);
    EXPECT_EQ(snk2.inport.getManager().getSelectedIndices(), selection);

    // trigger clear from same source
    snk1.outport.getManager().clearSelected(BrushingTarget::Row);

    EXPECT_TRUE(snk1.outport.getManager().getSelectedIndices().empty());
    EXPECT_TRUE(src.inport.getManager().getSelectedIndices().empty());
    EXPECT_TRUE(snk2.inport.getManager().getSelectedIndices().empty());
}

TEST(BnlPropagationTree, SelectFromChildClearFromRoot) {
    ProcessorNetwork network{InviwoApplication::getPtr()};

    auto source = createProcessor("source");
    auto sink1 = createSink("sink1");
    auto sink2 = createSink("sink2");

    auto& src = *source;
    auto& snk1 = *sink1;
    auto& snk2 = *sink2;

    network.addProcessor(std::move(source));
    network.addProcessor(std::move(sink1));
    network.addProcessor(std::move(sink2));
    network.addConnection(&src.outport, &snk1.inport);
    network.addConnection(&src.outport, &snk2.inport);

    const BitSet selection{1, 2, 4};
    snk1.outport.getManager().select(selection, BrushingTarget::Row);

    EXPECT_EQ(src.inport.getManager().getSelectedIndices(), selection);
    EXPECT_EQ(snk2.inport.getManager().getSelectedIndices(), selection);

    // trigger clear from root
    src.outport.getManager().clearSelected(BrushingTarget::Row);

    EXPECT_TRUE(snk1.outport.getManager().getSelectedIndices().empty());
    EXPECT_TRUE(src.inport.getManager().getSelectedIndices().empty());
    EXPECT_TRUE(snk2.inport.getManager().getSelectedIndices().empty());
}

TEST(BnlPropagationTree, SelectFromChildClearFromSibling) {
    ProcessorNetwork network{InviwoApplication::getPtr()};

    auto source = createProcessor("source");
    auto sink1 = createSink("sink1");
    auto sink2 = createSink("sink2");

    auto& src = *source;
    auto& snk1 = *sink1;
    auto& snk2 = *sink2;

    network.addProcessor(std::move(source));
    network.addProcessor(std::move(sink1));
    network.addProcessor(std::move(sink2));
    network.addConnection(&src.outport, &snk1.inport);
    network.addConnection(&src.outport, &snk2.inport);

    const BitSet selection{1, 2, 4};
    snk1.outport.getManager().select(selection, BrushingTarget::Row);

    EXPECT_EQ(src.inport.getManager().getSelectedIndices(), selection);
    EXPECT_EQ(src.outport.getManager().getSelectedIndices(), selection);
    EXPECT_EQ(snk1.inport.getManager().getSelectedIndices(), selection);
    EXPECT_EQ(snk2.inport.getManager().getSelectedIndices(), selection);

    // trigger clear from other sink
    snk2.outport.getManager().clearSelected(BrushingTarget::Row);

    EXPECT_TRUE(src.inport.getManager().getSelectedIndices().empty());
    EXPECT_TRUE(src.outport.getManager().getSelectedIndices().empty());
    EXPECT_TRUE(snk1.outport.getManager().getSelectedIndices().empty());
    EXPECT_TRUE(snk2.inport.getManager().getSelectedIndices().empty());
}

TEST(BnlPropagationTree, SelectFromSiblings) {
    ProcessorNetwork network{InviwoApplication::getPtr()};

    auto source = createProcessor("source");
    auto sink1 = createSink("sink1");
    auto sink2 = createSink("sink2");

    auto& src = *source;
    auto& snk1 = *sink1;
    auto& snk2 = *sink2;

    network.addProcessor(std::move(source));
    network.addProcessor(std::move(sink1));
    network.addProcessor(std::move(sink2));
    network.addConnection(&src.outport, &snk1.inport);
    network.addConnection(&src.outport, &snk2.inport);

    const BitSet selection{1, 2, 4};
    snk1.inport.getManager().select(selection, BrushingTarget::Row);

    EXPECT_TRUE(src.inport.getManager().isSelectionModified());
    EXPECT_TRUE(src.outport.getManager().isSelectionModified());
    EXPECT_TRUE(snk1.inport.getManager().isSelectionModified());
    EXPECT_TRUE(snk2.inport.getManager().isSelectionModified());

    EXPECT_EQ(src.inport.getManager().getSelectedIndices(), selection);
    EXPECT_EQ(src.outport.getManager().getSelectedIndices(), selection);
    EXPECT_EQ(snk1.inport.getManager().getSelectedIndices(), selection);
    EXPECT_EQ(snk2.inport.getManager().getSelectedIndices(), selection);

    // trigger selection from other sinke
    const BitSet selection2{5};
    snk2.outport.getManager().select(selection2, BrushingTarget::Row);

    EXPECT_EQ(src.inport.getManager().getSelectedIndices(), selection2);
    EXPECT_EQ(snk1.inport.getManager().getSelectedIndices(), selection2);
    EXPECT_EQ(snk2.inport.getManager().getSelectedIndices(), selection2);

    // trigger clear from other sink
    snk2.outport.getManager().clearSelected(BrushingTarget::Row);

    EXPECT_TRUE(snk1.outport.getManager().getSelectedIndices().empty());
    EXPECT_TRUE(src.inport.getManager().getSelectedIndices().empty());
    EXPECT_TRUE(snk2.inport.getManager().getSelectedIndices().empty());

    // trigger clear from initial source
    snk1.outport.getManager().clearSelected(BrushingTarget::Row);

    EXPECT_TRUE(snk1.outport.getManager().getSelectedIndices().empty());
    EXPECT_TRUE(src.inport.getManager().getSelectedIndices().empty());
    EXPECT_TRUE(snk2.inport.getManager().getSelectedIndices().empty());
}

}  // namespace inviwo
