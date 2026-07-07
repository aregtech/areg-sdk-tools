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
 *  \file        lusan/view/sm/SMInclude.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Includes page.
 *
 ************************************************************************/

#include "lusan/view/sm/SMInclude.hpp"

#include "lusan/app/LusanApplication.hpp"
#include "lusan/data/common/IncludeEntry.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/sm/SMIncludeModel.hpp"
#include "lusan/view/common/WorkspaceFileDialog.hpp"
#include "lusan/view/sm/SMIncludeDetails.hpp"
#include "lusan/view/sm/SMIncludeList.hpp"

#include <QCheckBox>
#include <QEvent>
#include <QFileInfo>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSignalBlocker>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

namespace
{
    //!< Refreshes a deprecated checkbox + hint pair from an entry's current state, without
    //!< re-triggering the edit signals that would otherwise push a spurious command.
    void applyDeprecatedDisplay(QCheckBox* checkBox, QLineEdit* hintEdit, const IncludeEntry* entry)
    {
        const QSignalBlocker blockCheck(checkBox);
        const QSignalBlocker blockHint(hintEdit);
        const bool deprecated = (entry != nullptr) && entry->getIsDeprecated();
        checkBox->setChecked(deprecated);
        hintEdit->setEnabled(deprecated);
        hintEdit->setText(deprecated ? entry->getDeprecateHint() : QString());
    }
}

SMInclude::SMInclude(SMIncludeModel& model, QWidget* parent /*= nullptr*/)
    : QScrollArea       (parent)
    , mModel            (model)
    , mList             (new SMIncludeList(this))
    , mDetails          (new SMIncludeDetails(this))
    , mNameCounter      (0)
{
    buildUi();
    setupSignals();
    refreshAll();
}

QStringList SMInclude::getSupportedExtensions()
{
    QStringList exts{};
    exts.append(LusanApplication::getExternalFileExtensions());
    exts.append(LusanApplication::getInternalFileExtensions());
    return exts;
}

void SMInclude::buildUi()
{
    QWidget* content = new QWidget(this);
    QVBoxLayout* root = new QVBoxLayout(content);

    QLabel* headline = new QLabel(tr("State Machine Includes Editor ..."), content);
    QFont headlineFont{ headline->font() };
    headlineFont.setPointSize(20);
    headlineFont.setBold(true);
    headlineFont.setItalic(true);
    headline->setFont(headlineFont);
    root->addWidget(headline);

    QHBoxLayout* columns = new QHBoxLayout();

    // Left panel: the multi-column include list.
    mList->setParent(content);
    mList->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    columns->addWidget(mList, 1);

    // Right panel: the selected-include details editor.
    mDetails->setParent(content);
    mDetails->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    columns->addWidget(mDetails, 1);

    root->addLayout(columns, 1);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setWidgetResizable(true);
    setWidget(content);
}

void SMInclude::setupSignals()
{
    connect(mList->ctrlTableList()     , &QTreeWidget::currentItemChanged, this, &SMInclude::onCurCellChanged);
    connect(mList->ctrlButtonAdd()     , &QToolButton::clicked           , this, &SMInclude::onAddClicked);
    connect(mList->ctrlButtonInsert()  , &QToolButton::clicked           , this, &SMInclude::onInsertClicked);
    connect(mList->ctrlButtonRemove()  , &QToolButton::clicked           , this, &SMInclude::onRemoveClicked);
    connect(mList->ctrlButtonMoveUp()  , &QToolButton::clicked           , this, &SMInclude::onMoveUpClicked);
    connect(mList->ctrlButtonMoveDown(), &QToolButton::clicked           , this, &SMInclude::onMoveDownClicked);
    connect(mList->ctrlButtonUpdate()  , &QToolButton::clicked           , this, &SMInclude::onUpdateClicked);

    connect(mDetails->ctrlInclude()      , &QLineEdit::editingFinished, this, &SMInclude::onLocationCommitted);
    connect(mDetails->ctrlBrowseButton() , &QPushButton::clicked      , this, &SMInclude::onBrowseClicked);
    connect(mDetails->ctrlDeprecated()   , &QCheckBox::toggled        , this, &SMInclude::onDeprecatedToggled);
    connect(mDetails->ctrlDeprecateHint(), &QLineEdit::editingFinished, this, &SMInclude::onDeprecateHintCommitted);
    mDetails->ctrlDescription()->installEventFilter(this);

    connect(&mModel.getNotifier(), &DocModelNotifier::documentReloaded, this, &SMInclude::onNotifierChanged);
    connect(&mModel.getNotifier(), &DocModelNotifier::elementAdded, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::Include) onNotifierChanged(); });
    connect(&mModel.getNotifier(), &DocModelNotifier::elementRemoved, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::Include) onNotifierChanged(); });
    connect(&mModel.getNotifier(), &DocModelNotifier::elementChanged, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::Include) onNotifierChanged(); });
    connect(&mModel.getNotifier(), &DocModelNotifier::listReordered, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::Include) onNotifierChanged(); });
}

bool SMInclude::eventFilter(QObject* watched, QEvent* event)
{
    if ((watched == mDetails->ctrlDescription()) && (event->type() == QEvent::FocusOut))
    {
        const uint32_t id = currentIncludeId();
        if (id != 0)
        {
            mModel.setDescription(id, mDetails->ctrlDescription()->toPlainText());
        }
    }

    return QScrollArea::eventFilter(watched, event);
}

QString SMInclude::includeType(const QString& location)
{
    const QString suffix = QFileInfo(location).suffix().toLower();
    if (suffix == QStringLiteral("fsml"))
        return tr("State Machine");
    else if (suffix == QStringLiteral("dtml"))
        return tr("Data Type");
    else
        return tr("Source");
}

QString SMInclude::includeName(const QString& location)
{
    const QFileInfo info(location);
    const QString suffix = info.suffix().toLower();
    // State machine / data type includes carry a declared name; until the file is parsed the
    // base name (no extension) is its best proxy. A source include is shown by its file name.
    if ((suffix == QStringLiteral("fsml")) || (suffix == QStringLiteral("dtml")))
        return info.completeBaseName();
    else
        return info.fileName();
}

void SMInclude::setNodeText(QTreeWidgetItem* node, const IncludeEntry& entry) const
{
    const QString location = entry.getLocation();
    node->setIcon(0, entry.getIcon(ElementBase::eDisplay::DisplayName));
    node->setText(0, entry.getString(ElementBase::eDisplay::DisplayName));
    node->setText(1, includeType(location));
    node->setText(2, includeName(location));
    // Version is populated once state machine / data type includes are parsed; blank for now.
    node->setText(3, QString());
    node->setData(0, Qt::ItemDataRole::UserRole, entry.getId());
}

void SMInclude::onCurCellChanged(QTreeWidgetItem* current, QTreeWidgetItem* /*previous*/)
{
    if (current == nullptr)
    {
        showClean();
        return;
    }

    const uint32_t id = current->data(0, Qt::ItemDataRole::UserRole).toUInt();
    IncludeEntry* entry = mModel.findInclude(id);
    if (entry == nullptr)
    {
        showClean();
        return;
    }

    selectedInclude(entry);
}

void SMInclude::selectedInclude(const IncludeEntry* entry)
{
    {
        const QSignalBlocker blockInclude(mDetails->ctrlInclude());
        const QSignalBlocker blockDescr(mDetails->ctrlDescription());
        mDetails->ctrlInclude()->setText(entry->getLocation());
        mDetails->ctrlDescription()->setPlainText(entry->getDescription());
    }
    applyDeprecatedDisplay(mDetails->ctrlDeprecated(), mDetails->ctrlDeprecateHint(), entry);

    mDetails->ctrlInclude()->setEnabled(true);
    mDetails->ctrlBrowseButton()->setEnabled(true);
    mDetails->ctrlDescription()->setEnabled(true);
    mDetails->ctrlDeprecated()->setEnabled(true);

    mList->ctrlButtonRemove()->setEnabled(true);
    updateMoveButtons(mModel.findIndex(entry->getId()), mModel.getIncludeCount());
}

void SMInclude::showClean()
{
    applyDeprecatedDisplay(mDetails->ctrlDeprecated(), mDetails->ctrlDeprecateHint(), nullptr);

    const QSignalBlocker blockInclude(mDetails->ctrlInclude());
    const QSignalBlocker blockDescr(mDetails->ctrlDescription());
    mDetails->ctrlInclude()->clear();
    mDetails->ctrlDescription()->clear();

    mDetails->ctrlInclude()->setEnabled(false);
    mDetails->ctrlBrowseButton()->setEnabled(false);
    mDetails->ctrlDescription()->setEnabled(false);
    mDetails->ctrlDeprecated()->setEnabled(false);

    mList->ctrlButtonRemove()->setEnabled(false);
    mList->ctrlButtonMoveUp()->setEnabled(false);
    mList->ctrlButtonMoveDown()->setEnabled(false);
}

void SMInclude::updateMoveButtons(int row, int rowCount)
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

uint32_t SMInclude::currentIncludeId() const
{
    QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
    return (item != nullptr ? item->data(0, Qt::ItemDataRole::UserRole).toUInt() : 0u);
}

QString SMInclude::genName()
{
    static const QString _defName("NewInclude");
    QString name;
    do
    {
        name = _defName + QString::number(++mNameCounter);
    } while (mModel.findInclude(name) != nullptr);

    return name;
}

void SMInclude::onAddClicked()
{
    const QString location = genName();
    IncludeEntry* entry = mModel.createInclude(location);
    if (entry != nullptr)
    {
        selectInclude(entry->getId());
        mDetails->ctrlInclude()->setFocus();
        mDetails->ctrlInclude()->selectAll();
    }
}

void SMInclude::onInsertClicked()
{
    const uint32_t id = currentIncludeId();
    const int position = (id != 0 ? mModel.findIndex(id) : 0);
    const QString location = genName();
    IncludeEntry* entry = mModel.insertInclude(position < 0 ? 0 : position, location);
    if (entry != nullptr)
    {
        selectInclude(entry->getId());
        mDetails->ctrlInclude()->setFocus();
        mDetails->ctrlInclude()->selectAll();
    }
}

void SMInclude::onRemoveClicked()
{
    const uint32_t id = currentIncludeId();
    if (id == 0)
        return;

    const QList<IncludeEntry>& list = mModel.getIncludes();
    const int index = mModel.findIndex(id);
    uint32_t neighborId = 0;
    if (list.size() > 1)
    {
        const int neighborIndex = ((index + 1) < list.size()) ? (index + 1) : (index - 1);
        neighborId = list.at(neighborIndex).getId();
    }

    mModel.deleteInclude(id);
    if (neighborId != 0)
    {
        selectInclude(neighborId);
    }
}

void SMInclude::onMoveUpClicked()
{
    const uint32_t id = currentIncludeId();
    if (id == 0)
        return;

    const int index = mModel.findIndex(id);
    if (index > 0)
    {
        // The swap keeps IDs attached to list position, so the moved entry's new ID is the
        // neighbor's old ID — reselect that to keep the moved row highlighted.
        const uint32_t neighborId = mModel.getIncludes().at(index - 1).getId();
        mModel.swapIncludes(id, neighborId);
        selectInclude(neighborId);
    }
}

void SMInclude::onMoveDownClicked()
{
    const uint32_t id = currentIncludeId();
    if (id == 0)
        return;

    const int index = mModel.findIndex(id);
    if ((index >= 0) && (index < (mModel.getIncludeCount() - 1)))
    {
        const uint32_t neighborId = mModel.getIncludes().at(index + 1).getId();
        mModel.swapIncludes(id, neighborId);
        selectInclude(neighborId);
    }
}

void SMInclude::onLocationCommitted()
{
    const uint32_t id = currentIncludeId();
    if (id != 0)
    {
        mModel.setLocation(id, mDetails->ctrlInclude()->text());
    }
}

void SMInclude::onBrowseClicked()
{
    const uint32_t id = currentIncludeId();
    if (id == 0)
        return;

    WorkspaceFileDialog dialog(   true
                                , false
                                , LusanApplication::getWorkspaceDirectories()
                                , SMInclude::getSupportedExtensions()
                                , tr("Select Include File")
                                , this);
    dialog.clearHistory();
    if (dialog.exec() == static_cast<int>(QDialog::DialogCode::Accepted))
    {
        mModel.setLocation(id, dialog.getSelectedFileRelativePath());
    }
}

void SMInclude::onDeprecatedToggled(bool checked)
{
    const uint32_t id = currentIncludeId();
    if (id == 0)
        return;

    mModel.setDeprecated(id, checked);
    IncludeEntry* entry = mModel.findInclude(id);
    const QSignalBlocker blockHint(mDetails->ctrlDeprecateHint());
    mDetails->ctrlDeprecateHint()->setEnabled(checked);
    mDetails->ctrlDeprecateHint()->setText((checked && (entry != nullptr)) ? entry->getDeprecateHint() : QString());
    if (checked)
    {
        mDetails->ctrlDeprecateHint()->setFocus();
    }
}

void SMInclude::onDeprecateHintCommitted()
{
    const uint32_t id = currentIncludeId();
    if (id != 0)
    {
        mModel.setDeprecateHint(id, mDetails->ctrlDeprecateHint()->text());
    }
}

void SMInclude::refreshAll()
{
    QTreeWidget* table = mList->ctrlTableList();
    const uint32_t selId = currentIncludeId();

    {
        const QSignalBlocker blocker(table);
        table->clear();
        for (const IncludeEntry& entry : mModel.getIncludes())
        {
            QTreeWidgetItem* item = new QTreeWidgetItem();
            setNodeText(item, entry);
            table->addTopLevelItem(item);
        }
    }

    if ((selId == 0) || (selectInclude(selId) == false))
    {
        showClean();
    }
}

bool SMInclude::selectInclude(uint32_t id)
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

void SMInclude::onUpdateClicked()
{
    // Re-derives the Type/Name/Version columns from the current locations. Once state machine
    // and data type includes are parsed, this is where their declared name and version are
    // re-read from disk; today it simply rebuilds the list from the live model.
    refreshAll();
}

void SMInclude::onNotifierChanged()
{
    refreshAll();
}
