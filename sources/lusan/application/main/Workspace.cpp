#include "lusan/application/main/Workspace.hpp"
#include "ui/ui_workspace.h"

#include "lusan/common/controls/DirSelectionDialog.hpp"

#include <QFile>
#include <QFileDialog>

Workspace::Workspace(QWidget * parent /*= nullptr*/)
    : QDialog(parent)
    , mWorkspace(new Ui::DialogWorkspace)
{
    mWorkspace->setupUi(static_cast<QDialog *>(this));
    mWorkspace->buttonBoxOkCancel->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(false);
    mWorkspace->comboboxWorkspacePath->setEditable(true);
    
    connect(mWorkspace->buttonBoxOkCancel, &QDialogButtonBox::accepted, this, &Workspace::onAccept);
    connect(mWorkspace->buttonBoxOkCancel, &QDialogButtonBox::rejected, this, &Workspace::onReject);
    connect(mWorkspace->comboboxWorkspacePath, &QComboBox::currentTextChanged, this, &Workspace::onWorskpacePathChanged);
    connect(mWorkspace->buttonBrowse, &QPushButton::clicked, this, &Workspace::onBrowseClicked);
}

Workspace::~Workspace(void)
{
    delete mWorkspace;
    mWorkspace = nullptr;
}

void Workspace::onAccept(void)
{
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
        mWorkspace->comboboxWorkspacePath->setCurrentText(newDir);
        mRoot = newDir;
    }
}
