/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <inviwo/qt/editor/dataopener.h>

#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/networkutils.h>
#include <inviwo/core/rendering/datavisualizer.h>
#include <inviwo/core/rendering/datavisualizermanager.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/common/inviwoapplication.h>

#include <modules/qtwidgets/inviwoqtutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QDialog>
#include <QLabel>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QCheckBox>
#include <QTextEdit>
#include <warn/pop>

namespace inviwo {

namespace {

class SelectVisualizerDialog : public QDialog {
public:
    SelectVisualizerDialog(const std::string& file, const std::vector<DataVisualizer*>& visualizers,
                           QWidget* parent = nullptr)
        : QDialog{parent} {

        setWindowTitle("Select Data Visualizer");

        auto mainLayout = new QGridLayout(this);
        mainLayout->setSpacing(15);

        int row = 0;

        auto fileNameLabel = new QLabel(utilqt::toQString(file));
        mainLayout->addWidget(fileNameLabel, row, 0, 1, 2);
        {
            loader_ = new QCheckBox("Data Loader");
            mainLayout->addWidget(loader_, ++row, 0, 1, 1);
            auto text = new QLabel("Add a data source processor");
            text->setWordWrap(true);
            mainLayout->addWidget(text, row, 1, 1, 1);
        }

        for (auto visualizer : visualizers) {
            auto use = new QCheckBox(utilqt::toQString(visualizer->getName()));
            useVisualuzers.push_back(use);
            mainLayout->addWidget(use, ++row, 0, 1, 1);
            auto text = new QLabel(utilqt::toQString(visualizer->getDescription()));
            text->setWordWrap(true);
            mainLayout->addWidget(text, row, 1, 1, 1);
        }

        auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
        mainLayout->addWidget(buttons, ++row, 0, 1, 2, Qt::AlignRight);
    }
    virtual ~SelectVisualizerDialog() = default;

    std::vector<QCheckBox*> useVisualuzers;
    QCheckBox* loader_;
};

}  // namespace

void util::insertNetworkForData(const std::string& dataFile, ProcessorNetwork* net,
                                bool alwaysFirst, bool onlySource, QWidget* parent) {
    auto app = net->getApplication();

    auto visualizers = app->getDataVisualizerManager()->getDataVisualizersForExtension(
        toLower(filesystem::getFileExtension(dataFile)));

    if (visualizers.empty()) return;

    auto addVisualizer = [&](DataVisualizer* visualizer, bool onlySource) {
        const auto orgBounds = util::getBoundingBox(net);

        std::vector<Processor*> added;
        if (onlySource || !visualizer->hasVisualizerNetwork()) {
            auto addedAndSource = visualizer->addSourceProcessor(dataFile, net);
            added.push_back(addedAndSource.first);
        } else {
            added = visualizer->addSourceAndVisualizerNetwork(dataFile, net);
        }

        // add to top right
        const auto bounds = util::getBoundingBox(added);
        const auto offset = ivec2{orgBounds.second.x, orgBounds.first.y} + ivec2{25, 0} +
                            ivec2{150, 0} - ivec2{bounds.first.x, bounds.first.y};
        util::offsetPosition(added, offset);
    };

    if (visualizers.size() == 1 || alwaysFirst) {
        addVisualizer(visualizers.front(), onlySource);

    } else {
        auto dialog = new SelectVisualizerDialog(dataFile, visualizers, parent);
        if (dialog->exec() == QDialog::Accepted) {
            if (dialog->loader_->isChecked()) {
                addVisualizer(visualizers.front(), true);
            }
            for (size_t i = 0; i < visualizers.size(); ++i) {
                if (dialog->useVisualuzers[i]->isChecked()) {
                    addVisualizer(visualizers[i], false);
                }
            }
        }
    }
}

}  // namespace inviwo
