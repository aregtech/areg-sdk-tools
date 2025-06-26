#ifndef LUSAN_VIEW_LOG_LOGTABLEHEADER_HPP
#define LUSAN_VIEW_LOG_LOGTABLEHEADER_HPP
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
 *  \file        lusan/view/log/LogTableHeader.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, log view table header.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/

#include "lusan/common/NELusanCommon.hpp"

#include <QFrame>
#include <QHeaderView>
#include <QMap>
#include <QPushButton>

/************************************************************************
 * Dependencies
 ************************************************************************/
class LogViewer;
class LogViewerModel;
class QTableView;
class QListWidget;
class QListWidgetItem;
class QLineEdit;

//////////////////////////////////////////////////////////////////////////
// LogComboFilter class declaration
//////////////////////////////////////////////////////////////////////////

class LogComboFilter : public QFrame
{
    Q_OBJECT
//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    explicit LogComboFilter(QWidget* parent = nullptr);

    void setItems(const QStringList& items);

    QStringList checkedItems() const;

//////////////////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////////////////////
signals:
    void filtersChanged();

//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
private slots:
    void onItemChanged(QListWidgetItem* item);

//////////////////////////////////////////////////////////////////////////
// Member variables.
//////////////////////////////////////////////////////////////////////////
private:
    QListWidget* mListWidget;
};

//////////////////////////////////////////////////////////////////////////
// LogTextFilter class declaration
//////////////////////////////////////////////////////////////////////////
class LogTextFilter : public QFrame
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    explicit LogTextFilter(QWidget* parent = nullptr);

    QString text() const;

//////////////////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////////////////////
signals:
    void filterTextChanged(const QString& text);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QLineEdit* mLineEdit;
};

//////////////////////////////////////////////////////////////////////////
// LogTableHeader class declaration
//////////////////////////////////////////////////////////////////////////
class LogTableHeader : public QHeaderView
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Internal Types and constants
//////////////////////////////////////////////////////////////////////////
public:
    enum FilterType { None, ComboFilter, TextFilter };

private:
    struct sFilterWidget
    {
        QPushButton*    button      { nullptr };
        FilterType      type        { None };
        LogComboFilter* comboPopup  { nullptr };
        LogTextFilter*  textPopup   { nullptr };
    };

//////////////////////////////////////////////////////////////////////////
// LogTableHeader attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    explicit LogTableHeader(LogViewer * viewer, QTableView* parent, LogViewerModel* model, Qt::Orientation orientation = Qt::Horizontal);

    void setFilterType(int column, FilterType type);

    void setComboItems(int column, const QStringList& items);

signals:
    void comboFilterChanged(int column, const QStringList& items);

    void textFilterChanged(int column, const QString& text);

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void showFilterPopup(int column);

private:
    void updateButtonGeometry();

    void initializeHeaderTypes(void);

    QRect sectionRect(int logicalIndex) const;

private:

    LogViewerModel*             mModel;
    LogViewer*                  mViewer;
    QMap<int, sFilterWidget>    mFilters;
};

#endif // LUSAN_VIEW_LOG_LOGTABLEHEADER_HPP
