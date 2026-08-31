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
 *  \file        lusan/view/common/NaviFileSystem.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       The view of the workspace related file system.
 *
 ************************************************************************/

#include "lusan/view/common/NaviFileSystem.hpp"

#include "lusan/app/LusanApplication.hpp"
#include "lusan/common/NELusanCommon.hpp"
#include "lusan/model/common/FileSystemFilter.hpp"
#include "lusan/model/common/FileSystemModel.hpp"
#include "lusan/view/common/MdiMainWindow.hpp"
#include "lusan/view/common/TableCell.hpp"

#include <QAbstractItemView>
#include <QAction>
#include <QComboBox>
#include <QDir>
#include <QFontInfo>
#include <QIcon>
#include <QMessageBox>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QTreeView>
#include <QToolButton>

namespace
{
    //!< The longest description kept before the popup measures the text.
    constexpr int MaxDescriptionLength{ 160 };

    //!< The height of the line drawn between the workspaces and the commands.
    constexpr int SeparatorHeight{ 7 };

    //!< Holds the command of the entries that follow the workspace list.
    constexpr int CommandRole{ Qt::ItemDataRole::UserRole + 1 };

    //!< The entry switches to the workspace it carries.
    constexpr int CommandSwitch{ 0 };

    //!< The entry creates a new workspace.
    constexpr int CommandNew{ 1 };

    //!< The entry opens the workspace page of the options dialog.
    constexpr int CommandManage{ 2 };

    /**
     * \brief   Builds the font of the root directory line from the font of the workspace name.
     *          The directory stays one pixel below the name, and never grows above it.
     **/
    QFont directoryFont(const QFont& nameFont)
    {
        QFont result(nameFont);
        const int namePixels{ QFontInfo(nameFont).pixelSize() };
        result.setPixelSize(qBound(1, qRound(namePixels * 0.85) + 1, namePixels));
        return result;
    }

    /**
     * \brief   Draws a workspace of the selector on two rows: the name, and the root directory
     *          below it. Entries that carry no directory, such as the command entry, keep the
     *          plain one row drawing.
     **/
    class WorkspaceItemDelegate : public QStyledItemDelegate
    {
    public:
        explicit WorkspaceItemDelegate(QObject* parent)
            : QStyledItemDelegate(parent)
        {
        }

        void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
        {
            if (isSeparator(index))
            {
                painter->save();
                painter->setPen(option.palette.color(QPalette::ColorGroup::Normal, QPalette::ColorRole::Mid));
                const int line{ option.rect.center().y() };
                painter->drawLine(option.rect.left() + 6, line, option.rect.right() - 6, line);
                painter->restore();
                return;
            }

            const QString root{ index.data(Qt::ItemDataRole::UserRole).toString() };
            if (root.isEmpty())
            {
                QStyledItemDelegate::paint(painter, option, index);
                return;
            }

            QStyleOptionViewItem opt(option);
            initStyleOption(&opt, index);
            // The name starts where a plain entry puts its text, so both kinds of row line up.
            const QRect textRect{ opt.widget->style()->subElementRect(QStyle::SubElement::SE_ItemViewItemText, &opt, opt.widget) };
            opt.text.clear();
            opt.widget->style()->drawControl(QStyle::ControlElement::CE_ItemViewItem, &opt, painter, opt.widget);

            const bool selected{ (option.state & QStyle::StateFlag::State_Selected) != 0 };
            const QPalette::ColorRole nameRole{ selected ? QPalette::ColorRole::HighlightedText : QPalette::ColorRole::Text };

            const QFont pathFont{ directoryFont(option.font) };
            const QFontMetrics nameMetrics(option.font);
            const QFontMetrics pathMetrics(pathFont);
            const QRect area{ textRect.left(), option.rect.top() + 3, option.rect.right() - textRect.left() - 6, option.rect.height() - 6 };
            const QRect nameRect{ area.left(), area.top(), area.width(), nameMetrics.height() };
            const QRect pathRect{ area.left(), nameRect.bottom(), area.width(), pathMetrics.height() };

            painter->save();
            painter->setFont(option.font);
            painter->setPen(option.palette.color(QPalette::ColorGroup::Normal, nameRole));
            painter->drawText(nameRect, Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignVCenter, index.data(Qt::ItemDataRole::DisplayRole).toString());

            painter->setFont(pathFont);
            QColor pathColor{ option.palette.color(QPalette::ColorGroup::Normal, nameRole) };
            pathColor.setAlpha(selected ? 200 : 150);
            painter->setPen(pathColor);
            // The tail of a path identifies the workspace, so the middle gives way first.
            painter->drawText(pathRect, Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignVCenter, pathMetrics.elidedText(root, Qt::TextElideMode::ElideMiddle, pathRect.width()));
            painter->restore();
        }

        QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override
        {
            QSize result{ QStyledItemDelegate::sizeHint(option, index) };
            if (isSeparator(index))
            {
                result.setHeight(SeparatorHeight);
            }
            else if (index.data(Qt::ItemDataRole::UserRole).toString().isEmpty() == false)
            {
                const int rows{ QFontMetrics(option.font).height() + QFontMetrics(directoryFont(option.font)).height() + 8 };
                result.setHeight(std::max(result.height(), rows));
            }

            return result;
        }

    private:
        //!< True for the line the selector puts between the workspaces and the commands.
        static bool isSeparator(const QModelIndex& index)
        {
            return index.data(Qt::ItemDataRole::AccessibleDescriptionRole).toString() == QLatin1String("separator");
        }
    };
}

NaviFileSystem::NaviFileSystem(MdiMainWindow* wndMain, QWidget* parent /*= nullptr*/)
    : NaviToolbarWindow(static_cast<int>(NavigationDock::eNaviWindow::NaviWorkspace), wndMain, parent)
    , IETableHelper ()

    , mNaviModel    (new FileSystemModel())
    , mGenModel     (nullptr)
    , mFileFilter   (nullptr)
    , mRootPaths    ( )
    , mTableCell    (nullptr)
    , mWorkspaces   (nullptr)
    , mToolRefresh  (nullptr)
    , mToolShowAll  (nullptr)
    , mToolCollapse (nullptr)
    , mToolNaviRoot (nullptr)
    , mToolOpen     (nullptr)
    , mToolNewFolder(nullptr)
    , mToolNewFile  (nullptr)
    , mToolEdit     (nullptr)
    , mToolDelete   (nullptr)
{
    setupWorkspaceSelector();
    setupToolbar();
    updateData();
    setupWidgets();
    setupSignals();
}

void NaviFileSystem::setupWorkspaceSelector()
{
    mWorkspaces = new QComboBox(this);
    mWorkspaces->setObjectName(QStringLiteral("naviWorkspaceSelector"));
    mWorkspaces->setToolTip(tr("Active workspace"));
    mWorkspaces->setStatusTip(tr("Switch to another workspace"));
    mWorkspaces->setAccessibleName(tr("Active workspace"));
    mWorkspaces->setSizeAdjustPolicy(QComboBox::SizeAdjustPolicy::AdjustToMinimumContentsLengthWithIcon);
    mWorkspaces->setMinimumContentsLength(12);
    mWorkspaces->setItemDelegate(new WorkspaceItemDelegate(mWorkspaces));
    // The closed selector is one text line high, the popup rows keep their own height.
    const int lineHeight{ QFontMetrics(mWorkspaces->font()).height() };
    mWorkspaces->setIconSize(QSize(lineHeight - 2, lineHeight - 2));
    mWorkspaces->setFixedHeight(NaviToolbarWindow::naviInputHeight(*mWorkspaces));
    // The root directory is elided to the width there is, so a sideways scrollbar would only
    // take the height the last entries need.
    mWorkspaces->view()->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    // Under the tool row, not above it: the tool row is the first line of every navigation
    // panel, so it does not move when the user switches from one panel to another.
    addNaviBar(mWorkspaces);
    populateWorkspaces();
}

void NaviFileSystem::populateWorkspaces()
{
    const QString active{ LusanApplication::getActiveWorkspace().getWorkspaceRoot() };

    const QSize iconSize{ mWorkspaces->iconSize() };
    const QIcon iconEntry{ NELusanCommon::iconWorkspace(iconSize) };

    mWorkspaces->blockSignals(true);
    mWorkspaces->clear();
    int widest{ 0 };
    const QFontMetrics metrics{ mWorkspaces->font() };
    for (const WorkspaceEntry& entry : LusanApplication::getOptions().getWorkspaceList())
    {
        const QString root{ entry.getWorkspaceRoot() };
        mWorkspaces->addItem(iconEntry, entry.getWorkspaceName(), root);

        QString tip{ root };
        QString describe{ entry.getWorkspaceDescription().section('\n', 0, 0).trimmed() };
        if (describe.isEmpty() == false)
        {
            if (describe.length() > MaxDescriptionLength)
            {
                describe = describe.left(MaxDescriptionLength) + "...";
            }

            tip += "\n" + describe;
        }

        mWorkspaces->setItemData(mWorkspaces->count() - 1, tip, Qt::ItemDataRole::ToolTipRole);
        widest = std::max(widest, metrics.horizontalAdvance(root));
        if (root == active)
        {
            mWorkspaces->setCurrentIndex(mWorkspaces->count() - 1);
        }
    }

    mWorkspaces->insertSeparator(mWorkspaces->count());

    mWorkspaces->addItem(NELusanCommon::iconNewWorkspace(iconSize), tr("New Workspace"), QString());
    mWorkspaces->setItemData(mWorkspaces->count() - 1, CommandNew, CommandRole);
    mWorkspaces->setItemData(mWorkspaces->count() - 1, tr("Create a new workspace, restarts application"), Qt::ItemDataRole::ToolTipRole);

    mWorkspaces->addItem(NELusanCommon::iconManageWorkspaces(iconSize), tr("Manage Workspaces..."), QString());
    mWorkspaces->setItemData(mWorkspaces->count() - 1, CommandManage, CommandRole);
    mWorkspaces->setItemData(mWorkspaces->count() - 1, tr("Add, edit or remove workspaces"), Qt::ItemDataRole::ToolTipRole);

    mWorkspaces->blockSignals(false);

    // The popup is a window of its own, so the paths stay readable in a narrow dock.
    mWorkspaces->view()->setMinimumWidth(std::min(widest + 48 + iconSize.width(), 640));
}

void NaviFileSystem::restoreWorkspaceSelection()
{
    const QString active{ LusanApplication::getActiveWorkspace().getWorkspaceRoot() };
    const int index = mWorkspaces->findData(active);
    mWorkspaces->blockSignals(true);
    mWorkspaces->setCurrentIndex(index >= 0 ? index : 0);
    mWorkspaces->blockSignals(false);
}

void NaviFileSystem::onWorkspaceSelected(int index)
{
    if (index < 0)
        return;

    const int command{ mWorkspaces->itemData(index, CommandRole).toInt() };
    if (command != CommandSwitch)
    {
        restoreWorkspaceSelection();
        if (command == CommandNew)
        {
            mMainWindow->actionNewWorkspace().trigger();
        }
        else
        {
            mMainWindow->showOptionPageWorkspace();
        }

        return;
    }

    const QString root{ mWorkspaces->itemData(index).toString() };
    if (root.isEmpty())
    {
        restoreWorkspaceSelection();
    }
    else if (LusanApplication::switchWorkspace(root) == false)
    {
        restoreWorkspaceSelection();
        QMessageBox::information( this
                                , tr("Switch Workspace") + " - Lusan"
                                , tr("The workspace was not switched. Either its directory is gone, or a document is still open."));
    }
}

void NaviFileSystem::setupToolbar()
{
    mToolRefresh = addToolButton( NELusanCommon::iconRefresh(NELusanCommon::SizeBig)
                                , tr("Refresh file system view")
                                , tr("Refresh file system view"));

    mToolShowAll = addToolButton( NELusanCommon::iconShowAll(NELusanCommon::SizeBig)
                                , tr("Show / Hide all files")
                                , tr("Show / Hide all files")
                                , true);
    mToolShowAll->setStyleSheet(NELusanCommon::getStyleToolbutton());

    mToolCollapse = addToolButton( NELusanCommon::iconNodeExpanded(NELusanCommon::SizeBig)
                                 , tr("Collapse all folders")
                                 , tr("Collapse all folders"));

    addToolSeparator();

    mToolNaviRoot = addToolButton( NELusanCommon::iconComputer(NELusanCommon::SizeBig)
                                 , tr("Show computer file system")
                                 , tr("Show computer file system")
                                 , true);
    mToolNaviRoot->setStyleSheet(NELusanCommon::getStyleToolbutton());

    addToolSeparator();

    mToolOpen = addToolButton( NELusanCommon::iconOpenFile(NELusanCommon::SizeBig)
                             , tr("Open selected file")
                             , tr("Open selected file"));

    mToolNewFolder = addToolButton( NELusanCommon::iconNewFolder(NELusanCommon::SizeBig)
                                  , tr("Add new folder")
                                  , tr("Add new folder"));
    mToolNewFolder->setWhatsThis(tr("Add new folder"));

    mToolNewFile = addToolButton( NELusanCommon::iconNewFile(NELusanCommon::SizeBig)
                                , tr("Add new file")
                                , tr("Add new file"));

    mToolEdit = addToolButton( NELusanCommon::iconRename(NELusanCommon::SizeBig)
                             , tr("Rename selected file / folder")
                             , tr("Rename selected file / folder"));

    mToolDelete = addToolButton( NELusanCommon::iconDelete(NELusanCommon::SizeBig)
                               , tr("Delete selected file / folder")
                               , tr("Delete selected file / folder"));

    setupTreeView(NELusanCommon::SizeSmall);
    ctrlTable()->setWordWrap(true);
}

int NaviFileSystem::getColumnCount() const
{
    return 1;
}

QString NaviFileSystem::getCellText(const QModelIndex& cell) const
{
    return (mNaviModel != nullptr ? mNaviModel->getFileInfo(cell).fileName() : QString());
}

void NaviFileSystem::optionApplied()
{
    NavigationWindow::optionApplied();
}

void NaviFileSystem::optionClosed(bool OKpressed)
{
    NavigationWindow::optionClosed(OKpressed);
}

void NaviFileSystem::onToolRefreshClicked(bool checked)
{
    QTreeView * table = ctrlTable();
    if (mNaviModel != nullptr)
    {
        table->collapseAll();
        table->clearSelection();
        mNaviModel->refresh();
        QModelIndex idxRoot = mNaviModel->getRootIndex();
        table->setRootIndex(idxRoot);
    }
    else if (mGenModel != nullptr)
    {
        table->collapseAll();
        table->clearSelection();
        table->reset();
    }
}

void NaviFileSystem::onToolShowAllToggled(bool checked)
{
    if (mNaviModel == nullptr)
        return;

    QTreeView * table = ctrlTable();
    table->collapseAll();
    table->clearSelection();
    QStringList filters{ LusanApplication::InternalExts };
    filters.append(LusanApplication::ExternalExts);
    mNaviModel->setFileFilter(checked ? QStringList() : filters);
    mNaviModel->refresh();
    QModelIndex idxRoot = mNaviModel->getRootIndex();
    table->setRootIndex(idxRoot);
    ctrlToolShowAll()->setChecked(checked);
}

void NaviFileSystem::onToolCollapseAllClicked(bool checked)
{
    ctrlTable()->collapseAll();
}

void NaviFileSystem::onToolNewFolderClicked(bool checked)
{
    static QString _defName("NewFolder");
    if (mNaviModel == nullptr)
        return;

    QTreeView* table = ctrlTable();
    QModelIndex index = table->selectionModel()->currentIndex();
    if (mNaviModel->isFile(index))
        index = mNaviModel->parent(index);

    uint32_t count{ 1 };
    QString name;
    do
    {
        name = _defName + QString::number(count ++);
    } while(mNaviModel->existsDirectory(index, name));
    
    QModelIndex newIndex = mNaviModel->insertDirectory(name, index);
    if (newIndex.isValid())
    {
        table->setCurrentIndex(newIndex);
        table->edit(newIndex);
    }
}

void NaviFileSystem::onToolNewFileClicked(bool checked)
{
    static QString _defName("NewStateMachine");
    static QString _defExt(".fsml");

    if (mNaviModel == nullptr)
        return;

    QTreeView* table = ctrlTable();
    QModelIndex index = table->currentIndex();
    if (mNaviModel->isFile(index))
        index = mNaviModel->parent(index);

    uint32_t count{ 1 };
    QString name;
    do
    {
        name = _defName + QString::number(count ++) + _defExt;
    } while(mNaviModel->existsDirectory(index, name));
    
    QModelIndex newIndex = mNaviModel->insertFile(name, index);
    if (newIndex.isValid())
    {
        table->setCurrentIndex(newIndex);
        table->edit(newIndex);
    }
}

void NaviFileSystem::onToolOpenSelectedClicked(bool checked)
{
    QTreeView * table = ctrlTable();
    QModelIndex index = table->selectionModel()->currentIndex();
    QFileInfo fi (getFileInfo(index));

    QString filePath = fi.isFile() ? fi.filePath() : "";
    if (filePath.isEmpty() == false)
    {
        mMainWindow->openFile(filePath);
    }
}

void NaviFileSystem::onToolEditSelectedClicked(bool checked)
{
    QTreeView * table = ctrlTable();
    QModelIndex index = table->selectionModel()->currentIndex();
    if (index.isValid())
    {
        table->edit(index);
    }
}

void NaviFileSystem::onToolDeleteSelectedClicked(bool checked)
{
    QTreeView * table = ctrlTable();
    QModelIndex index = table->selectionModel()->currentIndex();
    QFileInfo fi = mNaviModel->getFileInfo(index);
    QString filePath = fi.filePath();
    if (filePath.isEmpty() == false)
    {
        QModelIndex parent = index.parent();
        int result = QMessageBox::question(   mMainWindow
                                            , tr("Delete File") + " - Lusan"
                                            , tr("Are you sure you want to delete ") + (fi.isDir() ? tr("directory") : tr("file")) + "\n" + filePath
                                            , QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Cancel
                                            , QMessageBox::StandardButton::Cancel);
        
        if ((result == QMessageBox::StandardButton::Ok) && mNaviModel->deleteEntry(index))
        {
            Q_ASSERT(parent.isValid());
            int rowCount = mNaviModel->rowCount(parent);
            if (rowCount == 0)
            {
                index = parent;
            }
            else if (index.row() >= rowCount)
            {
                index = mNaviModel->index(rowCount - 1, 0, parent);
            }
            else
            {
                index  = mNaviModel->index(index.row(), 0, parent);
            }
            
            table->setCurrentIndex(index);
        }
    }
}

void NaviFileSystem::onToolNaviRootClicked(bool checked)
{
    if ((checked == true) && (mGenModel == nullptr))
    {
        disconnect(ctrlTable()->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &NaviFileSystem::onTreeSelectinoRowChanged);
        mGenModel = new GeneralFileSystemModel();
        mGenModel->setReadOnly(true);
        if (mFileFilter == nullptr)
        {
            mFileFilter = new FileSystemFilter(mGenModel, this);
        }
        else
        {
            mFileFilter->setSourceModel(nullptr);
        }

        ctrlTable()->setModel(nullptr);
        ctrlTable()->setModel(mFileFilter);
        ctrlTable()->setSortingEnabled(true);
        ctrlTable()->reset();
        delete mNaviModel;
        mNaviModel = nullptr;

        // QString rootPath = QDir::rootPath();
        QString rootPath = mGenModel->myComputer().toString();
        QModelIndex idxRoot = mGenModel->setRootPath(rootPath);
        ctrlTable()->setRootIndex(mFileFilter->mapFromSource(idxRoot));

        ctrlToolDelete()->setEnabled(false);
        ctrlToolNewFile()->setEnabled(false);
        ctrlToolNewFolder()->setEnabled(false);
        ctrlToolOpen()->setEnabled(true);
        ctrlToolEdit()->setEnabled(false);
    }
    else if ((checked == false) && (mNaviModel == nullptr))
    {
        mNaviModel = new FileSystemModel();
        updateData();
        setupWidgets();

        mFileFilter->setSourceModel(nullptr);
        delete mFileFilter;
        delete mGenModel;

        mFileFilter = nullptr;
        mGenModel = nullptr;

        connect(ctrlTable()->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &NaviFileSystem::onTreeSelectinoRowChanged);
    }
}

void NaviFileSystem::onTreeViewOpenRequested(const QModelIndex &index)
{
    if (index.isValid() == false)
        return;

    QFileInfo fi (getFileInfo(index));
    QString filePath = fi.isDir() ? "" : fi.filePath();
    if (filePath.isEmpty() == false)
    {
        mMainWindow->openFile(filePath);
    }
}

void NaviFileSystem::updateToolButtons(const QModelIndex &index)
{
    bool enable = (mNaviModel != nullptr) && (mNaviModel->isRoot(index) == false) && index.isValid();
    ctrlToolDelete()->setEnabled(enable && (mNaviModel->isWorkspaceEntry(index) == false));
    ctrlToolNewFile()->setEnabled(enable);
    ctrlToolNewFolder()->setEnabled(enable);
    ctrlToolOpen()->setEnabled(enable && mNaviModel->isFile(index));
    ctrlToolEdit()->setEnabled(enable && (mNaviModel->isWorkspaceEntry(index) == false));
}

void NaviFileSystem::onTreeSelectinoRowChanged(const QModelIndex &current, const QModelIndex &previous)
{
    updateToolButtons(current);
}

void NaviFileSystem::onEditorDataChanged(const QModelIndex& index, const QString& newValue)
{
    if ((index.isValid() == false) || (mNaviModel == nullptr))
        return;

    QTreeView * table = ctrlTable();
    QModelIndex newIndex = mNaviModel->renameEntry(newValue, index);
    if (newIndex.isValid())
    {
        table->setCurrentIndex(newIndex);
    }
}

void NaviFileSystem::updateData()
{
    mRootPaths = setupRootPaths(LusanApplication::getOptions().getActiveWorkspace());
    QStringList filters{ LusanApplication::InternalExts };
    filters.append(LusanApplication::ExternalExts);
    mNaviModel->setFileFilter(filters);
}

void NaviFileSystem::setupWidgets()
{
    QModelIndex idxRoot = mNaviModel->setRootPaths(mRootPaths);
    mTableCell = new TableCell(ctrlTable(), this, true);
    ctrlTable()->setModel(mNaviModel);
    ctrlTable()->setRootIndex(idxRoot);
    ctrlTable()->expand(idxRoot);
    ctrlTable()->setSortingEnabled(true);
    ctrlTable()->setAlternatingRowColors(false);
    ctrlTable()->setContextMenuPolicy(Qt::CustomContextMenu);
    ctrlTable()->setItemDelegateForColumn(0, mTableCell);
    
    ctrlToolShowAll()->setCheckable(true);
    ctrlToolDelete()->setEnabled(false);
    ctrlToolNewFile()->setEnabled(false);
    ctrlToolNewFolder()->setEnabled(false);
    ctrlToolOpen()->setEnabled(false);
    ctrlToolEdit()->setEnabled(false);
}

void NaviFileSystem::setupSignals()
{
    connect(ctrlToolRefresh()       , &QToolButton::clicked,      this, &NaviFileSystem::onToolRefreshClicked);
    connect(ctrlToolShowAll()       , &QToolButton::toggled,      this, &NaviFileSystem::onToolShowAllToggled);
    connect(ctrlToolCollapse()      , &QToolButton::clicked,      this, &NaviFileSystem::onToolCollapseAllClicked);
    connect(ctrlToolNewFolder()     , &QToolButton::clicked,      this, &NaviFileSystem::onToolNewFolderClicked);
    connect(ctrlToolNewFile()       , &QToolButton::clicked,      this, &NaviFileSystem::onToolNewFileClicked);
    connect(ctrlToolEdit()          , &QToolButton::clicked,      this, &NaviFileSystem::onToolEditSelectedClicked);
    connect(ctrlToolOpen()          , &QToolButton::clicked,      this, &NaviFileSystem::onToolOpenSelectedClicked);
    connect(ctrlToolDelete()        , &QToolButton::clicked,      this, &NaviFileSystem::onToolDeleteSelectedClicked);
    connect(ctrlToolNaviRoot()      , &QToolButton::clicked,      this, &NaviFileSystem::onToolNaviRootClicked);
    // `activated` rather than `doubleClicked`: it covers both the double-click and Enter, and it
    // fires once per gesture, so a file that fails to open cannot report its failure twice.
    connect(ctrlTable()             , &QTreeView::activated,      this, &NaviFileSystem::onTreeViewOpenRequested);
    connect(ctrlTable()             , &QTreeView::entered,        this, &NaviFileSystem::updateToolButtons);
    connect(ctrlTable()->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &NaviFileSystem::onTreeSelectinoRowChanged);

    connect(mWorkspaces, &QComboBox::activated, this, &NaviFileSystem::onWorkspaceSelected);

    connect(mTableCell, &TableCell::signalEditorDataChanged, this, &NaviFileSystem::onEditorDataChanged);

    connect(&LusanApplication::getOptions(), &OptionsManager::signalWorkspaceDirectoriesChanged, this, &NaviFileSystem::onWorkspaceDirectoriesChanged);
}

void NaviFileSystem::blockBasicSignals(bool block)
{
    ctrlTable()->blockSignals(block);
}

QFileInfo NaviFileSystem::getFileInfo(const QModelIndex & index) const
{
    if (mNaviModel != nullptr)
    {
        return mNaviModel->getFileInfo(index);
    }
    else
    {
        Q_ASSERT(mGenModel != nullptr);
        Q_ASSERT(mFileFilter != nullptr);
        QModelIndex src = mFileFilter->mapToSource(index);
        return mGenModel->fileInfo(src);
    }
}

WorkspaceElem NaviFileSystem::setupRootPaths(const WorkspaceEntry& workspace)
{
    WorkspaceElem result;
    
    QString root    { NELusanCommon::fixPath(workspace.getWorkspaceRoot()) };
    QString sources { NELusanCommon::fixPath(workspace.getDirSources()) };
    QString includes{ NELusanCommon::fixPath(workspace.getDirIncludes()) };
    QString delivery{ NELusanCommon::fixPath(workspace.getDirDelivery()) };
    QString logs    { NELusanCommon::fixPath(workspace.getDirLogs()) };

    // Without an active workspace there is no root to show. The tree stays empty then,
    // instead of drawing a workspace node that names no folder.
    if (root.isEmpty())
        return result;

    result[eWorkspaceElem::WorkspaceRoot] = {root, "[Workspace: " + root + "]"};
    if (!sources.isEmpty())
    {
        result[eWorkspaceElem::WorkspaceSources]    = {sources, "[Sources: " + sources + "]"};
    }

    if (!includes.isEmpty())
    {
        result[eWorkspaceElem::WorkspaceIncludes]   = {includes, "[Includes: " + includes + "]"};
    }

    if (!delivery.isEmpty())
    {
        result[eWorkspaceElem::WorkspaceDelivery]   = {delivery, "[Delivery: " + delivery + "]"};
    }

    if (!logs.isEmpty())
    {
        result[eWorkspaceElem::WorkspaceLogs]       = {logs, "[Logs: " + logs + "]"};
    }
    
    return result;
}

void NaviFileSystem::onWorkspaceDirectoriesChanged(const WorkspaceEntry& workspace, bool isActiveWorkspace)
{
    if (isActiveWorkspace == false)
        return;
    
    WorkspaceElem paths = setupRootPaths(workspace);
    if ((mNaviModel != nullptr) && mNaviModel->updateRootPaths(paths))
    {
        mRootPaths = paths;
        QTreeView* table = ctrlTable();
        table->collapseAll();
        table->clearSelection();
        mNaviModel->refresh();
        QModelIndex idxRoot = mNaviModel->getRootIndex();
        table->setRootIndex(idxRoot);
        table->expand(idxRoot);
        table->setSortingEnabled(true);
        ctrlToolShowAll()->setCheckable(true);
    }
}
