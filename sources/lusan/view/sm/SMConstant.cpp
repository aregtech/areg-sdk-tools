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
 *  \file        lusan/view/sm/SMConstant.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Constants page.
 *
 ************************************************************************/

#include "lusan/view/sm/SMConstant.hpp"

#include "lusan/common/NELusanCommon.hpp"
#include "lusan/data/common/ConstantEntry.hpp"
#include "lusan/data/common/DataTypeBase.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/DataTypeEnum.hpp"
#include "lusan/data/common/DataTypeFactory.hpp"
#include "lusan/data/common/EnumEntry.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/sm/SMConstantModel.hpp"
#include "lusan/model/sm/SMDataTypeModel.hpp"
#include "lusan/model/sm/SMGuardWhereUsed.hpp"
#include "lusan/model/sm/SMLiteralValidator.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/sm/SMConstantDetails.hpp"
#include "lusan/view/sm/SMConstantList.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QSignalBlocker>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

namespace
{
    //!< Refreshes a deprecated checkbox + hint pair from an entry's current state, without
    //!< re-triggering the edit signals that would otherwise push a spurious command.
    void applyDeprecatedDisplay(QCheckBox* checkBox, QLineEdit* hintEdit, const ConstantEntry* entry)
    {
        const QSignalBlocker blockCheck(checkBox);
        const QSignalBlocker blockHint(hintEdit);
        const bool deprecated = (entry != nullptr) && entry->getIsDeprecated();
        checkBox->setChecked(deprecated);
        hintEdit->setEnabled(deprecated);
        hintEdit->setText(deprecated ? entry->getDeprecateHint() : QString());
    }
}

SMConstant::SMConstant(SMConstantModel& model, QWidget* parent /*= nullptr*/)
    : QScrollArea       (parent)
    , mModel            (model)
    , mList             (new SMConstantList(this))
    , mDetails          (new SMConstantDetails(this))
    , mNameCounter      (0)
{
    buildUi();
    setupSignals();
    refreshAll();
}

SMConstantList* SMConstant::getList() const
{
    return mList;
}

void SMConstant::buildUi()
{
    QWidget* content = new QWidget(this);
    QVBoxLayout* root = new QVBoxLayout(content);

    QLabel* headline = new QLabel(tr("State Machine Constant Editor ..."), content);
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

    mDetails->setParent(content);
    mDetails->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    columns->addWidget(mDetails, 1);

    root->addLayout(columns, 1);

    populateTypeCombo();

    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setWidgetResizable(true);
    setWidget(content);
}

void SMConstant::setupSignals()
{
    connect(mList->ctrlTableList()     , &QTreeWidget::currentItemChanged, this, &SMConstant::onCurCellChanged);
    connect(mList->ctrlButtonAdd()     , &QToolButton::clicked           , this, &SMConstant::onAddClicked);
    connect(mList->ctrlButtonInsert()  , &QToolButton::clicked           , this, &SMConstant::onInsertClicked);
    connect(mList->ctrlButtonRemove()  , &QToolButton::clicked           , this, &SMConstant::onRemoveClicked);
    connect(mList->ctrlButtonMoveUp()  , &QToolButton::clicked           , this, &SMConstant::onMoveUpClicked);
    connect(mList->ctrlButtonMoveDown(), &QToolButton::clicked           , this, &SMConstant::onMoveDownClicked);

    connect(mDetails->ctrlName()   , &QLineEdit::editingFinished    , this, &SMConstant::onNameCommitted);
    connect(mDetails->ctrlTypes()  , &QComboBox::currentIndexChanged, this, &SMConstant::onTypeChanged);
    connect(mDetails->ctrlValue()  , &QLineEdit::editingFinished    , this, &SMConstant::onValueCommitted);
    connect(mDetails->ctrlValue()  , &QLineEdit::textChanged        , this, &SMConstant::onValueTextChanged);
    connect(mDetails->ctrlDeprecated()   , &QCheckBox::toggled        , this, &SMConstant::onDeprecatedToggled);
    connect(mDetails->ctrlDeprecateHint(), &QLineEdit::editingFinished, this, &SMConstant::onDeprecateHintCommitted);
    mDetails->ctrlDescription()->installEventFilter(this);

    connect(&mModel.getNotifier(), &DocModelNotifier::documentReloaded, this, &SMConstant::onNotifierChanged);
    connect(&mModel.getNotifier(), &DocModelNotifier::elementAdded, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::Constant) onNotifierChanged(); });
    connect(&mModel.getNotifier(), &DocModelNotifier::elementRemoved, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::Constant) onNotifierChanged(); });
    connect(&mModel.getNotifier(), &DocModelNotifier::elementChanged, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::Constant) onNotifierChanged(); });
    connect(&mModel.getNotifier(), &DocModelNotifier::listReordered, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::Constant) onNotifierChanged(); });

    connect(&mModel.getNotifier(), &DocModelNotifier::elementAdded, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) onDataTypesChanged(); });
    connect(&mModel.getNotifier(), &DocModelNotifier::elementRemoved, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) onDataTypesChanged(); });
    connect(&mModel.getNotifier(), &DocModelNotifier::elementChanged, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) onDataTypesChanged(); });
    connect(&mModel.getNotifier(), &DocModelNotifier::listReordered, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) onDataTypesChanged(); });
}

bool SMConstant::eventFilter(QObject* watched, QEvent* event)
{
    if ((watched == mDetails->ctrlDescription()) && (event->type() == QEvent::FocusOut))
    {
        const uint32_t id = currentConstantId();
        if (id != 0)
        {
            mModel.setDescription(id, mDetails->ctrlDescription()->toPlainText());
        }
    }

    return QScrollArea::eventFilter(watched, event);
}

void SMConstant::populateTypeCombo()
{
    QComboBox* combo = mDetails->ctrlTypes();
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

void SMConstant::updateValueControl(const ConstantEntry* entry)
{
    QLineEdit* value = mDetails->ctrlValue();
    const QSignalBlocker blocker(value);
    value->clear();
    mDetails->setValueChoices(QStringList());

    if (entry == nullptr)
    {
        value->setEnabled(false);
        value->setToolTip(QString());
        mDetails->showValueHint(QString());
        return;
    }

    const QString typeName = entry->getType();
    DataTypeCustom* custom = mModel.getDataTypeModel().findDataType(typeName);
    // Imported is deliberately excluded: the type is defined elsewhere and opaque to Lusan,
    // so any literal the user types is accepted as-is rather than rejected outright.
    const bool hasNoLiteral = (custom != nullptr)
        && ((custom->getCategory() == DataTypeBase::eCategory::Structure)
         || (custom->getCategory() == DataTypeBase::eCategory::Container));

    if (hasNoLiteral)
    {
        value->setEnabled(false);
        value->setToolTip(tr("'%1' has no literal value; this constant has no value.").arg(typeName));
        mDetails->showValueHint(QString());
        return;
    }

    value->setEnabled(true);
    value->setToolTip(QString());

    if ((custom != nullptr) && (custom->getCategory() == DataTypeBase::eCategory::Enumeration))
    {
        QStringList choices;
        for (const EnumEntry& field : static_cast<DataTypeEnum*>(custom)->getElements())
        {
            choices.append(field.getName());
        }

        mDetails->setValueChoices(choices);
    }

    value->setText(entry->getValue());
    updateValueValidation(typeName, entry->getValue());
}

QString SMConstant::valueValidationReason(const QString& typeName, const QString& value) const
{
    // No value is always valid - a missing literal simply means "not set", not "invalid".
    if (value.isEmpty())
        return QString();

    DataTypeCustom* custom = mModel.getDataTypeModel().findDataType(typeName);
    if (custom != nullptr)
    {
        if (custom->getCategory() == DataTypeBase::eCategory::Enumeration)
        {
            if (static_cast<DataTypeEnum*>(custom)->findElement(value) == nullptr)
            {
                return tr("'%1' is not an enumerator of '%2'").arg(value, typeName);
            }
        }
        // Structure/Container: the value control is disabled, never reaches here.
        // Imported: the type is opaque to Lusan - accept any literal as-is.
        return QString();
    }

    return SMLiteralValidator::validate(typeName, value);
}

void SMConstant::updateValueValidation(const QString& typeName, const QString& value)
{
    mDetails->showValueHint(valueValidationReason(typeName, value));
}

void SMConstant::setNodeText(QTreeWidgetItem* node, const ConstantEntry& entry) const
{
    node->setIcon(0, entry.getIcon(ElementBase::eDisplay::DisplayName));
    node->setText(0, entry.getString(ElementBase::eDisplay::DisplayName));
    node->setIcon(1, entry.getIcon(ElementBase::eDisplay::DisplayType));
    node->setText(1, entry.getString(ElementBase::eDisplay::DisplayType));

    const QString reason = valueValidationReason(entry.getType(), entry.getValue());
    node->setIcon(2, reason.isEmpty() ? QIcon() : NELusanCommon::iconWarning(NELusanCommon::SizeSmall));
    node->setText(2, entry.getString(ElementBase::eDisplay::DisplayValue));
    node->setToolTip(2, reason);
}

void SMConstant::onCurCellChanged(QTreeWidgetItem* current, QTreeWidgetItem* /*previous*/)
{
    if (current == nullptr)
    {
        showClean();
        return;
    }

    const uint32_t id = current->data(0, Qt::ItemDataRole::UserRole).toUInt();
    ConstantEntry* entry = mModel.findConstant(id);
    if (entry == nullptr)
    {
        showClean();
        return;
    }

    selectedConstant(entry);
}

void SMConstant::selectedConstant(const ConstantEntry* entry)
{
    {
        const QSignalBlocker blockName(mDetails->ctrlName());
        const QSignalBlocker blockType(mDetails->ctrlTypes());
        const QSignalBlocker blockDescr(mDetails->ctrlDescription());
        mDetails->ctrlName()->setText(entry->getName());
        mDetails->ctrlTypes()->setCurrentText(entry->getType());
        mDetails->ctrlDescription()->setPlainText(entry->getDescription());
    }
    updateValueControl(entry);
    applyDeprecatedDisplay(mDetails->ctrlDeprecated(), mDetails->ctrlDeprecateHint(), entry);

    mDetails->ctrlName()->setEnabled(true);
    mDetails->ctrlTypes()->setEnabled(true);

    mList->ctrlButtonRemove()->setEnabled(true);
    updateMoveButtons(mModel.findIndex(entry->getId()), mModel.getConstantCount());
}

void SMConstant::showClean()
{
    applyDeprecatedDisplay(mDetails->ctrlDeprecated(), mDetails->ctrlDeprecateHint(), nullptr);
    updateValueControl(nullptr);

    const QSignalBlocker blockName(mDetails->ctrlName());
    const QSignalBlocker blockType(mDetails->ctrlTypes());
    const QSignalBlocker blockDescr(mDetails->ctrlDescription());
    mDetails->ctrlName()->clear();
    mDetails->ctrlTypes()->setCurrentIndex(-1);
    mDetails->ctrlDescription()->clear();

    mDetails->ctrlName()->setEnabled(false);
    mDetails->ctrlTypes()->setEnabled(false);

    mList->ctrlButtonRemove()->setEnabled(false);
    mList->ctrlButtonMoveUp()->setEnabled(false);
    mList->ctrlButtonMoveDown()->setEnabled(false);
}

void SMConstant::updateMoveButtons(int row, int rowCount)
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

uint32_t SMConstant::currentConstantId() const
{
    QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
    return (item != nullptr ? item->data(0, Qt::ItemDataRole::UserRole).toUInt() : 0u);
}

QString SMConstant::genName()
{
    static const QString _defName("NewConstant");
    QString name;
    do
    {
        name = _defName + QString::number(++mNameCounter);
    } while (mModel.findConstant(name) != nullptr);

    return name;
}

void SMConstant::onAddClicked()
{
    const QString name = genName();
    ConstantEntry* entry = mModel.createConstant(name);
    if (entry != nullptr)
    {
        selectConstant(entry->getId());
        mDetails->ctrlName()->setFocus();
        mDetails->ctrlName()->selectAll();
    }
}

void SMConstant::onInsertClicked()
{
    const uint32_t id = currentConstantId();
    const int position = (id != 0 ? mModel.findIndex(id) : 0);
    const QString name = genName();
    ConstantEntry* entry = mModel.insertConstant(position < 0 ? 0 : position, name);
    if (entry != nullptr)
    {
        selectConstant(entry->getId());
        mDetails->ctrlName()->setFocus();
        mDetails->ctrlName()->selectAll();
    }
}

void SMConstant::onRemoveClicked()
{
    const uint32_t id = currentConstantId();
    if (id == 0)
        return;

    // Deleting a constant still referenced by guards is refused with the where-used list
    // (v6 3.3); `Delete anyway` breaks the references VISIBLY (validation reports ERR).
    const QList<SMGuardWhereUsed::Use> uses = SMGuardWhereUsed::symbolUses(mModel.getFacade().getData(), id);
    if (uses.isEmpty() == false)
    {
        QStringList places;
        for (const SMGuardWhereUsed::Use& use : uses)
        {
            places.append(QStringLiteral("  - ") + use.location);
        }

        const QMessageBox::StandardButton choice = QMessageBox::warning(this, tr("Constant is used by guards")
                            , tr("This constant is used by %1 guard%2:\n%3\n\nDelete anyway? The affected guards break and are listed by validation.")
                              .arg(uses.size())
                              .arg((uses.size() == 1) ? QString() : QStringLiteral("s"))
                              .arg(places.join(QLatin1Char('\n')))
                            , QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
        if (choice != QMessageBox::Yes)
        {
            return;
        }
    }

    const QList<ConstantEntry>& list = mModel.getConstants();
    const int index = mModel.findIndex(id);
    uint32_t neighborId = 0;
    if (list.size() > 1)
    {
        const int neighborIndex = ((index + 1) < list.size()) ? (index + 1) : (index - 1);
        neighborId = list.at(neighborIndex).getId();
    }

    mModel.deleteConstant(id);
    if (neighborId != 0)
    {
        selectConstant(neighborId);
    }
}

void SMConstant::onMoveUpClicked()
{
    const uint32_t id = currentConstantId();
    if (id == 0)
        return;

    const int index = mModel.findIndex(id);
    if (index > 0)
    {
        // The swap keeps IDs attached to list position, so the moved entry's new ID is the
        // neighbor's old ID - reselect that to keep the moved row highlighted.
        const uint32_t neighborId = mModel.getConstants().at(index - 1).getId();
        mModel.swapConstants(id, neighborId);
        selectConstant(neighborId);
    }
}

void SMConstant::onMoveDownClicked()
{
    const uint32_t id = currentConstantId();
    if (id == 0)
        return;

    const int index = mModel.findIndex(id);
    if ((index >= 0) && (index < (mModel.getConstantCount() - 1)))
    {
        const uint32_t neighborId = mModel.getConstants().at(index + 1).getId();
        mModel.swapConstants(id, neighborId);
        selectConstant(neighborId);
    }
}

void SMConstant::onNameCommitted()
{
    const uint32_t id = currentConstantId();
    if (id != 0)
    {
        mModel.renameConstant(id, mDetails->ctrlName()->text());
    }
}

void SMConstant::onTypeChanged(int index)
{
    const uint32_t id = currentConstantId();
    if ((id == 0) || (index < 0))
        return;

    DataTypeBase* selected = mDetails->ctrlTypes()->itemData(index, Qt::ItemDataRole::UserRole).value<DataTypeBase*>();
    if (selected == nullptr)
        return;

    mModel.setType(id, selected->getName());
    updateValueControl(mModel.findConstant(id));
}

void SMConstant::onValueCommitted()
{
    const uint32_t id = currentConstantId();
    if (id == 0)
        return;

    mModel.setValue(id, mDetails->ctrlValue()->text());
    ConstantEntry* entry = mModel.findConstant(id);
    if (entry != nullptr)
    {
        updateValueValidation(entry->getType(), entry->getValue());
    }
}

void SMConstant::onValueTextChanged(const QString& text)
{
    ConstantEntry* entry = mModel.findConstant(currentConstantId());
    if (entry == nullptr)
        return;

    const QString reason = valueValidationReason(entry->getType(), text);
    mDetails->showValueHint(reason);

    QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
    if (item != nullptr)
    {
        item->setText(2, text);
        item->setIcon(2, reason.isEmpty() ? QIcon() : NELusanCommon::iconWarning(NELusanCommon::SizeSmall));
        item->setToolTip(2, reason);
    }
}

void SMConstant::onDeprecatedToggled(bool checked)
{
    const uint32_t id = currentConstantId();
    if (id == 0)
        return;

    mModel.setDeprecated(id, checked);
    ConstantEntry* entry = mModel.findConstant(id);
    const QSignalBlocker blockHint(mDetails->ctrlDeprecateHint());
    mDetails->ctrlDeprecateHint()->setEnabled(checked);
    mDetails->ctrlDeprecateHint()->setText((checked && (entry != nullptr)) ? entry->getDeprecateHint() : QString());
    if (checked)
    {
        mDetails->ctrlDeprecateHint()->setFocus();
    }
}

void SMConstant::onDeprecateHintCommitted()
{
    const uint32_t id = currentConstantId();
    if (id != 0)
    {
        mModel.setDeprecateHint(id, mDetails->ctrlDeprecateHint()->text());
    }
}

void SMConstant::refreshAll()
{
    QTreeWidget* table = mList->ctrlTableList();
    const uint32_t selId = currentConstantId();

    {
        const QSignalBlocker blocker(table);
        table->clear();
        for (const ConstantEntry& entry : mModel.getConstants())
        {
            QTreeWidgetItem* item = new QTreeWidgetItem();
            setNodeText(item, entry);
            item->setData(0, Qt::ItemDataRole::UserRole, entry.getId());
            table->addTopLevelItem(item);
        }
    }

    if ((selId == 0) || (selectConstant(selId) == false))
    {
        showClean();
    }
}

bool SMConstant::selectConstant(uint32_t id)
{
    QTreeWidget* table = mList->ctrlTableList();
    for (int i = 0; i < table->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem* item = table->topLevelItem(i);
        if (item->data(0, Qt::ItemDataRole::UserRole).toUInt() == id)
        {
            table->setCurrentItem(item);
            return true;
        }
    }

    return false;
}

void SMConstant::onNotifierChanged()
{
    refreshAll();
}

void SMConstant::onDataTypesChanged()
{
    populateTypeCombo();
    updateValueControl(mModel.findConstant(currentConstantId()));
}
