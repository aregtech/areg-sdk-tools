#ifndef LUSAN_VIEW_LOG_LOGFILTERWIDGETS_HPP
#define LUSAN_VIEW_LOG_LOGFILTERWIDGETS_HPP
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
 *  \file        lusan/view/log/LogFilterWidgets.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, log view table header filter widgets.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/common/NELusanCommon.hpp"
#include "lusan/model/log/LiveLogsModel.hpp"

#include <QFrame>
#include <QList>

class QListWidget;
class QLineEdit;

//////////////////////////////////////////////////////////////////////////
// LogFilterBase class declaration
//////////////////////////////////////////////////////////////////////////
class LogFilterBase : public QFrame
{
    Q_OBJECT
public:
    explicit LogFilterBase(QWidget* parent = nullptr);

public:
    /**
     * \brief   Updates and sets the filter data of widget
     * \param   data    The data to set in the widget.
     **/
    virtual void setDataString(const QString& data) = 0;

    /**
     * \brief   Updates and sets the items widget.
     *          Mainly required for combo-boxes.
     * \param   data    The list of data to set in combo-box
     **/
    virtual void setDataList(const std::vector<NELusanCommon::FilterData>& data) = 0;

    /**
     * \brief   Updates and sets the items of combo-box
     * \param   items   The list of entries to set in combo-box
     * \param   data    The list of data to set in combo-box.
     **/
    virtual void setDataItems(const QStringList& items, const NELusanCommon::AnyList& data) = 0;

    /**
     * \brief   Returns list of selected (checked) entries.
     **/
    virtual QList<NELusanCommon::FilterData> getSelectedData() const = 0;

public:

    /**
     * \brief   Clears filter data.
     **/
    virtual void clearFilter(void);

    /**
     * \brief   Shows the filter widget and set focus on the list widget.
     **/
    virtual void showFilter(void);

    /**
     * \brief   Returns the list of filter data items.
     **/
    inline const QList<NELusanCommon::FilterData>& getData(void) const;

    /**
     * \brief   Returns the list of filter data items.
     **/
    inline QWidget* getWidget(void) const;

/************************************************************************
 * Signals
 ************************************************************************/
signals:
    /**
     * \brief   The signal, which is triggered when an element from combo-box is selected.
     **/
    void signalFiltersChanged(LogFilterBase * source);

protected:

    /**
     * \brief   Sets the widget of the header filter.
     * \param   widget  The widget to set.
     **/
    virtual void setWidget(QWidget* widget);

protected:
    QWidget*                            mWidget;//!< The widget to display data.
    QList<NELusanCommon::FilterData>    mData;  //!< The list of filter data items.
};

//////////////////////////////////////////////////////////////////////////
// LogComboFilterBase class declaration
//////////////////////////////////////////////////////////////////////////
class LogComboFilterBase : public LogFilterBase
{
    Q_OBJECT

public:
    explicit LogComboFilterBase(QWidget* parent = nullptr);

    /**
     * \brief   Updates and sets the filter data of widget
     * \param   data    The data to set in the widget.
     **/
    virtual void setDataString(const QString& data) override;

    /**
     * \brief   Updates and sets the items widget.
     *          Mainly required for combo-boxes.
     * \param   data    The list of data to set in combo-box
     **/
    virtual void setDataList(const std::vector<NELusanCommon::FilterData>& data) override;

    /**
     * \brief   Returns list of selected (checked) entries.
     **/
    virtual QList<NELusanCommon::FilterData> getSelectedData() const override;

    /**
     * \brief   Clears filter data.
     **/
    virtual void clearFilter(void) override;

protected:
    /**
     * \brief   Returns the list widget used to display filter items.
     **/
    inline QListWidget* listWidget() const;

};

//////////////////////////////////////////////////////////////////////////
// LogTextFilterBase class declaration
//////////////////////////////////////////////////////////////////////////
class LogTextFilterBase : public LogFilterBase
{
    Q_OBJECT

public:
    explicit LogTextFilterBase(bool extend, QWidget* parent = nullptr);

    /**
     * \brief   Updates and sets the filter data of widget
     * \param   data    The data to set in the widget.
     **/
    virtual void setDataString(const QString& data) override;

    /**
     * \brief   Updates and sets the items widget.
     *          Mainly required for combo-boxes.
     * \param   data    The list of data to set in combo-box
     **/
    virtual void setDataList(const std::vector<NELusanCommon::FilterData>& data) override;

    /**
     * \brief   Updates and sets the items of combo-box
     * \param   items   The list of entries to set in combo-box
     * \param   data    The list of data to set in combo-box.
     **/
    virtual void setDataItems(const QStringList& items, const NELusanCommon::AnyList& data) override;

    /**
     * \brief   Returns list of selected (checked) entries.
     **/
    virtual QList<NELusanCommon::FilterData> getSelectedData() const override;

    /**
     * \brief   Clears filter data.
     **/
    virtual void clearFilter(void) override;

    /**
     * \brief   Shows the filter widget and set focus on the list widget.
     **/
    virtual void showFilter(void) override;

protected:

    /**
     * \brief   Returns the line edit widget used to display filter items.
     **/
    inline QLineEdit* editWidget() const;

private:

    inline void _doSignalFilterChanged(const QString& text, bool isCaseSensitive, bool isWholeWord, bool isWildCard);
};

//////////////////////////////////////////////////////////////////////////
// LogPrioComboFilter class declaration
//////////////////////////////////////////////////////////////////////////

class LogPrioComboFilter : public LogComboFilterBase
{
    Q_OBJECT
public:
    explicit LogPrioComboFilter(QWidget* parent = nullptr);

    /**
     * \brief   Updates and sets the items of combo-box
     * \param   items   The list of entries to set in combo-box
     * \param   data    The list of data to set in combo-box.
     **/
    virtual void setDataItems(const QStringList& items, const NELusanCommon::AnyList& data) override;

    /**
     * \brief   Returns list of selected (checked) entries.
     **/
    virtual QList<NELusanCommon::FilterData> getSelectedData() const override;
};

//////////////////////////////////////////////////////////////////////////
// LogSourceComboFilter class declaration
//////////////////////////////////////////////////////////////////////////

class LogSourceComboFilter : public LogComboFilterBase
{
    Q_OBJECT
public:
    explicit LogSourceComboFilter(QWidget* parent = nullptr);

    /**
     * \brief   Updates and sets the items of combo-box
     * \param   items   The list of entries to set in combo-box
     * \param   data    The list of data to set in combo-box.
     **/
    virtual void setDataItems(const QStringList& items, const NELusanCommon::AnyList& data) override;

};

//////////////////////////////////////////////////////////////////////////
// LogSourceComboFilter class declaration
//////////////////////////////////////////////////////////////////////////

class LogSourceIdComboFilter : public LogComboFilterBase
{
    Q_OBJECT
public:
    explicit LogSourceIdComboFilter(QWidget* parent = nullptr);

    /**
     * \brief   Updates and sets the items of combo-box
     * \param   items   The list of entries to set in combo-box
     * \param   data    The list of data to set in combo-box.
     **/
    virtual void setDataItems(const QStringList& items, const NELusanCommon::AnyList& data) override;

};

//////////////////////////////////////////////////////////////////////////
// LogThreadComboFilter class declaration
//////////////////////////////////////////////////////////////////////////

class LogThreadComboFilter : public LogComboFilterBase
{
    Q_OBJECT
public:
    explicit LogThreadComboFilter(QWidget* parent = nullptr);

    /**
     * \brief   Updates and sets the items of combo-box
     * \param   items   The list of entries to set in combo-box
     * \param   data    The list of data to set in combo-box.
     **/
    virtual void setDataItems(const QStringList& items, const NELusanCommon::AnyList& data) override;

};

//////////////////////////////////////////////////////////////////////////
// LogThreadIdComboFilter class declaration
//////////////////////////////////////////////////////////////////////////

class LogThreadIdComboFilter : public LogComboFilterBase
{
    Q_OBJECT
public:
    explicit LogThreadIdComboFilter(QWidget* parent = nullptr);

    /**
     * \brief   Updates and sets the items of combo-box
     * \param   items   The list of entries to set in combo-box
     * \param   data    The list of data to set in combo-box.
     **/
    virtual void setDataItems(const QStringList& items, const NELusanCommon::AnyList& data) override;

};

//////////////////////////////////////////////////////////////////////////
// LogDurationEditFilter class declaration
//////////////////////////////////////////////////////////////////////////

class LogDurationEditFilter : public LogTextFilterBase
{
    Q_OBJECT

public:
    explicit LogDurationEditFilter(QWidget* parent = nullptr);

    /**
     * \brief   Returns list of selected (checked) entries.
     **/
    virtual QList<NELusanCommon::FilterData> getSelectedData() const override;
};

//////////////////////////////////////////////////////////////////////////
// LogMessageEditFilter class declaration
//////////////////////////////////////////////////////////////////////////

class LogMessageEditFilter : public LogTextFilterBase
{
    Q_OBJECT

public:
    explicit LogMessageEditFilter(QWidget* parent = nullptr);

    /**
     * \brief   Returns list of selected (checked) entries.
     **/
    virtual QList<NELusanCommon::FilterData> getSelectedData() const override;
};

//////////////////////////////////////////////////////////////////////////
// inline methods
//////////////////////////////////////////////////////////////////////////
inline const QList<NELusanCommon::FilterData>& LogFilterBase::getData(void) const
{
    return mData;
}

inline QWidget* LogFilterBase::getWidget(void) const
{
    return mWidget;
}

#endif // LUSAN_VIEW_LOG_LOGFILTERWIDGETS_HPP
