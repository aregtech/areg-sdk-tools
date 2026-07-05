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
 *  \file        lusan/view/sm/StateMachine.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, State Machine MDI window.
 *
 ************************************************************************/

#include "lusan/view/sm/StateMachine.hpp"

#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>

const QString& StateMachine::fileExtension(void)
{
    static const QString _extFSM{ "fsml" };
    return _extFSM;
}

StateMachine::StateMachine(MdiMainWindow* wndMain, const QString& filePath /*= QString()*/, const QString& sourcePath /*= QString()*/, QWidget* parent /*= nullptr*/)
    : MdiChild           (MdiChild::eMdiWindow::MdiStateMachine, wndMain, parent)
    , mModel             (this)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(new QLabel(tr("State Machine document"), this));
    setLayout(layout);

    connect(&mModel, &StateMachineModel::signalDirtyChanged, this, &MdiChild::setModified);

    if (filePath.isEmpty() == false)
    {
        mIsUntitled = false;
        if (loadDocument(filePath, sourcePath))
        {
            setCurrentFile(filePath);
        }
    }
}

void StateMachine::newFile()
{
    MdiChild::newFile();
    if (mModel.createNewDocument(mDocName) == false)
    {
        setModified(false);
    }
}

bool StateMachine::openSucceeded(void) const
{
    return mModel.openSucceeded();
}

void StateMachine::undo()
{
    mModel.getUndoStack().undo();
}

void StateMachine::redo()
{
    mModel.getUndoStack().redo();
}

QString StateMachine::newDocumentName(void)
{
    static uint32_t _seqNr{ 0 };
    mDocName = newDocument() + QString::number(++_seqNr);
    return mDocName + newDocumentExt();
}

const QString& StateMachine::newDocument(void) const
{
    static const QString _newFSMDoc{ "NewStateMachine" };
    return _newFSMDoc;
}

const QString& StateMachine::newDocumentExt(void) const
{
    static const QString _extFSM{ ".fsml" };
    return _extFSM;
}

const QString& StateMachine::fileSuffix(void) const
{
    static const QString _suffixFSM{ "fsml" };
    return _suffixFSM;
}

const QString& StateMachine::fileFilter(void) const
{
    static const QString _filterFSM{ "State Machine document (*.fsml)\nAll Files (*.*)" };
    return _filterFSM;
}

bool StateMachine::writeToFile(const QString& filePath)
{
    return mModel.saveToFile(filePath);
}

bool StateMachine::maybeSave()
{
    if (isModified() == false)
    {
        return true;
    }

    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this, tr("MDI"),
                               tr("'%1' has been modified.\nDo you want to save your changes?")
                                   .arg(userFriendlyCurrentFile()),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret)
    {
    case QMessageBox::Save:
        return save();
    case QMessageBox::Discard:
        mModel.removeAutosave();
        return true;
    case QMessageBox::Cancel:
        return false;
    default:
        return true;
    }
}

void StateMachine::onWindowClosing(bool isActive)
{
    Q_UNUSED(isActive);
    if (isModified() == false)
    {
        mModel.removeAutosave();
    }
}

bool StateMachine::loadDocument(const QString& documentPath, const QString& sourcePath /*= QString()*/)
{
    return mModel.loadFromFile(documentPath, sourcePath);
}
