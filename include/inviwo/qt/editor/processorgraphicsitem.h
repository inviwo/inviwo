/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2018 Inviwo Foundation
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

#ifndef IVW_PROCESSORGRAPHICSITEM_H
#define IVW_PROCESSORGRAPHICSITEM_H

#include <inviwo/core/processors/processorobserver.h>
#include <inviwo/core/util/clock.h>
#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <inviwo/qt/editor/editorgrapicsitem.h>
#include <modules/qtwidgets/labelgraphicsitem.h>
#include <inviwo/core/metadata/processormetadata.h>
#include <warn/push>
#include <warn/ignore/all>
#include <QEvent>
#include <warn/pop>

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

    void editDisplayName();
    void editIdentifier();

    bool isEditingProcessorName();

    void snapToGrid();

    // override for qgraphicsitem_cast (refer qt documentation)
    enum { Type = UserType + ProcessorGraphicsType };
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

protected:
    void paint(QPainter* p, const QStyleOptionGraphicsItem* options, QWidget* widget) override;
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

    void updateWidgets();

    void addInport(Inport* port);
    void addOutport(Outport* port);
    void removeInport(Inport* port);
    void removeOutport(Outport* port);

    // LabelGraphicsItem overrides
    void onLabelGraphicsItemChanged(LabelGraphicsItem* item) override;
    void onLabelGraphicsItemEdited(LabelGraphicsItem* item) override;

    // ProcessorObserver overrides
    virtual void onProcessorIdentifierChanged(Processor*, const std::string&) override;
    virtual void onProcessorDisplayNameChanged(Processor*, const std::string&) override;
    virtual void onProcessorReadyChanged(Processor*) override;
    virtual void onProcessorPortAdded(Processor*, Port*) override;
    virtual void onProcessorPortRemoved(Processor*, Port*) override;
#if IVW_PROFILING
    virtual void onProcessorAboutToProcess(Processor*) override;
    virtual void onProcessorFinishedProcess(Processor*) override;
#endif

    // ProcessorMetaDataObserver overrides
    virtual void onProcessorMetaDataPositionChange() override;
    virtual void onProcessorMetaDataVisibilityChange() override;
    virtual void onProcessorMetaDataSelectionChange() override;

private:
    void positionLablels();

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

    bool highlight_;

#if IVW_PROFILING
    size_t processCount_;
    LabelGraphicsItem* countLabel_;
    double maxEvalTime_;
    double evalTime_;
    double totEvalTime_;
    Clock clock_;
#endif
};

}  // namespace inviwo

#endif  // IVW_PROCESSORGRAPHICSITEM_H
