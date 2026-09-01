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
 *  \file        lusan/view/common/AttributePage.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the Attributes page shared by every document editor.
 *
 ************************************************************************/

#include "lusan/view/common/AttributePage.hpp"

#include "lusan/common/NELusanCommon.hpp"
#include "lusan/data/common/AttributeEntry.hpp"
#include "lusan/data/common/DataTypeBase.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/DataTypeEnum.hpp"
#include "lusan/data/common/DataTypeFactory.hpp"
#include "lusan/data/common/EnumEntry.hpp"
#include "lusan/model/common/AttributeModel.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/common/IEDocumentModel.hpp"
#include "lusan/model/common/DocRuleChecks.hpp"
#include "lusan/view/common/AttributeDetailsView.hpp"
#include "lusan/view/common/PendingEditWatcher.hpp"
#include "lusan/view/common/WidgetHighlight.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QShortcut>
#include <QSignalBlocker>
#include <QStringListModel>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

namespace
{
    //!< Refreshes a deprecated checkbox + hint pair from an entry's current state, without
    //!< re-triggering the edit signals that would otherwise push a spurious command.
    void applyDeprecatedDisplay(QCheckBox* checkBox, QLineEdit* hintEdit, const AttributeEntry* entry)
    {
        const QSignalBlocker blockCheck(checkBox);
        const QSignalBlocker blockHint(hintEdit);
        const bool deprecated = (entry != nullptr) && entry->getIsDeprecated();
        checkBox->setChecked(deprecated);
        hintEdit->setEnabled(deprecated);
        hintEdit->setText(deprecated ? entry->getDeprecateHint() : QString());
    }

    //!< The predefined types an attribute may be declared with. A container template is not
    //!< offered here: a container attribute is declared on the Data Types page first.
    const QList<DataTypeBase::eCategory>& attributeCategories(void)
    {
        static const QList<DataTypeBase::eCategory> _categories
        {
              DataTypeBase::eCategory::Primitive
            , DataTypeBase::eCategory::PrimitiveSint
            , DataTypeBase::eCategory::PrimitiveUint
            , DataTypeBase::eCategory::PrimitiveFloat
            , DataTypeBase::eCategory::BasicObject
        };

        return _categories;
    }

    //!< The two notification kinds, in the order the combo and the inline editor offer them.
    const QStringList& notificationNames(void)
    {
        static const QStringList _names
        {
              AttributeEntry::toString(AttributeEntry::eNotification::NotifyOnChange)
            , AttributeEntry::toString(AttributeEntry::eNotification::NotifyAlways)
        };

        return _names;
    }
}

AttributePage::AttributePage(AttributeModel& model, const QString& headline, QWidget* parent /*= nullptr*/)
    : QScrollArea       (parent)
    , IEDataTypeConsumer( )
    , IEditCommit       ( )
    , IETableHelper     ( )
    , mModel            (model)
    , mConfig           (model.getConfig())
    , mList             (new AttributeListView(model.getConfig(), this))
    , mDetails          (new AttributeDetailsView(model.getConfig(), this))
    , mTypeNames        (new QStringListModel(this))
    , mNotifyNames      (new QStringListModel(notificationNames(), this))
    , mTableCell        (nullptr)
    , mNameCounter      (0)
{
    buildUi(headline);
    setupSignals();
    refreshAll();
}

AttributeListView* AttributePage::getList(void) const
{
    return mList;
}

void AttributePage::buildUi(const QString& headline)
{
    QWidget* content = new QWidget(this);
    QVBoxLayout* root = new QVBoxLayout(content);

    QLabel* title = new QLabel(headline, content);
    QFont titleFont{ title->font() };
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    titleFont.setItalic(true);
    title->setFont(titleFont);
    root->addWidget(title);

    QHBoxLayout* columns = new QHBoxLayout();

    mList->setParent(content);
    mList->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    columns->addWidget(mList, 1);

    mDetails->setParent(content);
    mDetails->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    columns->addWidget(mDetails, 1);

    root->addLayout(columns, 1);

    // Every column is editable in the list as well as in the details panel, so a row can be filled
    // in without leaving the keyboard. The third column opens a picker where it holds a
    // notification kind, and a plain editor where it holds a value.
    QTreeWidget* table = mList->ctrlTableList();
    const int colExtra = static_cast<int>(AttributeListView::eColumn::ColExtra);
    QList<QAbstractItemModel*> pickers{ mTypeNames };
    QList<int> pickerColumns{ static_cast<int>(AttributeListView::eColumn::ColType) };
    if (hasValueColumn() == false)
    {
        pickers.append(mNotifyNames);
        pickerColumns.append(colExtra);
    }

    // The inline editors commit when the edit is done, not per keystroke: a commit rebuilds the
    // list, which would tear the open editor down after the first character typed.
    mTableCell = new TableCell(pickers, pickerColumns, table, this, true);
    mTableCell->setColumnValidation(static_cast<int>(AttributeListView::eColumn::ColName), TableCell::eCellValidation::Identifier);

    if (hasValueColumn())
    {
        // A structure or a container carries no literal, so its value cell stays closed, exactly
        // as the details Value field is disabled for those types.
        mTableCell->setEditableCheck([this, colExtra](const QModelIndex& index) -> bool
            {
                if (index.column() != colExtra)
                    return true;

                QTreeWidgetItem* item = mList->ctrlTableList()->topLevelItem(index.row());
                const AttributeEntry* entry = (item != nullptr)
                    ? mModel.findAttribute(item->data(static_cast<int>(AttributeListView::eColumn::ColName), Qt::ItemDataRole::UserRole).toUInt())
                    : nullptr;
                if (entry == nullptr)
                    return false;

                DataTypeCustom* custom = findCustomType(entry->getType());
                const bool hasNoLiteral = (custom != nullptr)
                    && ((custom->getCategory() == DataTypeBase::eCategory::Structure)
                     || (custom->getCategory() == DataTypeBase::eCategory::Container));
                return (hasNoLiteral == false);
            });
    }

    table->setItemDelegateForColumn(static_cast<int>(AttributeListView::eColumn::ColName), mTableCell);
    table->setItemDelegateForColumn(static_cast<int>(AttributeListView::eColumn::ColType), mTableCell);
    table->setItemDelegateForColumn(colExtra, mTableCell);

    if (hasValueColumn() == false)
    {
        // Both the details combo and the inline picker read the same canonical strings the entry
        // stores, so neither can offer a spelling the other refuses.
        QComboBox* notify = mDetails->ctrlNotification();
        const QSignalBlocker blocker(notify);
        notify->setModel(mNotifyNames);
    }

    populateTypes();

    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setWidgetResizable(true);
    setWidget(content);
}

void AttributePage::setupSignals(void)
{
    QTreeWidget* table = mList->ctrlTableList();

    connect(table                      , &QTreeWidget::currentItemChanged, this, &AttributePage::onCurCellChanged);
    connect(mList->ctrlButtonAdd()     , &QToolButton::clicked           , this, &AttributePage::onAddClicked);
    connect(mList->ctrlButtonInsert()  , &QToolButton::clicked           , this, &AttributePage::onInsertClicked);
    connect(mList->ctrlButtonRemove()  , &QToolButton::clicked           , this, &AttributePage::onRemoveClicked);
    connect(mList->ctrlButtonMoveUp()  , &QToolButton::clicked           , this, &AttributePage::onMoveUpClicked);
    connect(mList->ctrlButtonMoveDown(), &QToolButton::clicked           , this, &AttributePage::onMoveDownClicked);

    // List keys, each doing what the matching toolbar button does: Delete removes the selected
    // attribute, Insert adds one, F2 puts the caret in its name.
    QShortcut* scRemove = new QShortcut(QKeySequence(Qt::Key_Delete), table);
    QShortcut* scAdd    = new QShortcut(QKeySequence(Qt::Key_Insert), table);
    QShortcut* scRename = new QShortcut(QKeySequence(Qt::Key_F2), table);
    scRemove->setContext(Qt::WidgetWithChildrenShortcut);
    scAdd->setContext(Qt::WidgetWithChildrenShortcut);
    scRename->setContext(Qt::WidgetWithChildrenShortcut);
    connect(scRemove, &QShortcut::activated, this, &AttributePage::onRemoveClicked);
    connect(scAdd   , &QShortcut::activated, this, &AttributePage::onAddClicked);
    connect(scRename, &QShortcut::activated, this, &AttributePage::focusNameField);
    connect(mList, &ElementListView::signalRenameRequested, this, &AttributePage::focusNameField);

    // A commit from an inline editor rebuilds the list. Let the delegate close first.
    connect(mTableCell, &TableCell::signalEditorDataChanged, this, &AttributePage::onEditorDataChanged, Qt::QueuedConnection);
    connect(mTableCell, &TableCell::signalEditorTextChanged, this, &AttributePage::onEditorTextChanged);
    connect(mTableCell, &TableCell::signalEditorClosed     , this, &AttributePage::onEditorClosed);

    // The shared view already installs the C++ identifier validator on the Name field.
    connect(mDetails->ctrlName(), &QLineEdit::editingFinished, this, &AttributePage::onNameCommitted);
    connect(mDetails, &AttributeDetailsView::nameEdited, this, [this](const QString& text) {
        if (currentAttributeId() != 0)
        {
            if (QTreeWidgetItem* item = mList->ctrlTableList()->currentItem())
                item->setText(static_cast<int>(AttributeListView::eColumn::ColName), text);
        }
    });
    connect(mDetails->ctrlTypes(), &QComboBox::currentIndexChanged, this, &AttributePage::onTypeChanged);

    if (hasValueColumn())
    {
        connect(mDetails->ctrlValue(), &QLineEdit::editingFinished, this, &AttributePage::onValueCommitted);
        connect(mDetails->ctrlValue(), &QLineEdit::textChanged    , this, &AttributePage::onValueTextChanged);
    }
    else
    {
        connect(mDetails->ctrlNotification(), &QComboBox::currentIndexChanged, this, &AttributePage::onNotificationChanged);
    }

    connect(mDetails->ctrlDeprecated()   , &QCheckBox::toggled        , this, &AttributePage::onDeprecatedToggled);
    connect(mDetails->ctrlDeprecateHint(), &QLineEdit::editingFinished, this, &AttributePage::onDeprecateHintCommitted);
    mDetails->ctrlDescription()->installEventFilter(this);

    // The form carries document text. Typing in it marks the document changed at once, even though
    // the text itself is handed over when the field loses the focus.
    PendingEditWatcher::watchField(mDetails, mModel.getNotifier());

    DocModelNotifier& notifier = mModel.getNotifier();
    connect(&notifier, &DocModelNotifier::documentReloaded, this, &AttributePage::onNotifierChanged);
    connect(&notifier, &DocModelNotifier::elementAdded  , this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::Attribute) onNotifierChanged(); });
    connect(&notifier, &DocModelNotifier::elementRemoved, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::Attribute) onNotifierChanged(); });
    connect(&notifier, &DocModelNotifier::elementChanged, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::Attribute) onNotifierChanged(); });
    connect(&notifier, &DocModelNotifier::listReordered , this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::Attribute) onNotifierChanged(); });

    connect(&notifier, &DocModelNotifier::elementAdded  , this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) onDataTypesChanged(); });
    connect(&notifier, &DocModelNotifier::elementRemoved, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) onDataTypesChanged(); });
    connect(&notifier, &DocModelNotifier::elementChanged, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) onDataTypesChanged(); });
    connect(&notifier, &DocModelNotifier::listReordered , this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) onDataTypesChanged(); });
}

void AttributePage::commitPendingEdits(void)
{
    const uint32_t id = currentAttributeId();
    if (id != 0)
    {
        mModel.setDescription(id, mDetails->ctrlDescription()->toPlainText());
    }
}

bool AttributePage::eventFilter(QObject* watched, QEvent* event)
{
    if ((watched == mDetails->ctrlDescription()) && (event->type() == QEvent::FocusOut))
    {
        commitPendingEdits();
    }

    return QScrollArea::eventFilter(watched, event);
}

int AttributePage::getColumnCount(void) const
{
    return mList->ctrlTableList()->columnCount();
}

QString AttributePage::getCellText(const QModelIndex& cell) const
{
    QTreeWidgetItem* item = mList->ctrlTableList()->topLevelItem(cell.row());
    return (item != nullptr ? item->text(cell.column()) : QString());
}

DataTypeCustom* AttributePage::findCustomType(const QString& typeName) const
{
    for (DataTypeCustom* type : mModel.getDocument().getCustomDataTypes())
    {
        if ((type != nullptr) && type->hasTypeName(typeName))
            return type;
    }

    return nullptr;
}

void AttributePage::populateTypes(void)
{
    QComboBox* combo = mDetails->ctrlTypes();
    const QSignalBlocker blocker(combo);
    const QString current = combo->currentText();
    combo->clear();

    QStringList names;
    QList<DataTypeBase*> predefined;
    DataTypeFactory::getPredefinedTypes(predefined, attributeCategories());
    for (DataTypeBase* type : predefined)
    {
        combo->addItem(type->getName(), QVariant::fromValue(type));
        names.append(type->getName());
    }

    for (DataTypeCustom* type : mModel.getDocument().getCustomDataTypes())
    {
        combo->addItem(type->getQualifiedName(), QVariant::fromValue(static_cast<DataTypeBase*>(type)));
        names.append(type->getQualifiedName());
    }

    combo->setCurrentText(current);
    mTypeNames->setStringList(names);
}

void AttributePage::updateValueControl(const AttributeEntry* entry)
{
    if (hasValueColumn() == false)
        return;

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
    DataTypeCustom* custom = findCustomType(typeName);
    // Imported is deliberately excluded: the type is defined elsewhere and opaque to Lusan, so any
    // literal the user types is accepted as-is rather than rejected outright.
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

QString AttributePage::valueValidationReason(const QString& typeName, const QString& value) const
{
    // The same answer the validation engine gives, so the hint under the field and the finding
    // in the results panel can never disagree.
    return DocRuleChecks::literalReason(mModel.getDocument().getDataTypeSection(), typeName, value);
}

void AttributePage::updateValueValidation(const QString& typeName, const QString& value)
{
    if (hasValueColumn())
    {
        mDetails->showValueHint(valueValidationReason(typeName, value));
    }
}

void AttributePage::setNodeText(QTreeWidgetItem* node, const AttributeEntry& entry) const
{
    const int colName  = static_cast<int>(AttributeListView::eColumn::ColName);
    const int colType  = static_cast<int>(AttributeListView::eColumn::ColType);
    const int colExtra = static_cast<int>(AttributeListView::eColumn::ColExtra);

    node->setIcon(colName, entry.getIcon(ElementBase::eDisplay::DisplayName));
    node->setText(colName, entry.getString(ElementBase::eDisplay::DisplayName));
    node->setIcon(colType, entry.getIcon(ElementBase::eDisplay::DisplayType));
    node->setText(colType, entry.getString(ElementBase::eDisplay::DisplayType));

    const QString reason = hasValueColumn() ? valueValidationReason(entry.getType(), entry.getValue()) : QString();
    node->setIcon(colExtra, reason.isEmpty() ? QIcon() : NELusanCommon::iconWarning(NELusanCommon::SizeSmall));
    node->setText(colExtra, entry.getString(ElementBase::eDisplay::DisplayValue));
    node->setToolTip(colExtra, reason);
}

void AttributePage::onCurCellChanged(QTreeWidgetItem* current, QTreeWidgetItem* /*previous*/)
{
    if (current == nullptr)
    {
        showClean();
        return;
    }

    const uint32_t id = current->data(static_cast<int>(AttributeListView::eColumn::ColName), Qt::ItemDataRole::UserRole).toUInt();
    AttributeEntry* entry = mModel.findAttribute(id);
    if (entry == nullptr)
    {
        showClean();
        return;
    }

    selectedAttribute(entry);
}

void AttributePage::selectedAttribute(const AttributeEntry* entry)
{
    {
        const QSignalBlocker blockName(mDetails->ctrlName());
        const QSignalBlocker blockType(mDetails->ctrlTypes());
        const QSignalBlocker blockNotify(mDetails->ctrlNotification());
        const QSignalBlocker blockDescr(mDetails->ctrlDescription());
        mDetails->ctrlName()->setText(entry->getName());
        mDetails->ctrlTypes()->setCurrentText(entry->getType());
        mDetails->ctrlNotification()->setCurrentText(AttributeEntry::toString(entry->getNotification()));
        mDetails->ctrlDescription()->setPlainText(entry->getDescription());
    }

    updateValueControl(entry);
    applyDeprecatedDisplay(mDetails->ctrlDeprecated(), mDetails->ctrlDeprecateHint(), entry);

    mDetails->ctrlName()->setEnabled(true);
    mDetails->ctrlTypes()->setEnabled(true);
    mDetails->ctrlNotification()->setEnabled(true);

    mList->ctrlButtonRemove()->setEnabled(true);
    updateMoveButtons(mModel.findIndex(entry->getId()), mModel.getAttributeCount());
}

void AttributePage::showClean(void)
{
    applyDeprecatedDisplay(mDetails->ctrlDeprecated(), mDetails->ctrlDeprecateHint(), nullptr);
    updateValueControl(nullptr);

    const QSignalBlocker blockName(mDetails->ctrlName());
    const QSignalBlocker blockType(mDetails->ctrlTypes());
    const QSignalBlocker blockNotify(mDetails->ctrlNotification());
    const QSignalBlocker blockDescr(mDetails->ctrlDescription());
    mDetails->ctrlName()->clear();
    mDetails->ctrlTypes()->setCurrentIndex(-1);
    mDetails->ctrlNotification()->setCurrentIndex(0);
    mDetails->ctrlDescription()->clear();

    mDetails->ctrlName()->setEnabled(false);
    mDetails->ctrlTypes()->setEnabled(false);
    mDetails->ctrlNotification()->setEnabled(false);

    mList->ctrlButtonRemove()->setEnabled(false);
    mList->ctrlButtonMoveUp()->setEnabled(false);
    mList->ctrlButtonMoveDown()->setEnabled(false);
}

void AttributePage::updateMoveButtons(int row, int rowCount)
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

AttributeEntry* AttributePage::currentAttribute(void) const
{
    return mModel.findAttribute(currentAttributeId());
}

uint32_t AttributePage::currentAttributeId(void) const
{
    QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
    return (item != nullptr ? item->data(static_cast<int>(AttributeListView::eColumn::ColName), Qt::ItemDataRole::UserRole).toUInt() : 0u);
}

void AttributePage::revealElement(uint32_t id, eIssueField field /*= eIssueField::None*/)
{
    if (selectAttribute(id) == false)
    {
        return;
    }

    switch (field)
    {
    case eIssueField::Name:         WidgetHighlight::reveal(mDetails->ctrlName());        break;
    case eIssueField::Type:         WidgetHighlight::reveal(mDetails->ctrlTypes());       break;
    case eIssueField::Value:        WidgetHighlight::reveal(hasValueColumn() ? static_cast<QWidget*>(mDetails->ctrlValue()) : static_cast<QWidget*>(mDetails->ctrlNotification())); break;
    case eIssueField::Description:  WidgetHighlight::reveal(mDetails->ctrlDescription()); break;
    default:                                                                              break;
    }
}

void AttributePage::dataTypesChanged(void)
{
    populateTypes();
    // Every attribute keeps a resolved pointer to its declared type beside the type name, and a
    // type that is gone, renamed or converted leaves that pointer wrong.
    mModel.resolveDeclaredTypes();
    refreshAll();
}

void AttributePage::replaceDataType(DataTypeBase* oldType, DataTypeBase* newType)
{
    mModel.replaceDataType(oldType, newType);
    dataTypesChanged();
}

QString AttributePage::genName(void)
{
    static const QString _defName("NewAttribute");
    QString name;
    do
    {
        name = _defName + QString::number(++mNameCounter);
    } while (mModel.findAttribute(name) != nullptr);

    return name;
}

void AttributePage::onAddClicked(void)
{
    AttributeEntry* entry = mModel.createAttribute(genName());
    if (entry != nullptr)
    {
        selectAttribute(entry->getId());
        focusNameField();
    }
}

void AttributePage::onInsertClicked(void)
{
    const uint32_t id = currentAttributeId();
    const int position = (id != 0 ? mModel.findIndex(id) : 0);
    AttributeEntry* entry = mModel.insertAttribute(position < 0 ? 0 : position, genName());
    if (entry != nullptr)
    {
        selectAttribute(entry->getId());
        focusNameField();
    }
}

void AttributePage::focusNameField(void)
{
    if (currentAttributeId() != 0)
    {
        mDetails->ctrlName()->setFocus();
        mDetails->ctrlName()->selectAll();
    }
}

bool AttributePage::confirmRemove(uint32_t /*id*/)
{
    return true;
}

void AttributePage::onRemoveClicked(void)
{
    const uint32_t id = currentAttributeId();
    if ((id == 0) || (confirmRemove(id) == false))
        return;

    const QList<AttributeEntry>& list = mModel.getAttributes();
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

void AttributePage::onMoveUpClicked(void)
{
    const uint32_t moved = mModel.moveAttribute(currentAttributeId(), -1);
    if (moved != 0)
    {
        selectAttribute(moved);
    }
}

void AttributePage::onMoveDownClicked(void)
{
    const uint32_t moved = mModel.moveAttribute(currentAttributeId(), +1);
    if (moved != 0)
    {
        selectAttribute(moved);
    }
}

void AttributePage::onNameCommitted(void)
{
    const uint32_t id = currentAttributeId();
    if (id != 0)
    {
        mModel.renameAttribute(id, mDetails->ctrlName()->text());
    }
}

void AttributePage::onTypeChanged(int index)
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

void AttributePage::onValueCommitted(void)
{
    const uint32_t id = currentAttributeId();
    if (id == 0)
        return;

    mModel.setValue(id, mDetails->ctrlValue()->text());
    AttributeEntry* entry = mModel.findAttribute(id);
    if (entry != nullptr)
    {
        updateValueValidation(entry->getType(), entry->getValue());
    }
}

void AttributePage::onNotificationChanged(int index)
{
    const uint32_t id = currentAttributeId();
    if ((id == 0) || (index < 0))
        return;

    mModel.setNotification(id, AttributeEntry::fromString(mDetails->ctrlNotification()->itemText(index)));
}

void AttributePage::onValueTextChanged(const QString& text)
{
    AttributeEntry* entry = mModel.findAttribute(currentAttributeId());
    if (entry == nullptr)
        return;

    const QString reason = valueValidationReason(entry->getType(), text);
    mDetails->showValueHint(reason);

    QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
    if (item != nullptr)
    {
        const int colExtra = static_cast<int>(AttributeListView::eColumn::ColExtra);
        item->setText(colExtra, text);
        item->setIcon(colExtra, reason.isEmpty() ? QIcon() : NELusanCommon::iconWarning(NELusanCommon::SizeSmall));
        item->setToolTip(colExtra, reason);
    }
}

void AttributePage::onDeprecatedToggled(bool checked)
{
    const uint32_t id = currentAttributeId();
    if (id == 0)
        return;

    mModel.setDeprecated(id, checked);
    AttributeEntry* entry = mModel.findAttribute(id);
    const QSignalBlocker blockHint(mDetails->ctrlDeprecateHint());
    mDetails->ctrlDeprecateHint()->setEnabled(checked);
    mDetails->ctrlDeprecateHint()->setText((checked && (entry != nullptr)) ? entry->getDeprecateHint() : QString());
    if (checked)
    {
        mDetails->ctrlDeprecateHint()->setFocus();
    }
}

void AttributePage::onDeprecateHintCommitted(void)
{
    const uint32_t id = currentAttributeId();
    if (id != 0)
    {
        mModel.setDeprecateHint(id, mDetails->ctrlDeprecateHint()->text());
    }
}

void AttributePage::onEditorDataChanged(const QModelIndex& index, const QString& newValue)
{
    QTreeWidget* table = mList->ctrlTableList();
    if ((index.row() < 0) || (index.row() >= table->topLevelItemCount()))
        return;

    QTreeWidgetItem* item = table->topLevelItem(index.row());
    const uint32_t id = item->data(static_cast<int>(AttributeListView::eColumn::ColName), Qt::ItemDataRole::UserRole).toUInt();
    if (mModel.findAttribute(id) == nullptr)
        return;

    switch (index.column())
    {
    case static_cast<int>(AttributeListView::eColumn::ColName):
        mModel.renameAttribute(id, newValue);
        break;

    case static_cast<int>(AttributeListView::eColumn::ColType):
        mModel.setType(id, newValue);
        break;

    case static_cast<int>(AttributeListView::eColumn::ColExtra):
        if (hasValueColumn())
        {
            mModel.setValue(id, newValue);
        }
        else
        {
            mModel.setNotification(id, AttributeEntry::fromString(newValue));
        }
        break;

    default:
        break;
    }

    // The commit above rebuilt the list; put the details panel back on the edited row.
    selectAttribute(id);
}

void AttributePage::onEditorTextChanged(const QModelIndex& index, const QString& newText)
{
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* item = table->topLevelItem(index.row());
    if ((item == nullptr) || (item != table->currentItem()))
        return;

    if (index.column() == static_cast<int>(AttributeListView::eColumn::ColName))
    {
        // Blocked: the field re-emits its text as nameEdited, which writes it back into the row.
        const QSignalBlocker blockName(mDetails->ctrlName());
        mDetails->ctrlName()->setText(newText);
    }
    else if (hasValueColumn() && (index.column() == static_cast<int>(AttributeListView::eColumn::ColExtra)))
    {
        const AttributeEntry* entry = mModel.findAttribute(currentAttributeId());
        if (entry == nullptr)
            return;

        const QSignalBlocker blockValue(mDetails->ctrlValue());
        mDetails->ctrlValue()->setText(newText);
        updateValueValidation(entry->getType(), newText);
    }
}

void AttributePage::onEditorClosed(void)
{
    onCurCellChanged(mList->ctrlTableList()->currentItem(), nullptr);
}

void AttributePage::refreshAll(void)
{
    QTreeWidget* table = mList->ctrlTableList();
    const uint32_t selId = currentAttributeId();

    {
        const QSignalBlocker blocker(table);
        table->clear();
        for (const AttributeEntry& entry : mModel.getAttributes())
        {
            QTreeWidgetItem* item = new QTreeWidgetItem();
            // Editable lets the delegate open an inline editor on double-click.
            item->setFlags(item->flags() | Qt::ItemIsEditable);
            setNodeText(item, entry);
            item->setData(static_cast<int>(AttributeListView::eColumn::ColName), Qt::ItemDataRole::UserRole, entry.getId());
            table->addTopLevelItem(item);
        }
    }

    if ((selId == 0) || (selectAttribute(selId) == false))
    {
        showClean();
    }
}

bool AttributePage::selectAttribute(uint32_t id)
{
    QTreeWidget* table = mList->ctrlTableList();
    for (int i = 0; i < table->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem* item = table->topLevelItem(i);
        if (item->data(static_cast<int>(AttributeListView::eColumn::ColName), Qt::ItemDataRole::UserRole).toUInt() == id)
        {
            table->setCurrentItem(item);
            table->scrollToItem(item);
            return true;
        }
    }

    return false;
}

void AttributePage::onNotifierChanged(void)
{
    refreshAll();
}

void AttributePage::onDataTypesChanged(void)
{
    dataTypesChanged();
}
