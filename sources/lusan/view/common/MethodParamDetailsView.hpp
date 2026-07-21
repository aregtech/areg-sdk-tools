#ifndef LUSAN_VIEW_COMMON_METHODPARAMDETAILSVIEW_HPP
#define LUSAN_VIEW_COMMON_METHODPARAMDETAILSVIEW_HPP
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
 *  \file        lusan/view/common/MethodParamDetailsView.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, shared "method parameter details" editor used by the
 *               Service Interface and State Machine Methods pages.
 *
 ************************************************************************/

#include <QString>
#include <QWidget>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;

/**
 * \brief   The shared selected-parameter editor: a group with label-beside-control rows --
 *          Name (with an inline collision hint), Type, an optional Default (a check-box next
 *          to the literal value field, disabled until checked), Description and a Deprecated
 *          row (a check-box next to its hint field).
 *
 *          The view is controller-agnostic: it builds the widgets, wires the shared C++
 *          identifier validator on the Name field, and exposes ctrl*() accessors. Live list
 *          sync stays in the page controller, which listens to nameEdited(); the view never
 *          mutates a model. The controller owns whether the default value cell is enabled
 *          (the trailing-default C++ ordering rule lives there).
 **/
class MethodParamDetailsView : public QWidget
{
    Q_OBJECT

public:
    explicit MethodParamDetailsView(const QString& title, QWidget* parent = nullptr);

    QLineEdit* ctrlName() const;
    QComboBox* ctrlTypes() const;
    QCheckBox* ctrlHasDefault() const;
    QLineEdit* ctrlValue() const;
    QPlainTextEdit* ctrlDescription() const;
    QCheckBox* ctrlDeprecated() const;
    QLineEdit* ctrlDeprecateHint() const;

    //!< Shows the given reason below the name field; an empty reason hides the hint.
    void showNameHint(const QString& reason);

signals:
    //!< Emitted while the user types in the Name field (validator-filtered text). The page
    //!< controller uses it to mirror the name into the selected list row.
    void nameEdited(const QString& text);

private:
    void buildUi(const QString& title);

private:
    QLineEdit*      mName;
    QLabel*         mNameHint;
    QComboBox*      mType;
    QCheckBox*      mHasDefault;
    QLineEdit*      mValue;
    QPlainTextEdit* mDescription;
    QCheckBox*      mDeprecated;
    QLineEdit*      mDeprecateHint;
};

#endif  // LUSAN_VIEW_COMMON_METHODPARAMDETAILSVIEW_HPP
