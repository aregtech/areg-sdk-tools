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
#include "lusan/view/log/LogHeaderItem.hpp"

#include <QHeaderView>

/************************************************************************
 * Dependencies
 ************************************************************************/
class LogViewer;
class LogViewerModel;
class QTableView;

//////////////////////////////////////////////////////////////////////////
// LogTableHeader class declaration
//////////////////////////////////////////////////////////////////////////
class LogTableHeader : public QHeaderView
{
    friend class LogHeaderItem;

    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// LogTableHeader attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    explicit LogTableHeader(LogViewer * viewer, QTableView* parent, LogViewerModel* model, Qt::Orientation orientation = Qt::Horizontal);

signals:
/************************************************************************
 * Signals
 ************************************************************************/

    /**
     * \brief   The signal is triggered when a combo-box filter is changed.
     * \param   logicalColumn  The logical column index of the filter.
     * \param   items           The list of items selected in the combo-box filter.
     **/
    void signalComboFilterChanged(int logicalColumn, const QStringList& items);

    /**
     * \brief   The signal is triggered when a text filter is changed.
     * \param   logicalColumn  The logical column index of the filter.
     * \param   text           The text entered in the text filter.
     **/
    void signalTextFilterChanged(int logicalColumn, const QString& text);

protected:
/************************************************************************
 * Overrides
 ************************************************************************/

    virtual void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const override;

    virtual void mousePressEvent(QMouseEvent* event) override;

private:

    inline void drawingRects(const Rect& rect, Rect & rcButton, Rect & rcText) const;

    inline QRect sectionRect(int logicalIndex) const;

private:

    LogViewerModel*     mModel;
    LogViewer*          mViewer;
    QList<LogHeaderItem *> mHeaders;
};

#endif // LUSAN_VIEW_LOG_LOGTABLEHEADER_HPP
