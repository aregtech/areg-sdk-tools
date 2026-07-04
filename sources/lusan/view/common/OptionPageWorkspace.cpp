/************************************************************************
 *  This file is part of the Lusan project, an official component of the Areg SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the Areg Framework.
 *
 *  Lusan is available as free and open-source software under the Apache version 2.0 License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
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
#include "ui/ui_OptionPageWorkspace.h"

#include "lusan/app/LusanApplication.hpp"
#include "lusan/app/NEAppThemes.hpp"
#include "lusan/data/common/OptionsManager.hpp"

#include <QComboBox>
#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <algorithm>

OptionPageWorkspace::OptionPageWorkspace(QDialog* parent)
    : OptionPageBase        (parent)
    , mUi                   (std::make_unique<Ui::OptionPageWorkspace>())
    , mModifiedWorkspaces   ( )
    , mSources              ( )
    , mIncludes             ( )
    , mDelivery             ( )
    , mLogs                 ( )
    , mThemeCombo           ( nullptr )
    , mThemeLabel           ( nullptr )
    , mInitialTheme         ( static_cast<int>(OptionsManager::eAppTheme::SystemDefault) )
{
    mUi->setupUi(this);
    setupUi();
    connectSignalHandlers();
}

OptionPageWorkspace::~OptionPageWorkspace()
{
}

void OptionPageWorkspace::connectSignalHandlers()
{
    connect(mUi->deleteButton    , &QPushButton::clicked             , this, &OptionPageWorkspace::onDeleteButtonClicked);
    connect(mUi->listOfWorkspaces, &QListWidget::itemSelectionChanged, this, &OptionPageWorkspace::onWorkspaceSelectionChanged);
    connect(mUi->workspaceEdit   , &QPlainTextEdit::textChanged      , this, &OptionPageWorkspace::onWorkspaceDescChanged);
    connect(mUi->checkDefault    , &QCheckBox::clicked               , this, &OptionPageWorkspace::onDefaultChecked);
    connect(mThemeCombo          , qOverload<int>(&QComboBox::currentIndexChanged), this, &OptionPageWorkspace::onThemeChanged);
}

void OptionPageWorkspace::initializePathsWithSelectedWorkspaceData(uint32_t const workspaceId) const
{
    std::optional<WorkspaceEntry> const workspace{ getWorkspace(workspaceId) };
    if (!workspace)
        return;
    
    OptionsManager& opt { LusanApplication::getOptions() };
    bool isDefault = opt.isDefaultWorkspace(workspace->getId());
    mUi->checkDefault->setEnabled(true);
    mUi->checkDefault->setChecked(isDefault);
    mUi->workspaceEdit->setPlainText(workspace->getWorkspaceDescription());
    
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
    
    QListWidgetItem* selectedItem = mUi->listOfWorkspaces->currentItem();
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
    
    QListWidget* list = mUi->listOfWorkspaces;
    list->clear();

    for (WorkspaceEntry const& workspace : workspaces)
    {
        uint32_t wsId = workspace.getId();
        QString text(QString::number(wsId));
        text += " : " + workspace.getWorkspaceRoot();

        QListWidgetItem* item = new QListWidgetItem(NELusanCommon::iconWorkspaceOpen(NELusanCommon::SizeSmall), text, list);
        item->setData(Qt::ItemDataRole::UserRole, wsId);

        if (currentWorkspace.getId() == wsId)
        {
            QBrush const grayBrush{ QColor(Qt::gray) };
            item->setForeground(grayBrush);
        }
        
        list->addItem(item);
    }

    mUi->listOfWorkspaces->sortItems();
}

void OptionPageWorkspace::onDeleteButtonClicked()
{
    QListWidgetItem* selectedItem = mUi->listOfWorkspaces->currentItem();
    if (nullptr == selectedItem)
        return;

    uint32_t const selectedWorkspaceId{ selectedItem->data(Qt::ItemDataRole::UserRole).toUInt() };
    if (LusanApplication::getOptions().isActiveWorkspace(selectedWorkspaceId))
    {
        if ( LusanApplication::getOptions().isDefaultWorkspace(selectedWorkspaceId) )
        {
            LusanApplication::getOptions().setDefaultWorkspace(static_cast<uint32_t>(0));
            mUi->checkDefault->setChecked(false);
        }
        
        mModifiedWorkspaces[selectedWorkspaceId] = WorkspaceChangeData{ true, {} };
        deleteSelectedWorkspaceItem();
    }
}

void OptionPageWorkspace::deleteSelectedWorkspaceItem() const
{
    disconnect(mUi->workspaceEdit, &QPlainTextEdit::textChanged, this, &OptionPageWorkspace::onWorkspaceDescChanged);

    delete mUi->listOfWorkspaces->takeItem(mUi->listOfWorkspaces->currentRow());

    connect(mUi->workspaceEdit, &QPlainTextEdit::textChanged, this, &OptionPageWorkspace::onWorkspaceDescChanged);
}

void OptionPageWorkspace::onWorkspaceSelectionChanged() const
{
    std::optional<uint32_t> const selectedItemId{ getSelectedWorkspaceId() };
    if (!selectedItemId)
        return;

    mUi->deleteButton->setDisabled(LusanApplication::getActiveWorkspace().getId() == *selectedItemId);
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
    if (index < mUi->listOfWorkspaces->count())
    {
        mUi->listOfWorkspaces->setCurrentItem(mUi->listOfWorkspaces->item(index));
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
            else if (data.newDescription)
            {
                workspace->setWorkspaceDescription(*data.newDescription);
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

    mModifiedWorkspaces[*selectedItemId] = WorkspaceChangeData{false, mUi->workspaceEdit->toPlainText()};
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
        mUi->checkDefault->setChecked(opt.setDefaultWorkspace(*selectedItemId));
    }
}

void OptionPageWorkspace::onThemeChanged(int /*index*/)
{
    setDataModified(selectedTheme() != mInitialTheme);
}

std::optional<uint32_t> OptionPageWorkspace::getSelectedWorkspaceId() const
{
    QListWidgetItem* selectedItem = mUi->listOfWorkspaces->currentItem();
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
    QGridLayout* layout = mUi->layoutWidget->findChild<QGridLayout*>("gridLayout");
    if (layout == nullptr)
        return;

    mThemeLabel = new QLabel(tr("Application Theme:"), this);
    mThemeCombo = new QComboBox(this);
    mThemeCombo->setToolTip(tr("Select the application visual style"));
    const QList<OptionsManager::eAppTheme> themes = NEAppThemes::allThemes();
    for (OptionsManager::eAppTheme theme : themes)
    {
        mThemeCombo->addItem(NEAppThemes::themeDisplayName(theme), static_cast<int>(theme));
    }
    layout->addWidget(mThemeLabel, 7, 0, 1, 2);
    layout->addWidget(mThemeCombo, 7, 2, 1, 1);

    mInitialTheme = static_cast<int>(LusanApplication::getOptions().getTheme());
    const int index = mThemeCombo->findData(mInitialTheme);
    mThemeCombo->setCurrentIndex(index >= 0 ? index : 0);
}

int OptionPageWorkspace::selectedTheme(void) const
{
    return (mThemeCombo != nullptr ? mThemeCombo->currentData().toInt() : mInitialTheme);
}

inline QLineEdit* OptionPageWorkspace::ctrlRoot(void) const
{
    return mUi->rootDirEdit;
}

inline QLineEdit* OptionPageWorkspace::ctrlSources(void) const
{
    return mUi->sourceDirEdit;
}

inline QLineEdit* OptionPageWorkspace::ctrlIncludes(void) const
{
    return mUi->includeDirEdit;
}

inline QLineEdit* OptionPageWorkspace::ctrlDelivery(void) const
{
    return mUi->deliveryDirEdit;
}

inline QLineEdit* OptionPageWorkspace::ctrlLogs(void) const
{
    return mUi->logDirEdit;
}
