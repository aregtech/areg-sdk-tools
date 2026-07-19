#ifndef LUSAN_VIEW_COMMON_CONSTANTDETAILSVIEW_HPP
#define LUSAN_VIEW_COMMON_CONSTANTDETAILSVIEW_HPP
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
 *  \file        lusan/view/common/ConstantDetailsView.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, shared "constant details" editor used by the Service
 *               Interface, State Machine and (future) Data Type Constants pages.
 *
 ************************************************************************/

#include <QWidget>

class QCheckBox;
class QComboBox;
class QCompleter;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QStringListModel;

/**
 * \brief   The shared selected-constant editor: a "Details:" group with label-beside-control
 *          rows -- Name, Type, Value (a plain line edit: free text for a primitive type,
 *          completion of enumerator names for an enumeration type, disabled for a
 *          structure/container/imported type), an inline validation hint below the value,
 *          Description, Deprecated.
 *
 *          The view is controller-agnostic: it builds the widgets, wires the shared C++
 *          identifier validator on the Name field, and exposes the same ctrl*() accessors
 *          the former SIConstantDetails / SMConstantDetails classes exposed. Live table sync
 *          stays in the page controller, which listens to nameEdited(); the view never
 *          mutates a model.
 **/
class ConstantDetailsView : public QWidget
{
    Q_OBJECT

public:
    explicit ConstantDetailsView(QWidget* parent = nullptr);

    QLineEdit* ctrlName() const;
    QComboBox* ctrlTypes() const;
    QLineEdit* ctrlValue() const;
    QPlainTextEdit* ctrlDescription() const;
    QCheckBox* ctrlDeprecated() const;
    QLineEdit* ctrlDeprecateHint() const;

    //!< Shows the given reason below the value control; an empty reason hides the hint.
    void showValueHint(const QString& reason);

    //!< Sets the completion proposals of the value editor (the enumerator names of an
    //!< enumeration type); an empty list turns the completion off.
    void setValueChoices(const QStringList& choices);

signals:
    //!< Emitted while the user types in the Name field (validator-filtered text). The page
    //!< controller uses it to mirror the name into the selected list row.
    void nameEdited(const QString& text);

private:
    void buildUi();

private:
    QLineEdit*          mName;
    QComboBox*          mTypes;
    QLineEdit*          mValue;
    QCompleter*         mValueCompleter;
    QStringListModel*   mValueChoices;
    QLabel*             mValueHint;
    QPlainTextEdit*     mDescription;
    QCheckBox*          mDeprecated;
    QLineEdit*          mDeprecateHint;
};

#endif  // LUSAN_VIEW_COMMON_CONSTANTDETAILSVIEW_HPP
