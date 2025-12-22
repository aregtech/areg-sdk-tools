/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/view/common/Workspace.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application Workspace setup dialog.
 *
 ************************************************************************/

#include "lusan/view/common/Workspace.hpp"
#include "lusan/data/common/OptionsManager.hpp"
#include "lusan/app/LusanApplication.hpp"

#include "ui/ui_workspace.h"

#include <QFile>
#include <QFileDialog>

Workspace::Workspace(OptionsManager& options, QWidget * parent /*= nullptr*/)
    : QDialog   (parent)
    , mOptions  (options)
    , mWorkspace(new Ui::DialogWorkspace)
    , mModel    (options, nullptr)
{
    mWorkspace->setupUi(static_cast<QDialog *>(this));
    mWorkspace->buttonBoxOkCancel->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(false);
    mWorkspace->comboboxWorkspacePath->setEditable(true);
    mWorkspace->comboboxWorkspacePath->setInsertPolicy(QComboBox::InsertAtBottom);
    mWorkspace->comboboxWorkspacePath->setDuplicatesEnabled(false);
    mWorkspace->comboboxWorkspacePath->setModel(&mModel);
    if (mModel.rowCount() != 0)
    {
        const WorkspaceEntry & entry{mModel.getData(0)};
        mWorkspace->comboboxWorkspacePath->setCurrentText(entry.getWorkspaceRoot());
        mWorkspace->editWorkspaceDescription->setPlainText(entry.getWorkspaceDescription());
        mWorkspace->comboboxWorkspacePath->setCurrentIndex(0);
    }

    connect(mWorkspace->buttonBoxOkCancel       , &QDialogButtonBox::accepted   , this, &Workspace::onAccept);
    connect(mWorkspace->buttonBoxOkCancel       , &QDialogButtonBox::rejected   , this, &Workspace::onReject);
    connect(mWorkspace->buttonBrowse            , &QPushButton::clicked         , this, &Workspace::onBrowseClicked);
    connect(mWorkspace->checkDefault            , &QCheckBox::clicked           , this, &Workspace::onDefaultChecked);
    connect(mWorkspace->comboboxWorkspacePath   , &QComboBox::currentTextChanged, this, &Workspace::onWorskpacePathChanged);
    connect(mWorkspace->comboboxWorkspacePath   , &QComboBox::editTextChanged   , this, &Workspace::onWorskpacePathChanged);
    connect(mWorkspace->comboboxWorkspacePath   , &QComboBox::activated         , this, &Workspace::onWorskpaceIndexChanged);
    connect(&mModel, &QAbstractItemModel::dataChanged, this, &Workspace::onPathSelectionChanged);
    
    QString text {mWorkspace->comboboxWorkspacePath->currentText()};
    if (text.isEmpty() == false)
    {
        mWorkspace->checkDefault->setEnabled(true);
        onWorskpacePathChanged(text);
    }
    else
    {
        mWorkspace->checkDefault->setEnabled(false);
    }
}

Workspace::~Workspace(void)
{
    delete mWorkspace;
    mWorkspace = nullptr;
}

void Workspace::onAccept(void)
{
    QString path { mWorkspace->comboboxWorkspacePath->currentText() };
    QString describe { mWorkspace->editWorkspaceDescription->toPlainText() };

    if (mModel.hasNewWorkspace() && (mModel.getNewWorkspace().getWorkspaceRoot() != path))
    {
        QString removeEntry{ mModel.getNewWorkspace().getWorkspaceRoot() };
        mModel.removeWorkspaceEntry(removeEntry);
        Q_ASSERT(mModel.hasNewWorkspace() == false);
    }
    
    mOptions.addWorkspace(path, describe);
    if (mModel.isDefaultWorkspace(path))
    {
        mOptions.setDefaultWorkspace(path);
    }
    
    mOptions.writeOptions();
    done(static_cast<int>(QDialog::DialogCode::Accepted));
}

void Workspace::onReject(void)
{
    done(static_cast<int>(QDialog::DialogCode::Rejected));
}

void Workspace::onWorskpacePathChanged(const QString & newText)
{
    QPushButton * ok = mWorkspace->buttonBoxOkCancel->button(QDialogButtonBox::StandardButton::Ok);
    bool enableOK = newText.isEmpty() ? false : QDir(newText).exists();
    ok->setEnabled(enableOK);
    ok->setAutoDefault(enableOK);
    
    mWorkspace->checkDefault->setEnabled(enableOK);
    if (enableOK == false)
    {
        mWorkspace->checkDefault->setChecked(false);
    }
}

void Workspace::onBrowseClicked(bool checked /*= true*/)
{
    QDir curDir(QString(std::filesystem::current_path().string().c_str()));
    QString txt(mWorkspace->comboboxWorkspacePath->currentText());
    if (txt.isEmpty() == false)
    {
        QDir dir(txt);
        if (dir.exists())
        {
            curDir = dir;
        }
    }
    
    QString dirPath = curDir.path();
    QString parentName = curDir.filesystemPath().parent_path().string().c_str();
    
    QFileDialog dlgFile(  this
                        , QString(tr("Select Workspace Directory"))
                        , dirPath
                        , QString(""));
    dlgFile.setLabelText(QFileDialog::DialogLabel::FileName, QString(tr("Workspace Root:")));
    
    
    dlgFile.setOptions(QFileDialog::Option::ShowDirsOnly);
    dlgFile.setFileMode(QFileDialog::Directory);
    dlgFile.setDirectory(parentName);
    
    if (dlgFile.exec() == static_cast<int>(QDialog::DialogCode::Accepted))
    {
        QString newDir = dlgFile.directory().path();
        int index = mModel.find(newDir);            
        if (index >= 0)
        {
            mModel.activate(index);
            const WorkspaceEntry& entry = mModel.getData(index);
            mWorkspace->comboboxWorkspacePath->setCurrentIndex(index);
            mWorkspace->comboboxWorkspacePath->setCurrentText(newDir);
            mWorkspace->editWorkspaceDescription->setPlainText(entry.getWorkspaceDescription());
            mWorkspace->checkDefault->setEnabled(true);
            mWorkspace->checkDefault->setChecked(mModel.isDefaultWorkspace(entry.getWorkspaceRoot()));
        }
        else
        {
            WorkspaceEntry entry{ mModel.addWorkspaceEntry(newDir, "") };
            mWorkspace->comboboxWorkspacePath->setCurrentIndex(0);
            mWorkspace->comboboxWorkspacePath->setCurrentText(newDir);
            mWorkspace->editWorkspaceDescription->setPlainText("");
            mWorkspace->checkDefault->setEnabled(false);
            mWorkspace->checkDefault->setChecked(false);
        }        
    }
}

void Workspace::onWorskpaceIndexChanged(int index)
{
    blockSignals(true);
    const WorkspaceEntry& entry = mModel.getData(index);
    mWorkspace->editWorkspaceDescription->setPlainText(entry.getWorkspaceDescription());
    if (mWorkspace->comboboxWorkspacePath->currentText() != entry.getWorkspaceRoot())
    {
        mWorkspace->comboboxWorkspacePath->setCurrentText(entry.getWorkspaceRoot());
    }
    
    mWorkspace->checkDefault->setChecked(mModel.isDefaultWorkspace(entry.getWorkspaceRoot()));
    blockSignals(false);
}

void Workspace::onPathSelectionChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles)
{
    blockSignals(true);
    const WorkspaceEntry & entry = mModel.getData(topLeft.row());
    mWorkspace->editWorkspaceDescription->setPlainText(entry.getWorkspaceDescription());
    if (mWorkspace->comboboxWorkspacePath->currentText() != entry.getWorkspaceRoot())
    {
        mWorkspace->comboboxWorkspacePath->setCurrentText(entry.getWorkspaceRoot());
    }
    
    mWorkspace->checkDefault->setChecked(mModel.isDefaultWorkspace(entry.getWorkspaceRoot()));
    blockSignals(false);
}

void Workspace::onDefaultChecked(bool checked)
{
    mWorkspace->checkDefault->setChecked(mModel.setDefaultWorkspace(checked ? mWorkspace->comboboxWorkspacePath->currentText() : QString()));
}
