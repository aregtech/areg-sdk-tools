#ifndef LUSAN_VIEW_COMMON_METHODDETAILSVIEW_HPP
#define LUSAN_VIEW_COMMON_METHODDETAILSVIEW_HPP
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
 *  \file        lusan/view/common/MethodDetailsView.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, shared "method details" editor used by the Service
 *               Interface and State Machine Methods pages.
 *
 ************************************************************************/

#include <QList>
#include <QString>
#include <QWidget>

class SMCodeEditor;
class QCheckBox;
class QComboBox;
class QFormLayout;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QRadioButton;

/**
 * \brief   One entry of the method-type radio group (section-3 method-type difference):
 *          Service Interface = Request / Response / Broadcast; State Machine = Trigger /
 *          Action / Condition. The optional icon resource is applied to the radio button when
 *          not empty (both current editors leave it empty).
 **/
struct MethodTypeOption
{
    QString label;          //!< The radio button caption.
    QString iconResource;   //!< Optional icon resource path (empty = no icon).
};

/**
 * \brief   The section-3 differences between the Service Interface and State Machine Methods
 *          pages, parameterized so a single shared view serves both editors:
 *          - `types`        : the method-type radio set (labels + optional icons).
 *          - `hasReply`     : the request/response link combo (Service Interface only).
 *          - `hasReturn`    : the condition return-type combo (State Machine only).
 *          - `hasImplement` : the Handler / Embedded radio group (State Machine only).
 *          - `hasGuardInfo` : the guard-use line label (State Machine only).
 *          - `hasBody`      : the embedded-condition C++ body editor (State Machine only).
 *
 *          Service Interface: `{ types = [Request, Response, Broadcast], hasReply = true }`.
 *          State Machine    : `{ types = [Trigger, Action, Condition], hasReturn = true,
 *                                hasImplement = true, hasGuardInfo = true, hasBody = true }`.
 **/
struct MethodViewConfig
{
    QList<MethodTypeOption> types;      //!< The method-type radio buttons, in display order.
    bool hasReply;      //!< Show the request/response link combo (Service Interface).
    bool hasReturn;     //!< Show the condition return-type combo (State Machine).
    bool hasImplement;  //!< Show the Handler / Embedded implement radios (State Machine).
    bool hasGuardInfo;  //!< Show the guard-use line label (State Machine).
    bool hasBody;       //!< Show the embedded-condition body editor (State Machine).
};

/**
 * \brief   The shared selected-method editor: a "Details:" group with label-beside-control
 *          rows -- Name (with an inline collision hint), a method-type radio group, and the
 *          config-gated rows: an optional Reply combo (Service Interface request/response
 *          link), an optional Return combo and Implement radios plus a guard-use line and a
 *          verbatim C++ body editor (State Machine conditions). Description and a Deprecated
 *          flag with hint close the form.
 *
 *          The view is controller-agnostic: it builds the widgets, wires the shared C++
 *          identifier validator on the Name field, and exposes ctrl*() accessors. All controls
 *          are always constructed (so the accessors are always valid); a control's row is left
 *          out of the layout when its config flag is false. Live list sync stays in the page
 *          controller, which listens to nameEdited(); the view never mutates a model.
 **/
class MethodDetailsView : public QWidget
{
    Q_OBJECT

public:
    explicit MethodDetailsView(const MethodViewConfig& config, QWidget* parent = nullptr);

    QLineEdit* ctrlName() const;

    //!< The number of method-type radio buttons (the size of MethodViewConfig::types).
    int typeCount() const;
    //!< The method-type radio button at the given index, or nullptr if out of range.
    QRadioButton* ctrlType(int index) const;

    QComboBox* ctrlReply() const;
    QComboBox* ctrlReturn() const;
    QRadioButton* ctrlHandler() const;
    QRadioButton* ctrlEmbedded() const;
    QLabel* ctrlGuardInfo() const;
    SMCodeEditor* ctrlBody() const;
    QPlainTextEdit* ctrlDescription() const;
    QCheckBox* ctrlDeprecated() const;
    QLineEdit* ctrlDeprecateHint() const;

    //!< Shows the given reason below the name field; an empty reason hides the hint.
    void showNameHint(const QString& reason);

    //!< Shows or hides the condition-only Return and Implement rows.
    void setConditionVisible(bool visible);
    //!< Shows or hides the embedded-condition Body editor row.
    void setBodyVisible(bool visible);
    //!< Shows or hides the guard-use row.
    void setGuardInfoVisible(bool visible);

signals:
    //!< Emitted while the user types in the Name field (validator-filtered text). The page
    //!< controller uses it to mirror the name into the selected list row.
    void nameEdited(const QString& text);

private:
    void buildUi();

private:
    const MethodViewConfig  mConfig;
    QLineEdit*              mName;
    QLabel*                 mNameHint;
    QList<QRadioButton*>    mTypes;
    QComboBox*              mReply;
    QComboBox*              mReturn;
    QRadioButton*           mHandler;
    QRadioButton*           mEmbedded;
    QWidget*                mImplementRow;  //!< The Implement field cell (row toggled with the mode).
    QLabel*                 mGuardInfo;
    SMCodeEditor*           mBody;
    QPlainTextEdit*         mDescription;
    QCheckBox*              mDeprecated;
    QLineEdit*              mDeprecateHint;
    QFormLayout*            mForm;          //!< The details form (used to toggle whole rows).
};

#endif  // LUSAN_VIEW_COMMON_METHODDETAILSVIEW_HPP
