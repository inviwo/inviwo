/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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
#include <inviwo/core/processors/activityindicator.h>
#include <inviwo/core/processors/progressbar.h>
#include <inviwo/core/metadata/processormetadata.h>

#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <inviwo/qt/editor/editorgraphicsitem.h>
#include <inviwo/qt/editor/processorerroritem.h>

#include <modules/qtwidgets/labelgraphicsitem.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QEvent>
#include <QPropertyAnimation>
#include <QStaticText>
#include <QFont>
#include <QTimer>
#include <QElapsedTimer>
#include <warn/pop>

#include <optional>

class QGraphicsSimpleTextItem;
class QGraphicsLineItem;

namespace inviwo {

template <size_t minIntervalMs, typename F>
class RateLimiter {
public:
    explicit RateLimiter(F function = {})
        : function{std::move(function)}, timer_{}, callScheduled_{false} {
        timer_.start();
    }

    template <typename... Args>
    void operator()(QObject* owner, Args&&... args) {
        if (!callScheduled_) {
            if (timer_.hasExpired(minIntervalMs)) {
                invoke(std::forward<Args>(args)...);
            } else {
                callScheduled_ = true;
                QTimer::singleShot(minIntervalMs - timer_.elapsed(), owner,
                                   [this, ... a = std::forward<Args>(args)]() { invoke(a...); });
            }
        }
    }

    F function;

private:
    template <typename... Args>
    void invoke(Args&&... args) {
        timer_.restart();
        callScheduled_ = false;
        std::invoke(function, std::forward<Args>(args)...);
    }

    QElapsedTimer timer_;
    bool callScheduled_;
};

class Processor;
class ProcessorLinkGraphicsItem;
class ProcessorInportGraphicsItem;
class ProcessorOutportGraphicsItem;
class Port;
class Inport;
class Outport;

class IVW_QTEDITOR_API ProcessorGraphicsItem : public QObject,
                                               public EditorGraphicsItem,
                                               public ProcessorObserver,
                                               public ProcessorMetaDataObserver,
                                               public ActivityIndicatorObserver,
                                               public ProgressBarObserver {
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
public:
    explicit ProcessorGraphicsItem(Processor* processor);
    ProcessorGraphicsItem(const ProcessorGraphicsItem&) = delete;
    ProcessorGraphicsItem(ProcessorGraphicsItem&&) = delete;
    ProcessorGraphicsItem& operator=(const ProcessorGraphicsItem&) = delete;
    ProcessorGraphicsItem& operator=(ProcessorGraphicsItem&&) = delete;
    virtual ~ProcessorGraphicsItem();

    Processor* getProcessor() const;

    ProcessorInportGraphicsItem* getInportGraphicsItem(Inport* port) const;
    ProcessorOutportGraphicsItem* getOutportGraphicsItem(Outport* port) const;
    ProcessorLinkGraphicsItem* getLinkGraphicsItem() const;
    void setErrorText(std::string_view);

    void snapToGrid();

    // override for qgraphicsitem_cast (refer qt documentation)
    enum { Type = static_cast<int>(UserType) + static_cast<int>(ProcessorGraphicsType) };
    virtual int type() const override { return Type; }

    virtual void showToolTip(QGraphicsSceneHelpEvent* event) override;

#if IVW_PROFILING
    void resetTimeMeasurements();
    static void setShowCount(bool show);
#endif

    void setHighlight(bool val);

    static constexpr QSizeF size{150.0, 50.0};

    enum class PortType : std::uint8_t { In, Out };
    static QPointF portOffset(PortType type, size_t index);
    QPointF portPosition(PortType type, size_t index);

    void adoptWidget(std::unique_ptr<QWidget> widget) {
        ownedWidgets_.push_back(std::move(widget));
    }

    enum class State : std::uint8_t { Error, Invalid, Running, Ready };

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

    // ActivityIndicatorObserver overrides
    virtual void activityIndicatorChanged(bool active) override;

    // ProgressBarObserver overrides
    virtual void progressChanged(float) override;
    virtual void progressBarVisibilityChanged(bool visible) override;

    // QObject override
    virtual bool event(QEvent* e) override;

private:
    void delayedUpdate();

    enum class Running : std::uint8_t { Yes, No };
    void updateStatus(Running running);

    Processor* processor_;
    ProcessorMetaData* processorMeta_;

    QStaticText nameText_;
    QStaticText identifierText_;
    double identifierSize_;
    QStaticText tagText_;
    double tagSize_;

    QPropertyAnimation* animation_;
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
    size_t currentProcessCount_;
    double maxEvalTime_;
    double evalTime_;
    double totEvalTime_;
    Clock clock_;
    static bool showCount_;  // NOLINT
#endif

    State state_;
    State currentState_;
    std::unique_ptr<ProcessorErrorItem> errorText_;

    std::optional<float> progress_;
    std::optional<float> currentProgress_;
    bool dirty_;
    RateLimiter<250, void (*)(QGraphicsItem* p)> limitedUpdate_;
};

}  // namespace inviwo
