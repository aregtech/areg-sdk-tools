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
 *  \file        lusan/view/si/SIInclude.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/

#include "lusan/view/si/SIInclude.hpp"
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include "lusan/app/LusanApplication.hpp"
#include "lusan/data/common/IncludeEntry.hpp"
#include "lusan/model/si/SIIncludeModel.hpp"
#include "lusan/view/common/IncludeDetailsView.hpp"
#include "lusan/view/common/IncludeListView.hpp"
#include "lusan/view/common/WorkspaceFileDialog.hpp"

#include <QCheckBox>
#include <QFileInfo>
#include <QHeaderView>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include "lusan/view/si/SICommon.hpp"

namespace
{
    constexpr int COL_LOCATION = static_cast<int>(IncludeListView::eColumn::ColLocation);
    constexpr int COL_TYPE     = static_cast<int>(IncludeListView::eColumn::ColType);
    constexpr int COL_NAME     = static_cast<int>(IncludeListView::eColumn::ColName);
    constexpr int COL_VERSION  = static_cast<int>(IncludeListView::eColumn::ColVersion);
}

QStringList SIInclude::getSupportedExtensions()
{
    QStringList exts{};
    exts.append(LusanApplication::getExternalFileExtensions());
    exts.append(LusanApplication::getInternalFileExtensions());
    return exts;
}

SIIncludeWidget::SIIncludeWidget(QWidget* parent)
    : QWidget{ parent }
    , mPanels(nullptr)
{
    QVBoxLayout* root = new QVBoxLayout(this);

    QLabel* headline = new QLabel(tr("Service Interface Includes Editor ..."), this);
    QFont headlineFont{ headline->font() };
    headlineFont.setPointSize(20);
    headlineFont.setBold(true);
    headlineFont.setItalic(true);
    headline->setFont(headlineFont);
    root->addWidget(headline);

    mPanels = new QHBoxLayout();
    root->addLayout(mPanels, 1);
}

SIInclude::SIInclude(SIIncludeModel & model, QWidget* parent)
    : QScrollArea(parent)
    , mModel    (model)
    , mDetails  (new IncludeDetailsView(this))
    , mList     (new IncludeListView(IncludeTypeConfig{ QStringLiteral("siml"), tr("Service Interface") }, this))
    , mWidget   (new SIIncludeWidget(this))
    , mTableCell(nullptr)
    , mCurUrl   ( )
    , mCurFile  ( )
    , mCurFilter( )
    , mCurView  ( -1 )
    , mCount    ( 0 )
{
    mList->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    mDetails->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    mWidget->mPanels->addWidget(mList, 1);
    mWidget->mPanels->addWidget(mDetails, 1);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setWidgetResizable(true);
    setWidget(mWidget);

    updateWidgets();
    updateData();
    setupSignals();
    updateDetails(nullptr, true);
}

SIInclude::~SIInclude()
{
    mWidget->mPanels->removeWidget(mList);
    mWidget->mPanels->removeWidget(mDetails);
}

int SIInclude::getColumnCount() const
{
    return mList->ctrlTableList()->columnCount();
}

QString SIInclude::getCellText(const QModelIndex& cell) const
{
    QTreeWidgetItem* item = mList->ctrlTableList()->topLevelItem(cell.row());
    return (item != nullptr ? item->text(cell.column()) : QString());
}

void SIInclude::onCurCellChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
    if (current == previous)
        return;

    blockBasicSignals(true);
    QTreeWidget* table = mList->ctrlTableList();
    const int row = table->indexOfTopLevelItem(current);
    const IncludeEntry* entry = findInclude(row);
    updateDetails(entry, true);
    updateToolBottons(entry != nullptr ? row : -1, table->topLevelItemCount());
    blockBasicSignals(false);
}

void SIInclude::onAddClicked()
{
    QString location(genName());
    QTreeWidget* table = mList->ctrlTableList();

    blockBasicSignals(true);
    IncludeEntry* entry = mModel.createInclude(location);
    if (entry != nullptr)
    {
        mDetails->ctrlInclude()->setEnabled(true);

        int row = table->topLevelItemCount();
        setTexts(-1, *entry);
        table->setCurrentItem(table->topLevelItem(row));
        table->scrollToBottom();
        updateDetails(entry, true);
        mDetails->ctrlInclude()->setFocus();
        mDetails->ctrlInclude()->selectAll();
        updateToolBottons(row, row + 1);
    }

    blockBasicSignals(false);
}

void SIInclude::onRemoveClicked()
{
    QTreeWidget* table = mList->ctrlTableList();
    int row = table->indexOfTopLevelItem(table->currentItem());
    IncludeEntry* entry = findInclude(row);
    IncludeEntry* nextEntry{ nullptr };
    if (entry == nullptr)
        return;

    blockBasicSignals(true);
    int count = table->topLevelItemCount();
    int nextRow = (row + 1 == count) ? row - 1 : row + 1;
    QTreeWidgetItem* next = ((nextRow >= 0) && (nextRow < count)) ? table->topLevelItem(nextRow) : nullptr;
    if (next != nullptr)
    {
        nextEntry = findInclude(nextRow);
        table->setCurrentItem(next);
    }

    updateDetails(nextEntry, true);

    delete table->takeTopLevelItem(row);
    mModel.deleteInclude(entry->getId());
    updateToolBottons(next != nullptr ? table->indexOfTopLevelItem(table->currentItem()) : -1, table->topLevelItemCount());
    blockBasicSignals(false);
}

void SIInclude::onInsertClicked()
{
    QString location(genName());
    QTreeWidget* table = mList->ctrlTableList();

    blockBasicSignals(true);
    int row = table->indexOfTopLevelItem(table->currentItem());
    row = row < 0 ? 0 : row;
    IncludeEntry* entry = mModel.insertInclude(row, location);
    if (entry != nullptr)
    {
        mDetails->ctrlInclude()->setEnabled(true);

        setTexts(row, *entry, true);
        const QList<IncludeEntry>& list = mModel.getIncludes();
        int rowCount = table->topLevelItemCount();
        Q_ASSERT(list.size() == rowCount);
        for (int i = row + 1; i < rowCount; ++i)
        {
            table->topLevelItem(i)->setData(COL_LOCATION, Qt::ItemDataRole::UserRole, QVariant::fromValue<uint32_t>(list.at(i).getId()));
        }

        table->setCurrentItem(table->topLevelItem(row));
        updateDetails(entry, true);
        mDetails->ctrlInclude()->setFocus();
        mDetails->ctrlInclude()->selectAll();
        updateToolBottons(row, table->topLevelItemCount());
    }

    blockBasicSignals(false);
}

void SIInclude::onMoveUpClicked()
{
    QTreeWidget* table = mList->ctrlTableList();
    int row = table->indexOfTopLevelItem(table->currentItem());
    if (row > 0)
    {
        blockBasicSignals(true);
        uint32_t idFirst = table->topLevelItem(row)->data(COL_LOCATION, Qt::ItemDataRole::UserRole).toUInt();
        uint32_t idSecond = table->topLevelItem(row - 1)->data(COL_LOCATION, Qt::ItemDataRole::UserRole).toUInt();
        mModel.swapIncludes(idFirst, idSecond);
        swapIncludes(row, row - 1);
        blockBasicSignals(false);
    }
}

void SIInclude::onMoveDownClicked()
{
    QTreeWidget* table = mList->ctrlTableList();
    int row = table->indexOfTopLevelItem(table->currentItem());
    if ((row >= 0) && (row < (table->topLevelItemCount() - 1)))
    {
        blockBasicSignals(true);
        uint32_t idFirst = table->topLevelItem(row)->data(COL_LOCATION, Qt::ItemDataRole::UserRole).toUInt();
        uint32_t idSecond = table->topLevelItem(row + 1)->data(COL_LOCATION, Qt::ItemDataRole::UserRole).toUInt();
        mModel.swapIncludes(idFirst, idSecond);
        swapIncludes(row, row + 1);
        blockBasicSignals(false);
    }
}

void SIInclude::onBrowseClicked()
{
    WorkspaceFileDialog dialog(   true
                                , false
                                , LusanApplication::getWorkspaceDirectories()
                                , SIInclude::getSupportedExtensions()
                                , tr("Select Include File")
                                , this);

    if (mCurUrl.isEmpty() == false)
    {
        dialog.setDirectoryUrl(QUrl::fromLocalFile(mCurUrl));
        dialog.setDirectory(mCurUrl);
    }

    if (mCurFile.isEmpty() == false)
    {
        QFileInfo info(mCurFile);
        dialog.setDirectory(info.absoluteDir());
        dialog.selectFile(mCurFile);
    }

    if (mCurFilter.isEmpty() == false)
    {
        dialog.setNameFilter(mCurFilter);
    }

    if (mCurView != -1)
    {
        dialog.setViewMode(static_cast<QFileDialog::ViewMode>(mCurView));
    }

    dialog.clearHistory();
    if (dialog.exec() == static_cast<int>(QDialog::DialogCode::Accepted))
    {
        blockBasicSignals(true);

        QString location = dialog.getSelectedFileRelativePath();
        mDetails->ctrlInclude()->setText(location);
        mDetails->ctrlDescription()->setFocus();
        mDetails->ctrlDescription()->selectAll();

        QTreeWidget* table = mList->ctrlTableList();
        int row = table->indexOfTopLevelItem(table->currentItem());
        if (row >= 0)
        {
            IncludeEntry* entry = findInclude(row);
            entry->setLocation(location);
            setTexts(row, *entry);
        }

        mCurUrl = dialog.directoryUrl().path();
        mCurFile = dialog.getSelectedFilePath();
        mCurFilter = dialog.selectedNameFilter();
        mCurView = static_cast<int>(dialog.viewMode());

        blockBasicSignals(false);
    }
}

void SIInclude::onUpdateClicked()
{
    // Re-derives the Type/Name/Version columns from the current locations. Once service
    // interface / data type includes are parsed, this is where their declared name and version
    // are re-read from disk; today it simply rebuilds the derived columns from the live model.
    QTreeWidget* table = mList->ctrlTableList();
    blockBasicSignals(true);
    int count = table->topLevelItemCount();
    for (int i = 0; i < count; ++i)
    {
        IncludeEntry* entry = findInclude(i);
        if (entry != nullptr)
        {
            setTexts(i, *entry);
        }
    }

    blockBasicSignals(false);
}

void SIInclude::onIncludeChanged(const QString& newText)
{
    QTreeWidget* table = mList->ctrlTableList();
    int row = table->indexOfTopLevelItem(table->currentItem());
    IncludeEntry* entry = findInclude(row);
    if (entry != nullptr)
    {
        blockBasicSignals(true);
        entry->setLocation(newText);
        setTexts(row, *entry);
        blockBasicSignals(false);
    }
}

void SIInclude::onDescriptionChanged()
{
    QTreeWidget* table = mList->ctrlTableList();
    int row = table->indexOfTopLevelItem(table->currentItem());
    if (row != -1)
    {
        IncludeEntry* entry = findInclude(row);
        Q_ASSERT(entry != nullptr);
        entry->setDescription(mDetails->ctrlDescription()->toPlainText());
    }
}

void SIInclude::onDeprecatedChecked(bool isChecked)
{
    QTreeWidget* table = mList->ctrlTableList();
    int row = table->indexOfTopLevelItem(table->currentItem());
    if (row >= 0)
    {
        IncludeEntry* entry = findInclude(row);
        Q_ASSERT(entry != nullptr);
        SICommon::checkedDeprecated<IncludeDetailsView, IncludeEntry>(mDetails, entry, isChecked);
    }
}

void SIInclude::onDeprecateHint(const QString& newText)
{
    QTreeWidget* table = mList->ctrlTableList();
    int row = table->indexOfTopLevelItem(table->currentItem());
    if (row != -1)
    {
        IncludeEntry* entry = findInclude(row);
        Q_ASSERT(entry != nullptr);
        SICommon::setDeprecateHint<IncludeDetailsView, IncludeEntry>(mDetails, entry, newText);
    }
}

void SIInclude::onEditorDataChanged(const QModelIndex &index, const QString &newValue)
{
    QTreeWidget* table = mList->ctrlTableList();
    if ((index.row() < 0) || (index.row() >= table->topLevelItemCount()) || (index.column() < 0))
        return;

    cellChanged(index.row(), index.column(), newValue);
}

void SIInclude::cellChanged(int row, int col, const QString& newValue)
{
    IncludeEntry* entry = findInclude(row);
    Q_ASSERT(entry != nullptr);

    if (col == COL_LOCATION)
    {
        if (mDetails->ctrlInclude()->text() != newValue)
        {
            blockBasicSignals(true);
            entry->setLocation(newValue);
            setTexts(row, *entry);
            updateDetails(entry, false);
            blockBasicSignals(false);
        }
    }
}

void SIInclude::updateData()
{
    QTreeWidget* table = mList->ctrlTableList();
    const QList<IncludeEntry>& list = mModel.getIncludes();
    if (list.isEmpty() == false)
    {
        for (const IncludeEntry& entry : list)
        {
            setTexts(-1, entry);
        }

        table->scrollToTop();
    }
}

void SIInclude::updateWidgets()
{
    QTreeWidget* table = mList->ctrlTableList();
    mTableCell = new TableCell(QList<QAbstractItemModel*>(), QList<int>(), table, this, false);
    // Forbid invalid include path characters when editing the Location column inline, exactly
    // as the details panel's Include File field does.
    mTableCell->setColumnValidation(COL_LOCATION, TableCell::eCellValidation::Path);
    // Only Location is user-editable; Type and Name are derived from the location and Version is
    // read from disk, so the delegate refuses an editor on every other column. The item keeps the
    // ItemIsEditable flag (needed to open the Location editor), and this predicate gates the rest.
    mTableCell->setEditableCheck([](const QModelIndex& index) { return index.column() == COL_LOCATION; });
    table->setItemDelegate(mTableCell);

    SICommon::enableDeprecated<IncludeDetailsView, IncludeEntry>(mDetails, nullptr, false);

    mDetails->ctrlInclude()->setEnabled(false);
}

void SIInclude::setupSignals()
{
    Q_ASSERT(mDetails != nullptr);
    Q_ASSERT(mList != nullptr);

    connect(mList->ctrlTableList()      , &QTreeWidget::currentItemChanged, this, &SIInclude::onCurCellChanged);
    connect(mList->ctrlButtonAdd()      , &QToolButton::clicked, this, &SIInclude::onAddClicked);
    connect(mList->ctrlButtonRemove()   , &QToolButton::clicked, this, &SIInclude::onRemoveClicked);
    connect(mList->ctrlButtonInsert()   , &QToolButton::clicked, this, &SIInclude::onInsertClicked);
    connect(mList->ctrlButtonMoveUp()   , &QToolButton::clicked, this, &SIInclude::onMoveUpClicked);
    connect(mList->ctrlButtonMoveDown() , &QToolButton::clicked, this, &SIInclude::onMoveDownClicked);
    connect(mList->ctrlButtonUpdate()   , &QToolButton::clicked, this, &SIInclude::onUpdateClicked);

    connect(mDetails->ctrlInclude()      , &QLineEdit::textChanged, this, &SIInclude::onIncludeChanged);
    connect(mDetails->ctrlBrowseButton() , &QPushButton::clicked, this, &SIInclude::onBrowseClicked);
    connect(mDetails->ctrlDeprecated()   , &QCheckBox::toggled, this, &SIInclude::onDeprecatedChecked);
    connect(mDetails->ctrlDeprecateHint(), &QLineEdit::textChanged, this, &SIInclude::onDeprecateHint);
    connect(mDetails->ctrlDescription()  , &QPlainTextEdit::textChanged, this, &SIInclude::onDescriptionChanged);

    connect(mTableCell, &TableCell::signalEditorDataChanged, this, &SIInclude::onEditorDataChanged);
}

void SIInclude::blockBasicSignals(bool doBlock)
{
    mList->ctrlTableList()->blockSignals(doBlock);

    mDetails->ctrlInclude()->blockSignals(doBlock);
    mDetails->ctrlDescription()->blockSignals(doBlock);
    mDetails->ctrlDeprecated()->blockSignals(doBlock);
    mDetails->ctrlDeprecateHint()->blockSignals(doBlock);
}

inline void SIInclude::setTexts(int row, const IncludeEntry& entry, bool insert /*= false*/)
{
    QTreeWidget* table = mList->ctrlTableList();
    const QString location = entry.getLocation();
    if ((row < 0) || insert)
    {
        row = row < 0 ? table->topLevelItemCount() : row;
        QTreeWidgetItem* item = new QTreeWidgetItem();
        // Editable flag lets the TableCell delegate open an inline editor on double-click.
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        item->setIcon(COL_LOCATION, entry.getIcon(ElementBase::eDisplay::DisplayName));
        item->setText(COL_LOCATION, entry.getString(ElementBase::eDisplay::DisplayName));
        item->setText(COL_TYPE    , mList->typeForLocation(location));
        item->setText(COL_NAME    , mList->nameForLocation(location));
        item->setText(COL_VERSION , QString());
        item->setData(COL_LOCATION, Qt::ItemDataRole::UserRole, entry.getId());
        table->insertTopLevelItem(row, item);
    }
    else
    {
        QTreeWidgetItem* item = table->topLevelItem(row);
        Q_ASSERT(item->data(COL_LOCATION, Qt::ItemDataRole::UserRole).toUInt() == entry.getId());

        item->setIcon(COL_LOCATION, entry.getIcon(ElementBase::eDisplay::DisplayName));
        item->setText(COL_LOCATION, entry.getString(ElementBase::eDisplay::DisplayName));
        item->setText(COL_TYPE    , mList->typeForLocation(location));
        item->setText(COL_NAME    , mList->nameForLocation(location));
    }
}

inline void SIInclude::updateDetails(const IncludeEntry* entry, bool updateAll /*= false*/)
{
    QTreeWidget* table = mList->ctrlTableList();
    if (entry != nullptr)
    {
        mDetails->ctrlInclude()->setEnabled(true);
        mDetails->ctrlInclude()->setText(entry->getName());
        if (table->indexOfTopLevelItem(table->currentItem()) >= 0)
        {
            mDetails->ctrlBrowseButton()->setEnabled(true);
            mList->ctrlButtonRemove()->setEnabled(true);
        }

        if (updateAll)
        {
            mDetails->ctrlDescription()->setPlainText(entry->getDescription());
            SICommon::enableDeprecated<IncludeDetailsView, IncludeEntry>(mDetails, entry, true);
        }
    }
    else
    {
        mDetails->ctrlInclude()->setText("");
        mDetails->ctrlDescription()->setPlainText("");

        SICommon::enableDeprecated<IncludeDetailsView, IncludeEntry>(mDetails, nullptr, false);

        mDetails->ctrlInclude()->setEnabled(false);
        mDetails->ctrlBrowseButton()->setEnabled(false);

        mList->ctrlButtonMoveUp()->setEnabled(false);
        mList->ctrlButtonMoveDown()->setEnabled(false);
        mList->ctrlButtonRemove()->setEnabled(false);
    }
}

inline IncludeEntry* SIInclude::findInclude(int row)
{
    QTreeWidget* table = mList->ctrlTableList();
    if ((row < 0) || (row >= table->topLevelItemCount()))
        return nullptr;

    QTreeWidgetItem* item = table->topLevelItem(row);
    uint32_t id = item->data(COL_LOCATION, Qt::ItemDataRole::UserRole).toUInt();
    return mModel.findInclude(id);
}

inline const IncludeEntry* SIInclude::findInclude(int row) const
{
    QTreeWidget* table = mList->ctrlTableList();
    if ((row < 0) || (row >= table->topLevelItemCount()))
        return nullptr;

    QTreeWidgetItem* item = table->topLevelItem(row);
    uint32_t id = item->data(COL_LOCATION, Qt::ItemDataRole::UserRole).toUInt();
    return mModel.findInclude(id);
}

inline void SIInclude::swapIncludes(int firstRow, int secondRow)
{
    QTreeWidget* table = mList->ctrlTableList();
    Q_ASSERT(firstRow >= 0 && firstRow < table->topLevelItemCount());
    Q_ASSERT(secondRow >= 0 && secondRow < table->topLevelItemCount());

    const IncludeEntry* first = findInclude(firstRow);
    const IncludeEntry* second = findInclude(secondRow);

    Q_ASSERT((first != nullptr) && (second != nullptr));
    setTexts(firstRow, *first);
    setTexts(secondRow, *second);
    table->setCurrentItem(table->topLevelItem(secondRow));
    updateToolBottons(secondRow, table->topLevelItemCount());
}

inline void SIInclude::updateToolBottons(int row, int rowCount)
{
    if ((row >= 0) && (row < rowCount))
    {
        if ((row == 0) && (rowCount == 1))
        {
            mList->ctrlButtonMoveUp()->setEnabled(false);
            mList->ctrlButtonMoveDown()->setEnabled(false);
        }
        else if (row == 0)
        {
            mList->ctrlButtonMoveUp()->setEnabled(false);
            mList->ctrlButtonMoveDown()->setEnabled(true);
        }
        else if (row == (rowCount - 1))
        {
            mList->ctrlButtonMoveUp()->setEnabled(true);
            mList->ctrlButtonMoveDown()->setEnabled(false);
        }
        else
        {
            mList->ctrlButtonMoveUp()->setEnabled(true);
            mList->ctrlButtonMoveDown()->setEnabled(true);
        }

        mList->ctrlButtonRemove()->setEnabled(true);
    }
    else
    {
        mList->ctrlButtonMoveUp()->setEnabled(false);
        mList->ctrlButtonMoveDown()->setEnabled(false);
        mList->ctrlButtonRemove()->setEnabled(false);
    }
}

inline QString SIInclude::genName()
{
    static const QString _defName("NewInclude");

    QTreeWidget* table = mList->ctrlTableList();
    QString name;
    do
    {
        name = _defName + QString::number(++mCount);
    } while (table->findItems(name, Qt::MatchFlag::MatchExactly, COL_LOCATION).isEmpty() == false);

    return name;
}
