#ifndef LUSAN_VIEW_SM_STATEMACHINE_HPP
#define LUSAN_VIEW_SM_STATEMACHINE_HPP
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
 *  \file        lusan/view/sm/StateMachine.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, State Machine MDI window.
 *
 ************************************************************************/

#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/common/MdiChild.hpp"

class MdiMainWindow;

class StateMachine : public MdiChild
{
    Q_OBJECT

public:
    static const QString& fileExtension(void);

public:
    StateMachine(MdiMainWindow* wndMain, const QString& filePath = QString(), const QString& sourcePath = QString(), QWidget* parent = nullptr);
    virtual ~StateMachine(void) = default;

public:
    virtual void newFile() override;
    virtual bool openSucceeded(void) const override;
    virtual void undo() override;
    virtual void redo() override;

protected:
    virtual QString newDocumentName(void) override;
    virtual const QString& newDocument(void) const override;
    virtual const QString& newDocumentExt(void) const override;
    virtual const QString& fileSuffix(void) const override;
    virtual const QString& fileFilter(void) const override;
    virtual bool writeToFile(const QString& filePath) override;
    virtual bool maybeSave() override;
    virtual void onWindowClosing(bool isActive) override;

private:
    bool loadDocument(const QString& documentPath, const QString& sourcePath = QString());

private:
    StateMachineModel    mModel;
};

#endif  // LUSAN_VIEW_SM_STATEMACHINE_HPP
