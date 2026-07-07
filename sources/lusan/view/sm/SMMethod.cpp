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
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/view/sm/SMMethod.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Methods page.
 *
 ************************************************************************/

#include "lusan/view/sm/SMMethod.hpp"

#include "lusan/common/NELusanCommon.hpp"
#include "lusan/data/common/DataTypeBase.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/DataTypeFactory.hpp"
#include "lusan/data/common/DocumentElem.hpp"
#include "lusan/data/common/MethodParameter.hpp"
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/sm/SMDataTypeModel.hpp"
#include "lusan/model/sm/SMMethodModel.hpp"
#include "lusan/view/sm/SMCodeEditor.hpp"
#include "lusan/view/sm/SMEventParamDetails.hpp"
#include "lusan/view/sm/SMMethodDetails.hpp"
#include "lusan/view/sm/SMMethodList.hpp"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QEvent>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QRadioButton>
#include <QShortcut>
#include <QSignalBlocker>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

SMMethod::SMMethod(SMMethodModel& model, QWidget* parent /*= nullptr*/)
    : QScrollArea       (parent)
    , mModel            (model)
    , mList             (new SMMethodList(this))
    , mDetails          (new SMMethodDetails(this))
    , mParamDetails     (new SMEventParamDetails(this))
    , mMethodNameCounter(0)
{
    buildUi();
    setupSignals();
    refreshAll();
}

void SMMethod::buildUi()
{
    QWidget* content = new QWidget(this);
    QVBoxLayout* root = new QVBoxLayout(content);

    QLabel* headline = new QLabel(tr("State Machine Methods Editor ..."), content);
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
    rightLayout->addWidget(mDetails);
    rightLayout->addWidget(mParamDetails);
    columns->addWidget(rightColumn, 1);

    root->addLayout(columns, 1);

    populateReturnCombo();
    populateParamTypeCombo();

    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setWidgetResizable(true);
    setWidget(content);
}

void SMMethod::setupSignals()
{
    QTreeWidget* table = mList->ctrlTableList();
    connect(table                       , &QTreeWidget::currentItemChanged, this, &SMMethod::onCurCellChanged);
    connect(mList->ctrlButtonAdd()      , &QToolButton::clicked           , this, &SMMethod::onAddClicked);
    connect(mList->ctrlButtonInsert()   , &QToolButton::clicked           , this, &SMMethod::onInsertClicked);
    connect(mList->ctrlButtonRemove()   , &QToolButton::clicked           , this, &SMMethod::onRemoveClicked);
    connect(mList->ctrlButtonMoveUp()   , &QToolButton::clicked           , this, &SMMethod::onMoveUpClicked);
    connect(mList->ctrlButtonMoveDown() , &QToolButton::clicked           , this, &SMMethod::onMoveDownClicked);
    connect(mList->ctrlButtonAddParam() , &QToolButton::clicked           , this, [this]() { addNewParam(); });
    connect(mList->actionNewTrigger()   , &QAction::triggered             , this, [this]() { addNewMethod(SMMethodEntry::eMethodType::Trigger); });
    connect(mList->actionNewAction()    , &QAction::triggered             , this, [this]() { addNewMethod(SMMethodEntry::eMethodType::Action); });
    connect(mList->actionNewCondition() , &QAction::triggered             , this, [this]() { addNewMethod(SMMethodEntry::eMethodType::Condition); });

    QShortcut* scAdd    = new QShortcut(QKeySequence(Qt::Key_Insert), table);
    QShortcut* scRemove = new QShortcut(QKeySequence(Qt::Key_Delete), table);
    scAdd->setContext(Qt::WidgetWithChildrenShortcut);
    scRemove->setContext(Qt::WidgetWithChildrenShortcut);
    connect(scAdd   , &QShortcut::activated, this, &SMMethod::onAddClicked);
    connect(scRemove, &QShortcut::activated, this, &SMMethod::onRemoveClicked);

    connect(mDetails->ctrlName()     , &QLineEdit::textChanged    , this, &SMMethod::onMethodNameTextChanged);
    connect(mDetails->ctrlName()     , &QLineEdit::editingFinished, this, &SMMethod::onMethodNameCommitted);
    connect(mDetails->ctrlTrigger()  , &QRadioButton::toggled     , this, &SMMethod::onMethodTypeToggled);
    connect(mDetails->ctrlAction()   , &QRadioButton::toggled     , this, &SMMethod::onMethodTypeToggled);
    connect(mDetails->ctrlCondition(), &QRadioButton::toggled     , this, &SMMethod::onMethodTypeToggled);
    connect(mDetails->ctrlReturn()->lineEdit(), &QLineEdit::editingFinished, this, &SMMethod::onReturnCommitted);
    connect(mDetails->ctrlReturn()   , &QComboBox::activated      , this, [this](int) { onReturnCommitted(); });
    connect(mDetails->ctrlHandler()  , &QRadioButton::toggled     , this, &SMMethod::onImplementToggled);
    connect(mDetails->ctrlEmbedded() , &QRadioButton::toggled     , this, &SMMethod::onImplementToggled);
    mDetails->ctrlBody()->ctrlBody()->installEventFilter(this);
    mDetails->ctrlDescription()->installEventFilter(this);

    connect(mParamDetails->ctrlName()      , &QLineEdit::textChanged        , this, &SMMethod::onParamNameTextChanged);
    connect(mParamDetails->ctrlName()      , &QLineEdit::editingFinished    , this, &SMMethod::onParamNameCommitted);
    connect(mParamDetails->ctrlTypes()     , &QComboBox::currentIndexChanged, this, &SMMethod::onParamTypeChanged);
    connect(mParamDetails->ctrlHasDefault(), &QCheckBox::toggled            , this, &SMMethod::onParamHasDefaultToggled);
    connect(mParamDetails->ctrlValue()     , &QLineEdit::editingFinished    , this, &SMMethod::onParamValueCommitted);
    mParamDetails->ctrlDescription()->installEventFilter(this);

    DocModelNotifier& notifier = mModel.getNotifier();
    connect(&notifier, &DocModelNotifier::documentReloaded, this, &SMMethod::onNotifierChanged);
    connect(&notifier, &DocModelNotifier::elementAdded  , this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::Method) onNotifierChanged(); });
    connect(&notifier, &DocModelNotifier::elementRemoved, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::Method) onNotifierChanged(); });
    connect(&notifier, &DocModelNotifier::elementChanged, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::Method) onNotifierChanged(); });
    connect(&notifier, &DocModelNotifier::listReordered , this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::Method) onNotifierChanged(); });
    // A trigger method shares its name space with events and timers; a rename elsewhere may
    // resolve or introduce a collision with the selected method's name.
    connect(&notifier, &DocModelNotifier::nameChanged, this, [this](uint32_t, const QString&, const QString&) {
        if (currentKind() == eRowKind::Method)
        {
            SMMethodEntry* method = currentMethod();
            mDetails->showNameHint(nameCollisionReason(method, mDetails->ctrlName()->text(), method != nullptr ? method->getId() : 0u));
        }
    });

    connect(&notifier, &DocModelNotifier::elementAdded  , this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) onDataTypesChanged(); });
    connect(&notifier, &DocModelNotifier::elementRemoved, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) onDataTypesChanged(); });
    connect(&notifier, &DocModelNotifier::elementChanged, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) onDataTypesChanged(); });
    connect(&notifier, &DocModelNotifier::listReordered , this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) onDataTypesChanged(); });
}

bool SMMethod::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::FocusOut)
    {
        if (watched == mDetails->ctrlDescription())
        {
            SMMethodEntry* method = currentMethod();
            if ((method != nullptr) && (currentKind() == eRowKind::Method))
            {
                mModel.setDescription(method->getId(), mDetails->ctrlDescription()->toPlainText());
            }
        }
        else if (watched == mDetails->ctrlBody()->ctrlBody())
        {
            SMMethodEntry* method = currentMethod();
            if ((method != nullptr) && (currentKind() == eRowKind::Method))
            {
                mModel.setBody(method->getId(), mDetails->ctrlBody()->ctrlBody()->toPlainText());
            }
        }
        else if (watched == mParamDetails->ctrlDescription())
        {
            SMMethodEntry* method = currentMethod();
            const uint32_t paramId = currentParamId();
            if ((method != nullptr) && (paramId != 0))
            {
                mModel.setParamDescription(method, paramId, mParamDetails->ctrlDescription()->toPlainText());
            }
        }
    }

    return QScrollArea::eventFilter(watched, event);
}

QString SMMethod::nameCollisionReason(const SMMethodEntry* method, const QString& name, uint32_t selfId) const
{
    if (name.isEmpty())
        return QString();

    SMMethodEntry* other = mModel.findMethod(name);
    if ((other != nullptr) && (other->getId() != selfId))
        return tr("'%1' is already used by another method").arg(name);

    // Only trigger methods participate in the shared stimulus name space (spec 6.10).
    if ((method != nullptr) && method->isTrigger())
    {
        const StateMachineData::StimulusRef ref = mModel.findStimulus(name);
        if ((ref.element != nullptr) && (ref.element->getId() != selfId))
        {
            if (ref.type == StateMachineData::eStimulusType::Event)
                return tr("'%1' is already used by an event").arg(name);
            if (ref.type == StateMachineData::eStimulusType::Timer)
                return tr("'%1' is already used by a timer").arg(name);
        }
    }

    return QString();
}

QString SMMethod::paramNameCollisionReason(const SMMethodEntry* owner, const QString& name, uint32_t selfId) const
{
    if ((owner == nullptr) || name.isEmpty())
        return QString();

    const MethodParameter* found = mModel.findParam(owner, name);
    return ((found != nullptr) && (found->getId() != selfId))
        ? tr("'%1' is already used by another parameter of this method").arg(name)
        : QString();
}

void SMMethod::populateReturnCombo()
{
    QComboBox* combo = mDetails->ctrlReturn();
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
        combo->addItem(type->getName());
    }
    for (DataTypeCustom* type : mModel.getDataTypeModel().getCustomDataTypes())
    {
        combo->addItem(type->getName());
    }

    combo->setCurrentText(current.isEmpty() ? QString::fromLatin1(SMMethodEntry::DEFAULT_RETURN) : current);
}

void SMMethod::populateParamTypeCombo()
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
    for (DataTypeCustom* type : mModel.getDataTypeModel().getCustomDataTypes())
    {
        combo->addItem(type->getName(), QVariant::fromValue(static_cast<DataTypeBase*>(type)));
    }

    combo->setCurrentText(current);
}

void SMMethod::setNodeText(QTreeWidgetItem* node, const DocumentElem* elem) const
{
    node->setIcon(0, elem->getIcon(ElementBase::eDisplay::DisplayName));
    node->setText(0, elem->getString(ElementBase::eDisplay::DisplayName));
    node->setIcon(1, elem->getIcon(ElementBase::eDisplay::DisplayType));
    node->setText(1, elem->getString(ElementBase::eDisplay::DisplayType));
    node->setIcon(2, elem->getIcon(ElementBase::eDisplay::DisplayValue));
    node->setText(2, elem->getString(ElementBase::eDisplay::DisplayValue));
}

QTreeWidgetItem* SMMethod::createMethodNode(SMMethodEntry* method) const
{
    QTreeWidgetItem* item = new QTreeWidgetItem();
    setNodeText(item, method);
    const QString reason = nameCollisionReason(method, method->getName(), method->getId());
    if (reason.isEmpty() == false)
    {
        item->setIcon(0, NELusanCommon::iconWarning(NELusanCommon::SizeSmall));
        item->setToolTip(0, reason);
    }
    item->setData(0, Qt::ItemDataRole::UserRole, static_cast<int>(eRowKind::Method));
    item->setData(1, Qt::ItemDataRole::UserRole, method->getId());
    item->setData(2, Qt::ItemDataRole::UserRole, 0u);

    for (const MethodParameter& param : method->getElements())
    {
        QTreeWidgetItem* child = new QTreeWidgetItem();
        setNodeText(child, &param);
        child->setData(0, Qt::ItemDataRole::UserRole, static_cast<int>(eRowKind::Param));
        child->setData(1, Qt::ItemDataRole::UserRole, method->getId());
        child->setData(2, Qt::ItemDataRole::UserRole, param.getId());
        item->addChild(child);
    }

    return item;
}

SMMethod::eRowKind SMMethod::currentKind() const
{
    QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
    if (item == nullptr)
        return eRowKind::None;

    return static_cast<eRowKind>(item->data(0, Qt::ItemDataRole::UserRole).toInt());
}

SMMethodEntry* SMMethod::currentMethod() const
{
    QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
    const eRowKind kind = currentKind();
    if ((item == nullptr) || ((kind != eRowKind::Method) && (kind != eRowKind::Param)))
        return nullptr;

    return mModel.findMethod(item->data(1, Qt::ItemDataRole::UserRole).toUInt());
}

uint32_t SMMethod::currentParamId() const
{
    QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
    return ((item != nullptr) && (currentKind() == eRowKind::Param)) ? item->data(2, Qt::ItemDataRole::UserRole).toUInt() : 0u;
}

void SMMethod::onCurCellChanged(QTreeWidgetItem* current, QTreeWidgetItem* /*previous*/)
{
    if (current == nullptr)
    {
        showCleanForm();
        updateToolbar(eRowKind::None);
        return;
    }

    switch (currentKind())
    {
    case eRowKind::Method:
    {
        SMMethodEntry* method = currentMethod();
        if (method != nullptr)
            selectedMethod(method);
        break;
    }
    case eRowKind::Param:
    {
        SMMethodEntry* method = currentMethod();
        if (method != nullptr)
            selectedParam(method, currentParamId());
        break;
    }
    default:
        showCleanForm();
        updateToolbar(eRowKind::None);
        break;
    }
}

void SMMethod::showDetails(eRowKind kind)
{
    const bool showParam = (kind == eRowKind::Param);
    if (mParamDetails->isHidden() != (showParam == false))
        mParamDetails->setHidden(showParam == false);
    if (mDetails->isHidden() != showParam)
        mDetails->setHidden(showParam);
}

void SMMethod::showCleanForm()
{
    showDetails(eRowKind::None);
    mDetails->showNameHint(QString());
    mDetails->setConditionVisible(false);
    mDetails->setBodyVisible(false);

    const QSignalBlocker blockName(mDetails->ctrlName());
    const QSignalBlocker blockDescr(mDetails->ctrlDescription());
    mDetails->ctrlName()->clear();
    mDetails->ctrlDescription()->clear();
    mDetails->ctrlName()->setPlaceholderText(tr("Select a method, or click Add"));
    mDetails->ctrlName()->setEnabled(false);
    mDetails->ctrlDescription()->setEnabled(false);
}

void SMMethod::updateToolbar(eRowKind kind)
{
    const bool hasEntry = (kind == eRowKind::Method) || (kind == eRowKind::Param);

    mList->ctrlButtonAdd()->setEnabled(true);
    mList->ctrlButtonInsert()->setEnabled(hasEntry);
    mList->ctrlButtonRemove()->setEnabled(hasEntry);
    mList->ctrlButtonAddParam()->setEnabled(hasEntry);

    if (hasEntry == false)
    {
        mList->ctrlButtonMoveUp()->setEnabled(false);
        mList->ctrlButtonMoveDown()->setEnabled(false);
    }
}

void SMMethod::showMethodForm(SMMethodEntry* method)
{
    const bool isCondition = method->isCondition();
    const bool isEmbedded = (method->getImplement() == SMMethodEntry::eImplement::Embedded);

    const QSignalBlocker blockName(mDetails->ctrlName());
    const QSignalBlocker blockTrigger(mDetails->ctrlTrigger());
    const QSignalBlocker blockAction(mDetails->ctrlAction());
    const QSignalBlocker blockCondition(mDetails->ctrlCondition());
    const QSignalBlocker blockReturn(mDetails->ctrlReturn());
    const QSignalBlocker blockHandler(mDetails->ctrlHandler());
    const QSignalBlocker blockEmbedded(mDetails->ctrlEmbedded());
    const QSignalBlocker blockBody(mDetails->ctrlBody()->ctrlBody());
    const QSignalBlocker blockDescr(mDetails->ctrlDescription());

    mDetails->ctrlName()->setEnabled(true);
    mDetails->ctrlName()->setPlaceholderText(QString());
    mDetails->ctrlName()->setText(method->getName());
    mDetails->ctrlDescription()->setEnabled(true);
    mDetails->ctrlDescription()->setPlainText(method->getDescription());

    mDetails->ctrlTrigger()->setChecked(method->isTrigger());
    mDetails->ctrlAction()->setChecked(method->isAction());
    mDetails->ctrlCondition()->setChecked(isCondition);

    mDetails->ctrlReturn()->setCurrentText(method->getReturn());
    mDetails->ctrlHandler()->setChecked(isEmbedded == false);
    mDetails->ctrlEmbedded()->setChecked(isEmbedded);
    mDetails->ctrlBody()->ctrlBody()->setPlainText(method->getBody());

    mDetails->setConditionVisible(isCondition);
    mDetails->setBodyVisible(isCondition && isEmbedded);
    updateBodyEditor(method);

    mDetails->showNameHint(nameCollisionReason(method, method->getName(), method->getId()));
}

void SMMethod::updateBodyEditor(SMMethodEntry* method)
{
    QString params;
    for (const MethodParameter& param : method->getElements())
    {
        if (params.isEmpty() == false)
            params += QStringLiteral(", ");
        params += QStringLiteral("%1 %2").arg(param.getType(), param.getName());
    }

    const QString ret = method->getReturn().isEmpty() ? QString::fromLatin1(SMMethodEntry::DEFAULT_RETURN) : method->getReturn();
    mDetails->ctrlBody()->setSignature(QStringLiteral("%1 %2(%3)").arg(ret, method->getName(), params));
    mDetails->ctrlBody()->setNote(tr("The machine instance is always captured: attributes, constants and accessors are in scope. "
                                     "The body must end in 'return <value>;' of the declared type."));
}

void SMMethod::selectedMethod(SMMethodEntry* method)
{
    showDetails(eRowKind::Method);
    showMethodForm(method);
    updateToolbar(eRowKind::Method);
    updateMoveButtons(mModel.findIndex(method), mModel.getMethodCount());
}

void SMMethod::selectedParam(SMMethodEntry* owner, uint32_t paramId)
{
    MethodParameter* param = mModel.findParam(owner, paramId);
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
    mParamDetails->showNameHint(paramNameCollisionReason(owner, param->getName(), param->getId()));

    updateToolbar(eRowKind::Param);
    updateMoveButtons(mModel.findParamIndex(owner, paramId), mModel.getParamCount(owner));
}

void SMMethod::updateMoveButtons(int row, int rowCount)
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

QString SMMethod::genMethodName()
{
    static const QString _defName("NewMethod");
    QString name;
    do
    {
        name = _defName + QString::number(++mMethodNameCounter);
    } while (mModel.findMethod(name) != nullptr);

    return name;
}

QString SMMethod::genParamName(const SMMethodEntry* method) const
{
    static const QString _defName("newParam");
    uint32_t count{ 0 };
    QString name;
    do
    {
        name = _defName + QString::number(++count);
    } while (mModel.findParam(method, name) != nullptr);

    return name;
}

void SMMethod::onAddClicked()
{
    // The Add button (and the Insert-key shortcut) always creates a method; parameters are
    // added through the separate Add-Parameter toolbar button.
    addNewMethod(SMMethodEntry::eMethodType::Trigger);
}

void SMMethod::addNewMethod(SMMethodEntry::eMethodType type)
{
    const QString name = genMethodName();
    SMMethodEntry* entry = mModel.createMethod(name, type);
    if (entry != nullptr)
    {
        selectMethod(entry->getId());
        mDetails->ctrlName()->setFocus();
        mDetails->ctrlName()->selectAll();
    }
}

void SMMethod::addNewParam()
{
    SMMethodEntry* method = currentMethod();
    if (method == nullptr)
        return;

    const QString name = genParamName(method);
    MethodParameter* param = mModel.createParam(method, name);
    if (param != nullptr)
    {
        selectMethod(method->getId(), param->getId());
        mParamDetails->ctrlName()->setFocus();
        mParamDetails->ctrlName()->selectAll();
    }
}

void SMMethod::onInsertClicked()
{
    switch (currentKind())
    {
    case eRowKind::Method:
    {
        SMMethodEntry* current = currentMethod();
        const int position = (current != nullptr ? mModel.findIndex(current) : 0);
        const QString name = genMethodName();
        SMMethodEntry* entry = mModel.insertMethod(position < 0 ? 0 : position, name, SMMethodEntry::eMethodType::Trigger);
        if (entry != nullptr)
        {
            selectMethod(entry->getId());
            mDetails->ctrlName()->setFocus();
            mDetails->ctrlName()->selectAll();
        }
        break;
    }
    case eRowKind::Param:
    {
        SMMethodEntry* method = currentMethod();
        if (method == nullptr)
            break;

        const int position = mModel.findParamIndex(method, currentParamId());
        const QString name = genParamName(method);
        MethodParameter* param = mModel.insertParam(method, position < 0 ? 0 : position, name);
        if (param != nullptr)
        {
            selectMethod(method->getId(), param->getId());
            mParamDetails->ctrlName()->setFocus();
            mParamDetails->ctrlName()->selectAll();
        }
        break;
    }
    default:
        break;
    }
}

void SMMethod::onRemoveClicked()
{
    switch (currentKind())
    {
    case eRowKind::Method:
    {
        SMMethodEntry* method = currentMethod();
        if (method == nullptr)
            break;

        const QList<SMMethodEntry*>& list = mModel.getMethods();
        const int index = mModel.findIndex(method);
        uint32_t neighborId = 0;
        if (list.size() > 1)
        {
            const int neighborIndex = ((index + 1) < list.size()) ? (index + 1) : (index - 1);
            neighborId = list.at(neighborIndex)->getId();
        }

        mModel.deleteMethod(method->getId());
        if (neighborId != 0)
        {
            selectMethod(neighborId);
        }
        break;
    }
    case eRowKind::Param:
    {
        SMMethodEntry* method = currentMethod();
        const uint32_t paramId = currentParamId();
        if ((method == nullptr) || (paramId == 0))
            break;

        const QList<MethodParameter>& params = mModel.getParams(method);
        const int index = mModel.findParamIndex(method, paramId);
        uint32_t neighborId = 0;
        if (params.size() > 1)
        {
            const int neighborIndex = ((index + 1) < params.size()) ? (index + 1) : (index - 1);
            neighborId = params.at(neighborIndex).getId();
        }

        mModel.deleteParam(method, paramId);
        selectMethod(method->getId(), neighborId);
        break;
    }
    default:
        break;
    }
}

void SMMethod::onMoveUpClicked()
{
    switch (currentKind())
    {
    case eRowKind::Method:
    {
        SMMethodEntry* method = currentMethod();
        if (method == nullptr)
            break;

        const int index = mModel.findIndex(method);
        if (index > 0)
        {
            const uint32_t neighborId = mModel.getMethods().at(index - 1)->getId();
            mModel.swapMethods(method->getId(), neighborId);
            selectMethod(method->getId());
        }
        break;
    }
    case eRowKind::Param:
    {
        SMMethodEntry* method = currentMethod();
        const uint32_t paramId = currentParamId();
        if ((method == nullptr) || (paramId == 0))
            break;

        const int index = mModel.findParamIndex(method, paramId);
        if (index > 0)
        {
            const uint32_t neighborId = mModel.getParams(method).at(index - 1).getId();
            mModel.swapParams(method, paramId, neighborId);
            selectMethod(method->getId(), paramId);
        }
        break;
    }
    default:
        break;
    }
}

void SMMethod::onMoveDownClicked()
{
    switch (currentKind())
    {
    case eRowKind::Method:
    {
        SMMethodEntry* method = currentMethod();
        if (method == nullptr)
            break;

        const int index = mModel.findIndex(method);
        if ((index >= 0) && (index < (mModel.getMethodCount() - 1)))
        {
            const uint32_t neighborId = mModel.getMethods().at(index + 1)->getId();
            mModel.swapMethods(method->getId(), neighborId);
            selectMethod(method->getId());
        }
        break;
    }
    case eRowKind::Param:
    {
        SMMethodEntry* method = currentMethod();
        const uint32_t paramId = currentParamId();
        if ((method == nullptr) || (paramId == 0))
            break;

        const int index = mModel.findParamIndex(method, paramId);
        const int count = mModel.getParamCount(method);
        if ((index >= 0) && (index < (count - 1)))
        {
            const uint32_t neighborId = mModel.getParams(method).at(index + 1).getId();
            mModel.swapParams(method, paramId, neighborId);
            selectMethod(method->getId(), paramId);
        }
        break;
    }
    default:
        break;
    }
}

void SMMethod::onMethodNameTextChanged(const QString& text)
{
    if (currentKind() == eRowKind::Method)
    {
        SMMethodEntry* method = currentMethod();
        mDetails->showNameHint(nameCollisionReason(method, text, method != nullptr ? method->getId() : 0u));
    }
}

void SMMethod::onMethodNameCommitted()
{
    SMMethodEntry* method = currentMethod();
    if ((method != nullptr) && (currentKind() == eRowKind::Method))
    {
        mModel.renameMethod(method->getId(), mDetails->ctrlName()->text());
    }
}

void SMMethod::onMethodTypeToggled(bool checked)
{
    if (checked == false)
        return;

    SMMethodEntry* method = currentMethod();
    if ((method == nullptr) || (currentKind() != eRowKind::Method))
        return;

    SMMethodEntry::eMethodType type = SMMethodEntry::eMethodType::Trigger;
    if (mDetails->ctrlAction()->isChecked())
        type = SMMethodEntry::eMethodType::Action;
    else if (mDetails->ctrlCondition()->isChecked())
        type = SMMethodEntry::eMethodType::Condition;

    mModel.setMethodType(method->getId(), type);
}

void SMMethod::onReturnCommitted()
{
    SMMethodEntry* method = currentMethod();
    if ((method == nullptr) || (currentKind() != eRowKind::Method) || (method->isCondition() == false))
        return;

    QString text = mDetails->ctrlReturn()->currentText().trimmed();
    if (text.isEmpty())
    {
        // A condition must always return a value; fall back to the default type.
        text = QString::fromLatin1(SMMethodEntry::DEFAULT_RETURN);
        const QSignalBlocker blocker(mDetails->ctrlReturn());
        mDetails->ctrlReturn()->setCurrentText(text);
    }

    mModel.setReturn(method->getId(), text);
}

void SMMethod::onImplementToggled(bool checked)
{
    if (checked == false)
        return;

    SMMethodEntry* method = currentMethod();
    if ((method == nullptr) || (currentKind() != eRowKind::Method))
        return;

    const SMMethodEntry::eImplement implement = mDetails->ctrlEmbedded()->isChecked()
        ? SMMethodEntry::eImplement::Embedded
        : SMMethodEntry::eImplement::Handler;
    mModel.setImplement(method->getId(), implement);
}

void SMMethod::onParamNameTextChanged(const QString& text)
{
    SMMethodEntry* method = currentMethod();
    const uint32_t paramId = currentParamId();
    if ((method != nullptr) && (paramId != 0))
    {
        mParamDetails->showNameHint(paramNameCollisionReason(method, text, paramId));
    }
}

void SMMethod::onParamNameCommitted()
{
    SMMethodEntry* method = currentMethod();
    const uint32_t paramId = currentParamId();
    if ((method != nullptr) && (paramId != 0))
    {
        mModel.setParamName(method, paramId, mParamDetails->ctrlName()->text());
    }
}

void SMMethod::onParamTypeChanged(int index)
{
    SMMethodEntry* method = currentMethod();
    const uint32_t paramId = currentParamId();
    if ((method == nullptr) || (paramId == 0) || (index < 0))
        return;

    DataTypeBase* selected = mParamDetails->ctrlTypes()->itemData(index, Qt::ItemDataRole::UserRole).value<DataTypeBase*>();
    if (selected != nullptr)
    {
        mModel.setParamType(method, paramId, selected->getName());
    }
}

void SMMethod::onParamHasDefaultToggled(bool checked)
{
    SMMethodEntry* method = currentMethod();
    const uint32_t paramId = currentParamId();
    if ((method == nullptr) || (paramId == 0))
        return;

    const QSignalBlocker blockValue(mParamDetails->ctrlValue());
    mParamDetails->ctrlValue()->setEnabled(checked);
    if (checked)
    {
        mParamDetails->ctrlValue()->setFocus();
    }

    mModel.setParamDefault(method, paramId, checked, mParamDetails->ctrlValue()->text());
}

void SMMethod::onParamValueCommitted()
{
    SMMethodEntry* method = currentMethod();
    const uint32_t paramId = currentParamId();
    if ((method == nullptr) || (paramId == 0))
        return;

    mModel.setParamDefault(method, paramId, mParamDetails->ctrlHasDefault()->isChecked(), mParamDetails->ctrlValue()->text());
}

void SMMethod::refreshAll()
{
    QTreeWidget* table = mList->ctrlTableList();

    const eRowKind selKind = currentKind();
    uint32_t selPrimary = 0;
    uint32_t selParam = 0;
    if (QTreeWidgetItem* cur = table->currentItem())
    {
        selPrimary = cur->data(1, Qt::ItemDataRole::UserRole).toUInt();
        selParam = cur->data(2, Qt::ItemDataRole::UserRole).toUInt();
    }

    QList<uint32_t> expandedMethods;
    for (int i = 0; i < table->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem* top = table->topLevelItem(i);
        if (top->isExpanded())
        {
            expandedMethods.append(top->data(1, Qt::ItemDataRole::UserRole).toUInt());
        }
    }

    {
        const QSignalBlocker blocker(table);
        table->clear();
        for (SMMethodEntry* entry : mModel.getMethods())
        {
            QTreeWidgetItem* node = createMethodNode(entry);
            table->addTopLevelItem(node);
            if (expandedMethods.contains(entry->getId()))
            {
                node->setExpanded(true);
            }
        }
    }

    bool restored = false;
    if ((selKind == eRowKind::Method) || (selKind == eRowKind::Param))
    {
        restored = selectMethod(selPrimary, selParam);
    }

    if (restored == false)
    {
        table->setCurrentItem(nullptr);
        showCleanForm();
        updateToolbar(eRowKind::None);
    }
}

bool SMMethod::selectMethod(uint32_t methodId, uint32_t paramId /*= 0*/)
{
    QTreeWidget* table = mList->ctrlTableList();
    for (int i = 0; i < table->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem* top = table->topLevelItem(i);
        if (top->data(1, Qt::ItemDataRole::UserRole).toUInt() != methodId)
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

void SMMethod::onNotifierChanged()
{
    refreshAll();
}

void SMMethod::onDataTypesChanged()
{
    populateReturnCombo();
    populateParamTypeCombo();

    SMMethodEntry* method = currentMethod();
    const uint32_t paramId = currentParamId();
    if ((method != nullptr) && (paramId != 0))
    {
        const QSignalBlocker blockType(mParamDetails->ctrlTypes());
        MethodParameter* param = mModel.findParam(method, paramId);
        if (param != nullptr)
        {
            mParamDetails->ctrlTypes()->setCurrentText(param->getType());
        }
    }
}
