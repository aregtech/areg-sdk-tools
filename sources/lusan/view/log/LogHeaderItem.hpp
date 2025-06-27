#ifndef LUSAN_VIEW_LOG_LOGHEADERITEM_HPP
#define LUSAN_VIEW_LOG_LOGHEADERITEM_HPP
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
 *  \file        lusan/view/log/LogHeaderItem.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, log view table header item.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/common/NELusanCommon.hpp"
#include "lusan/model/log/LogViewerModel.hpp"

#include <QFrame>
#include <QList>

/************************************************************************
 * Dependencies
 ************************************************************************/
class LogTableHeader;
class QHBoxLayout;
class QPushButton;
class QLabel;
class QLineEdit;
class QListWidget;

/************************************************************************
 * Implemented classes
 ************************************************************************/

class LogComboFilter;
class LogTextFilter;
class LogHeaderItem;

//////////////////////////////////////////////////////////////////////////
// LogComboFilter class declaration
//////////////////////////////////////////////////////////////////////////

class LogComboFilter : public QFrame
{
    Q_OBJECT

private:
    struct sComboItem
    {
        QString text{};
        bool    checked{false};
    };
    
public:
    explicit LogComboFilter(QWidget* parent = nullptr);

    void setItems(const QStringList& items);

    QStringList getCheckedItems() const;

//////////////////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////////////////////
signals:
    void signalFiltersChanged(void);

//////////////////////////////////////////////////////////////////////////
// Member variables.
//////////////////////////////////////////////////////////////////////////
private:
    QListWidget*        mListWidget;
    QList<sComboItem>   mItems;
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

    QString getText() const;

//////////////////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////////////////////
signals:
    void signalFilterTextChanged(const QString& text);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QLineEdit* mLineEdit;
};

//////////////////////////////////////////////////////////////////////////
// LogHeaderItem class declaration
//////////////////////////////////////////////////////////////////////////
class LogHeaderItem : public QObject
{
    Q_OBJECT

public:
    enum eType
    {
          None
        , Combo
        , Text
    };

public:
    LogHeaderItem(LogTableHeader& header, int logicalIndex, const QString& text);

    void showFilters(void);
    
    inline bool canPopupFilter(void) const;

signals:

    void signalComboFilterChanged(int logicalIndex, QStringList checkedItems);

    void signalTextFilterChanged(int logicalIndex, QString newText);

private:

    inline int fromColumnToIndex(void) const;

    inline LogViewerModel::eColumn fromIndexToColum(int logicalIndex) const;

private:

    LogViewerModel::eColumn mColumn;    //!< Logical index.
    eType           mType;
    LogTableHeader& mHeader;
    LogComboFilter* mCombo;
    LogTextFilter*  mEdit;
};

bool LogHeaderItem::canPopupFilter(void) const
{
    return (mType != None);    
}

#endif  // LUSAN_VIEW_LOG_LOGHEADERITEM_HPP
