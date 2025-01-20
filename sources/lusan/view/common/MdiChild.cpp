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
 *  \file        lusan/view/common/MdiChild.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application Multi-document interface (MDI) child window.
 *
 ************************************************************************/

#include "lusan/view/common/MdiChild.hpp"
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QGuiApplication>
#include <QSaveFile>
#include <QScrollBar>
#include <QTextDocument>
#include <QDir>
#include <QFileInfo>
#include <QEvent>

MdiChild::MdiChild(QWidget* parent /*= nullptr*/)
    : QWidget(parent)
    , mCurFile      ( )
    , mDocName      ( )
    , mIsUntitled   ( true )
    , mMdiSubWindow ( nullptr )
{
    setAttribute(Qt::WA_DeleteOnClose, true);
}

bool MdiChild::openSucceeded(void) const
{
    return false;
}

QString MdiChild::newDocumentName(void)
{
    static uint32_t _seqNr{0};
    mDocName = newDocument() + QString::number(++_seqNr);
    return (mDocName + newDocumentExt());
}

const QString& MdiChild::newDocument(void) const
{
    static const QString _newDoc{"document"};
    return _newDoc;
}

const QString& MdiChild::newDocumentExt(void) const
{
    static const QString _newExt("");
    return _newExt;
}

void MdiChild::newFile()
{
    mIsUntitled = true;
    mCurFile = newDocumentName();
    setWindowTitle(mCurFile + "[*]");
    setWindowModified(true);
#if 0
    connect(document(), &QTextDocument::contentsChanged, this, &MdiChild::documentWasModified);
#endif
}

bool MdiChild::loadFile(const QString& fileName)
{
#if 0
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
    connect(document(), &QTextDocument::contentsChanged, this, &MdiChild::documentWasModified);
#endif // 0
    
    setCurrentFile(fileName);
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
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    mIsUntitled = writeToFile(fileName);
    QGuiApplication::restoreOverrideCursor();
    setCurrentFile(fileName);
    return mIsUntitled;
}

bool MdiChild::writeToFile(const QString& filePath)
{
    return true;
}

QString MdiChild::userFriendlyCurrentFile()
{
    return strippedName(mCurFile);
}

void MdiChild::closeEvent(QCloseEvent* event)
{
#if 0
    if (maybeSave())
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
#endif
}

void MdiChild::documentWasModified()
{
    // setWindowModified(document()->isModified());
}

bool MdiChild::maybeSave()
{
//    if (!document()->isModified())
//        return true;

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
    // document()->setModified(false);
    setWindowModified(false);
    setWindowTitle(userFriendlyCurrentFile() + "[*]");
}

QString MdiChild::strippedName(const QString& fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MdiChild::cut()
{
    // Implement cut functionality
#if 0
    if (textCursor().hasSelection())
    {
        textCursor().removeSelectedText();
    }
#endif
}

void MdiChild::copy()
{
#if 0
    // Implement copy functionality
    if (textCursor().hasSelection())
    {
        QApplication::clipboard()->setText(textCursor().selectedText());
    }
#endif
}

void MdiChild::paste()
{
    // Implement paste functionality
    // textCursor().insertText(QApplication::clipboard()->text());
}

void MdiChild::undo()
{
    // Implement undo functionality
    // document()->undo();
}

void MdiChild::redo()
{
    // Implement redo functionality
    // document()->redo();
}

void MdiChild::clear()
{
    // Implement clear functionality
    // document()->clear();
}

void MdiChild::selectAll()
{
    // Implement select all functionality
    // textCursor().select(QTextCursor::Document);
}

void MdiChild::zoomIn(int range)
{
    // Implement zoom in functionality
    QFont font = this->font();
    font.setPointSize(font.pointSize() + range);
    setFont(font);
}

void MdiChild::zoomOut(int range)
{
    // Implement zoom out functionality
    QFont font = this->font();
    font.setPointSize(font.pointSize() - range);
    setFont(font);
}

void MdiChild::copyAvailable(bool available)
{
    
}
