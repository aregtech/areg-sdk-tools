/************************************************************************
 *  This file is part of the Lusan project, an official component of the Areg SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the Areg Framework.
 *
 *  Lusan is available as free and open-source software under the Apache version 2.0 License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   (c) 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/view/sm/SMEvent.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Events page.
 *
 ************************************************************************/

#include "lusan/view/sm/SMEvent.hpp"

#include "lusan/common/NELusanCommon.hpp"
#include "lusan/data/common/DataTypeBase.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/DataTypeFactory.hpp"
#include "lusan/data/common/DocumentElem.hpp"
#include "lusan/data/common/MethodParameter.hpp"
#include "lusan/data/sm/SMEventData.hpp"
#include "lusan/data/sm/SMTimerData.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/sm/SMDataTypeModel.hpp"
#include "lusan/model/sm/SMEventModel.hpp"
#include "lusan/model/sm/SMTimerModel.hpp"
#include "lusan/view/sm/SMEventDetails.hpp"
#include "lusan/view/sm/SMEventList.hpp"
#include "lusan/view/sm/SMEventParamDetails.hpp"
#include "lusan/view/sm/SMTimerDetails.hpp"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QEvent>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QShortcut>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

namespace
{
    //!< Refreshes a deprecated check-box + hint pair from a flag/hint, without re-triggering
    //!< the edit signals that would otherwise push a spurious command.
    void applyDeprecatedDisplay(QCheckBox* checkBox, QLineEdit* hintEdit, bool deprecated, const QString& hint)
    {
        const QSignalBlocker blockCheck(checkBox);
        const QSignalBlocker blockHint(hintEdit);
        checkBox->setChecked(deprecated);
        hintEdit->setEnabled(deprecated);
        hintEdit->setText(deprecated ? hint : QString());
    }
}

SMEvent::SMEvent(SMEventModel& eventModel, SMTimerModel& timerModel, QWidget* parent /*= nullptr*/)
    : QScrollArea       (parent)
    , mEventModel       (eventModel)
    , mTimerModel       (timerModel)
    , mList             (new SMEventList(this))
    , mDetails          (new SMEventDetails(this))
    , mParamDetails     (new SMEventParamDetails(this))
    , mTimerDetails     (new SMTimerDetails(this))
    , mEventNameCounter (0)
    , mTimerNameCounter (0)
{
    buildUi();
    setupSignals();
    refreshAll();
}

SMEventList* SMEvent::getList() const
{
    return mList;
}

void SMEvent::buildUi()
{
    QWidget* content = new QWidget(this);
    QVBoxLayout* root = new QVBoxLayout(content);

    QLabel* headline = new QLabel(tr("State Machine Events and Timers Editor ..."), content);
    QFont headlineFont{ headline->font() };
    headlineFont.setPointSize(20);
    headlineFont.setBold(true);
    headlineFont.setItalic(true);
    headline->setFont(headlineFont);
    root->addWidget(headline);

    QHBoxLayout* columns = new QHBoxLayout();

    mList->setParent(content);
    mList->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    columns->addWidget(mList, 1);

    QWidget* rightColumn = new QWidget(content);
    rightColumn->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightColumn);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    mDetails->setParent(rightColumn);
    mParamDetails->setParent(rightColumn);
    mParamDetails->setHidden(true);
    mTimerDetails->setParent(rightColumn);
    mTimerDetails->setHidden(true);
    rightLayout->addWidget(mDetails);
    rightLayout->addWidget(mParamDetails);
    rightLayout->addWidget(mTimerDetails);
    columns->addWidget(rightColumn, 1);

    root->addLayout(columns, 1);

    populateTypeCombo();

    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setWidgetResizable(true);
    setWidget(content);
}

void SMEvent::setupSignals()
{
    QTreeWidget* table = mList->ctrlTableList();
    connect(table                       , &QTreeWidget::currentItemChanged, this, &SMEvent::onCurCellChanged);
    connect(mList->ctrlButtonAdd()      , &QToolButton::clicked           , this, &SMEvent::onAddClicked);
    connect(mList->ctrlButtonInsert()   , &QToolButton::clicked           , this, &SMEvent::onInsertClicked);
    connect(mList->ctrlButtonRemove()   , &QToolButton::clicked           , this, &SMEvent::onRemoveClicked);
    connect(mList->ctrlButtonMoveUp()   , &QToolButton::clicked           , this, &SMEvent::onMoveUpClicked);
    connect(mList->ctrlButtonMoveDown() , &QToolButton::clicked           , this, &SMEvent::onMoveDownClicked);
    connect(mList->ctrlButtonAddParam() , &QToolButton::clicked           , this, [this]() { addNewParam(); });
    connect(mList->actionNewEvent()     , &QAction::triggered             , this, [this]() { addNewEvent(); });
    connect(mList->actionNewTimer()     , &QAction::triggered             , this, [this]() { addNewTimer(); });

    QShortcut* scAdd    = new QShortcut(QKeySequence(Qt::Key_Insert), table);
    QShortcut* scRemove = new QShortcut(QKeySequence(Qt::Key_Delete), table);
    QShortcut* scRename = new QShortcut(QKeySequence(Qt::Key_F2), table);
    scAdd->setContext(Qt::WidgetWithChildrenShortcut);
    scRemove->setContext(Qt::WidgetWithChildrenShortcut);
    scRename->setContext(Qt::WidgetWithChildrenShortcut);
    connect(scAdd   , &QShortcut::activated, this, &SMEvent::onAddClicked);
    connect(scRemove, &QShortcut::activated, this, &SMEvent::onRemoveClicked);
    connect(scRename, &QShortcut::activated, this, [this]() { focusNameField(); });

    connect(mDetails->ctrlName()   , &QLineEdit::textChanged    , this, &SMEvent::onEventNameTextChanged);
    connect(mDetails->ctrlName()   , &QLineEdit::editingFinished, this, &SMEvent::onEventNameCommitted);
    connect(mDetails->ctrlDeprecated()   , &QCheckBox::toggled        , this, &SMEvent::onEventDeprecatedToggled);
    connect(mDetails->ctrlDeprecateHint(), &QLineEdit::editingFinished, this, &SMEvent::onEventDeprecateHintCommitted);
    mDetails->ctrlDescription()->installEventFilter(this);

    connect(mParamDetails->ctrlName()      , &QLineEdit::textChanged          , this, &SMEvent::onParamNameTextChanged);
    connect(mParamDetails->ctrlName()      , &QLineEdit::editingFinished      , this, &SMEvent::onParamNameCommitted);
    connect(mParamDetails->ctrlTypes()     , &QComboBox::currentIndexChanged  , this, &SMEvent::onParamTypeChanged);
    connect(mParamDetails->ctrlHasDefault(), &QCheckBox::toggled              , this, &SMEvent::onParamHasDefaultToggled);
    connect(mParamDetails->ctrlValue()     , &QLineEdit::editingFinished      , this, &SMEvent::onParamValueCommitted);
    connect(mParamDetails->ctrlDeprecated()   , &QCheckBox::toggled        , this, &SMEvent::onParamDeprecatedToggled);
    connect(mParamDetails->ctrlDeprecateHint(), &QLineEdit::editingFinished, this, &SMEvent::onParamDeprecateHintCommitted);
    mParamDetails->ctrlDescription()->installEventFilter(this);

    connect(mTimerDetails->ctrlName()      , &QLineEdit::textChanged            , this, &SMEvent::onTimerNameTextChanged);
    connect(mTimerDetails->ctrlName()      , &QLineEdit::editingFinished        , this, &SMEvent::onTimerNameCommitted);
    connect(mTimerDetails->ctrlTimeout()   , &QAbstractSpinBox::editingFinished , this, &SMEvent::onTimeoutCommitted);
    connect(mTimerDetails->ctrlRepeat()    , &QAbstractSpinBox::editingFinished , this, &SMEvent::onRepeatCommitted);
    connect(mTimerDetails->ctrlContinuous(), &QCheckBox::toggled                , this, &SMEvent::onContinuousToggled);
    connect(mTimerDetails->ctrlDeprecated()   , &QCheckBox::toggled        , this, &SMEvent::onTimerDeprecatedToggled);
    connect(mTimerDetails->ctrlDeprecateHint(), &QLineEdit::editingFinished, this, &SMEvent::onTimerDeprecateHintCommitted);
    mTimerDetails->ctrlDescription()->installEventFilter(this);

    DocModelNotifier& notifier = mEventModel.getNotifier();
    connect(&notifier, &DocModelNotifier::documentReloaded, this, &SMEvent::onNotifierChanged);
    connect(&notifier, &DocModelNotifier::elementAdded  , this, [this](uint32_t, eDocElementKind kind) { if ((kind == eDocElementKind::Event) || (kind == eDocElementKind::Timer)) onNotifierChanged(); });
    connect(&notifier, &DocModelNotifier::elementRemoved, this, [this](uint32_t, eDocElementKind kind) { if ((kind == eDocElementKind::Event) || (kind == eDocElementKind::Timer)) onNotifierChanged(); });
    connect(&notifier, &DocModelNotifier::elementChanged, this, [this](uint32_t, eDocElementKind kind) { if ((kind == eDocElementKind::Event) || (kind == eDocElementKind::Timer)) onNotifierChanged(); });
    connect(&notifier, &DocModelNotifier::listReordered , this, [this](uint32_t, eDocElementKind kind) { if ((kind == eDocElementKind::Event) || (kind == eDocElementKind::Timer)) onNotifierChanged(); });
    // Renaming a trigger/event/timer elsewhere may resolve or introduce a collision with the
    // selected element's name; the collision hint depends on the whole stimulus name space.
    connect(&notifier, &DocModelNotifier::nameChanged, this, [this](uint32_t, const QString&, const QString&) {
        switch (currentKind())
        {
        case eRowKind::Event:
        {
            SMEventEntry* event = currentEvent();
            mDetails->showNameHint(stimulusCollisionReason(mDetails->ctrlName()->text(), event != nullptr ? event->getId() : 0u));
            break;
        }
        case eRowKind::Timer:
            mTimerDetails->showNameHint(stimulusCollisionReason(mTimerDetails->ctrlName()->text(), currentTimerId()));
            break;
        default:
            break;
        }
    });

    connect(&notifier, &DocModelNotifier::elementAdded  , this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) onDataTypesChanged(); });
    connect(&notifier, &DocModelNotifier::elementRemoved, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) onDataTypesChanged(); });
    connect(&notifier, &DocModelNotifier::elementChanged, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) onDataTypesChanged(); });
    connect(&notifier, &DocModelNotifier::listReordered , this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) onDataTypesChanged(); });
}

bool SMEvent::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::FocusOut)
    {
        if (watched == mDetails->ctrlDescription())
        {
            SMEventEntry* ev = currentEvent();
            if ((ev != nullptr) && (currentKind() == eRowKind::Event))
            {
                mEventModel.setDescription(ev->getId(), mDetails->ctrlDescription()->toPlainText());
            }
        }
        else if (watched == mParamDetails->ctrlDescription())
        {
            SMEventEntry* ev = currentEvent();
            const uint32_t paramId = currentParamId();
            if ((ev != nullptr) && (paramId != 0))
            {
                mEventModel.setParamDescription(ev, paramId, mParamDetails->ctrlDescription()->toPlainText());
            }
        }
        else if (watched == mTimerDetails->ctrlDescription())
        {
            const uint32_t timerId = currentTimerId();
            if (timerId != 0)
            {
                mTimerModel.setDescription(timerId, mTimerDetails->ctrlDescription()->toPlainText());
            }
        }
    }

    return QScrollArea::eventFilter(watched, event);
}

QString SMEvent::stimulusCollisionReason(const QString& name, uint32_t selfId) const
{
    if (name.isEmpty())
        return QString();

    const StateMachineData::StimulusRef ref = mEventModel.findStimulus(name);
    if ((ref.type == StateMachineData::eStimulusType::None) || (ref.element == nullptr) || (ref.element->getId() == selfId))
        return QString();

    switch (ref.type)
    {
    case StateMachineData::eStimulusType::Trigger:
        return tr("'%1' is already used by a trigger method").arg(name);
    case StateMachineData::eStimulusType::Event:
        return tr("'%1' is already used by an event").arg(name);
    case StateMachineData::eStimulusType::Timer:
        return tr("'%1' is already used by a timer").arg(name);
    default:
        return QString();
    }
}

QString SMEvent::paramNameCollisionReason(const SMEventEntry* owner, const QString& name, uint32_t selfId) const
{
    if ((owner == nullptr) || name.isEmpty())
        return QString();

    const MethodParameter* found = mEventModel.findParam(owner, name);
    return ((found != nullptr) && (found->getId() != selfId))
        ? tr("'%1' is already used by another parameter of this event").arg(name)
        : QString();
}

void SMEvent::populateTypeCombo()
{
    QComboBox* combo = mParamDetails->ctrlTypes();
    const QSignalBlocker blocker(combo);
    const QString current = combo->currentText();
    combo->clear();

    QList<DataTypeBase*> predefined;
    DataTypeFactory::getPredefinedTypes(predefined, QList<DataTypeBase::eCategory>{
          DataTypeBase::eCategory::Primitive
        , DataTypeBase::eCategory::PrimitiveSint
        , DataTypeBase::eCategory::PrimitiveUint
        , DataTypeBase::eCategory::PrimitiveFloat
        , DataTypeBase::eCategory::BasicObject
    });

    for (DataTypeBase* type : predefined)
    {
        combo->addItem(type->getName(), QVariant::fromValue(type));
    }

    for (DataTypeCustom* type : mEventModel.getDataTypeModel().getCustomDataTypes())
    {
        combo->addItem(type->getName(), QVariant::fromValue(static_cast<DataTypeBase*>(type)));
    }

    combo->setCurrentText(current);
}

void SMEvent::setNodeText(QTreeWidgetItem* node, const DocumentElem* elem) const
{
    node->setIcon(0, elem->getIcon(ElementBase::eDisplay::DisplayName));
    node->setText(0, elem->getString(ElementBase::eDisplay::DisplayName));
    node->setIcon(1, elem->getIcon(ElementBase::eDisplay::DisplayType));
    node->setText(1, elem->getString(ElementBase::eDisplay::DisplayType));
    node->setIcon(2, elem->getIcon(ElementBase::eDisplay::DisplayValue));
    node->setText(2, elem->getString(ElementBase::eDisplay::DisplayValue));
}

QTreeWidgetItem* SMEvent::createEventNode(SMEventEntry* event) const
{
    QTreeWidgetItem* item = new QTreeWidgetItem();
    setNodeText(item, event);
    const QString reason = stimulusCollisionReason(event->getName(), event->getId());
    if (reason.isEmpty() == false)
    {
        item->setIcon(0, NELusanCommon::iconWarning(NELusanCommon::SizeSmall));
        item->setToolTip(0, reason);
    }
    item->setData(0, Qt::ItemDataRole::UserRole, static_cast<int>(eRowKind::Event));
    item->setData(1, Qt::ItemDataRole::UserRole, event->getId());
    item->setData(2, Qt::ItemDataRole::UserRole, 0u);

    for (const MethodParameter& param : event->getElements())
    {
        QTreeWidgetItem* child = new QTreeWidgetItem();
        setNodeText(child, &param);
        child->setData(0, Qt::ItemDataRole::UserRole, static_cast<int>(eRowKind::Param));
        child->setData(1, Qt::ItemDataRole::UserRole, event->getId());
        child->setData(2, Qt::ItemDataRole::UserRole, param.getId());
        item->addChild(child);
    }

    return item;
}

QTreeWidgetItem* SMEvent::createTimerNode(const SMTimerEntry& entry) const
{
    QTreeWidgetItem* item = new QTreeWidgetItem();
    setNodeText(item, &entry);
    const QString reason = stimulusCollisionReason(entry.getName(), entry.getId());
    if (reason.isEmpty() == false)
    {
        item->setIcon(0, NELusanCommon::iconWarning(NELusanCommon::SizeSmall));
        item->setToolTip(0, reason);
    }
    item->setData(0, Qt::ItemDataRole::UserRole, static_cast<int>(eRowKind::Timer));
    item->setData(1, Qt::ItemDataRole::UserRole, entry.getId());
    item->setData(2, Qt::ItemDataRole::UserRole, 0u);

    return item;
}

SMEvent::eRowKind SMEvent::currentKind() const
{
    QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
    if (item == nullptr)
        return eRowKind::None;
    if (item == mList->ctrlGroupEvents())
        return eRowKind::GroupEvents;
    if (item == mList->ctrlGroupTimers())
        return eRowKind::GroupTimers;

    return static_cast<eRowKind>(item->data(0, Qt::ItemDataRole::UserRole).toInt());
}

SMEventEntry* SMEvent::currentEvent() const
{
    QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
    const eRowKind kind = currentKind();
    if ((item == nullptr) || ((kind != eRowKind::Event) && (kind != eRowKind::Param)))
        return nullptr;

    return mEventModel.findEvent(item->data(1, Qt::ItemDataRole::UserRole).toUInt());
}

uint32_t SMEvent::currentParamId() const
{
    QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
    return ((item != nullptr) && (currentKind() == eRowKind::Param)) ? item->data(2, Qt::ItemDataRole::UserRole).toUInt() : 0u;
}

uint32_t SMEvent::currentTimerId() const
{
    QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
    return ((item != nullptr) && (currentKind() == eRowKind::Timer)) ? item->data(1, Qt::ItemDataRole::UserRole).toUInt() : 0u;
}

void SMEvent::onCurCellChanged(QTreeWidgetItem* current, QTreeWidgetItem* /*previous*/)
{
    if (current == nullptr)
    {
        showCleanForm();
        updateToolbar(eRowKind::None);
        return;
    }

    switch (currentKind())
    {
    case eRowKind::GroupEvents:
        selectedGroup(eRowKind::GroupEvents);
        break;
    case eRowKind::GroupTimers:
        selectedGroup(eRowKind::GroupTimers);
        break;
    case eRowKind::Event:
    {
        SMEventEntry* event = currentEvent();
        if (event != nullptr)
            selectedEvent(event);
        break;
    }
    case eRowKind::Param:
    {
        SMEventEntry* event = currentEvent();
        if (event != nullptr)
            selectedParam(event, currentParamId());
        break;
    }
    case eRowKind::Timer:
    {
        SMTimerEntry* entry = mTimerModel.findTimer(currentTimerId());
        if (entry != nullptr)
            selectedTimer(entry);
        break;
    }
    default:
        showCleanForm();
        updateToolbar(eRowKind::None);
        break;
    }
}

void SMEvent::showDetails(eRowKind kind)
{
    QWidget* visible = mDetails;
    if (kind == eRowKind::Param)
        visible = mParamDetails;
    else if (kind == eRowKind::Timer)
        visible = mTimerDetails;

    for (QWidget* form : { static_cast<QWidget*>(mDetails), static_cast<QWidget*>(mParamDetails), static_cast<QWidget*>(mTimerDetails) })
    {
        if (form == visible)
        {
            if (form->isHidden())
                form->show();
        }
        else if (form->isHidden() == false)
        {
            form->hide();
        }
    }
}

void SMEvent::showCleanForm()
{
    showDetails(eRowKind::None);
    mDetails->showNameHint(QString());

    const QSignalBlocker blockName(mDetails->ctrlName());
    const QSignalBlocker blockDescr(mDetails->ctrlDescription());
    mDetails->ctrlName()->clear();
    mDetails->ctrlDescription()->clear();
    mDetails->ctrlName()->setPlaceholderText(tr("Select an event or timer, or click Add"));
    mDetails->ctrlName()->setEnabled(false);
    mDetails->ctrlDescription()->setEnabled(false);
    applyDeprecatedDisplay(mDetails->ctrlDeprecated(), mDetails->ctrlDeprecateHint(), false, QString());
}

void SMEvent::updateToolbar(eRowKind kind)
{
    const bool hasEntry = (kind == eRowKind::Event) || (kind == eRowKind::Param) || (kind == eRowKind::Timer);

    // The Add button always opens its menu, so it stays enabled regardless of selection.
    mList->ctrlButtonAdd()->setEnabled(true);
    mList->ctrlButtonInsert()->setEnabled(hasEntry);
    mList->ctrlButtonRemove()->setEnabled(hasEntry);
    mList->ctrlButtonAddParam()->setEnabled((kind == eRowKind::Event) || (kind == eRowKind::Param));

    if (hasEntry == false)
    {
        mList->ctrlButtonMoveUp()->setEnabled(false);
        mList->ctrlButtonMoveDown()->setEnabled(false);
    }
}

void SMEvent::selectedGroup(eRowKind kind)
{
    showCleanForm();
    updateToolbar(kind);
}

void SMEvent::selectedEvent(SMEventEntry* event)
{
    showDetails(eRowKind::Event);
    mDetails->ctrlName()->setEnabled(true);
    mDetails->ctrlName()->setPlaceholderText(QString());
    mDetails->ctrlDescription()->setEnabled(true);

    {
        const QSignalBlocker blockName(mDetails->ctrlName());
        const QSignalBlocker blockDescr(mDetails->ctrlDescription());
        mDetails->ctrlName()->setText(event->getName());
        mDetails->ctrlDescription()->setPlainText(event->getDescription());
    }
    applyDeprecatedDisplay(mDetails->ctrlDeprecated(), mDetails->ctrlDeprecateHint(), event->getIsDeprecated(), event->getDeprecateHint());
    mDetails->showNameHint(stimulusCollisionReason(event->getName(), event->getId()));

    updateToolbar(eRowKind::Event);
    updateMoveButtons(mEventModel.findIndex(event), mEventModel.getEventCount());
}

void SMEvent::selectedParam(SMEventEntry* owner, uint32_t paramId)
{
    MethodParameter* param = mEventModel.findParam(owner, paramId);
    if (param == nullptr)
        return;

    showDetails(eRowKind::Param);

    {
        const QSignalBlocker blockName(mParamDetails->ctrlName());
        const QSignalBlocker blockType(mParamDetails->ctrlTypes());
        const QSignalBlocker blockHasDefault(mParamDetails->ctrlHasDefault());
        const QSignalBlocker blockValue(mParamDetails->ctrlValue());
        const QSignalBlocker blockDescr(mParamDetails->ctrlDescription());
        mParamDetails->ctrlName()->setText(param->getName());
        mParamDetails->ctrlTypes()->setCurrentText(param->getType());
        mParamDetails->ctrlHasDefault()->setChecked(param->hasDefault());
        mParamDetails->ctrlValue()->setEnabled(param->hasDefault());
        mParamDetails->ctrlValue()->setText(param->getValue());
        mParamDetails->ctrlDescription()->setPlainText(param->getDescription());
    }
    applyDeprecatedDisplay(mParamDetails->ctrlDeprecated(), mParamDetails->ctrlDeprecateHint(), param->getIsDeprecated(), param->getDeprecateHint());
    mParamDetails->showNameHint(paramNameCollisionReason(owner, param->getName(), param->getId()));

    updateToolbar(eRowKind::Param);
    updateMoveButtons(mEventModel.findParamIndex(owner, paramId), mEventModel.getParamCount(owner));
}

void SMEvent::selectedTimer(const SMTimerEntry* entry)
{
    showDetails(eRowKind::Timer);

    {
        const QSignalBlocker blockName(mTimerDetails->ctrlName());
        const QSignalBlocker blockTimeout(mTimerDetails->ctrlTimeout());
        const QSignalBlocker blockRepeat(mTimerDetails->ctrlRepeat());
        const QSignalBlocker blockCont(mTimerDetails->ctrlContinuous());
        const QSignalBlocker blockDescr(mTimerDetails->ctrlDescription());
        mTimerDetails->ctrlName()->setText(entry->getName());
        mTimerDetails->ctrlTimeout()->setValue(static_cast<int>(entry->getTimeout()));
        const bool continuous = entry->isContinuous();
        mTimerDetails->ctrlContinuous()->setChecked(continuous);
        mTimerDetails->ctrlRepeat()->setEnabled(continuous == false);
        mTimerDetails->ctrlRepeat()->setValue(continuous ? 1 : static_cast<int>(entry->getRepeat()));
        mTimerDetails->ctrlDescription()->setPlainText(entry->getDescription());
    }
    applyDeprecatedDisplay(mTimerDetails->ctrlDeprecated(), mTimerDetails->ctrlDeprecateHint(), entry->getIsDeprecated(), entry->getDeprecateHint());
    mTimerDetails->showNameHint(stimulusCollisionReason(entry->getName(), entry->getId()));

    updateToolbar(eRowKind::Timer);
    updateMoveButtons(mTimerModel.findIndex(entry->getId()), mTimerModel.getTimerCount());
}

void SMEvent::updateMoveButtons(int row, int rowCount)
{
    if ((row < 0) || (row >= rowCount))
    {
        mList->ctrlButtonMoveUp()->setEnabled(false);
        mList->ctrlButtonMoveDown()->setEnabled(false);
        return;
    }

    mList->ctrlButtonMoveUp()->setEnabled(row > 0);
    mList->ctrlButtonMoveDown()->setEnabled(row < (rowCount - 1));
}

void SMEvent::focusNameField()
{
    QLineEdit* name = nullptr;
    switch (currentKind())
    {
    case eRowKind::Event:
        name = mDetails->ctrlName();
        break;
    case eRowKind::Param:
        name = mParamDetails->ctrlName();
        break;
    case eRowKind::Timer:
        name = mTimerDetails->ctrlName();
        break;
    default:
        break;
    }

    if (name != nullptr)
    {
        name->setFocus();
        name->selectAll();
    }
}

QString SMEvent::genEventName()
{
    static const QString _defName("NewEvent");
    QString name;
    do
    {
        name = _defName + QString::number(++mEventNameCounter);
    } while (mEventModel.findEvent(name) != nullptr);

    return name;
}

QString SMEvent::genTimerName()
{
    static const QString _defName("NewTimer");
    QString name;
    do
    {
        name = _defName + QString::number(++mTimerNameCounter);
    } while (mTimerModel.findTimer(name) != nullptr);

    return name;
}

QString SMEvent::genParamName(const SMEventEntry* event) const
{
    static const QString _defName("newParam");
    uint32_t count{ 0 };
    QString name;
    do
    {
        name = _defName + QString::number(++count);
    } while (mEventModel.findParam(event, name) != nullptr);

    return name;
}

void SMEvent::onAddClicked()
{
    switch (currentKind())
    {
    case eRowKind::GroupEvents:
    case eRowKind::Event:
        addNewEvent();
        break;
    case eRowKind::Param:
        addNewParam();
        break;
    case eRowKind::GroupTimers:
    case eRowKind::Timer:
        addNewTimer();
        break;
    default:
        break;
    }
}

void SMEvent::addNewEvent()
{
    const QString name = genEventName();
    SMEventEntry* entry = mEventModel.createEvent(name);
    if (entry != nullptr)
    {
        selectEvent(entry->getId());
        mDetails->ctrlName()->setFocus();
        mDetails->ctrlName()->selectAll();
    }
}

void SMEvent::addNewTimer()
{
    const QString name = genTimerName();
    SMTimerEntry* entry = mTimerModel.createTimer(name);
    if (entry != nullptr)
    {
        selectTimer(entry->getId());
        mTimerDetails->ctrlName()->setFocus();
        mTimerDetails->ctrlName()->selectAll();
    }
}

void SMEvent::addNewParam()
{
    SMEventEntry* event = currentEvent();
    if (event == nullptr)
        return;

    const QString name = genParamName(event);
    MethodParameter* param = mEventModel.createParam(event, name);
    if (param != nullptr)
    {
        selectEvent(event->getId(), param->getId());
        mParamDetails->ctrlName()->setFocus();
        mParamDetails->ctrlName()->selectAll();
    }
}

void SMEvent::onInsertClicked()
{
    switch (currentKind())
    {
    case eRowKind::Event:
    {
        SMEventEntry* current = currentEvent();
        const int position = (current != nullptr ? mEventModel.findIndex(current) : 0);
        const QString name = genEventName();
        SMEventEntry* entry = mEventModel.insertEvent(position < 0 ? 0 : position, name);
        if (entry != nullptr)
        {
            selectEvent(entry->getId());
            mDetails->ctrlName()->setFocus();
            mDetails->ctrlName()->selectAll();
        }
        break;
    }
    case eRowKind::Param:
    {
        SMEventEntry* event = currentEvent();
        if (event == nullptr)
            break;

        const int position = mEventModel.findParamIndex(event, currentParamId());
        const QString name = genParamName(event);
        MethodParameter* param = mEventModel.insertParam(event, position < 0 ? 0 : position, name);
        if (param != nullptr)
        {
            selectEvent(event->getId(), param->getId());
            mParamDetails->ctrlName()->setFocus();
            mParamDetails->ctrlName()->selectAll();
        }
        break;
    }
    case eRowKind::Timer:
    {
        const uint32_t id = currentTimerId();
        const int position = (id != 0 ? mTimerModel.findIndex(id) : 0);
        const QString name = genTimerName();
        SMTimerEntry* entry = mTimerModel.insertTimer(position < 0 ? 0 : position, name);
        if (entry != nullptr)
        {
            selectTimer(entry->getId());
            mTimerDetails->ctrlName()->setFocus();
            mTimerDetails->ctrlName()->selectAll();
        }
        break;
    }
    default:
        break;
    }
}

void SMEvent::onRemoveClicked()
{
    switch (currentKind())
    {
    case eRowKind::Event:
    {
        SMEventEntry* event = currentEvent();
        if (event == nullptr)
            break;

        const QList<SMEventEntry*>& list = mEventModel.getEvents();
        const int index = mEventModel.findIndex(event);
        uint32_t neighborId = 0;
        if (list.size() > 1)
        {
            const int neighborIndex = ((index + 1) < list.size()) ? (index + 1) : (index - 1);
            neighborId = list.at(neighborIndex)->getId();
        }

        mEventModel.deleteEvent(event->getId());
        if (neighborId != 0)
        {
            selectEvent(neighborId);
        }
        break;
    }
    case eRowKind::Param:
    {
        SMEventEntry* event = currentEvent();
        const uint32_t paramId = currentParamId();
        if ((event == nullptr) || (paramId == 0))
            break;

        const QList<MethodParameter>& params = mEventModel.getParams(event);
        const int index = mEventModel.findParamIndex(event, paramId);
        uint32_t neighborId = 0;
        if (params.size() > 1)
        {
            const int neighborIndex = ((index + 1) < params.size()) ? (index + 1) : (index - 1);
            neighborId = params.at(neighborIndex).getId();
        }

        mEventModel.deleteParam(event, paramId);
        selectEvent(event->getId(), neighborId);
        break;
    }
    case eRowKind::Timer:
    {
        const uint32_t id = currentTimerId();
        if (id == 0)
            break;

        const QList<SMTimerEntry>& list = mTimerModel.getTimers();
        const int index = mTimerModel.findIndex(id);
        uint32_t neighborId = 0;
        if (list.size() > 1)
        {
            const int neighborIndex = ((index + 1) < list.size()) ? (index + 1) : (index - 1);
            neighborId = list.at(neighborIndex).getId();
        }

        mTimerModel.deleteTimer(id);
        if (neighborId != 0)
        {
            selectTimer(neighborId);
        }
        break;
    }
    default:
        break;
    }
}

void SMEvent::onMoveUpClicked()
{
    switch (currentKind())
    {
    case eRowKind::Event:
    {
        SMEventEntry* event = currentEvent();
        if (event == nullptr)
            break;

        const int index = mEventModel.findIndex(event);
        if (index > 0)
        {
            const uint32_t neighborId = mEventModel.getEvents().at(index - 1)->getId();
            mEventModel.swapEvents(event->getId(), neighborId);
            selectEvent(event->getId());
        }
        break;
    }
    case eRowKind::Param:
    {
        SMEventEntry* event = currentEvent();
        const uint32_t paramId = currentParamId();
        if ((event == nullptr) || (paramId == 0))
            break;

        const int index = mEventModel.findParamIndex(event, paramId);
        if (index > 0)
        {
            const uint32_t neighborId = mEventModel.getParams(event).at(index - 1).getId();
            mEventModel.swapParams(event, paramId, neighborId);
            selectEvent(event->getId(), paramId);
        }
        break;
    }
    case eRowKind::Timer:
    {
        const uint32_t id = currentTimerId();
        if (id == 0)
            break;

        const int index = mTimerModel.findIndex(id);
        if (index > 0)
        {
            const uint32_t neighborId = mTimerModel.getTimers().at(index - 1).getId();
            mTimerModel.swapTimers(id, neighborId);
            selectTimer(id);
        }
        break;
    }
    default:
        break;
    }
}

void SMEvent::onMoveDownClicked()
{
    switch (currentKind())
    {
    case eRowKind::Event:
    {
        SMEventEntry* event = currentEvent();
        if (event == nullptr)
            break;

        const int index = mEventModel.findIndex(event);
        if ((index >= 0) && (index < (mEventModel.getEventCount() - 1)))
        {
            const uint32_t neighborId = mEventModel.getEvents().at(index + 1)->getId();
            mEventModel.swapEvents(event->getId(), neighborId);
            selectEvent(event->getId());
        }
        break;
    }
    case eRowKind::Param:
    {
        SMEventEntry* event = currentEvent();
        const uint32_t paramId = currentParamId();
        if ((event == nullptr) || (paramId == 0))
            break;

        const int index = mEventModel.findParamIndex(event, paramId);
        const int count = mEventModel.getParamCount(event);
        if ((index >= 0) && (index < (count - 1)))
        {
            const uint32_t neighborId = mEventModel.getParams(event).at(index + 1).getId();
            mEventModel.swapParams(event, paramId, neighborId);
            selectEvent(event->getId(), paramId);
        }
        break;
    }
    case eRowKind::Timer:
    {
        const uint32_t id = currentTimerId();
        if (id == 0)
            break;

        const int index = mTimerModel.findIndex(id);
        if ((index >= 0) && (index < (mTimerModel.getTimerCount() - 1)))
        {
            const uint32_t neighborId = mTimerModel.getTimers().at(index + 1).getId();
            mTimerModel.swapTimers(id, neighborId);
            selectTimer(id);
        }
        break;
    }
    default:
        break;
    }
}

void SMEvent::onEventNameTextChanged(const QString& text)
{
    if (currentKind() == eRowKind::Event)
    {
        SMEventEntry* event = currentEvent();
        mDetails->showNameHint(stimulusCollisionReason(text, event != nullptr ? event->getId() : 0u));
    }
}

void SMEvent::onEventNameCommitted()
{
    SMEventEntry* event = currentEvent();
    if ((event != nullptr) && (currentKind() == eRowKind::Event))
    {
        mEventModel.renameEvent(event->getId(), mDetails->ctrlName()->text());
    }
}

void SMEvent::onEventDeprecatedToggled(bool checked)
{
    SMEventEntry* event = currentEvent();
    if ((event == nullptr) || (currentKind() != eRowKind::Event))
        return;

    mEventModel.setDeprecated(event->getId(), checked);
    const QSignalBlocker blockHint(mDetails->ctrlDeprecateHint());
    mDetails->ctrlDeprecateHint()->setEnabled(checked);
    mDetails->ctrlDeprecateHint()->setText(checked ? event->getDeprecateHint() : QString());
    if (checked)
    {
        mDetails->ctrlDeprecateHint()->setFocus();
    }
}

void SMEvent::onEventDeprecateHintCommitted()
{
    SMEventEntry* event = currentEvent();
    if ((event != nullptr) && (currentKind() == eRowKind::Event))
    {
        mEventModel.setDeprecateHint(event->getId(), mDetails->ctrlDeprecateHint()->text());
    }
}

void SMEvent::onParamNameTextChanged(const QString& text)
{
    SMEventEntry* event = currentEvent();
    const uint32_t paramId = currentParamId();
    if ((event != nullptr) && (paramId != 0))
    {
        mParamDetails->showNameHint(paramNameCollisionReason(event, text, paramId));
    }
}

void SMEvent::onParamNameCommitted()
{
    SMEventEntry* event = currentEvent();
    const uint32_t paramId = currentParamId();
    if ((event != nullptr) && (paramId != 0))
    {
        mEventModel.setParamName(event, paramId, mParamDetails->ctrlName()->text());
    }
}

void SMEvent::onParamTypeChanged(int index)
{
    SMEventEntry* event = currentEvent();
    const uint32_t paramId = currentParamId();
    if ((event == nullptr) || (paramId == 0) || (index < 0))
        return;

    DataTypeBase* selected = mParamDetails->ctrlTypes()->itemData(index, Qt::ItemDataRole::UserRole).value<DataTypeBase*>();
    if (selected != nullptr)
    {
        mEventModel.setParamType(event, paramId, selected->getName());
    }
}

void SMEvent::onParamHasDefaultToggled(bool checked)
{
    SMEventEntry* event = currentEvent();
    const uint32_t paramId = currentParamId();
    if ((event == nullptr) || (paramId == 0))
        return;

    const QSignalBlocker blockValue(mParamDetails->ctrlValue());
    mParamDetails->ctrlValue()->setEnabled(checked);
    if (checked)
    {
        mParamDetails->ctrlValue()->setFocus();
    }

    mEventModel.setParamDefault(event, paramId, checked, mParamDetails->ctrlValue()->text());
}

void SMEvent::onParamValueCommitted()
{
    SMEventEntry* event = currentEvent();
    const uint32_t paramId = currentParamId();
    if ((event == nullptr) || (paramId == 0))
        return;

    mEventModel.setParamDefault(event, paramId, mParamDetails->ctrlHasDefault()->isChecked(), mParamDetails->ctrlValue()->text());
}

void SMEvent::onParamDeprecatedToggled(bool checked)
{
    SMEventEntry* event = currentEvent();
    const uint32_t paramId = currentParamId();
    if ((event == nullptr) || (paramId == 0))
        return;

    mEventModel.setParamDeprecated(event, paramId, checked);
    MethodParameter* param = mEventModel.findParam(event, paramId);
    const QSignalBlocker blockHint(mParamDetails->ctrlDeprecateHint());
    mParamDetails->ctrlDeprecateHint()->setEnabled(checked);
    mParamDetails->ctrlDeprecateHint()->setText((checked && (param != nullptr)) ? param->getDeprecateHint() : QString());
    if (checked)
    {
        mParamDetails->ctrlDeprecateHint()->setFocus();
    }
}

void SMEvent::onParamDeprecateHintCommitted()
{
    SMEventEntry* event = currentEvent();
    const uint32_t paramId = currentParamId();
    if ((event != nullptr) && (paramId != 0))
    {
        mEventModel.setParamDeprecateHint(event, paramId, mParamDetails->ctrlDeprecateHint()->text());
    }
}

void SMEvent::onTimerNameTextChanged(const QString& text)
{
    if (currentKind() == eRowKind::Timer)
    {
        mTimerDetails->showNameHint(stimulusCollisionReason(text, currentTimerId()));
    }
}

void SMEvent::onTimerNameCommitted()
{
    const uint32_t id = currentTimerId();
    if (id != 0)
    {
        mTimerModel.renameTimer(id, mTimerDetails->ctrlName()->text());
    }
}

void SMEvent::onTimeoutCommitted()
{
    const uint32_t id = currentTimerId();
    if (id != 0)
    {
        mTimerModel.setTimeout(id, static_cast<uint32_t>(mTimerDetails->ctrlTimeout()->value()));
    }
}

void SMEvent::onRepeatCommitted()
{
    const uint32_t id = currentTimerId();
    if (id != 0)
    {
        mTimerModel.setRepeat(id, static_cast<uint32_t>(mTimerDetails->ctrlRepeat()->value()));
    }
}

void SMEvent::onContinuousToggled(bool checked)
{
    const uint32_t id = currentTimerId();
    if (id == 0)
        return;

    const QSignalBlocker blockRepeat(mTimerDetails->ctrlRepeat());
    mTimerDetails->ctrlRepeat()->setEnabled(checked == false);
    mTimerDetails->ctrlRepeat()->setValue(1);
    mTimerModel.setRepeat(id, checked ? 0u : 1u);
}

void SMEvent::onTimerDeprecatedToggled(bool checked)
{
    const uint32_t id = currentTimerId();
    if (id == 0)
        return;

    mTimerModel.setDeprecated(id, checked);
    SMTimerEntry* entry = mTimerModel.findTimer(id);
    const QSignalBlocker blockHint(mTimerDetails->ctrlDeprecateHint());
    mTimerDetails->ctrlDeprecateHint()->setEnabled(checked);
    mTimerDetails->ctrlDeprecateHint()->setText((checked && (entry != nullptr)) ? entry->getDeprecateHint() : QString());
    if (checked)
    {
        mTimerDetails->ctrlDeprecateHint()->setFocus();
    }
}

void SMEvent::onTimerDeprecateHintCommitted()
{
    const uint32_t id = currentTimerId();
    if (id != 0)
    {
        mTimerModel.setDeprecateHint(id, mTimerDetails->ctrlDeprecateHint()->text());
    }
}

void SMEvent::refreshAll()
{
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* groupEvents = mList->ctrlGroupEvents();
    QTreeWidgetItem* groupTimers = mList->ctrlGroupTimers();

    const eRowKind selKind = currentKind();
    uint32_t selPrimary = 0;
    uint32_t selParam = 0;
    if (QTreeWidgetItem* cur = table->currentItem())
    {
        selPrimary = cur->data(1, Qt::ItemDataRole::UserRole).toUInt();
        selParam = cur->data(2, Qt::ItemDataRole::UserRole).toUInt();
    }

    QList<uint32_t> expandedEvents;
    for (int i = 0; i < groupEvents->childCount(); ++i)
    {
        QTreeWidgetItem* child = groupEvents->child(i);
        if (child->isExpanded())
        {
            expandedEvents.append(child->data(1, Qt::ItemDataRole::UserRole).toUInt());
        }
    }

    {
        const QSignalBlocker blocker(table);
        qDeleteAll(groupEvents->takeChildren());
        qDeleteAll(groupTimers->takeChildren());

        for (SMEventEntry* entry : mEventModel.getEvents())
        {
            groupEvents->addChild(createEventNode(entry));
        }

        for (const SMTimerEntry& entry : mTimerModel.getTimers())
        {
            groupTimers->addChild(createTimerNode(entry));
        }

        mList->updateGroups(mEventModel.getEventCount(), mTimerModel.getTimerCount());

        for (int i = 0; i < groupEvents->childCount(); ++i)
        {
            QTreeWidgetItem* child = groupEvents->child(i);
            if (expandedEvents.contains(child->data(1, Qt::ItemDataRole::UserRole).toUInt()))
            {
                child->setExpanded(true);
            }
        }
    }

    bool restored = false;
    switch (selKind)
    {
    case eRowKind::GroupEvents:
    case eRowKind::GroupTimers:
    {
        QTreeWidgetItem* group = (selKind == eRowKind::GroupEvents) ? groupEvents : groupTimers;
        table->setCurrentItem(group);
        // setCurrentItem on the already-current group emits nothing; re-apply explicitly.
        selectedGroup(selKind);
        restored = true;
        break;
    }
    case eRowKind::Event:
    case eRowKind::Param:
        restored = selectEvent(selPrimary, selParam);
        break;
    case eRowKind::Timer:
        restored = selectTimer(selPrimary);
        break;
    default:
        break;
    }

    if (restored == false)
    {
        // Clearing current is a real change here, so the handler applies the clean state.
        table->setCurrentItem(nullptr);
        showCleanForm();
        updateToolbar(eRowKind::None);
    }
}

bool SMEvent::selectEvent(uint32_t eventId, uint32_t paramId /*= 0*/)
{
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* groupEvents = mList->ctrlGroupEvents();
    for (int i = 0; i < groupEvents->childCount(); ++i)
    {
        QTreeWidgetItem* top = groupEvents->child(i);
        if (top->data(1, Qt::ItemDataRole::UserRole).toUInt() != eventId)
            continue;

        if (paramId != 0)
        {
            for (int j = 0; j < top->childCount(); ++j)
            {
                QTreeWidgetItem* child = top->child(j);
                if (child->data(2, Qt::ItemDataRole::UserRole).toUInt() == paramId)
                {
                    table->setCurrentItem(child);
                    return true;
                }
            }
        }

        table->setCurrentItem(top);
        return true;
    }

    return false;
}

bool SMEvent::selectTimer(uint32_t timerId)
{
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* groupTimers = mList->ctrlGroupTimers();
    for (int i = 0; i < groupTimers->childCount(); ++i)
    {
        QTreeWidgetItem* item = groupTimers->child(i);
        if (item->data(1, Qt::ItemDataRole::UserRole).toUInt() == timerId)
        {
            table->setCurrentItem(item);
            return true;
        }
    }

    return false;
}

void SMEvent::onNotifierChanged()
{
    refreshAll();
}

void SMEvent::onDataTypesChanged()
{
    populateTypeCombo();
    SMEventEntry* event = currentEvent();
    const uint32_t paramId = currentParamId();
    if ((event != nullptr) && (paramId != 0))
    {
        const QSignalBlocker blockType(mParamDetails->ctrlTypes());
        MethodParameter* param = mEventModel.findParam(event, paramId);
        if (param != nullptr)
        {
            mParamDetails->ctrlTypes()->setCurrentText(param->getType());
        }
    }
}
