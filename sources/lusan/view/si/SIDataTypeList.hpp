#ifndef LUSAN_APPLICATION_SI_SIDATATYPELIST_HPP
#define LUSAN_APPLICATION_SI_SIDATATYPELIST_HPP
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
 *  \file        lusan/view/si/SIDataTypeList.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/
#include <QWidget>

class QAction;
class QKeySequence;
class QString;
class QTreeWidget;
class QToolButton;


class SIDataTypeList : public QWidget
{
    Q_OBJECT

public:
    explicit SIDataTypeList(QWidget *parent = nullptr);

    QTreeWidget* ctrlTableList() const;

    QToolButton* ctrlButtonAdd() const;

    QToolButton* ctrlButtonInsert() const;

    QToolButton* ctrlButtonRemove() const;

    QToolButton* ctrlButtonMoveUp() const;

    QToolButton* ctrlButtonMoveDown() const;

    QToolButton* ctrlButtonAddField() const;

    QToolButton* ctrlButtonRemoveField() const;

    QToolButton* ctrlButtonInsertField() const;

    QAction* actionNewStruct() const;
    QAction* actionNewEnum() const;
    QAction* actionNewImport() const;
    QAction* actionNewContainer() const;

private:
    void buildUi();

private:
    QTreeWidget* mTable;
    QToolButton* mButtonAdd;
    QToolButton* mButtonInsert;
    QToolButton* mButtonRemove;
    QToolButton* mButtonMoveUp;
    QToolButton* mButtonMoveDown;
    QToolButton* mButtonAddField;
    QToolButton* mButtonRemoveField;
    QToolButton* mButtonInsertField;
    QAction*     mActNewStruct;
    QAction*     mActNewEnum;
    QAction*     mActNewImport;
    QAction*     mActNewContainer;
};

#endif // LUSAN_APPLICATION_SI_SIDATATYPELIST_HPP
