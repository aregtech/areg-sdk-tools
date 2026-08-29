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
 *  \file        lusan/view/common/MethodPage.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the Methods page shared by every document editor.
 *
 ************************************************************************/

#include "lusan/view/common/MethodPage.hpp"

#include "lusan/common/NELusanCommon.hpp"
#include "lusan/data/common/DataTypeBase.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/DataTypeFactory.hpp"
#include "lusan/data/common/DocumentElem.hpp"
#include "lusan/data/common/MethodParameter.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/common/MethodModel.hpp"
#include "lusan/view/common/MethodDetailsView.hpp"
#include "lusan/view/common/MethodListView.hpp"
#include "lusan/view/common/MethodParamDetailsView.hpp"
#include "lusan/view/common/PendingEditWatcher.hpp"
#include "lusan/view/common/WidgetHighlight.hpp"
#include "lusan/view/sm/SMCodeEditor.hpp"

#include <QAbstractItemModel>
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
#include <QStandardItemModel>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

namespace
{
    //!< Refreshes a deprecated check-box and hint pair from a flag and a hint, without
    //!< re-triggering the edit signals that would otherwise push a spurious command.
    void applyDeprecatedDisplay(QCheckBox* checkBox, QLineEdit* hintEdit, bool deprecated, const QString& hint)
    {
        const QSignalBlocker blockCheck(checkBox);
        const QSignalBlocker blockHint(hintEdit);
        checkBox->setChecked(deprecated);
        hintEdit->setEnabled(deprecated);
        hintEdit->setText(deprecated ? hint : QString());
    }

    //!< The predefined types a parameter or a return value may be declared with.
    const QList<DataTypeBase::eCategory>& declarableCategories(void)
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

    MethodViewConfig detailsConfigOf(const MethodConfig& config, bool hasGuardInfo)
    {
        MethodViewConfig result{ QList<MethodTypeOption>{}, false, false, false, hasGuardInfo, false };
        for (const MethodKind& kind : config.kinds)
        {
            result.types.append(MethodTypeOption{ kind.label, kind.icon });
            result.hasReply     = result.hasReply     || kind.hasReply;
            result.hasReturn    = result.hasReturn    || kind.hasReturn;
            result.hasImplement = result.hasImplement || kind.hasImplement;
            result.hasBody      = result.hasBody      || kind.hasImplement;
        }

        return result;
    }

    MethodListConfig listConfigOf(const MethodConfig& config, const MethodPageConfig& pageConfig)
    {
        MethodListConfig result{ pageConfig.listTitle, QStringList{}, false };
        for (const MethodKind& kind : config.kinds)
        {
            result.typeMenuLabels.append(kind.label);
            result.hasReplyColumn = result.hasReplyColumn || kind.hasReply;
        }

        return result;
    }

    //!< The kinds a method may be declared as, as a list model the type cell's drop-down uses.
    //!< The rows are the kinds in configuration order, so a row index is a kind index.
    QStandardItemModel* buildKindModel(const MethodConfig& config, QObject* owner)
    {
        QStandardItemModel* model = new QStandardItemModel(owner);
        for (const MethodKind& kind : config.kinds)
        {
            QStandardItem* item = new QStandardItem(kind.label);
            if (kind.icon.isEmpty() == false)
            {
                item->setIcon(QIcon(kind.icon));
            }

            model->appendRow(item);
        }

        return model;
    }
}

//////////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////////

MethodPage::MethodPage(MethodModel& model, const MethodPageConfig& config, QWidget* parent /*= nullptr*/)
    : QScrollArea           (parent)
    , IEDataTypeConsumer    ( )
    , IEditCommit           ( )
    , IETableHelper         ( )
    , mModel                (model)
    , mPageConfig           (config)
    , mList                 (new MethodListView(listConfigOf(model.getConfig(), config), this))
    , mDetails              (new MethodDetailsView(detailsConfigOf(model.getConfig(), config.hasGuardInfo), this))
    , mParamDetails         (new MethodParamDetailsView(tr("Details:"), this))
    , mTableCell            (nullptr)
    , mKindModel            (buildKindModel(model.getConfig(), this))
    , mMethodNameCounter    (0)
{
    buildUi();
    setupSignals();
    refreshAll();
}

MethodListView* MethodPage::getList(void) const
{
    return mList;
}

void MethodPage::buildUi(void)
{
    QWidget* content = new QWidget(this);
    QVBoxLayout* root = new QVBoxLayout(content);

    QLabel* headline = new QLabel(mPageConfig.headline, content);
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
    populateReplyCombo();

    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setWidgetResizable(true);
    setWidget(content);
}

void MethodPage::setupSignals(void)
{
    QTreeWidget* table = mList->ctrlTableList();
    connect(table                       , &QTreeWidget::currentItemChanged, this, &MethodPage::onCurCellChanged);
    connect(mList->ctrlButtonAdd()      , &QToolButton::clicked           , this, &MethodPage::onAddClicked);
    connect(mList->ctrlButtonInsert()   , &QToolButton::clicked           , this, &MethodPage::onInsertClicked);
    connect(mList->ctrlButtonRemove()   , &QToolButton::clicked           , this, &MethodPage::onRemoveClicked);
    connect(mList->ctrlButtonMoveUp()   , &QToolButton::clicked           , this, &MethodPage::onMoveUpClicked);
    connect(mList->ctrlButtonMoveDown() , &QToolButton::clicked           , this, &MethodPage::onMoveDownClicked);
    connect(mList->ctrlButtonParamAdd() , &QToolButton::clicked           , this, [this]() { addNewParam(); });

    const int kindCount = mModel.getConfig().kinds.size();
    for (int i = 0; i < kindCount; ++i)
    {
        if (QAction* action = mList->typeAction(i))
        {
            connect(action, &QAction::triggered, this, [this, i]() { addNewMethod(i); });
        }

        if (QRadioButton* radio = mDetails->ctrlType(i))
        {
            connect(radio, &QRadioButton::toggled, this, &MethodPage::onMethodKindToggled);
        }
    }

    // List keys: Delete removes the selected row, Insert adds one at the selected level, F2
    // puts the caret in its name.
    QShortcut* scAdd    = new QShortcut(QKeySequence(Qt::Key_Insert), table);
    QShortcut* scRemove = new QShortcut(QKeySequence(Qt::Key_Delete), table);
    QShortcut* scRename = new QShortcut(QKeySequence(Qt::Key_F2), table);
    scAdd->setContext(Qt::WidgetWithChildrenShortcut);
    scRemove->setContext(Qt::WidgetWithChildrenShortcut);
    scRename->setContext(Qt::WidgetWithChildrenShortcut);
    connect(scAdd, &QShortcut::activated, this, [this]()
    {
        if (currentKind() == eRowKind::Param)
        {
            addNewParam();
        }
        else
        {
            onAddClicked();
        }
    });
    connect(scRemove, &QShortcut::activated, this, &MethodPage::onRemoveClicked);
    connect(scRename, &QShortcut::activated, this, &MethodPage::focusNameField);

    // The shared MethodDetailsView installs the C++ identifier validator on the Name field.
    connect(mDetails                     , &MethodDetailsView::nameEdited, this, &MethodPage::onMethodNameTextChanged);
    connect(mDetails->ctrlName()         , &QLineEdit::editingFinished   , this, &MethodPage::onMethodNameCommitted);
    connect(mDetails->ctrlReply()        , &QComboBox::activated         , this, &MethodPage::onReplyChanged);
    connect(mDetails->ctrlReturn()->lineEdit(), &QLineEdit::editingFinished, this, &MethodPage::onReturnCommitted);
    connect(mDetails->ctrlReturn()       , &QComboBox::activated         , this, [this](int) { onReturnCommitted(); });
    connect(mDetails->ctrlHandler()      , &QRadioButton::toggled        , this, &MethodPage::onImplementToggled);
    connect(mDetails->ctrlEmbedded()     , &QRadioButton::toggled        , this, &MethodPage::onImplementToggled);
    connect(mDetails->ctrlDeprecated()   , &QCheckBox::toggled           , this, &MethodPage::onMethodDeprecatedToggled);
    connect(mDetails->ctrlDeprecateHint(), &QLineEdit::editingFinished   , this, &MethodPage::onMethodDeprecateHintCommitted);
    mDetails->ctrlBody()->ctrlBody()->installEventFilter(this);
    mDetails->ctrlDescription()->installEventFilter(this);

    // The shared MethodParamDetailsView installs the same validator on its Name field.
    connect(mParamDetails                     , &MethodParamDetailsView::nameEdited, this, &MethodPage::onParamNameTextChanged);
    connect(mParamDetails->ctrlName()         , &QLineEdit::editingFinished    , this, &MethodPage::onParamNameCommitted);
    connect(mParamDetails->ctrlTypes()        , &QComboBox::currentIndexChanged, this, &MethodPage::onParamTypeChanged);
    connect(mParamDetails->ctrlHasDefault()   , &QCheckBox::toggled            , this, &MethodPage::onParamHasDefaultToggled);
    connect(mParamDetails->ctrlValue()        , &QLineEdit::editingFinished    , this, &MethodPage::onParamValueCommitted);
    connect(mParamDetails->ctrlDeprecated()   , &QCheckBox::toggled            , this, &MethodPage::onParamDeprecatedToggled);
    connect(mParamDetails->ctrlDeprecateHint(), &QLineEdit::editingFinished    , this, &MethodPage::onParamDeprecateHintCommitted);
    mParamDetails->ctrlDescription()->installEventFilter(this);

    DocModelNotifier& notifier = mModel.getNotifier();

    // Both forms carry document text, the body included. Typing in them marks the document
    // changed at once, even though the text itself is handed over on focus loss.
    PendingEditWatcher::watchField(mDetails, notifier);
    PendingEditWatcher::watchField(mParamDetails, notifier);

    connect(&notifier, &DocModelNotifier::documentReloaded, this, &MethodPage::onNotifierChanged);
    connect(&notifier, &DocModelNotifier::elementAdded  , this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::Method) onNotifierChanged(); });
    connect(&notifier, &DocModelNotifier::elementRemoved, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::Method) onNotifierChanged(); });
    connect(&notifier, &DocModelNotifier::elementChanged, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::Method) onNotifierChanged(); });
    connect(&notifier, &DocModelNotifier::listReordered , this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::Method) onNotifierChanged(); });

    // A method name may live in a wider name space than the method list, so a rename elsewhere
    // can resolve or introduce a collision with the selected method's name.
    connect(&notifier, &DocModelNotifier::nameChanged, this, [this](uint32_t, const QString&, const QString&)
    {
        if (currentKind() == eRowKind::Method)
        {
            MethodEntry* method = currentMethod();
            mDetails->showNameHint(nameCollisionReason(method, mDetails->ctrlName()->text(), method != nullptr ? method->getId() : 0u));
        }
    });

    connect(&notifier, &DocModelNotifier::elementAdded  , this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) onDataTypesChanged(); });
    connect(&notifier, &DocModelNotifier::elementRemoved, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) onDataTypesChanged(); });
    connect(&notifier, &DocModelNotifier::elementChanged, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) onDataTypesChanged(); });
    connect(&notifier, &DocModelNotifier::listReordered , this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) onDataTypesChanged(); });

    // Inline editing through the shared TableCell delegate. Wait-for-end mode commits a text
    // edit once, so an inline rename pushes a single undo command.
    mTableCell = new TableCell(table, this, true);
    mTableCell->setEditableCheck([this](const QModelIndex& idx) { return isCellEditable(idx); });
    mTableCell->setEditorModelResolver([this](const QModelIndex& idx) { return editorModelFor(idx); });
    mTableCell->setValidationResolver([this](const QModelIndex& idx) { return validationFor(idx); });
    for (int col = 0; col < table->columnCount(); ++col)
    {
        table->setItemDelegateForColumn(col, mTableCell);
    }
    connect(mTableCell, &TableCell::signalEditorDataChanged, this, &MethodPage::onEditorDataChanged);
    connect(mTableCell, &TableCell::signalEditorTextChanged, this, &MethodPage::onEditorTextChanged);
    connect(mTableCell, &TableCell::signalEditorClosed     , this, &MethodPage::onEditorClosed);
}

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////

void MethodPage::commitPendingEdits(void)
{
    // Both forms hold the text of what is selected right now, so the selection decides which of
    // them has something to give. Handing over an unchanged text does nothing.
    MethodEntry* method = currentMethod();
    if (method == nullptr)
        return;

    if (currentKind() == eRowKind::Method)
    {
        mModel.setDescription(method->getId(), mDetails->ctrlDescription()->toPlainText());
        if (method->hasImplement())
        {
            mModel.setBody(method->getId(), mDetails->ctrlBody()->ctrlBody()->toPlainText());
        }
    }

    const uint32_t paramId = currentParamId();
    if (paramId != 0)
    {
        mModel.setParamDescription(method, paramId, mParamDetails->ctrlDescription()->toPlainText());
    }
}

void MethodPage::dataTypesChanged(void)
{
    // A declared type is a name plus a lazily resolved pointer, and the object that came back
    // from an undone category conversion is not the one the parameter still points at.
    mModel.resolveDeclaredTypes();
    onDataTypesChanged();
    refreshAll();
}

void MethodPage::revealElement(uint32_t id, eIssueField field /*= eIssueField::None*/)
{
    // A finding about a parameter carries the parameter's own ID, so a method lookup that only
    // tries the top-level rows would come back empty and reveal nothing.
    uint32_t methodId = id;
    uint32_t paramId = 0;
    if (mModel.findMethod(id) == nullptr)
    {
        for (MethodEntry* method : mModel.getMethods())
        {
            if ((method != nullptr) && (mModel.findParam(method, id) != nullptr))
            {
                methodId = method->getId();
                paramId = id;
                break;
            }
        }
    }

    if (selectMethod(methodId, paramId) == false)
        return;

    if (paramId != 0)
    {
        switch (field)
        {
        case eIssueField::Name:         WidgetHighlight::reveal(mParamDetails->ctrlName());        break;
        case eIssueField::Type:         WidgetHighlight::reveal(mParamDetails->ctrlTypes());       break;
        case eIssueField::Value:        WidgetHighlight::reveal(mParamDetails->ctrlValue());       break;
        case eIssueField::Description:  WidgetHighlight::reveal(mParamDetails->ctrlDescription()); break;
        default:                                                                                   break;
        }
    }
    else
    {
        switch (field)
        {
        case eIssueField::Name:         WidgetHighlight::reveal(mDetails->ctrlName());        break;
        case eIssueField::Type:         WidgetHighlight::reveal(mDetails->ctrlReturn());      break;
        case eIssueField::Link:         WidgetHighlight::reveal(mDetails->ctrlReply());       break;
        case eIssueField::Description:  WidgetHighlight::reveal(mDetails->ctrlDescription()); break;
        default:                                                                              break;
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Overrides a document may replace
//////////////////////////////////////////////////////////////////////////

bool MethodPage::confirmRemove(uint32_t /*id*/)
{
    return true;
}

QString MethodPage::nameCollisionReason(const MethodEntry* method, const QString& name, uint32_t selfId) const
{
    if ((method == nullptr) || name.isEmpty())
        return QString();

    // Names collide only within the same kind: each kind becomes a member of a different
    // generated class, so a request and a broadcast may both be called `start`.
    const MethodEntry* other = mModel.findMethod(name, method->getKind());
    if ((other == nullptr) || (other->getId() == selfId))
        return QString();

    return tr("'%1' is already declared as another %2").arg(name, method->kind().label.toLower());
}

void MethodPage::decorateMethodNode(QTreeWidgetItem* /*node*/, const MethodEntry& /*method*/) const
{
}

void MethodPage::updateExtraFields(MethodEntry* /*method*/)
{
}

void MethodPage::updateBodyEditor(MethodEntry* method)
{
    if ((method == nullptr) || (method->hasImplement() == false))
        return;

    QString params;
    for (const MethodParameter& param : method->getElements())
    {
        if (params.isEmpty() == false)
            params += QStringLiteral(", ");

        params += QStringLiteral("%1 %2").arg(param.getType(), param.getName());
    }

    const QString ret = method->getReturn().isEmpty() ? QString::fromLatin1(MethodEntry::DEFAULT_RETURN) : method->getReturn();
    mDetails->ctrlBody()->setSignature(QStringLiteral("%1 %2(%3)").arg(ret, method->getName(), params));
}

bool MethodPage::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::FocusOut)
    {
        if ((watched == mDetails->ctrlDescription())
            || (watched == mDetails->ctrlBody()->ctrlBody())
            || (watched == mParamDetails->ctrlDescription()))
        {
            commitPendingEdits();
        }
    }

    return QScrollArea::eventFilter(watched, event);
}

int MethodPage::getColumnCount(void) const
{
    return mList->ctrlTableList()->columnCount();
}

QString MethodPage::getCellText(const QModelIndex& cell) const
{
    return cell.isValid() ? cell.data(Qt::DisplayRole).toString() : QString();
}

//////////////////////////////////////////////////////////////////////////
// The tree
//////////////////////////////////////////////////////////////////////////

void MethodPage::setNodeText(QTreeWidgetItem* node, const DocumentElem* elem) const
{
    // Let the TableCell delegate open an inline editor; which cells actually edit is gated per
    // cell by isCellEditable() -- tree items are not editable by default, unlike table items.
    node->setFlags(node->flags() | Qt::ItemIsEditable);

    node->setIcon(0, elem->getIcon(ElementBase::eDisplay::DisplayName));
    node->setText(0, elem->getString(ElementBase::eDisplay::DisplayName));
    node->setIcon(1, elem->getIcon(ElementBase::eDisplay::DisplayType));
    node->setText(1, elem->getString(ElementBase::eDisplay::DisplayType));
    node->setIcon(2, elem->getIcon(ElementBase::eDisplay::DisplayValue));
    node->setText(2, elem->getString(ElementBase::eDisplay::DisplayValue));
}

QTreeWidgetItem* MethodPage::createMethodNode(MethodEntry* method) const
{
    QTreeWidgetItem* item = new QTreeWidgetItem();
    setNodeText(item, method);

    if (mList->ctrlTableList()->columnCount() > static_cast<int>(MethodListView::ColReply))
    {
        const QString reply = method->hasReply() ? method->getReply() : QString();
        item->setText(MethodListView::ColReply, reply);
        if (reply.isEmpty())
        {
            item->setIcon(MethodListView::ColReply, QIcon());
            item->setToolTip(MethodListView::ColReply, QString());
        }
        else if (mModel.findMethod(reply, replyKindIndex()) != nullptr)
        {
            item->setIcon(MethodListView::ColReply, method->getIcon(ElementBase::eDisplay::DisplayLink));
            item->setToolTip(MethodListView::ColReply, QString());
        }
        else
        {
            item->setIcon(MethodListView::ColReply, NELusanCommon::iconWarning(NELusanCommon::SizeSmall));
            item->setToolTip(MethodListView::ColReply, tr("'%1' is not declared").arg(reply));
        }
    }

    decorateMethodNode(item, *method);

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

void MethodPage::refreshAll(void)
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
        for (MethodEntry* entry : mModel.getMethods())
        {
            if (entry == nullptr)
                continue;

            QTreeWidgetItem* node = createMethodNode(entry);
            table->addTopLevelItem(node);
            if (expandedMethods.contains(entry->getId()))
            {
                node->setExpanded(true);
            }
        }
    }

    // The methods that may answer another one are part of the form, so the combo follows the
    // list rather than being filled once.
    populateReplyCombo();

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

bool MethodPage::selectMethod(uint32_t methodId, uint32_t paramId /*= 0*/)
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

MethodPage::eRowKind MethodPage::currentKind(void) const
{
    QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
    if (item == nullptr)
        return eRowKind::None;

    return static_cast<eRowKind>(item->data(0, Qt::ItemDataRole::UserRole).toInt());
}

MethodEntry* MethodPage::currentMethod(void) const
{
    QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
    const eRowKind kind = currentKind();
    if ((item == nullptr) || ((kind != eRowKind::Method) && (kind != eRowKind::Param)))
        return nullptr;

    return mModel.findMethod(item->data(1, Qt::ItemDataRole::UserRole).toUInt());
}

uint32_t MethodPage::currentParamId(void) const
{
    QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
    return ((item != nullptr) && (currentKind() == eRowKind::Param)) ? item->data(2, Qt::ItemDataRole::UserRole).toUInt() : 0u;
}

int MethodPage::replyKindIndex(void) const
{
    const QList<MethodKind>& kinds = mModel.getConfig().kinds;
    for (int i = 0; i < kinds.size(); ++i)
    {
        if (kinds.at(i).isReply)
            return i;
    }

    return -1;
}

int MethodPage::kindIndexOf(const QString& label) const
{
    const QList<MethodKind>& kinds = mModel.getConfig().kinds;
    for (int i = 0; i < kinds.size(); ++i)
    {
        if (kinds.at(i).label == label)
            return i;
    }

    return -1;
}

//////////////////////////////////////////////////////////////////////////
// The forms
//////////////////////////////////////////////////////////////////////////

void MethodPage::onCurCellChanged(QTreeWidgetItem* current, QTreeWidgetItem* /*previous*/)
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
        MethodEntry* method = currentMethod();
        if (method != nullptr)
            selectedMethod(method);
        break;
    }

    case eRowKind::Param:
    {
        MethodEntry* method = currentMethod();
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

void MethodPage::showDetails(eRowKind kind)
{
    const bool showParam = (kind == eRowKind::Param);
    if (mParamDetails->isHidden() != (showParam == false))
        mParamDetails->setHidden(showParam == false);
    if (mDetails->isHidden() != showParam)
        mDetails->setHidden(showParam);
}

void MethodPage::showCleanForm(void)
{
    showDetails(eRowKind::None);
    mDetails->showNameHint(QString());
    mDetails->setConditionVisible(false);
    mDetails->setBodyVisible(false);
    mDetails->setGuardInfoVisible(false);

    const QSignalBlocker blockName(mDetails->ctrlName());
    const QSignalBlocker blockDescr(mDetails->ctrlDescription());
    const QSignalBlocker blockReply(mDetails->ctrlReply());
    mDetails->ctrlName()->clear();
    mDetails->ctrlDescription()->clear();
    mDetails->ctrlReply()->setCurrentText(QString());
    mDetails->ctrlReply()->setEnabled(false);
    mDetails->ctrlName()->setPlaceholderText(tr("Select a method, or click Add"));
    mDetails->ctrlName()->setEnabled(false);
    mDetails->ctrlDescription()->setEnabled(false);
    applyDeprecatedDisplay(mDetails->ctrlDeprecated(), mDetails->ctrlDeprecateHint(), false, QString());
}

void MethodPage::updateToolbar(eRowKind kind)
{
    const bool hasEntry = (kind == eRowKind::Method) || (kind == eRowKind::Param);

    mList->ctrlButtonAdd()->setEnabled(true);
    mList->ctrlButtonInsert()->setEnabled(hasEntry);
    mList->ctrlButtonRemove()->setEnabled(hasEntry);
    mList->ctrlButtonParamAdd()->setEnabled(hasEntry);

    if (hasEntry == false)
    {
        mList->ctrlButtonMoveUp()->setEnabled(false);
        mList->ctrlButtonMoveDown()->setEnabled(false);
    }
}

void MethodPage::updateMoveButtons(int row, int rowCount)
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

void MethodPage::showMethodForm(MethodEntry* method)
{
    const int kindCount = mModel.getConfig().kinds.size();

    const QSignalBlocker blockName(mDetails->ctrlName());
    const QSignalBlocker blockReply(mDetails->ctrlReply());
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

    for (int i = 0; i < kindCount; ++i)
    {
        if (QRadioButton* radio = mDetails->ctrlType(i))
        {
            const QSignalBlocker blockKind(radio);
            radio->setChecked(i == method->getKind());
        }
    }

    // Only a method whose kind is answered by another one may name that answer.
    mDetails->ctrlReply()->setEnabled(method->hasReply());
    mDetails->ctrlReply()->setCurrentText(method->hasReply() ? method->getReply() : QString());

    mDetails->ctrlReturn()->setCurrentText(method->getReturn());
    mDetails->ctrlHandler()->setChecked(method->isEmbedded() == false);
    mDetails->ctrlEmbedded()->setChecked(method->isEmbedded());
    mDetails->ctrlBody()->ctrlBody()->setPlainText(method->getBody());

    mDetails->setConditionVisible(method->hasReturn() || method->hasImplement());
    mDetails->setBodyVisible(method->isEmbedded());
    updateBodyEditor(method);
    updateExtraFields(method);

    applyDeprecatedDisplay(mDetails->ctrlDeprecated(), mDetails->ctrlDeprecateHint(), method->getIsDeprecated(), method->getDeprecateHint());
    mDetails->showNameHint(nameCollisionReason(method, method->getName(), method->getId()));
}

void MethodPage::selectedMethod(MethodEntry* method)
{
    showDetails(eRowKind::Method);
    showMethodForm(method);
    updateToolbar(eRowKind::Method);
    updateMoveButtons(mModel.findIndex(method), mModel.getMethodCount());
}

void MethodPage::selectedParam(MethodEntry* owner, uint32_t paramId)
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

    applyDeprecatedDisplay(mParamDetails->ctrlDeprecated(), mParamDetails->ctrlDeprecateHint(), param->getIsDeprecated(), param->getDeprecateHint());
    mParamDetails->showNameHint(paramNameCollisionReason(owner, param->getName(), param->getId()));

    updateToolbar(eRowKind::Param);
    updateMoveButtons(mModel.findParamIndex(owner, paramId), mModel.getParamCount(owner));
}

QString MethodPage::paramNameCollisionReason(const MethodEntry* owner, const QString& name, uint32_t selfId) const
{
    if ((owner == nullptr) || name.isEmpty())
        return QString();

    const MethodParameter* found = mModel.findParam(owner, name);
    return ((found != nullptr) && (found->getId() != selfId))
        ? tr("'%1' is already used by another parameter of this method").arg(name)
        : QString();
}

//////////////////////////////////////////////////////////////////////////
// The type lists
//////////////////////////////////////////////////////////////////////////

void MethodPage::populateReplyCombo(void)
{
    const int replyKind = replyKindIndex();
    if (replyKind < 0)
        return;

    QComboBox* combo = mDetails->ctrlReply();
    const QSignalBlocker blocker(combo);
    const QString current = combo->currentText();
    combo->clear();
    combo->addItem(QString());      // naming no answer is a choice of its own
    for (const MethodEntry* entry : mModel.methodsOfKind(replyKind))
    {
        combo->addItem(entry->getName());
    }

    combo->setCurrentText(current);
}

void MethodPage::populateReturnCombo(void)
{
    QComboBox* combo = mDetails->ctrlReturn();
    const QSignalBlocker blocker(combo);
    const QString current = combo->currentText();
    combo->clear();

    QList<DataTypeBase*> predefined;
    DataTypeFactory::getPredefinedTypes(predefined, declarableCategories());
    for (DataTypeBase* type : predefined)
    {
        combo->addItem(type->getName());
    }
    for (DataTypeCustom* type : mModel.getCustomDataTypes())
    {
        combo->addItem(type->getQualifiedName());
    }

    combo->setCurrentText(current.isEmpty() ? QString::fromLatin1(MethodEntry::DEFAULT_RETURN) : current);
}

void MethodPage::populateParamTypeCombo(void)
{
    QComboBox* combo = mParamDetails->ctrlTypes();
    const QSignalBlocker blocker(combo);
    const QString current = combo->currentText();
    combo->clear();

    QList<DataTypeBase*> predefined;
    DataTypeFactory::getPredefinedTypes(predefined, declarableCategories());
    for (DataTypeBase* type : predefined)
    {
        combo->addItem(type->getName(), QVariant::fromValue(type));
    }
    for (DataTypeCustom* type : mModel.getCustomDataTypes())
    {
        combo->addItem(type->getQualifiedName(), QVariant::fromValue(static_cast<DataTypeBase*>(type)));
    }

    combo->setCurrentText(current);
}

//////////////////////////////////////////////////////////////////////////
// The toolbar
//////////////////////////////////////////////////////////////////////////

QString MethodPage::genMethodName(void)
{
    static const QString _defName("new_method");
    QString name;
    do
    {
        name = _defName + QString::number(++mMethodNameCounter);
    } while (mModel.findMethod(name) != nullptr);

    return name;
}

QString MethodPage::genParamName(const MethodEntry* method) const
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

void MethodPage::focusNameField(void)
{
    QLineEdit* name = nullptr;
    switch (currentKind())
    {
    case eRowKind::Method:  name = mDetails->ctrlName();      break;
    case eRowKind::Param:   name = mParamDetails->ctrlName(); break;
    default:                                                  break;
    }

    if (name != nullptr)
    {
        name->setFocus();
        name->selectAll();
    }
}

void MethodPage::addNewMethod(int kind)
{
    MethodEntry* entry = mModel.createMethod(genMethodName(), kind);
    if (entry != nullptr)
    {
        selectMethod(entry->getId());
        mDetails->ctrlName()->setFocus();
        mDetails->ctrlName()->selectAll();
    }
}

void MethodPage::addNewParam(void)
{
    MethodEntry* method = currentMethod();
    if (method == nullptr)
        return;

    MethodParameter* param = mModel.createParam(method, genParamName(method));
    if (param != nullptr)
    {
        selectMethod(method->getId(), param->getId());
        mParamDetails->ctrlName()->setFocus();
        mParamDetails->ctrlName()->selectAll();
    }
}

void MethodPage::onAddClicked(void)
{
    // A plain Add always creates a method of the first kind; the drop-down offers the rest.
    addNewMethod(0);
}

void MethodPage::onInsertClicked(void)
{
    switch (currentKind())
    {
    case eRowKind::Method:
    {
        MethodEntry* current = currentMethod();
        const int position = (current != nullptr ? mModel.findIndex(current) : 0);
        const int kind = (current != nullptr ? current->getKind() : 0);
        MethodEntry* entry = mModel.insertMethod(position < 0 ? 0 : position, genMethodName(), kind);
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
        MethodEntry* method = currentMethod();
        if (method == nullptr)
            break;

        const int position = mModel.findParamIndex(method, currentParamId());
        MethodParameter* param = mModel.insertParam(method, position < 0 ? 0 : position, genParamName(method));
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

void MethodPage::onRemoveClicked(void)
{
    switch (currentKind())
    {
    case eRowKind::Method:
    {
        MethodEntry* method = currentMethod();
        if ((method == nullptr) || (confirmRemove(method->getId()) == false))
            break;

        const QList<MethodEntry*>& list = mModel.getMethods();
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
        MethodEntry* method = currentMethod();
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

void MethodPage::onMoveUpClicked(void)
{
    switch (currentKind())
    {
    case eRowKind::Method:
    {
        MethodEntry* method = currentMethod();
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
        MethodEntry* method = currentMethod();
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

void MethodPage::onMoveDownClicked(void)
{
    switch (currentKind())
    {
    case eRowKind::Method:
    {
        MethodEntry* method = currentMethod();
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
        MethodEntry* method = currentMethod();
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

//////////////////////////////////////////////////////////////////////////
// The method form
//////////////////////////////////////////////////////////////////////////

void MethodPage::onMethodNameTextChanged(const QString& text)
{
    if (currentKind() != eRowKind::Method)
        return;

    MethodEntry* method = currentMethod();
    mDetails->showNameHint(nameCollisionReason(method, text, method != nullptr ? method->getId() : 0u));

    // Live-preview the typed name into the selected row; the commit waits for editing-finished.
    if (QTreeWidgetItem* item = mList->ctrlTableList()->currentItem())
    {
        item->setText(0, text);
    }
}

void MethodPage::onMethodNameCommitted(void)
{
    MethodEntry* method = currentMethod();
    if ((method != nullptr) && (currentKind() == eRowKind::Method))
    {
        mModel.renameMethod(method->getId(), mDetails->ctrlName()->text());
    }
}

void MethodPage::onMethodKindToggled(bool checked)
{
    if (checked == false)
        return;

    MethodEntry* method = currentMethod();
    if ((method == nullptr) || (currentKind() != eRowKind::Method))
        return;

    const int kindCount = mModel.getConfig().kinds.size();
    for (int i = 0; i < kindCount; ++i)
    {
        QRadioButton* radio = mDetails->ctrlType(i);
        if ((radio != nullptr) && radio->isChecked())
        {
            mModel.setKind(method->getId(), i);
            return;
        }
    }
}

void MethodPage::onReplyChanged(int /*index*/)
{
    MethodEntry* method = currentMethod();
    if ((method == nullptr) || (currentKind() != eRowKind::Method) || (method->hasReply() == false))
        return;

    mModel.setReply(method->getId(), mDetails->ctrlReply()->currentText());
}

void MethodPage::onReturnCommitted(void)
{
    MethodEntry* method = currentMethod();
    if ((method == nullptr) || (currentKind() != eRowKind::Method) || (method->hasReturn() == false))
        return;

    QString text = mDetails->ctrlReturn()->currentText().trimmed();
    if (text.isEmpty())
    {
        // A method that returns a value must return something; fall back to the default type.
        text = mModel.getConfig().defaultReturn.isEmpty()
                    ? QString::fromLatin1(MethodEntry::DEFAULT_RETURN)
                    : mModel.getConfig().defaultReturn;
        const QSignalBlocker blocker(mDetails->ctrlReturn());
        mDetails->ctrlReturn()->setCurrentText(text);
    }

    mModel.setReturn(method->getId(), text);
}

void MethodPage::onImplementToggled(bool checked)
{
    if (checked == false)
        return;

    MethodEntry* method = currentMethod();
    if ((method == nullptr) || (currentKind() != eRowKind::Method) || (method->hasImplement() == false))
        return;

    mModel.setImplement(method->getId(), mDetails->ctrlEmbedded()->isChecked()
                                            ? MethodEntry::eImplement::Embedded
                                            : MethodEntry::eImplement::Handler);
}

void MethodPage::onMethodDeprecatedToggled(bool checked)
{
    MethodEntry* method = currentMethod();
    if ((method == nullptr) || (currentKind() != eRowKind::Method))
        return;

    mModel.setDeprecated(method->getId(), checked);
    const QSignalBlocker blockHint(mDetails->ctrlDeprecateHint());
    mDetails->ctrlDeprecateHint()->setEnabled(checked);
    mDetails->ctrlDeprecateHint()->setText(checked ? method->getDeprecateHint() : QString());
    if (checked)
    {
        mDetails->ctrlDeprecateHint()->setFocus();
    }
}

void MethodPage::onMethodDeprecateHintCommitted(void)
{
    MethodEntry* method = currentMethod();
    if ((method != nullptr) && (currentKind() == eRowKind::Method))
    {
        mModel.setDeprecateHint(method->getId(), mDetails->ctrlDeprecateHint()->text());
    }
}

//////////////////////////////////////////////////////////////////////////
// The parameter form
//////////////////////////////////////////////////////////////////////////

void MethodPage::onParamNameTextChanged(const QString& text)
{
    MethodEntry* method = currentMethod();
    const uint32_t paramId = currentParamId();
    if ((method == nullptr) || (paramId == 0))
        return;

    mParamDetails->showNameHint(paramNameCollisionReason(method, text, paramId));
    if (QTreeWidgetItem* item = mList->ctrlTableList()->currentItem())
    {
        item->setText(0, text);
    }
}

void MethodPage::onParamNameCommitted(void)
{
    MethodEntry* method = currentMethod();
    const uint32_t paramId = currentParamId();
    if ((method != nullptr) && (paramId != 0))
    {
        mModel.setParamName(method, paramId, mParamDetails->ctrlName()->text());
    }
}

void MethodPage::onParamTypeChanged(int index)
{
    MethodEntry* method = currentMethod();
    const uint32_t paramId = currentParamId();
    if ((method == nullptr) || (paramId == 0) || (index < 0))
        return;

    DataTypeBase* selected = mParamDetails->ctrlTypes()->itemData(index, Qt::ItemDataRole::UserRole).value<DataTypeBase*>();
    if (selected != nullptr)
    {
        mModel.setParamType(method, paramId, selected->getName());
    }
}

void MethodPage::onParamHasDefaultToggled(bool checked)
{
    MethodEntry* method = currentMethod();
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

void MethodPage::onParamValueCommitted(void)
{
    MethodEntry* method = currentMethod();
    const uint32_t paramId = currentParamId();
    if ((method == nullptr) || (paramId == 0))
        return;

    mModel.setParamDefault(method, paramId, mParamDetails->ctrlHasDefault()->isChecked(), mParamDetails->ctrlValue()->text());
}

void MethodPage::onParamDeprecatedToggled(bool checked)
{
    MethodEntry* method = currentMethod();
    const uint32_t paramId = currentParamId();
    if ((method == nullptr) || (paramId == 0))
        return;

    mModel.setParamDeprecated(method, paramId, checked);
    MethodParameter* param = mModel.findParam(method, paramId);
    const QSignalBlocker blockHint(mParamDetails->ctrlDeprecateHint());
    mParamDetails->ctrlDeprecateHint()->setEnabled(checked);
    mParamDetails->ctrlDeprecateHint()->setText((checked && (param != nullptr)) ? param->getDeprecateHint() : QString());
    if (checked)
    {
        mParamDetails->ctrlDeprecateHint()->setFocus();
    }
}

void MethodPage::onParamDeprecateHintCommitted(void)
{
    MethodEntry* method = currentMethod();
    const uint32_t paramId = currentParamId();
    if ((method != nullptr) && (paramId != 0))
    {
        mModel.setParamDeprecateHint(method, paramId, mParamDetails->ctrlDeprecateHint()->text());
    }
}

//////////////////////////////////////////////////////////////////////////
// The notifier
//////////////////////////////////////////////////////////////////////////

void MethodPage::onNotifierChanged(void)
{
    refreshAll();
}

void MethodPage::onDataTypesChanged(void)
{
    populateReturnCombo();
    populateParamTypeCombo();

    MethodEntry* method = currentMethod();
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

//////////////////////////////////////////////////////////////////////////
// Inline editing
//////////////////////////////////////////////////////////////////////////

bool MethodPage::isCellEditable(const QModelIndex& index) const
{
    if (index.isValid() == false)
        return false;

    const eRowKind kind = static_cast<eRowKind>(index.sibling(index.row(), 0).data(Qt::ItemDataRole::UserRole).toInt());
    const int col = index.column();
    if (kind == eRowKind::Method)
    {
        if ((col == static_cast<int>(MethodListView::ColName)) || (col == static_cast<int>(MethodListView::ColType)))
            return true;

        if (col == static_cast<int>(MethodListView::ColReply))
        {
            // Only a kind that is answered by another one may name that answer, exactly as the
            // Reply combo of the form is enabled under the same condition.
            const MethodEntry* method = mModel.findMethod(index.sibling(index.row(), static_cast<int>(MethodListView::ColType)).data(Qt::ItemDataRole::UserRole).toUInt());
            return (method != nullptr) && method->hasReply();
        }

        // The value column carries nothing for a method.
        return false;
    }

    if (kind == eRowKind::Param)
    {
        if ((col == static_cast<int>(MethodListView::ColName)) || (col == static_cast<int>(MethodListView::ColType)))
            return true;

        if (col == static_cast<int>(MethodListView::ColValue))
        {
            // The value is editable only when the parameter has a default, mirroring the form,
            // where the value field is enabled under the same condition.
            MethodEntry* method = mModel.findMethod(index.sibling(index.row(), static_cast<int>(MethodListView::ColType)).data(Qt::ItemDataRole::UserRole).toUInt());
            const uint32_t paramId = index.sibling(index.row(), static_cast<int>(MethodListView::ColValue)).data(Qt::ItemDataRole::UserRole).toUInt();
            MethodParameter* param = (method != nullptr) ? mModel.findParam(method, paramId) : nullptr;
            return (param != nullptr) && param->hasDefault();
        }
    }

    return false;
}

QAbstractItemModel* MethodPage::editorModelFor(const QModelIndex& index) const
{
    if (index.isValid() == false)
        return nullptr;

    const eRowKind kind = static_cast<eRowKind>(index.sibling(index.row(), 0).data(Qt::ItemDataRole::UserRole).toInt());
    const int col = index.column();

    // The parameter's type column reuses the form's type combo model, so the inline list and
    // the form list are the same list by construction.
    if ((kind == eRowKind::Param) && (col == static_cast<int>(MethodListView::ColType)))
        return mParamDetails->ctrlTypes()->model();

    if (kind == eRowKind::Method)
    {
        // The kinds the details radios offer, and the answers the Reply combo offers -- picking
        // one in the cell is the same edit as picking it in the form.
        if (col == static_cast<int>(MethodListView::ColType))
            return mKindModel;

        if (col == static_cast<int>(MethodListView::ColReply))
            return mDetails->ctrlReply()->model();
    }

    return nullptr;
}

TableCell::eCellValidation MethodPage::validationFor(const QModelIndex& index) const
{
    return (index.isValid() && (index.column() == static_cast<int>(MethodListView::ColName)))
        ? TableCell::eCellValidation::Identifier
        : TableCell::eCellValidation::NoValidation;
}

void MethodPage::onEditorDataChanged(const QModelIndex& index, const QString& newValue)
{
    if (index.isValid() == false)
        return;

    // Inline editing starts on the current item, so the edited row is the current one.
    QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
    if (item == nullptr)
        return;

    const int kind          = item->data(0, Qt::ItemDataRole::UserRole).toInt();
    const uint32_t methodId = item->data(1, Qt::ItemDataRole::UserRole).toUInt();
    const uint32_t paramId  = item->data(2, Qt::ItemDataRole::UserRole).toUInt();
    const int col           = index.column();

    // Committing here would rebuild the tree while the delegate editor is still closing; defer
    // to the next event-loop turn so the editor tears down cleanly first.
    QMetaObject::invokeMethod(this, [this, kind, methodId, paramId, col, newValue]()
    {
        commitInlineEdit(kind, methodId, paramId, col, newValue);
    }, Qt::QueuedConnection);
}

void MethodPage::onEditorTextChanged(const QModelIndex& index, const QString& newText)
{
    const eRowKind kind = currentKind();
    const int column = index.column();
    if ((kind == eRowKind::Method) && (column == static_cast<int>(MethodListView::ColName)))
    {
        const QSignalBlocker blockName(mDetails->ctrlName());
        mDetails->ctrlName()->setText(newText);
    }
    else if (kind == eRowKind::Param)
    {
        if (column == static_cast<int>(MethodListView::ColName))
        {
            const QSignalBlocker blockName(mParamDetails->ctrlName());
            mParamDetails->ctrlName()->setText(newText);
        }
        else if (column == static_cast<int>(MethodListView::ColValue))
        {
            const QSignalBlocker blockValue(mParamDetails->ctrlValue());
            mParamDetails->ctrlValue()->setText(newText);
        }
    }
}

void MethodPage::onEditorClosed(void)
{
    onCurCellChanged(mList->ctrlTableList()->currentItem(), nullptr);
}

void MethodPage::commitInlineEdit(int kind, uint32_t methodId, uint32_t paramId, int column, const QString& newValue)
{
    MethodEntry* method = mModel.findMethod(methodId);
    if (method == nullptr)
        return;

    if (static_cast<eRowKind>(kind) == eRowKind::Method)
    {
        if (column == static_cast<int>(MethodListView::ColName))
        {
            if (method->getName() != newValue)
                mModel.renameMethod(methodId, newValue);
        }
        else if (column == static_cast<int>(MethodListView::ColType))
        {
            const int picked = kindIndexOf(newValue);
            if ((picked >= 0) && (picked != method->getKind()))
                mModel.setKind(methodId, picked);
        }
        else if (column == static_cast<int>(MethodListView::ColReply))
        {
            if (method->hasReply() && (method->getReply() != newValue))
                mModel.setReply(methodId, newValue);
        }
    }
    else if (static_cast<eRowKind>(kind) == eRowKind::Param)
    {
        MethodParameter* param = mModel.findParam(method, paramId);
        if (param == nullptr)
            return;

        if (column == static_cast<int>(MethodListView::ColName))
        {
            if (param->getName() != newValue)
                mModel.setParamName(method, paramId, newValue);
        }
        else if (column == static_cast<int>(MethodListView::ColType))
        {
            if (param->getType() != newValue)
                mModel.setParamType(method, paramId, newValue);
        }
        else if (column == static_cast<int>(MethodListView::ColValue))
        {
            if (param->hasDefault() && (param->getValue() != newValue))
                mModel.setParamDefault(method, paramId, true, newValue);
        }
    }
}
