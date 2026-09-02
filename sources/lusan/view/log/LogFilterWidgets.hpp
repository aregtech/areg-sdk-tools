#ifndef LUSAN_VIEW_LOG_LOGFILTERWIDGETS_HPP
#define LUSAN_VIEW_LOG_LOGFILTERWIDGETS_HPP
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
 *  \file        lusan/view/log/LogFilterWidgets.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
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

/************************************************************************
 * Dependencies
 ************************************************************************/
class QLabel;
class QLineEdit;
class QListWidget;
class QToolButton;
class QVBoxLayout;

//////////////////////////////////////////////////////////////////////////
// LogFilterBase class declaration
//////////////////////////////////////////////////////////////////////////
/**
 * \brief   The panel a column of the log table drops under its header to be narrowed.
 *
 *          The panel is a window of its own. It is opened and closed as a whole, and its
 *          content is built once and kept for every following open.
 **/
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
     * \brief   Sets the phrase and the match options of the widget. A widget that carries no
     *          match options keeps the phrase alone.
     * \param   filter  The phrase and the options to match it with.
     **/
    virtual void setDataFilter(const NELusanCommon::FilterString& filter);

    /**
     * \brief   Drops what the column filters by and closes the panel.
     * \note    The content of the panel is never hidden, only the panel itself.
     **/
    virtual void clearFilter();

    /**
     * \brief   Shows the panel where it stands and moves the focus into it.
     **/
    virtual void showFilter();

    /**
     * \brief   Opens the panel under the given header section.
     *          The panel takes the width of the section unless its own content asks for
     *          more, and it stays inside the screen it opens on.
     * \param   anchor  The rectangle of the header section, in screen coordinates.
     **/
    void showFilterAt(const QRect& anchor);

    /**
     * \brief   Returns the list of filter data items.
     **/
    inline const QList<NELusanCommon::FilterData>& getData() const;

    /**
     * \brief   Returns the control the panel holds.
     **/
    inline QWidget* getWidget() const;

/************************************************************************
 * Signals
 ************************************************************************/
signals:
    /**
     * \brief   The signal, which is triggered when what the column filters by has changed.
     **/
    void signalFiltersChanged(LogFilterBase * source);

protected:

    /**
     * \brief   Sets the widget of the header filter.
     * \param   widget  The widget to set.
     **/
    virtual void setWidget(QWidget* widget);

    /**
     * \brief   Returns the size the panel opens at under a section of the given width.
     * \param   anchorWidth The width of the header section the panel belongs to.
     **/
    virtual QSize popupSize(int anchorWidth) const;

    /**
     * \brief   Returns the layout the panel stacks its content in.
     **/
    inline QVBoxLayout* panelLayout() const;

/************************************************************************
 * Overrides
 ************************************************************************/
protected:

    /**
     * \brief   Draws the ground and the border of the panel.
     **/
    void paintEvent(QPaintEvent* event) override;

    /**
     * \brief   Closes the panel on Escape.
     **/
    void keyPressEvent(QKeyEvent* event) override;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
protected:
    QWidget*                            mWidget;//!< The widget to display data.
    QList<NELusanCommon::FilterData>    mData;  //!< The list of filter data items.

private:
    QVBoxLayout*                        mLayout;//!< The layout the panel stacks its content in.
};

//////////////////////////////////////////////////////////////////////////
// LogComboFilterBase class declaration
//////////////////////////////////////////////////////////////////////////
/**
 * \brief   The panel of a column that is narrowed by picking values from a list.
 *          It carries the list, a find box for the long ones and the two shortcuts
 *          that pick every value or none of them.
 **/
class LogComboFilterBase : public LogFilterBase
{
    Q_OBJECT

public:
    explicit LogComboFilterBase(QWidget* parent = nullptr);

    /**
     * \brief   Updates and sets the filter data of widget
     * \param   data    The data to set in the widget.
     **/
    void setDataString(const QString& data) override;

    /**
     * \brief   Fills the list with the given entries, keeping what the reader had picked.
     * \param   data    The entries to show.
     **/
    void setDataList(const std::vector<NELusanCommon::FilterData>& data) override;

    /**
     * \brief   Fills the list with the values of the column.
     * \param   items   The names of the values.
     * \param   data    The value behind each name, in the same order.
     **/
    void setDataItems(const QStringList& items, const NELusanCommon::AnyList& data) override;

    /**
     * \brief   Returns list of selected (checked) entries.
     **/
    QList<NELusanCommon::FilterData> getSelectedData() const override;

    /**
     * \brief   Unpicks every entry and closes the panel.
     **/
    void clearFilter() override;

    /**
     * \brief   Picks the entry carrying the given value, or every entry but that one, and
     *          reports the change.
     * \param   value   The value to pick, in the form the list holds it.
     * \param   exclude True to pick every entry except the one carrying the value.
     * \return  True when the list holds the value.
     **/
    bool pickValue(const NELusanCommon::AnyData& value, bool exclude);

protected:

    /**
     * \brief   Returns the entry the list shows for one value of the column.
     * \param   name    The name of the value.
     * \param   data    The value itself.
     **/
    virtual QString itemLabel(const QString& name, const NELusanCommon::AnyData& data) const;

    /**
     * \brief   Returns the size the panel opens at under a section of the given width.
     **/
    QSize popupSize(int anchorWidth) const override;

    /**
     * \brief   Returns the list widget used to display filter items.
     **/
    inline QListWidget* listWidget() const;

private:

    //!< Sets every entry to the given state and reports the change once.
    void setAllChecked(bool checked);

    //!< Leaves in the list only the entries whose text carries the given phrase.
    void applySearch(const QString& phrase);

    //!< Writes how many entries of the list are picked.
    void updateSummary();

    //!< Returns the text of every entry the reader has picked.
    QSet<QString> checkedLabels() const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QLineEdit*      mSearch;    //!< The box that narrows the list itself, shown for the long lists.
    QLabel*         mSummary;   //!< The line that says how many entries are picked.
    QToolButton*    mBtnAll;    //!< Picks every entry of the list.
    QToolButton*    mBtnNone;   //!< Unpicks every entry of the list.
};

//////////////////////////////////////////////////////////////////////////
// LogTextFilterBase class declaration
//////////////////////////////////////////////////////////////////////////
/**
 * \brief   The panel of a column that is narrowed by a phrase.
 **/
class LogTextFilterBase : public LogFilterBase
{
    Q_OBJECT

public:
    explicit LogTextFilterBase(bool extend, QWidget* parent = nullptr);

    /**
     * \brief   Updates and sets the filter data of widget
     * \param   data    The data to set in the widget.
     **/
    void setDataString(const QString& data) override;

    /**
     * \brief   Does nothing. A phrase filter carries no list of values.
     **/
    void setDataList(const std::vector<NELusanCommon::FilterData>& data) override;

    /**
     * \brief   Does nothing. A phrase filter carries no list of values.
     **/
    void setDataItems(const QStringList& items, const NELusanCommon::AnyList& data) override;

    /**
     * \brief   Returns list of selected (checked) entries.
     **/
    QList<NELusanCommon::FilterData> getSelectedData() const override;

    /**
     * \brief   Sets the phrase and, when the widget carries them, the match options.
     * \param   filter  The phrase and the options to match it with.
     **/
    void setDataFilter(const NELusanCommon::FilterString& filter) override;

    /**
     * \brief   Drops the phrase, the match options with it, and closes the panel.
     **/
    void clearFilter() override;

    /**
     * \brief   Shows the panel and selects the phrase that is already in the box.
     **/
    void showFilter() override;

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
     * \brief   Returns the picked priorities merged into one mask.
     **/
    QList<NELusanCommon::FilterData> getSelectedData() const override;
};

//////////////////////////////////////////////////////////////////////////
// LogSourceComboFilter class declaration
//////////////////////////////////////////////////////////////////////////

class LogSourceComboFilter : public LogComboFilterBase
{
    Q_OBJECT
public:
    explicit LogSourceComboFilter(QWidget* parent = nullptr);

protected:
    /**
     * \brief   Returns the name of the source followed by its identifier.
     **/
    QString itemLabel(const QString& name, const NELusanCommon::AnyData& data) const override;
};

//////////////////////////////////////////////////////////////////////////
// LogSourceIdComboFilter class declaration
//////////////////////////////////////////////////////////////////////////

class LogSourceIdComboFilter : public LogComboFilterBase
{
    Q_OBJECT
public:
    explicit LogSourceIdComboFilter(QWidget* parent = nullptr);

protected:
    /**
     * \brief   Returns the identifier of the source.
     **/
    QString itemLabel(const QString& name, const NELusanCommon::AnyData& data) const override;
};

//////////////////////////////////////////////////////////////////////////
// LogThreadComboFilter class declaration
//////////////////////////////////////////////////////////////////////////

class LogThreadComboFilter : public LogComboFilterBase
{
    Q_OBJECT
public:
    explicit LogThreadComboFilter(QWidget* parent = nullptr);

protected:
    /**
     * \brief   Returns the name of the thread followed by its identifier.
     **/
    QString itemLabel(const QString& name, const NELusanCommon::AnyData& data) const override;
};

//////////////////////////////////////////////////////////////////////////
// LogThreadIdComboFilter class declaration
//////////////////////////////////////////////////////////////////////////

class LogThreadIdComboFilter : public LogComboFilterBase
{
    Q_OBJECT
public:
    explicit LogThreadIdComboFilter(QWidget* parent = nullptr);

protected:
    /**
     * \brief   Returns the identifier of the thread.
     **/
    QString itemLabel(const QString& name, const NELusanCommon::AnyData& data) const override;
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
     * \brief   Returns the shortest duration the column lets through, or nothing.
     **/
    QList<NELusanCommon::FilterData> getSelectedData() const override;
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
     * \brief   Returns the phrase and the match options the column filters by.
     **/
    QList<NELusanCommon::FilterData> getSelectedData() const override;
};

//////////////////////////////////////////////////////////////////////////
// inline methods
//////////////////////////////////////////////////////////////////////////
inline const QList<NELusanCommon::FilterData>& LogFilterBase::getData() const
{
    return mData;
}

inline QWidget* LogFilterBase::getWidget() const
{
    return mWidget;
}

inline QVBoxLayout* LogFilterBase::panelLayout() const
{
    return mLayout;
}

#endif // LUSAN_VIEW_LOG_LOGFILTERWIDGETS_HPP
