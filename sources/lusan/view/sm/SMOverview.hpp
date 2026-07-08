#ifndef LUSAN_VIEW_SM_SMOVERVIEW_HPP
#define LUSAN_VIEW_SM_SMOVERVIEW_HPP
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
 *  \file        lusan/view/sm/SMOverview.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Overview page.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QScrollArea>
#include <QIntValidator>

/************************************************************************
 * Dependencies
 ************************************************************************/
class SMOverviewModel;
class QCheckBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QRadioButton;

/**
 * \brief   The FSM Overview page: machine name, user version, threading mode and
 *          description. Every edit is committed through the document's undo stack via
 *          SMOverviewModel; the page mutates no model state directly.
 **/
class SMOverview : public QScrollArea
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMOverview(SMOverviewModel& model, QWidget* parent = nullptr);
    virtual ~SMOverview() = default;

//////////////////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////////////////////
signals:
    /**
     * \brief   Emitted when a quick-link is clicked, to switch to the given page.
     * \param   page    The target page index (StateMachine::eSMPages value).
     **/
    void signalPageLinkClicked(int page);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
private slots:
    void onNameEdited(const QString& text);
    void onNameCommitted();
    void onVersionEdited();
    void onThreadingToggled(bool checked);
    void onDeprecatedToggled(bool checked);
    void onDeprecateHintCommitted();
    void onOverviewChanged();

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    void buildUi();
    QWidget* buildLinksPanel();
    void setupSignals();
    void updateData();
    void commitDescription();
    void showNameValid(bool valid);
    static bool isValidMachineName(const QString& name);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    SMOverviewModel&    mModel;
    QLineEdit*          mName;
    QLabel*             mNameError;
    QLineEdit*          mMajor;
    QLineEdit*          mMinor;
    QLineEdit*          mPatch;
    QRadioButton*       mShared;
    QRadioButton*       mLocal;
    QPlainTextEdit*     mDescription;
    QCheckBox*          mDeprecated;
    QLineEdit*          mDeprecateHint;
    QIntValidator       mVersionValidator;
    bool                mCommitting;    //!< True while a page edit is being pushed, so the resulting notification does not refresh the field being edited.
};

#endif  // LUSAN_VIEW_SM_SMOVERVIEW_HPP
