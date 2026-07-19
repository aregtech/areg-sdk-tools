#ifndef LUSAN_VIEW_COMMON_INCLUDELISTVIEW_HPP
#define LUSAN_VIEW_COMMON_INCLUDELISTVIEW_HPP
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
 *  \file        lusan/view/common/IncludeListView.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, shared "include list" panel used by the Service
 *               Interface, State Machine and (future) Data Type Includes pages.
 *
 ************************************************************************/

#include <QKeySequence>
#include <QString>
#include <QWidget>

class QToolButton;
class QTreeWidget;

/**
 * \brief   The section-3 difference between the Service Interface and State Machine Includes
 *          pages: the third recognized include extension (besides a C++ header and a `.dtml`
 *          data type document) and the label shown for it in the Type column.
 *
 *          Service Interface: `docExtension = "siml"`, `docTypeLabel = "Service Interface"`.
 *          State Machine    : `docExtension = "fsml"`, `docTypeLabel = "State Machine"`.
 **/
struct IncludeTypeConfig
{
    QString docExtension;   //!< The document file extension (no dot), e.g. "siml" or "fsml".
    QString docTypeLabel;   //!< The Type column label for that extension, e.g. "Service Interface".
};

/**
 * \brief   The shared include list panel: an "Include Files:" group holding the
 *          add/insert/delete + move up/down + update toolbar above a 4-column flat list
 *          (Location / Type / Name / Version), built on a QTreeWidget.
 *
 *          The view is controller-agnostic: it only builds the widgets and exposes ctrl*()
 *          accessors, plus the config-driven typeForLocation()/nameForLocation() helpers so
 *          both editors derive the Type and Name columns from a file location identically.
 *          The page controller owns all row population and selection logic.
 **/
class IncludeListView : public QWidget
{
    Q_OBJECT

public:
    /**
     * \brief   Column indexes of the include list.
     **/
    enum eColumn
    {
          ColLocation   = 0 //!< The include file location (editable inline).
        , ColType       = 1 //!< The derived include type (read-only).
        , ColName       = 2 //!< The derived include name (read-only).
        , ColVersion    = 3 //!< The include version (read-only, blank until parsed).
    };

    explicit IncludeListView(const IncludeTypeConfig& config, QWidget* parent = nullptr);

    QTreeWidget* ctrlTableList() const;

    QToolButton* ctrlButtonAdd() const;
    QToolButton* ctrlButtonInsert() const;
    QToolButton* ctrlButtonRemove() const;
    QToolButton* ctrlButtonMoveUp() const;
    QToolButton* ctrlButtonMoveDown() const;
    QToolButton* ctrlButtonUpdate() const;

    /**
     * \brief   Returns the Type column text for a location: the configured document label for
     *          the document extension, "Data Type" for `.dtml`, "Source" for anything else.
     **/
    QString typeForLocation(const QString& location) const;

    /**
     * \brief   Returns the Name column text for a location: the base name (no extension) for a
     *          document or data type include, the file name for a source include.
     **/
    QString nameForLocation(const QString& location) const;

private:
    void buildUi();

private:
    const IncludeTypeConfig mConfig;
    QTreeWidget*    mTable;
    QToolButton*    mButtonAdd;
    QToolButton*    mButtonInsert;
    QToolButton*    mButtonRemove;
    QToolButton*    mButtonMoveUp;
    QToolButton*    mButtonMoveDown;
    QToolButton*    mButtonUpdate;
};

#endif  // LUSAN_VIEW_COMMON_INCLUDELISTVIEW_HPP
