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
 *  \file        lusan/view/sm/SMAttribute.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Attributes page.
 *
 ************************************************************************/

#include "lusan/view/sm/SMAttribute.hpp"

#include "lusan/common/NELusanCommon.hpp"
#include "lusan/data/common/DataTypeBase.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/DataTypeEnum.hpp"
#include "lusan/data/common/DataTypeFactory.hpp"
#include "lusan/data/common/EnumEntry.hpp"
#include "lusan/data/sm/SMAttributeData.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/sm/SMAttributeModel.hpp"
#include "lusan/model/sm/SMDataTypeModel.hpp"
#include "lusan/model/sm/SMGuardWhereUsed.hpp"
#include "lusan/model/sm/SMLiteralValidator.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/sm/SMAttributeDetails.hpp"
#include "lusan/view/sm/SMAttributeList.hpp"

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
    void applyDeprecatedDisplay(QCheckBox* checkBox, QLineEdit* hintEdit, const SMAttributeEntry* entry)
    {
        const QSignalBlocker blockCheck(checkBox);
        const QSignalBlocker blockHint(hintEdit);
        const bool deprecated = (entry != nullptr) && entry->getIsDeprecated();
        checkBox->setChecked(deprecated);
        hintEdit->setEnabled(deprecated);
        hintEdit->setText(deprecated ? entry->getDeprecateHint() : QString());
    }
}

SMAttribute::SMAttribute(SMAttributeModel& model, QWidget* parent /*= nullptr*/)
    : QScrollArea       (parent)
    , mModel            (model)
    , mList             (new SMAttributeList(this))
    , mDetails          (new SMAttributeDetails(this))
    , mNameCounter      (0)
{
    buildUi();
    setupSignals();
    refreshAll();
}

SMAttributeList* SMAttribute::getList() const
{
    return mList;
}

void SMAttribute::buildUi()
{
    QWidget* content = new QWidget(this);
    QVBoxLayout* root = new QVBoxLayout(content);

    QLabel* headline = new QLabel(tr("State Machine Attribute Editor ..."), content);
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

void SMAttribute::setupSignals()
{
    connect(mList->ctrlTableList()     , &QTreeWidget::currentItemChanged, this, &SMAttribute::onCurCellChanged);
    connect(mList->ctrlButtonAdd()     , &QToolButton::clicked           , this, &SMAttribute::onAddClicked);
    connect(mList->ctrlButtonInsert()  , &QToolButton::clicked           , this, &SMAttribute::onInsertClicked);
    connect(mList->ctrlButtonRemove()  , &QToolButton::clicked           , this, &SMAttribute::onRemoveClicked);
    connect(mList->ctrlButtonMoveUp()  , &QToolButton::clicked           , this, &SMAttribute::onMoveUpClicked);
    connect(mList->ctrlButtonMoveDown(), &QToolButton::clicked           , this, &SMAttribute::onMoveDownClicked);

    mDetails->ctrlName()->setValidator(NELusanCommon::createIdentifierValidator(mDetails->ctrlName()));
    connect(mDetails->ctrlName()   , &QLineEdit::editingFinished    , this, &SMAttribute::onNameCommitted);
    // Live-preview the typed name into the selected attribute's Name column; the rename commits
    // on editingFinished. Selection sets the field under a QSignalBlocker, so this only fires
    // for genuine user edits.
    connect(mDetails->ctrlName()   , &QLineEdit::textChanged        , this, [this](const QString& text) {
        if (currentAttributeId() != 0)
        {
            if (QTreeWidgetItem* item = mList->ctrlTableList()->currentItem())
                item->setText(0, text);
        }
    });
    connect(mDetails->ctrlTypes()  , &QComboBox::currentIndexChanged, this, &SMAttribute::onTypeChanged);
    connect(mDetails->ctrlValue()  , &QLineEdit::editingFinished    , this, &SMAttribute::onValueCommitted);
    connect(mDetails->ctrlValue()  , &QLineEdit::textChanged        , this, &SMAttribute::onValueTextChanged);
    connect(mDetails->ctrlDeprecated()   , &QCheckBox::toggled        , this, &SMAttribute::onDeprecatedToggled);
    connect(mDetails->ctrlDeprecateHint(), &QLineEdit::editingFinished, this, &SMAttribute::onDeprecateHintCommitted);
    mDetails->ctrlDescription()->installEventFilter(this);

    connect(&mModel.getNotifier(), &DocModelNotifier::documentReloaded, this, &SMAttribute::onNotifierChanged);
    connect(&mModel.getNotifier(), &DocModelNotifier::elementAdded, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::Attribute) onNotifierChanged(); });
    connect(&mModel.getNotifier(), &DocModelNotifier::elementRemoved, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::Attribute) onNotifierChanged(); });
    connect(&mModel.getNotifier(), &DocModelNotifier::elementChanged, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::Attribute) onNotifierChanged(); });
    connect(&mModel.getNotifier(), &DocModelNotifier::listReordered, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::Attribute) onNotifierChanged(); });

    connect(&mModel.getNotifier(), &DocModelNotifier::elementAdded, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) onDataTypesChanged(); });
    connect(&mModel.getNotifier(), &DocModelNotifier::elementRemoved, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) onDataTypesChanged(); });
    connect(&mModel.getNotifier(), &DocModelNotifier::elementChanged, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) onDataTypesChanged(); });
    connect(&mModel.getNotifier(), &DocModelNotifier::listReordered, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) onDataTypesChanged(); });
}

bool SMAttribute::eventFilter(QObject* watched, QEvent* event)
{
    if ((watched == mDetails->ctrlDescription()) && (event->type() == QEvent::FocusOut))
    {
        const uint32_t id = currentAttributeId();
        if (id != 0)
        {
            mModel.setDescription(id, mDetails->ctrlDescription()->toPlainText());
        }
    }

    return QScrollArea::eventFilter(watched, event);
}

void SMAttribute::populateTypeCombo()
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

void SMAttribute::updateValueControl(const SMAttributeEntry* entry)
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
        value->setToolTip(tr("'%1' has no literal value; this attribute has no default.").arg(typeName));
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

QString SMAttribute::valueValidationReason(const QString& typeName, const QString& value) const
{
    // No default is always valid - a missing value simply means "not set", not "invalid".
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

void SMAttribute::updateValueValidation(const QString& typeName, const QString& value)
{
    mDetails->showValueHint(valueValidationReason(typeName, value));
}

void SMAttribute::setNodeText(QTreeWidgetItem* node, const SMAttributeEntry& entry) const
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

void SMAttribute::onCurCellChanged(QTreeWidgetItem* current, QTreeWidgetItem* /*previous*/)
{
    if (current == nullptr)
    {
        showClean();
        return;
    }

    const uint32_t id = current->data(0, Qt::ItemDataRole::UserRole).toUInt();
    SMAttributeEntry* entry = mModel.findAttribute(id);
    if (entry == nullptr)
    {
        showClean();
        return;
    }

    selectedAttribute(entry);
}

void SMAttribute::selectedAttribute(const SMAttributeEntry* entry)
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
    updateMoveButtons(mModel.findIndex(entry->getId()), mModel.getAttributeCount());
}

void SMAttribute::showClean()
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

void SMAttribute::updateMoveButtons(int row, int rowCount)
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

uint32_t SMAttribute::currentAttributeId() const
{
    QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
    return (item != nullptr ? item->data(0, Qt::ItemDataRole::UserRole).toUInt() : 0u);
}

QString SMAttribute::genName()
{
    static const QString _defName("NewAttribute");
    QString name;
    do
    {
        name = _defName + QString::number(++mNameCounter);
    } while (mModel.findAttribute(name) != nullptr);

    return name;
}

void SMAttribute::onAddClicked()
{
    const QString name = genName();
    SMAttributeEntry* entry = mModel.createAttribute(name);
    if (entry != nullptr)
    {
        selectAttribute(entry->getId());
        mDetails->ctrlName()->setFocus();
        mDetails->ctrlName()->selectAll();
    }
}

void SMAttribute::onInsertClicked()
{
    const uint32_t id = currentAttributeId();
    const int position = (id != 0 ? mModel.findIndex(id) : 0);
    const QString name = genName();
    SMAttributeEntry* entry = mModel.insertAttribute(position < 0 ? 0 : position, name);
    if (entry != nullptr)
    {
        selectAttribute(entry->getId());
        mDetails->ctrlName()->setFocus();
        mDetails->ctrlName()->selectAll();
    }
}

void SMAttribute::onRemoveClicked()
{
    const uint32_t id = currentAttributeId();
    if (id == 0)
        return;

    // Deleting an attribute still referenced by guards is refused with the where-used list
    // (v6 3.3); `Delete anyway` breaks the references VISIBLY (validation reports ERR).
    const QList<SMGuardWhereUsed::Use> uses = SMGuardWhereUsed::symbolUses(mModel.getFacade().getData(), id);
    if (uses.isEmpty() == false)
    {
        QStringList places;
        for (const SMGuardWhereUsed::Use& use : uses)
        {
            places.append(QStringLiteral("  - ") + use.location);
        }

        const QMessageBox::StandardButton choice = QMessageBox::warning(this, tr("Attribute is used by guards")
                            , tr("This attribute is used by %1 guard%2:\n%3\n\nDelete anyway? The affected guards break and are listed by validation.")
                              .arg(uses.size())
                              .arg((uses.size() == 1) ? QString() : QStringLiteral("s"))
                              .arg(places.join(QLatin1Char('\n')))
                            , QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
        if (choice != QMessageBox::Yes)
        {
            return;
        }
    }

    const QList<SMAttributeEntry>& list = mModel.getAttributes();
    const int index = mModel.findIndex(id);
    uint32_t neighborId = 0;
    if (list.size() > 1)
    {
        const int neighborIndex = ((index + 1) < list.size()) ? (index + 1) : (index - 1);
        neighborId = list.at(neighborIndex).getId();
    }

    mModel.deleteAttribute(id);
    if (neighborId != 0)
    {
        selectAttribute(neighborId);
    }
}

void SMAttribute::onMoveUpClicked()
{
    const uint32_t id = currentAttributeId();
    if (id == 0)
        return;

    const int index = mModel.findIndex(id);
    if (index > 0)
    {
        // The swap keeps IDs attached to list position, so the moved entry's new ID is the
        // neighbor's old ID - reselect that to keep the moved row highlighted.
        const uint32_t neighborId = mModel.getAttributes().at(index - 1).getId();
        mModel.swapAttributes(id, neighborId);
        selectAttribute(neighborId);
    }
}

void SMAttribute::onMoveDownClicked()
{
    const uint32_t id = currentAttributeId();
    if (id == 0)
        return;

    const int index = mModel.findIndex(id);
    if ((index >= 0) && (index < (mModel.getAttributeCount() - 1)))
    {
        const uint32_t neighborId = mModel.getAttributes().at(index + 1).getId();
        mModel.swapAttributes(id, neighborId);
        selectAttribute(neighborId);
    }
}

void SMAttribute::onNameCommitted()
{
    const uint32_t id = currentAttributeId();
    if (id != 0)
    {
        mModel.renameAttribute(id, mDetails->ctrlName()->text());
    }
}

void SMAttribute::onTypeChanged(int index)
{
    const uint32_t id = currentAttributeId();
    if ((id == 0) || (index < 0))
        return;

    DataTypeBase* selected = mDetails->ctrlTypes()->itemData(index, Qt::ItemDataRole::UserRole).value<DataTypeBase*>();
    if (selected == nullptr)
        return;

    mModel.setType(id, selected->getName());
    updateValueControl(mModel.findAttribute(id));
}

void SMAttribute::onValueCommitted()
{
    const uint32_t id = currentAttributeId();
    if (id == 0)
        return;

    mModel.setValue(id, mDetails->ctrlValue()->text());
    SMAttributeEntry* entry = mModel.findAttribute(id);
    if (entry != nullptr)
    {
        updateValueValidation(entry->getType(), entry->getValue());
    }
}

void SMAttribute::onValueTextChanged(const QString& text)
{
    SMAttributeEntry* entry = mModel.findAttribute(currentAttributeId());
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

void SMAttribute::onDeprecatedToggled(bool checked)
{
    const uint32_t id = currentAttributeId();
    if (id == 0)
        return;

    mModel.setDeprecated(id, checked);
    SMAttributeEntry* entry = mModel.findAttribute(id);
    const QSignalBlocker blockHint(mDetails->ctrlDeprecateHint());
    mDetails->ctrlDeprecateHint()->setEnabled(checked);
    mDetails->ctrlDeprecateHint()->setText((checked && (entry != nullptr)) ? entry->getDeprecateHint() : QString());
    if (checked)
    {
        mDetails->ctrlDeprecateHint()->setFocus();
    }
}

void SMAttribute::onDeprecateHintCommitted()
{
    const uint32_t id = currentAttributeId();
    if (id != 0)
    {
        mModel.setDeprecateHint(id, mDetails->ctrlDeprecateHint()->text());
    }
}

void SMAttribute::refreshAll()
{
    QTreeWidget* table = mList->ctrlTableList();
    const uint32_t selId = currentAttributeId();

    {
        const QSignalBlocker blocker(table);
        table->clear();
        for (const SMAttributeEntry& entry : mModel.getAttributes())
        {
            QTreeWidgetItem* item = new QTreeWidgetItem();
            setNodeText(item, entry);
            item->setData(0, Qt::ItemDataRole::UserRole, entry.getId());
            table->addTopLevelItem(item);
        }
    }

    if ((selId == 0) || (selectAttribute(selId) == false))
    {
        showClean();
    }
}

bool SMAttribute::selectAttribute(uint32_t id)
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

void SMAttribute::onNotifierChanged()
{
    refreshAll();
}

void SMAttribute::onDataTypesChanged()
{
    populateTypeCombo();
    updateValueControl(mModel.findAttribute(currentAttributeId()));
}
