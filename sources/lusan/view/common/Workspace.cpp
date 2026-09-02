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
 *  \file        lusan/view/common/Workspace.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application Workspace setup dialog.
 *
 ************************************************************************/

#include "lusan/view/common/Workspace.hpp"
#include "lusan/app/NEAppThemes.hpp"
#include "lusan/data/common/OptionsManager.hpp"
#include "lusan/app/LusanApplication.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include <filesystem>

namespace
{
    //!< The widest workspace name that still reads well in the workspace lists.
    constexpr int MaxNameLength{ 48 };
}

Workspace::Workspace(OptionsManager& options, QWidget * parent /*= nullptr*/)
    : QDialog       (parent)
    , mOptions      (options)
    , mModel        (options, nullptr)
    , mRoot         (nullptr)
    , mBrowse       (nullptr)
    , mName         (nullptr)
    , mDescription  (nullptr)
    , mDefault      (nullptr)
    , mHint         (nullptr)
    , mButtons      (nullptr)
    , mNameEdited   (false)
{
    // Opened before the main window on a first run, so it carries the theme itself.
    NEAppThemes::applyThemeToWindow(*this);
    setupDialog();

    if (mModel.rowCount() != 0)
    {
        const WorkspaceEntry & entry{mModel.getData(0)};
        mRoot->setCurrentText(entry.getWorkspaceRoot());
        mDescription->setPlainText(entry.getWorkspaceDescription());
        mRoot->setCurrentIndex(0);
        showWorkspaceName(entry);
    }

    connect(mButtons    , &QDialogButtonBox::accepted   , this, &Workspace::onAccept);
    connect(mButtons    , &QDialogButtonBox::rejected   , this, &Workspace::onReject);
    connect(mBrowse     , &QPushButton::clicked         , this, &Workspace::onBrowseClicked);
    connect(mDefault    , &QCheckBox::clicked           , this, &Workspace::onDefaultChecked);
    connect(mName       , &QLineEdit::textEdited        , this, &Workspace::onWorkspaceNameChanged);
    connect(mRoot       , &QComboBox::currentTextChanged, this, &Workspace::onWorskpacePathChanged);
    connect(mRoot       , &QComboBox::editTextChanged   , this, &Workspace::onWorskpacePathChanged);
    connect(mRoot       , &QComboBox::activated         , this, &Workspace::onWorskpaceIndexChanged);
    connect(&mModel, &QAbstractItemModel::dataChanged, this, &Workspace::onPathSelectionChanged);

    mDefault->setEnabled(mRoot->currentText().isEmpty() == false);
    validateInput();
    mRoot->setFocus();
}

Workspace::~Workspace()
{
}

void Workspace::setupDialog()
{
    setWindowTitle(tr("Setup project workspace"));
    resize(600, 340);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);

    QGroupBox* group = new QGroupBox(tr("Select Areg based workspace"), this);
    QFormLayout* form = new QFormLayout(group);
    form->setFieldGrowthPolicy(QFormLayout::FieldGrowthPolicy::AllNonFixedFieldsGrow);
    form->setLabelAlignment(Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);

    mRoot = new QComboBox(group);
    mRoot->setEditable(true);
    mRoot->setInsertPolicy(QComboBox::InsertPolicy::InsertAtBottom);
    mRoot->setDuplicatesEnabled(false);
    mRoot->setModel(&mModel);
    mRoot->setToolTip(tr("Select a known workspace or enter a new one."));
    mRoot->lineEdit()->setPlaceholderText(tr("Path to project workspace"));
    mRoot->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);

    mBrowse = new QPushButton(tr("Browse..."), group);
    mBrowse->setToolTip(tr("Browse the file system and select the project workspace."));

    QWidget* rootRow = new QWidget(group);
    QHBoxLayout* rootLayout = new QHBoxLayout(rootRow);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->addWidget(mRoot, 1);
    rootLayout->addWidget(mBrowse, 0);
    form->addRow(tr("Workspace &Root:"), rootRow);

    mName = new QLineEdit(group);
    mName->setMaxLength(MaxNameLength);
    mName->setPlaceholderText(tr("Short name shown in the workspace list"));
    mName->setToolTip(tr("The name of this workspace. It must differ from the other workspace names."));
    form->addRow(tr("Workspace &Name:"), mName);

    mDescription = new QPlainTextEdit(group);
    mDescription->setPlaceholderText(tr("Describe your workspace here (optional)"));
    mDescription->setToolTip(tr("A longer note about this workspace."));
    form->addRow(tr("&Description:"), mDescription);

    mDefault = new QCheckBox(tr("Open this workspace at start and do not ask again."), group);
    form->addRow(QString(), mDefault);

    layout->addWidget(group, 1);

    mHint = new QLabel(this);
    mHint->setWordWrap(true);
    mHint->setTextFormat(Qt::TextFormat::PlainText);
    layout->addWidget(mHint, 0);

    mButtons = new QDialogButtonBox(QDialogButtonBox::StandardButton::Ok | QDialogButtonBox::StandardButton::Cancel, this);
    layout->addWidget(mButtons, 0);

    setTabOrder(mRoot, mBrowse);
    setTabOrder(mBrowse, mName);
    setTabOrder(mName, mDescription);
    setTabOrder(mDescription, mDefault);
    setTabOrder(mDefault, mButtons);
}

void Workspace::showWorkspaceName(const WorkspaceEntry& entry)
{
    const QString name{ entry.getWorkspaceName().isEmpty() ? WorkspaceEntry::nameFromRoot(entry.getWorkspaceRoot()) : entry.getWorkspaceName() };
    mName->blockSignals(true);
    mName->setText(name);
    mName->blockSignals(false);
    mNameEdited = false;
}

void Workspace::validateInput()
{
    const QString root{ mRoot->currentText() };
    const QString name{ mName->text().trimmed() };

    QString hint;
    if (root.isEmpty())
    {
        hint = tr("Select the root directory of the workspace.");
    }
    else if (QDir(root).exists() == false)
    {
        hint = tr("The directory %1 does not exist.").arg(root);
    }
    else if (name.isEmpty())
    {
        hint = tr("Enter a name for this workspace.");
    }
    else
    {
        const int index = mModel.find(root);
        const uint32_t ownId = index >= 0 ? mModel.getData(index).getId() : 0;
        if (mOptions.existsWorkspaceName(name, ownId))
        {
            hint = tr("Another workspace is already named \"%1\".").arg(name);
        }
    }

    QPushButton* ok = mButtons->button(QDialogButtonBox::StandardButton::Ok);
    const bool accept = hint.isEmpty();
    ok->setEnabled(accept);
    ok->setDefault(accept);
    mHint->setText(hint);
}

void Workspace::onAccept()
{
    QString path { mRoot->currentText() };
    QString name { mName->text().trimmed() };
    QString describe { mDescription->toPlainText() };

    if (mModel.hasNewWorkspace() && (mModel.getNewWorkspace().getWorkspaceRoot() != path))
    {
        QString removeEntry{ mModel.getNewWorkspace().getWorkspaceRoot() };
        mModel.removeWorkspaceEntry(removeEntry);
        Q_ASSERT(mModel.hasNewWorkspace() == false);
    }

    mOptions.addWorkspace(path, name, describe);
    if (mModel.isDefaultWorkspace(path))
    {
        mOptions.setDefaultWorkspace(path);
    }

    mOptions.writeOptions();
    done(static_cast<int>(QDialog::DialogCode::Accepted));
}

void Workspace::onReject()
{
    done(static_cast<int>(QDialog::DialogCode::Rejected));
}

void Workspace::onWorkspaceNameChanged(const QString& newText)
{
    mNameEdited = newText.trimmed().isEmpty() == false;
    validateInput();
}

void Workspace::onWorskpacePathChanged(const QString & newText)
{
    const bool exists = (newText.isEmpty() == false) && QDir(newText).exists();
    mDefault->setEnabled(exists);
    if (exists == false)
    {
        mDefault->setChecked(false);
    }

    // A name the user did not type follows the chosen directory.
    if ((mNameEdited == false) && exists)
    {
        mName->blockSignals(true);
        mName->setText(WorkspaceEntry::nameFromRoot(newText));
        mName->blockSignals(false);
    }

    validateInput();
}

void Workspace::onBrowseClicked(bool checked /*= true*/)
{
    QDir curDir(QString(std::filesystem::current_path().string().c_str()));
    QString txt(mRoot->currentText());
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
            mRoot->setCurrentIndex(index);
            mRoot->setCurrentText(newDir);
            mDescription->setPlainText(entry.getWorkspaceDescription());
            showWorkspaceName(entry);
            mDefault->setEnabled(true);
            mDefault->setChecked(mModel.isDefaultWorkspace(entry.getWorkspaceRoot()));
        }
        else
        {
            WorkspaceEntry entry{ mModel.addWorkspaceEntry(newDir, WorkspaceEntry::nameFromRoot(newDir), "") };
            mRoot->setCurrentIndex(0);
            mRoot->setCurrentText(newDir);
            mDescription->setPlainText("");
            showWorkspaceName(entry);
            mDefault->setEnabled(false);
            mDefault->setChecked(false);
        }

        validateInput();
    }
}

void Workspace::onWorskpaceIndexChanged(int index)
{
    blockSignals(true);
    const WorkspaceEntry& entry = mModel.getData(index);
    mDescription->setPlainText(entry.getWorkspaceDescription());
    if (mRoot->currentText() != entry.getWorkspaceRoot())
    {
        mRoot->setCurrentText(entry.getWorkspaceRoot());
    }

    showWorkspaceName(entry);
    mDefault->setChecked(mModel.isDefaultWorkspace(entry.getWorkspaceRoot()));
    blockSignals(false);
    validateInput();
}

void Workspace::onPathSelectionChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles)
{
    blockSignals(true);
    const WorkspaceEntry & entry = mModel.getData(topLeft.row());
    mDescription->setPlainText(entry.getWorkspaceDescription());
    if (mRoot->currentText() != entry.getWorkspaceRoot())
    {
        mRoot->setCurrentText(entry.getWorkspaceRoot());
    }

    showWorkspaceName(entry);
    mDefault->setChecked(mModel.isDefaultWorkspace(entry.getWorkspaceRoot()));
    blockSignals(false);
    validateInput();
}

void Workspace::onDefaultChecked(bool checked)
{
    mDefault->setChecked(mModel.setDefaultWorkspace(checked ? mRoot->currentText() : QString()));
}
