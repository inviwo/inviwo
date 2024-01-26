/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2024 Inviwo Foundation
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

#include <inviwo/core/processors/processorobserver.h>
#include <inviwo/core/util/clock.h>
#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <inviwo/qt/editor/editorgrapicsitem.h>
#include <inviwo/qt/editor/processorerroritem.h>
#include <modules/qtwidgets/labelgraphicsitem.h>
#include <inviwo/core/metadata/processormetadata.h>
#include <warn/push>
#include <warn/ignore/all>
#include <QEvent>
#include <warn/pop>

class QGraphicsSimpleTextItem;
class QGraphicsLineItem;

namespace inviwo {

class Processor;
class ProcessorProgressGraphicsItem;
class ProcessorStatusGraphicsItem;
class ProcessorLinkGraphicsItem;
class ProcessorInportGraphicsItem;
class ProcessorOutportGraphicsItem;
class Port;
class Inport;
class Outport;

class IVW_QTEDITOR_API ProcessorGraphicsItem : public EditorGraphicsItem,
                                               public ProcessorObserver,
                                               public LabelGraphicsItemObserver,
                                               public ProcessorMetaDataObserver {
public:
    ProcessorGraphicsItem(Processor* processor);
    virtual ~ProcessorGraphicsItem();

    Processor* getProcessor() const;
    std::string getIdentifier() const;

    ProcessorInportGraphicsItem* getInportGraphicsItem(Inport* port) const;
    ProcessorOutportGraphicsItem* getOutportGraphicsItem(Outport* port) const;
    ProcessorLinkGraphicsItem* getLinkGraphicsItem() const;
    ProcessorStatusGraphicsItem* getStatusItem() const;
    void setErrorText(std::string_view);

    void editDisplayName();
    void editIdentifier();

    bool isEditingProcessorName();

    void snapToGrid();

    // override for qgraphicsitem_cast (refer qt documentation)
    enum { Type = static_cast<int>(UserType) + static_cast<int>(ProcessorGraphicsType) };
    virtual int type() const override { return Type; }

    virtual void showToolTip(QGraphicsSceneHelpEvent* event) override;

#if IVW_PROFILING
    void resetTimeMeasurements();
#endif

    void setHighlight(bool val);

    static const QSizeF size_;

    enum class PortType { In, Out };
    static QPointF portOffset(PortType type, size_t index);
    QPointF portPosition(PortType type, size_t index);

    void adoptWidget(std::unique_ptr<QWidget> widget) {
        ownedWidgets_.push_back(std::move(widget));
    }

protected:
    void paint(QPainter* p, const QStyleOptionGraphicsItem* options, QWidget* widget) override;
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

    virtual void mousePressEvent(QGraphicsSceneMouseEvent* e) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* e) override;

    void updateWidgets();

    void addInport(Inport* port);
    void addOutport(Outport* port);
    void removeInport(Inport* port);
    void removeOutport(Outport* port);

    // LabelGraphicsItem overrides
    void onLabelGraphicsItemChanged(LabelGraphicsItem* item) override;
    void onLabelGraphicsItemEdited(LabelGraphicsItem* item) override;

    // ProcessorObserver overrides
    virtual void onProcessorReadyChanged(Processor*) override;
    virtual void onProcessorPortAdded(Processor*, Port*) override;
    virtual void onProcessorPortRemoved(Processor*, Port*) override;
    virtual void onProcessorAboutToProcess(Processor*) override;
    virtual void onProcessorFinishedProcess(Processor*) override;

    virtual void onProcessorStartBackgroundWork(Processor*, size_t jobs) override {
        backgroundJobs_ += jobs;
    };
    virtual void onProcessorFinishBackgroundWork(Processor*, size_t jobs) override {
        backgroundJobs_ -= jobs;
    };

    // ProcessorMetaDataObserver overrides
    virtual void onProcessorMetaDataPositionChange() override;
    virtual void onProcessorMetaDataVisibilityChange() override;
    virtual void onProcessorMetaDataSelectionChange() override;

private:
    void positionLabels();

    Processor* processor_;
    LabelGraphicsItem* displayNameLabel_;
    LabelGraphicsItem* identifierLabel_;
    LabelGraphicsItem* tagLabel_;
    ProcessorMetaData* processorMeta_;

    ProcessorProgressGraphicsItem* progressItem_;
    ProcessorStatusGraphicsItem* statusItem_;
    ProcessorLinkGraphicsItem* linkItem_;

    std::map<Inport*, ProcessorInportGraphicsItem*> inportItems_;
    std::map<Outport*, ProcessorOutportGraphicsItem*> outportItems_;

    std::vector<std::unique_ptr<QWidget>> ownedWidgets_;

    bool highlight_;
    QColor backgroundColor_;
    size_t backgroundJobs_;

    std::shared_ptr<std::function<void(std::string_view, std::string_view)>> idChange_;
    std::shared_ptr<std::function<void(std::string_view, std::string_view)>> nameChange_;

#if IVW_PROFILING
    size_t processCount_;
    LabelGraphicsItem* countLabel_;
    double maxEvalTime_;
    double evalTime_;
    double totEvalTime_;
    Clock clock_;
#endif

    std::unique_ptr<ProcessorErrorItem> errorText_;
};

}  // namespace inviwo
