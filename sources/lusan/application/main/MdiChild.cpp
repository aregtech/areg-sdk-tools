
/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]aregtech.com.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/application/main/MdiChild.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application Multi-document interface (MDI) child window.
 *
 ************************************************************************/

#include "lusan/application/main/MdiChild.hpp"

#include <QtWidgets>

MdiChild::MdiChild()
    : QTextEdit     ( )
    , mCurFile      ( )
    , mIsUntitled   ( true )
{
    setAttribute(Qt::WA_DeleteOnClose);
}

void MdiChild::newFile()
{
    static int sequenceNumber = 1;

    mIsUntitled = true;
    mCurFile = tr("document%1.txt").arg(sequenceNumber++);
    setWindowTitle(mCurFile + "[*]");

    connect(document(), &QTextDocument::contentsChanged, this, &MdiChild::documentWasModified);
}

bool MdiChild::loadFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::warning(this, tr("MDI"), tr("Cannot read file %1:\n%2.").arg(fileName).arg(file.errorString()));
        return false;
    }

    QTextStream in(&file);
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    setPlainText(in.readAll());
    QGuiApplication::restoreOverrideCursor();

    setCurrentFile(fileName);

    connect(document(), &QTextDocument::contentsChanged, this, &MdiChild::documentWasModified);

    return true;
}

bool MdiChild::save()
{
    return (mIsUntitled ? saveAs() : saveFile(mCurFile));
}

bool MdiChild::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"), mCurFile);
    return (fileName.isEmpty() ? false : saveFile(fileName));
}

bool MdiChild::saveFile(const QString& fileName)
{
    QString errorMessage;

    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    QSaveFile file(fileName);
    if (file.open(QFile::WriteOnly | QFile::Text))
    {
        QTextStream out(&file);
        out << toPlainText();
        if (!file.commit()) 
        {
            errorMessage = tr("Cannot write file %1:\n%2.").arg(QDir::toNativeSeparators(fileName), file.errorString());
        }
    }
    else
    {
        errorMessage = tr("Cannot open file %1 for writing:\n%2.").arg(QDir::toNativeSeparators(fileName), file.errorString());
    }

    QGuiApplication::restoreOverrideCursor();

    if (!errorMessage.isEmpty())
    {
        QMessageBox::warning(this, tr("MDI"), errorMessage);
        return false;
    }

    setCurrentFile(fileName);
    return true;
}

QString MdiChild::userFriendlyCurrentFile()
{
    return strippedName(mCurFile);
}

void MdiChild::closeEvent(QCloseEvent* event)
{
    if (maybeSave())
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void MdiChild::documentWasModified()
{
    setWindowModified(document()->isModified());
}

bool MdiChild::maybeSave()
{
    if (!document()->isModified())
        return true;

    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this, tr("MDI"),
            tr("'%1' has been modified.\n"
               "Do you want to save your changes?")
            .arg(userFriendlyCurrentFile()),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret)
    {
    case QMessageBox::Save:
        return save();
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }

    return true;
}

void MdiChild::setCurrentFile(const QString& fileName)
{
    mCurFile = QFileInfo(fileName).canonicalFilePath();
    mIsUntitled = false;
    document()->setModified(false);
    setWindowModified(false);
    setWindowTitle(userFriendlyCurrentFile() + "[*]");
}

QString MdiChild::strippedName(const QString& fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}