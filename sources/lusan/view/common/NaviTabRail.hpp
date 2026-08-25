#ifndef LUSAN_VIEW_COMMON_NAVITABRAIL_HPP
#define LUSAN_VIEW_COMMON_NAVITABRAIL_HPP
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
 *  \file        lusan/view/common/NaviTabRail.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       The vertical selector strip of the navigation panel.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/

#include <QWidget>

#include <QColor>
#include <QFont>
#include <QList>
#include <QRect>
#include <QString>

//////////////////////////////////////////////////////////////////////////
// NaviTabRail class declaration
//////////////////////////////////////////////////////////////////////////
/**
 * \brief   The strip of icons that picks the navigator shown in the navigation panel.
 *          It stands on the panel edge that faces the window frame, keeps one item
 *          current, and reports every user request through its signals. It stores no
 *          layout state of its own: the side, the caption mode and the item set are
 *          all set by the panel that owns it.
 **/
class NaviTabRail : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constants and types
//////////////////////////////////////////////////////////////////////////
public:

    //!< The panel edge the rail stands on.
    enum class eSide
    {
          West      //!< Left edge of the panel.
        , East      //!< Right edge of the panel.
    };

    //!< The mark drawn over the icon of an item.
    enum class eBadge
    {
          None      //!< No mark.
        , Active    //!< The navigator has a live source attached.
        , Attention //!< The navigator has something the user should look at.
    };

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit NaviTabRail(QWidget* parent = nullptr);

    virtual ~NaviTabRail(void) = default;

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Adds an item, or updates it when the identifier is already known.
     *          Items are kept sorted by identifier, so the position of an item never
     *          changes when another one is added or removed.
     * \param   id          The identifier of the item, also its place in the strip.
     * \param   iconPath    The resource path of the monochrome icon to draw.
     * \param   label       The caption shown under the icon in the labelled mode.
     * \param   hint        One line describing the navigator, shown in the tool tip.
     * \param   shortcut    The key sequence text shown in the tool tip, may be empty.
     **/
    void addItem(int id, const QString& iconPath, const QString& label, const QString& hint, const QString& shortcut);

    /**
     * \brief   Removes the item with the given identifier.
     **/
    void removeItem(int id);

    /**
     * \brief   True when an item with the given identifier is known to the rail.
     **/
    bool hasItem(int id) const;

    /**
     * \brief   Shows or hides an item without forgetting it. A hidden item stays in
     *          the context menu, so the user can bring it back.
     **/
    void setItemVisible(int id, bool visible);

    /**
     * \brief   True when the item exists and is drawn in the strip.
     **/
    bool isItemVisible(int id) const;

    /**
     * \brief   Sets the mark drawn over the icon of the given item.
     **/
    void setItemBadge(int id, NaviTabRail::eBadge badge);

    /**
     * \brief   Makes the given item current. Does nothing when the item is unknown or
     *          hidden. Emits signalItemActivated when the current item changes.
     **/
    void setCurrentItem(int id);

    /**
     * \brief   Returns the identifier of the current item, or -1 when there is none.
     **/
    inline int currentItem(void) const;

    /**
     * \brief   Returns the identifier of the first visible item, or -1 when the strip is empty.
     **/
    int firstVisibleItem(void) const;

    /**
     * \brief   Returns the number of visible items.
     **/
    int visibleCount(void) const;

    /**
     * \brief   Sets the panel edge the rail stands on.
     **/
    void setSide(NaviTabRail::eSide side);

    /**
     * \brief   Returns the panel edge the rail stands on.
     **/
    inline NaviTabRail::eSide side(void) const;

    /**
     * \brief   Turns the captions under the icons on or off. This is the user choice;
     *          a narrow panel may still drop them, see applyPanelWidth.
     **/
    void setLabelsPreferred(bool labels);

    /**
     * \brief   Returns the caption choice of the user.
     **/
    inline bool labelsPreferred(void) const;

    /**
     * \brief   Recomputes the width of the rail for the room the whole panel has.
     *          Captions are dropped while the panel is too narrow to carry both them
     *          and a usable content area.
     * \param   panelWidth  The width of the navigation panel in pixels.
     **/
    void applyPanelWidth(int panelWidth);

signals:

    /**
     * \brief   The current item changed, by mouse, by keyboard or by setCurrentItem.
     **/
    void signalItemActivated(int id);

    /**
     * \brief   The user clicked the item that is already current, asking the panel to collapse.
     **/
    void signalCurrentItemClicked(void);

    /**
     * \brief   The user asked to show or hide an item from the context menu.
     **/
    void signalItemVisibilityToggled(int id, bool visible);

    /**
     * \brief   The user turned the captions on or off from the context menu.
     **/
    void signalLabelsToggled(bool labels);

    /**
     * \brief   The user asked to close the whole navigation panel.
     **/
    void signalHidePanelRequested(void);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void leaveEvent(QEvent* event) override;
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void focusInEvent(QFocusEvent* event) override;
    virtual void focusOutEvent(QFocusEvent* event) override;
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void contextMenuEvent(QContextMenuEvent* event) override;
    virtual void changeEvent(QEvent* event) override;
    virtual QSize sizeHint(void) const override;
    virtual QSize minimumSizeHint(void) const override;

//////////////////////////////////////////////////////////////////////////
// Hidden types and methods
//////////////////////////////////////////////////////////////////////////
private:

    //!< One entry of the strip.
    struct sRailItem
    {
        int             id;         //!< The identifier and the sort key of the item.
        QString         iconPath;   //!< The resource path of the icon.
        QString         label;      //!< The caption drawn under the icon.
        QString         hint;       //!< The description line of the tool tip.
        QString         shortcut;   //!< The key sequence text of the tool tip.
        NaviTabRail::eBadge badge;  //!< The mark drawn over the icon.
        bool            visible;    //!< False while the item is kept out of the strip.
        QRect           rect;       //!< The area of the item, empty while it does not fit.
    };

    //!< Returns the position of the item in the list, or -1.
    int indexOf(int id) const;

    //!< Returns the position of the item under the point, -1 for none.
    int indexAt(const QPoint& pos) const;

    //!< Recomputes the item and rail sizes from the current font and caption mode.
    void recalcMetrics(void);

    //!< Recomputes the item areas and decides how many of them fit.
    void relayout(void);

    //!< Moves the keyboard focus by the given number of visible items.
    void stepFocus(int delta);

    //!< Makes the given position current and tells the owner about it.
    void activateIndex(int index);

    //!< Opens the menu of the items that did not fit in the strip.
    void showOverflowMenu(void);

    //!< Builds the tool tip text of the item at the given position.
    QString toolTipOf(int index) const;

    //!< Returns the font of the captions.
    QFont captionFont(void) const;

    //!< Returns the icon of the item filled with the given color.
    QPixmap itemPixmap(const NaviTabRail::sRailItem& item, const QColor& color) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QList<sRailItem>    mItems;             //!< The entries of the strip, sorted by identifier.
    int                 mCurrent;           //!< The position of the current item, -1 for none.
    int                 mHovered;           //!< The position under the mouse, -1 for none.
    int                 mFocused;           //!< The position the keyboard focus is on, -1 for none.
    eSide               mSide;              //!< The panel edge the rail stands on.
    bool                mLabelsPreferred;   //!< The caption choice of the user.
    bool                mLabels;            //!< True while the captions are actually drawn.
    int                 mCaptionLines;      //!< The number of caption lines every item reserves.
    int                 mIconExtent;        //!< The edge of the icon square in pixels.
    int                 mItemHeight;        //!< The height of one item in pixels.
    int                 mRailWidth;         //!< The width of the rail in the current mode.
    int                 mLabelledWidth;     //!< The width the rail needs to carry captions.
    QRect               mOverflowRect;      //!< The area of the overflow button, empty when unused.
    bool                mOverflowHovered;   //!< True while the mouse rests on the overflow button.
};

//////////////////////////////////////////////////////////////////////////
// NaviTabRail class inline methods
//////////////////////////////////////////////////////////////////////////

inline int NaviTabRail::currentItem(void) const
{
    return ((mCurrent >= 0) && (mCurrent < mItems.size()) ? mItems.at(mCurrent).id : -1);
}

inline NaviTabRail::eSide NaviTabRail::side(void) const
{
    return mSide;
}

inline bool NaviTabRail::labelsPreferred(void) const
{
    return mLabelsPreferred;
}

#endif  // LUSAN_VIEW_COMMON_NAVITABRAIL_HPP
