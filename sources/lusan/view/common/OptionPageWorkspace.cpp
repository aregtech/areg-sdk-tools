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
 *  \file        lusan/view/common/OptionPageWorkspace.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Tamas Csillag
 *  \brief       Lusan application, workspace manager widget.
 *
 ************************************************************************/

#include "lusan/view/common/OptionPageWorkspace.hpp"

#include "lusan/app/LusanApplication.hpp"
#include "lusan/app/NEAppThemes.hpp"
#include "lusan/data/common/OptionsManager.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>
#include <algorithm>

namespace
{
    //!< The widest workspace name that still reads well in the workspace lists.
    constexpr int MaxNameLength{ 48 };
}

OptionPageWorkspace::OptionPageWorkspace(QDialog* parent)
    : OptionPageBase        (parent)
    , mModifiedWorkspaces   ( )
    , mList                 ( nullptr )
    , mName                 ( nullptr )
    , mRootDir              ( nullptr )
    , mSourceDir            ( nullptr )
    , mIncludeDir           ( nullptr )
    , mDeliveryDir          ( nullptr )
    , mLogDir               ( nullptr )
    , mDefault              ( nullptr )
    , mDescription          ( nullptr )
    , mDelete               ( nullptr )
    , mHint                 ( nullptr )
    , mSources              ( )
    , mIncludes             ( )
    , mDelivery             ( )
    , mLogs                 ( )
    , mThemeCombo           ( nullptr )
    , mThemeLabel           ( nullptr )
    , mInitialTheme         ( static_cast<int>(OptionsManager::eAppTheme::SystemDefault) )
{
    setupWidgets();
    setupUi();
    connectSignalHandlers();
}

OptionPageWorkspace::~OptionPageWorkspace()
{
}

void OptionPageWorkspace::setupWidgets()
{
    setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(8);

    QLabel* title = new QLabel(tr("List of Setup Workspaces"), this);
    QFont titleFont{ title->font() };
    titleFont.setBold(true);
    title->setFont(titleFont);
    layout->addWidget(title, 0);

    QSplitter* splitter = new QSplitter(Qt::Orientation::Horizontal, this);

    mList = new QListWidget(splitter);
    mList->setToolTip(tr("List of workspaces"));
    mList->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
    mList->setIconSize(NELusanCommon::SizeSmall);
    splitter->addWidget(mList);

    QWidget* details = new QWidget(splitter);
    QFormLayout* form = new QFormLayout(details);
    form->setContentsMargins(10, 0, 0, 0);
    form->setFieldGrowthPolicy(QFormLayout::FieldGrowthPolicy::AllNonFixedFieldsGrow);
    form->setLabelAlignment(Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);

    mName = new QLineEdit(details);
    mName->setMaxLength(MaxNameLength);
    mName->setToolTip(tr("The name of this workspace. It must differ from the other workspace names."));
    form->addRow(tr("&Name:"), mName);

    auto addPathRow = [details, form](const QString& label, const QString& tip, const QString& empty) -> QLineEdit*
    {
        QLineEdit* edit = new QLineEdit(details);
        edit->setReadOnly(true);
        edit->setToolTip(tip);
        edit->setPlaceholderText(empty);
        form->addRow(label, edit);
        return edit;
    };

    mRootDir     = addPathRow(tr("Root Directory:")    , tr("Root directory of the selected workspace")                       , tr("The root directory path is empty"));
    mSourceDir   = addPathRow(tr("Source Directory:")  , tr("The source codes directory path of the selected workspace")      , tr("The source codes directory path is empty"));
    mIncludeDir  = addPathRow(tr("Include Directory:") , tr("The include files directory path of the selected workspace")     , tr("The include directory path is empty"));
    mDeliveryDir = addPathRow(tr("Delivery Directory:"), tr("The thirdparty delivery directory path of the selected workspace"), tr("The thirdparty delivery directory path is empty"));
    mLogDir      = addPathRow(tr("Log Directory:")     , tr("The log files directory path of the selected workspace")         , tr("The logs directory path is empty"));

    mDefault = new QCheckBox(details);
    mDefault->setToolTip(tr("Open this workspace at start and do not ask again"));
    form->addRow(tr("Is default workspace?"), mDefault);

    mDescription = new QPlainTextEdit(details);
    mDescription->setPlaceholderText(tr("Add workspace description here"));
    form->addRow(tr("Description:"), mDescription);

    mThemeLabel = new QLabel(tr("Application Theme:"), details);
    mThemeCombo = new QComboBox(details);
    mThemeCombo->setToolTip(tr("Select the application visual style"));
    form->addRow(mThemeLabel, mThemeCombo);

    splitter->addWidget(details);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setChildrenCollapsible(false);
    layout->addWidget(splitter, 1);

    mHint = new QLabel(this);
    mHint->setWordWrap(true);
    mHint->setTextFormat(Qt::TextFormat::PlainText);
    layout->addWidget(mHint, 0);

    QHBoxLayout* buttons = new QHBoxLayout();
    buttons->addStretch(1);
    mDelete = new QPushButton(tr("Delete"), this);
    mDelete->setToolTip(tr("Delete currently selected not active workspace settings."));
    buttons->addWidget(mDelete);
    layout->addLayout(buttons, 0);

    setTabOrder(mList, mName);
    setTabOrder(mName, mDescription);
    setTabOrder(mDescription, mDefault);
    setTabOrder(mDefault, mThemeCombo);
    setTabOrder(mThemeCombo, mDelete);
}

void OptionPageWorkspace::connectSignalHandlers()
{
    connect(mDelete    , &QPushButton::clicked             , this, &OptionPageWorkspace::onDeleteButtonClicked);
    connect(mList, &QListWidget::itemSelectionChanged, this, &OptionPageWorkspace::onWorkspaceSelectionChanged);
    connect(mDescription   , &QPlainTextEdit::textChanged      , this, &OptionPageWorkspace::onWorkspaceDescChanged);
    connect(mName            , &QLineEdit::textEdited            , this, &OptionPageWorkspace::onWorkspaceNameChanged);
    connect(mDefault    , &QCheckBox::clicked               , this, &OptionPageWorkspace::onDefaultChecked);
    connect(mThemeCombo          , qOverload<int>(&QComboBox::currentIndexChanged), this, &OptionPageWorkspace::onThemeChanged);
}

void OptionPageWorkspace::initializePathsWithSelectedWorkspaceData(uint32_t const workspaceId) const
{
    std::optional<WorkspaceEntry> const workspace{ getWorkspace(workspaceId) };
    if (!workspace)
        return;
    
    OptionsManager& opt { LusanApplication::getOptions() };
    bool isDefault = opt.isDefaultWorkspace(workspace->getId());
    mDefault->setEnabled(true);
    mDefault->setChecked(isDefault);
    mDescription->setPlainText(workspace->getWorkspaceDescription());
    mName->blockSignals(true);
    mName->setText(workspace->getWorkspaceName());
    mName->blockSignals(false);
    mHint->clear();
    
    ctrlRoot()->setText(workspace->getWorkspaceRoot());
    if (opt.isActiveWorkspace(workspace->getId()))
    {
        ctrlSources()->setText(mSources);
        ctrlIncludes()->setText(mIncludes);
        ctrlDelivery()->setText(mDelivery);
        ctrlLogs()->setText(mLogs);
    }
    else
    {
        ctrlSources()->setText(workspace->getDirSources());
        ctrlIncludes()->setText(workspace->getDirIncludes());
        ctrlDelivery()->setText(workspace->getDirDelivery());
        ctrlLogs()->setText(workspace->getDirLogs());
    }
}

void OptionPageWorkspace::updateWorkspaceDirectories(  const sWorkspaceDir& sources
                                                     , const sWorkspaceDir& includes
                                                     , const sWorkspaceDir& delivery
                                                     , const sWorkspaceDir& logs)
{
    if (sources.isValid)
        mSources = sources.location;
    if (includes.isValid)
        mIncludes = includes.location;
    if (delivery.isValid)
        mDelivery = delivery.location;
    if (logs.isValid)
        mLogs = logs.location;
    
    QListWidgetItem* selectedItem = mList->currentItem();
    if (nullptr == selectedItem)
        return;
    
    uint32_t const selectedWorkspaceId{ selectedItem->data(Qt::ItemDataRole::UserRole).toUInt() };
    if (LusanApplication::getOptions().isActiveWorkspace(selectedWorkspaceId))
    {
        ctrlSources()->setText(mSources);
        ctrlIncludes()->setText(mIncludes);
        ctrlDelivery()->setText(mDelivery);
        ctrlLogs()->setText(mLogs);
    }
}

void OptionPageWorkspace::populateListOfWorkspaces()
{
    WorkspaceEntry const currentWorkspace{ LusanApplication::getActiveWorkspace() };
    std::vector<WorkspaceEntry> const& workspaces { LusanApplication::getOptions().getWorkspaceList() };
    
    mSources = currentWorkspace.getDirSources();
    mIncludes = currentWorkspace.getDirIncludes();
    mDelivery = currentWorkspace.getDirDelivery();
    mLogs = currentWorkspace.getDirLogs();
    
    QListWidget* list = mList;
    list->clear();

    for (WorkspaceEntry const& workspace : workspaces)
    {
        uint32_t wsId = workspace.getId();
        QListWidgetItem* item = new QListWidgetItem(NELusanCommon::iconWorkspaceOpen(NELusanCommon::SizeSmall), workspace.getWorkspaceName(), list);
        item->setData(Qt::ItemDataRole::UserRole, wsId);
        item->setToolTip(workspace.getWorkspaceRoot());

        if (currentWorkspace.getId() == wsId)
        {
            QBrush const grayBrush{ QColor(Qt::gray) };
            item->setForeground(grayBrush);
        }
        
        list->addItem(item);
    }

    mList->sortItems();
}

void OptionPageWorkspace::onDeleteButtonClicked()
{
    QListWidgetItem* selectedItem = mList->currentItem();
    if (nullptr == selectedItem)
        return;

    uint32_t const selectedWorkspaceId{ selectedItem->data(Qt::ItemDataRole::UserRole).toUInt() };
    if (LusanApplication::getOptions().isActiveWorkspace(selectedWorkspaceId))
    {
        if ( LusanApplication::getOptions().isDefaultWorkspace(selectedWorkspaceId) )
        {
            LusanApplication::getOptions().setDefaultWorkspace(static_cast<uint32_t>(0));
            mDefault->setChecked(false);
        }
        
        mModifiedWorkspaces[selectedWorkspaceId] = WorkspaceChangeData{ true, {} };
        deleteSelectedWorkspaceItem();
    }
}

void OptionPageWorkspace::deleteSelectedWorkspaceItem() const
{
    disconnect(mDescription, &QPlainTextEdit::textChanged, this, &OptionPageWorkspace::onWorkspaceDescChanged);

    delete mList->takeItem(mList->currentRow());

    connect(mDescription, &QPlainTextEdit::textChanged, this, &OptionPageWorkspace::onWorkspaceDescChanged);
}

void OptionPageWorkspace::onWorkspaceSelectionChanged() const
{
    std::optional<uint32_t> const selectedItemId{ getSelectedWorkspaceId() };
    if (!selectedItemId)
        return;

    mDelete->setDisabled(LusanApplication::getActiveWorkspace().getId() == *selectedItemId);
    initializePathsWithSelectedWorkspaceData(*selectedItemId);
}

void OptionPageWorkspace::setupUi()
{
    populateListOfWorkspaces();
    setupThemeControls();
    selectWorkspace(0);
}

void OptionPageWorkspace::selectWorkspace(int const index) const
{
    if (index < mList->count())
    {
        mList->setCurrentItem(mList->item(index));
        onWorkspaceSelectionChanged();
    }
}

void OptionPageWorkspace::applyChanges()
{
    const bool hasWorkspaceChanges = (mModifiedWorkspaces.empty() == false);
    const int selectedThemeValue = selectedTheme();
    const bool hasThemeChanges = (selectedThemeValue != mInitialTheme);
    if ((hasWorkspaceChanges == false) && (hasThemeChanges == false))
        return;
    
    OptionsManager& options = LusanApplication::getOptions();
    if (hasWorkspaceChanges)
    {
        for (auto const& [id, data] : mModifiedWorkspaces)
        {
            std::optional<WorkspaceEntry> workspace{ getWorkspace(id) };
            if (!workspace)
            {
                Q_ASSERT(false);
                continue;
            }

            if (data.hasDeleted)
            {
                options.removeWorkspace(workspace->getKey());
            }
            else if (data.newName || data.newDescription)
            {
                if (data.newName)
                {
                    workspace->setWorkspaceName(*data.newName);
                }

                if (data.newDescription)
                {
                    workspace->setWorkspaceDescription(*data.newDescription);
                }

                options.updateWorkspace(*workspace);
            }
        }
        
        mModifiedWorkspaces.clear();
    }
    
    if (hasThemeChanges)
    {
        options.setTheme(static_cast<OptionsManager::eAppTheme>(selectedThemeValue));
        mInitialTheme = selectedThemeValue;
    }
    
    options.writeOptions();
    LusanApplication::applyConfiguredTheme();
    
    OptionPageBase::applyChanges();
}

std::optional<WorkspaceEntry> OptionPageWorkspace::getWorkspace(uint32_t const workspaceId)
{
    std::vector<WorkspaceEntry> const& workspaces { LusanApplication::getOptions().getWorkspaceList() };

    auto workspacesIter{std::find_if(std::begin(workspaces), std::end(workspaces),
            [workspaceId](WorkspaceEntry const& we){ return we.getId() == workspaceId; }) };

    if (std::end(workspaces) != workspacesIter)
    {
        return *workspacesIter;
    }
    else
    {
        Q_ASSERT(false);
        return std::nullopt;
    }
}

void OptionPageWorkspace::onWorkspaceDescChanged()
{
    std::optional<uint32_t> const selectedItemId{ getSelectedWorkspaceId() };
    if (!selectedItemId)
        return;

    WorkspaceChangeData& data = mModifiedWorkspaces[*selectedItemId];
    data.hasDeleted = false;
    data.newDescription = mDescription->toPlainText();
}

void OptionPageWorkspace::onWorkspaceNameChanged()
{
    std::optional<uint32_t> const selectedItemId{ getSelectedWorkspaceId() };
    if (!selectedItemId)
        return;

    if (validateName() == false)
        return;

    WorkspaceChangeData& data = mModifiedWorkspaces[*selectedItemId];
    data.hasDeleted = false;
    data.newName = mName->text().trimmed();

    QListWidgetItem* item = mList->currentItem();
    if (item != nullptr)
    {
        item->setText(*data.newName);
    }
}

bool OptionPageWorkspace::validateName()
{
    std::optional<uint32_t> const selectedItemId{ getSelectedWorkspaceId() };
    if (!selectedItemId)
        return false;

    const QString name{ mName->text().trimmed() };
    if (name.isEmpty())
    {
        mHint->setText(tr("A workspace needs a name."));
        return false;
    }
    else if (LusanApplication::getOptions().existsWorkspaceName(name, *selectedItemId))
    {
        mHint->setText(tr("Another workspace is already named \"%1\".").arg(name));
        return false;
    }

    mHint->clear();
    return true;
}

void OptionPageWorkspace::onDefaultChecked(bool checked)
{
    OptionsManager& opt { LusanApplication::getOptions() };
    std::optional<uint32_t> const selectedItemId{ getSelectedWorkspaceId() };
    if (!selectedItemId)
        return;
    
    if (checked == false)
    {
        opt.setDefaultWorkspace(static_cast<uint32_t>(0u));
    }
    else
    {
        mDefault->setChecked(opt.setDefaultWorkspace(*selectedItemId));
    }
}

void OptionPageWorkspace::onThemeChanged(int /*index*/)
{
    setDataModified(selectedTheme() != mInitialTheme);
}

std::optional<uint32_t> OptionPageWorkspace::getSelectedWorkspaceId() const
{
    QListWidgetItem* selectedItem = mList->currentItem();
    if (nullptr != selectedItem)
    {
        return selectedItem->data(Qt::ItemDataRole::UserRole).toUInt();
    }
    else
    {
        return std::nullopt;
    }
}

void OptionPageWorkspace::setupThemeControls()
{
    const QList<OptionsManager::eAppTheme> themes = NEAppThemes::allThemes();
    for (OptionsManager::eAppTheme theme : themes)
    {
        mThemeCombo->addItem(NEAppThemes::themeDisplayName(theme), static_cast<int>(theme));
    }

    mInitialTheme = static_cast<int>(LusanApplication::getOptions().getTheme());
    const int index = mThemeCombo->findData(mInitialTheme);
    mThemeCombo->setCurrentIndex(index >= 0 ? index : 0);
}

int OptionPageWorkspace::selectedTheme() const
{
    return (mThemeCombo != nullptr ? mThemeCombo->currentData().toInt() : mInitialTheme);
}

inline QLineEdit* OptionPageWorkspace::ctrlRoot() const
{
    return mRootDir;
}

inline QLineEdit* OptionPageWorkspace::ctrlSources() const
{
    return mSourceDir;
}

inline QLineEdit* OptionPageWorkspace::ctrlIncludes() const
{
    return mIncludeDir;
}

inline QLineEdit* OptionPageWorkspace::ctrlDelivery() const
{
    return mDeliveryDir;
}

inline QLineEdit* OptionPageWorkspace::ctrlLogs() const
{
    return mLogDir;
}
