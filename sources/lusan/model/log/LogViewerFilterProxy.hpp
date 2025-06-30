#ifndef LUSAN_MODEL_LOG_LOGVIEWERFILTERPROXY_HPP
#define LUSAN_MODEL_LOG_LOGVIEWERFILTERPROXY_HPP
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
 *  \file        lusan/model/log/LogViewerFilterProxy.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log Viewer Filter Proxy Model.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/

#include <QSortFilterProxyModel>
#include <QStringList>
#include <QMap>

class LogViewerModel;

/**
 * \brief   Filter proxy model for the log viewer to enable filtering of log messages.
 *          This proxy model filters the LogViewerModel based on user-selected criteria
 *          from the header filters (combo boxes and text filters).
 **/
class LogViewerFilterProxy : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    /**
     * \brief   Constructor with parent object.
     * \param   model   The logging data model object.
     **/
    explicit LogViewerFilterProxy(LogViewerModel* model);

public slots:
    /**
     * \brief   Sets combo box filter for a specific column.
     * \param   logicalColumn   The logical column index to filter.
     * \param   items          The list of selected items to filter by.
     **/
    void setComboFilter(int logicalColumn, const QStringList& items);

    /**
     * \brief   Sets text filter for a specific column.
     * \param   logicalColumn   The logical column index to filter.
     * \param   text           The text to filter by.
     **/
    void setTextFilter(int logicalColumn, const QString& text);

    /**
     * \brief   Clears all filters.
     **/
    void clearFilters();

protected:
    /**
     * \brief   Returns true if the given source row should be included in the model.
     * \param   source_row      The row index in the source model.
     * \param   source_parent   The parent index in the source model.
     * \return  True if the row should be included, false otherwise.
     **/
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private:
    /**
     * \brief   Helper method to check if a row matches the combo filters.
     * \param   source_row  The row index in the source model.
     * \return  True if the row matches all combo filters.
     **/
    bool matchesComboFilters(int source_row) const;

    /**
     * \brief   Helper method to check if a row matches the text filters.
     * \param   source_row  The row index in the source model.
     * \return  True if the row matches all text filters.
     **/
    bool matchesTextFilters(int source_row) const;

private:
    QMap<int, QStringList> mComboFilters;   //!< Map of column index to selected filter items
    QMap<int, QString>     mTextFilters;    //!< Map of column index to filter text
    LogViewerModel*        mLogModel;       //!< Pointer to the log viewer source model
};

#endif // LUSAN_MODEL_LOG_LOGVIEWERFILTERPROXY_HPP
