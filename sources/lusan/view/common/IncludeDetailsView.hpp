#ifndef LUSAN_VIEW_COMMON_INCLUDEDETAILSVIEW_HPP
#define LUSAN_VIEW_COMMON_INCLUDEDETAILSVIEW_HPP
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
 *  \file        lusan/view/common/IncludeDetailsView.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, shared "include details" editor used by the Service
 *               Interface, State Machine and (future) Data Type Includes pages.
 *
 ************************************************************************/

#include <QWidget>

class QCheckBox;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;

/**
 * \brief   The shared selected-include editor: a "Details:" group with label-beside-control
 *          rows -- the include file location with a Browse button, Description, and a
 *          Deprecated flag with hint.
 *
 *          The view is controller-agnostic: it builds the widgets, wires the shared include
 *          path validator on the location field, and exposes the same ctrl*() accessors the
 *          former SIIncludeDetails / SMIncludeDetails classes exposed. Live list sync stays in
 *          the page controller, which listens to nameEdited(); the view never mutates a model.
 **/
class IncludeDetailsView : public QWidget
{
    Q_OBJECT

public:
    explicit IncludeDetailsView(QWidget* parent = nullptr);

    QLineEdit* ctrlInclude() const;
    QPushButton* ctrlBrowseButton() const;
    QPlainTextEdit* ctrlDescription() const;
    QCheckBox* ctrlDeprecated() const;
    QLineEdit* ctrlDeprecateHint() const;

signals:
    //!< Emitted while the user types in the include location field (validator-filtered text).
    //!< The page controller uses it to mirror the location into the selected list row.
    void nameEdited(const QString& text);

private:
    void buildUi();

private:
    QLineEdit*      mInclude;
    QPushButton*    mBrowse;
    QPlainTextEdit* mDescription;
    QCheckBox*      mDeprecated;
    QLineEdit*      mDeprecateHint;
};

#endif  // LUSAN_VIEW_COMMON_INCLUDEDETAILSVIEW_HPP
