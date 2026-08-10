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
#include "lusan/view/common/WidgetHighlight.hpp"
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include "lusan/app/LusanApplication.hpp"
#include "lusan/common/NELusanCommon.hpp"
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
    // A service interface is an API contract and a state machine is a behaviour, so `.fsml` is not
    // offered here. A data type belongs to neither and is shared.
    QStringList exts{};
    exts.append(LusanApplication::getExternalFileExtensions());
    exts.append(LusanApplication::buildFileFilter(tr("Service Interface Files")
                                                 , QStringList{ QStringLiteral("*.siml"), QStringLiteral("*.dtml") }));
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
    , mList     (new IncludeListView(IncludeTypeConfig{ QStringLiteral("siml"), tr("Service Interface"), tr("Service Interfaces")
                                                     , NELusanCommon::iconServiceInterface(NELusanCommon::SizeSmall) }, this))
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

void SIInclude::revealElement(uint32_t id, eIssueField field /*= eIssueField::None*/)
{
    selectById(id);
    switch (field)
    {
    case eIssueField::Name:         WidgetHighlight::reveal(mDetails->ctrlInclude());     break;
    case eIssueField::Description:  WidgetHighlight::reveal(mDetails->ctrlDescription()); break;
    default:                                                                              break;
    }
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
    QTreeWidgetItem* item = mList->itemAt(cell);
    return (item != nullptr ? item->text(cell.column()) : QString());
}

void SIInclude::onCurCellChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
    if (current == previous)
        return;

    blockBasicSignals(true);
    const uint32_t id = IncludeListView::rowId(current);
    IncludeEntry* entry = mModel.findInclude(id);
    updateDetails(entry, true);
    updateToolBottons(entry != nullptr ? modelIndexOf(id) : -1, static_cast<int>(mModel.getIncludes().size()));
    blockBasicSignals(false);
}

void SIInclude::onAddClicked()
{
    const QString location(genName());

    blockBasicSignals(true);
    IncludeEntry* entry = mModel.createInclude(location);
    if (entry != nullptr)
    {
        const uint32_t id = entry->getId();
        mDetails->ctrlInclude()->setEnabled(true);
        rebuildRows();
        selectById(id);
        updateDetails(mModel.findInclude(id), true);
        mDetails->ctrlInclude()->setFocus();
        mDetails->ctrlInclude()->selectAll();
        updateToolBottons(modelIndexOf(id), static_cast<int>(mModel.getIncludes().size()));
    }

    blockBasicSignals(false);
}

void SIInclude::onRemoveClicked()
{
    const uint32_t id = currentId();
    const int row = modelIndexOf(id);
    if (row < 0)
        return;

    blockBasicSignals(true);
    const QList<IncludeEntry>& list = mModel.getIncludes();
    const int nextRow = ((row + 1) == list.size()) ? (row - 1) : (row + 1);
    const uint32_t nextId = ((nextRow >= 0) && (nextRow < list.size())) ? list.at(nextRow).getId() : 0u;

    mModel.deleteInclude(id);
    rebuildRows();
    selectById(nextId);
    updateDetails(mModel.findInclude(currentId()), true);
    updateToolBottons(modelIndexOf(currentId()), static_cast<int>(mModel.getIncludes().size()));
    blockBasicSignals(false);
}

void SIInclude::onInsertClicked()
{
    const QString location(genName());

    blockBasicSignals(true);
    int row = modelIndexOf(currentId());
    row = (row < 0 ? 0 : row);
    IncludeEntry* entry = mModel.insertInclude(row, location);
    if (entry != nullptr)
    {
        // Inserting renumbers the IDs below it, so the rows are rebuilt rather than patched: the
        // row carrying an ID has to be the row the model says carries it.
        mDetails->ctrlInclude()->setEnabled(true);
        rebuildRows();
        const uint32_t id = mModel.getIncludes().at(row).getId();
        selectById(id);
        updateDetails(mModel.findInclude(id), true);
        mDetails->ctrlInclude()->setFocus();
        mDetails->ctrlInclude()->selectAll();
        updateToolBottons(row, static_cast<int>(mModel.getIncludes().size()));
    }

    blockBasicSignals(false);
}

void SIInclude::onMoveUpClicked()
{
    const int row = modelIndexOf(currentId());
    if (row > 0)
    {
        blockBasicSignals(true);
        // The swap keeps IDs attached to list positions, so afterwards the moved entry answers to
        // the neighbour's old ID. That is the row to reselect.
        const uint32_t neighborId = mModel.getIncludes().at(row - 1).getId();
        mModel.swapIncludes(mModel.getIncludes().at(row).getId(), neighborId);
        rebuildRows();
        selectById(neighborId);
        updateToolBottons(row - 1, static_cast<int>(mModel.getIncludes().size()));
        blockBasicSignals(false);
    }
}

void SIInclude::onMoveDownClicked()
{
    const int row = modelIndexOf(currentId());
    if ((row >= 0) && (row < (mModel.getIncludes().size() - 1)))
    {
        blockBasicSignals(true);
        const uint32_t neighborId = mModel.getIncludes().at(row + 1).getId();
        mModel.swapIncludes(mModel.getIncludes().at(row).getId(), neighborId);
        rebuildRows();
        selectById(neighborId);
        updateToolBottons(row + 1, static_cast<int>(mModel.getIncludes().size()));
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

        const QString location = dialog.getSelectedFileRelativePath();
        mDetails->ctrlInclude()->setText(location);
        mDetails->ctrlDescription()->setFocus();
        mDetails->ctrlDescription()->selectAll();

        const uint32_t id = currentId();
        IncludeEntry* entry = mModel.findInclude(id);
        if (entry != nullptr)
        {
            entry->setLocation(location);
            setTexts(mList->findRow(id), *entry);
            selectById(id);
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
    // Re-derives the Type, Name and Version columns from the current locations. Today it rebuilds
    // them from the live model; once includes are parsed, this is where disk is re-read.
    blockBasicSignals(true);
    const uint32_t id = currentId();
    rebuildRows();
    selectById(id);
    blockBasicSignals(false);
}

void SIInclude::onIncludeChanged(const QString& newText)
{
    const uint32_t id = currentId();
    IncludeEntry* entry = mModel.findInclude(id);
    if (entry != nullptr)
    {
        blockBasicSignals(true);
        entry->setLocation(newText);
        setTexts(mList->findRow(id), *entry);
        selectById(id);
        blockBasicSignals(false);
    }
}

void SIInclude::onDescriptionChanged()
{
    IncludeEntry* entry = mModel.findInclude(currentId());
    if (entry != nullptr)
    {
        entry->setDescription(mDetails->ctrlDescription()->toPlainText());
    }
}

void SIInclude::onDeprecatedChecked(bool isChecked)
{
    IncludeEntry* entry = mModel.findInclude(currentId());
    if (entry != nullptr)
    {
        SICommon::checkedDeprecated<IncludeDetailsView, IncludeEntry>(mDetails, entry, isChecked);
    }
}

void SIInclude::onDeprecateHint(const QString& newText)
{
    IncludeEntry* entry = mModel.findInclude(currentId());
    if (entry != nullptr)
    {
        SICommon::setDeprecateHint<IncludeDetailsView, IncludeEntry>(mDetails, entry, newText);
    }
}

void SIInclude::onEditorDataChanged(const QModelIndex &index, const QString &newValue)
{
    QTreeWidgetItem* item = mList->itemAt(index);
    const uint32_t id = IncludeListView::rowId(item);
    if ((id == 0) || (index.column() < 0))
        return;

    cellChanged(static_cast<int>(id), index.column(), newValue);
}

void SIInclude::cellChanged(int row, int col, const QString& newValue)
{
    IncludeEntry* entry = mModel.findInclude(static_cast<uint32_t>(row));
    if (entry == nullptr)
        return;

    if (col == COL_LOCATION)
    {
        if (mDetails->ctrlInclude()->text() != newValue)
        {
            blockBasicSignals(true);
            entry->setLocation(newValue);
            setTexts(mList->findRow(entry->getId()), *entry);
            selectById(entry->getId());
            updateDetails(entry, false);
            blockBasicSignals(false);
        }
    }
}

void SIInclude::updateData()
{
    rebuildRows();
    mList->ctrlTableList()->scrollToTop();
}

void SIInclude::updateWidgets()
{
    QTreeWidget* table = mList->ctrlTableList();
    mTableCell = new TableCell(QList<QAbstractItemModel*>(), QList<int>(), table, this, false);
    // Forbid invalid include path characters when editing the Location column inline, exactly
    // as the details panel's Include File field does.
    mTableCell->setColumnValidation(COL_LOCATION, TableCell::eCellValidation::Path);
    // Only Location is user-editable: Type and Name are derived and Version is read from disk, so
    // the delegate refuses an editor elsewhere. The item keeps the flag the Location editor needs.
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

inline void SIInclude::setTexts(QTreeWidgetItem* item, const IncludeEntry& entry)
{
    if (item == nullptr)
        return;

    const QString location = entry.getLocation();
    const eIncludeKind kind = mList->kindForLocation(location);
    mList->placeRow(item, kind, entry.getId());
    item->setIcon(COL_LOCATION, mList->iconForKind(kind));
    item->setText(COL_LOCATION, entry.getString(ElementBase::eDisplay::DisplayName));
    item->setText(COL_TYPE    , mList->typeForLocation(location));
    item->setText(COL_NAME    , mList->nameForLocation(location));
    item->setText(COL_VERSION , QString());
}

inline void SIInclude::rebuildRows()
{
    mList->clearRows();
    int counts[3]{ 0, 0, 0 };
    for (const IncludeEntry& entry : mModel.getIncludes())
    {
        const eIncludeKind kind = mList->kindForLocation(entry.getLocation());
        setTexts(mList->placeRow(nullptr, kind, entry.getId()), entry);
        ++counts[static_cast<int>(kind)];
    }

    mList->updateGroupCounts(counts[static_cast<int>(eIncludeKind::Source)]
                             , counts[static_cast<int>(eIncludeKind::DataType)]
                             , counts[static_cast<int>(eIncludeKind::Document)]);
}

inline uint32_t SIInclude::currentId() const
{
    return IncludeListView::rowId(mList->ctrlTableList()->currentItem());
}

inline int SIInclude::modelIndexOf(uint32_t id) const
{
    const QList<IncludeEntry>& list = mModel.getIncludes();
    for (int i = 0; i < list.size(); ++i)
    {
        if (list.at(i).getId() == id)
            return i;
    }

    return -1;
}

inline void SIInclude::selectById(uint32_t id)
{
    QTreeWidgetItem* item = mList->findRow(id);
    if (item != nullptr)
    {
        mList->ctrlTableList()->setCurrentItem(item);
    }
}

inline void SIInclude::updateDetails(const IncludeEntry* entry, bool updateAll /*= false*/)
{
    if (entry != nullptr)
    {
        mDetails->ctrlInclude()->setEnabled(true);
        mDetails->ctrlInclude()->setText(entry->getName());
        mDetails->ctrlBrowseButton()->setEnabled(true);
        mList->ctrlButtonRemove()->setEnabled(true);

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

inline void SIInclude::updateToolBottons(int row, int rowCount)
{
    if ((row >= 0) && (row < rowCount))
    {
        mList->ctrlButtonMoveUp()->setEnabled(row > 0);
        mList->ctrlButtonMoveDown()->setEnabled(row < (rowCount - 1));
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

    QString name;
    bool taken = true;
    while (taken)
    {
        name = _defName + QString::number(++mCount);
        taken = false;
        for (const IncludeEntry& entry : mModel.getIncludes())
        {
            if (entry.getLocation() == name)
            {
                taken = true;
                break;
            }
        }
    }

    return name;
}
