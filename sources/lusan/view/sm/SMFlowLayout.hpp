#ifndef LUSAN_VIEW_SM_SMFLOWLAYOUT_HPP
#define LUSAN_VIEW_SM_SMFLOWLAYOUT_HPP
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
 *  \file        lusan/view/sm/SMFlowLayout.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM structure lens flow layout: items wrap
 *               like text, so clause pills flow across lines.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QLayout>

#include <QList>
#include <QRect>

/**
 * \class   SMFlowLayout
 * \brief   The classic Qt flow layout: children are laid out left-to-right and wrap to the
 *          next line when the row is full -- the structure lens uses it so its pills and
 *          joiner words read like the guard text itself.
 **/
class SMFlowLayout : public QLayout
{
public:
    explicit SMFlowLayout(QWidget* parent = nullptr, int margin = 0, int hSpacing = 4, int vSpacing = 4);
    virtual ~SMFlowLayout();

    void addItem(QLayoutItem* item) override;
    int count() const override;
    QLayoutItem* itemAt(int index) const override;
    QLayoutItem* takeAt(int index) override;
    Qt::Orientations expandingDirections() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int width) const override;
    QSize minimumSize() const override;
    QSize sizeHint() const override;
    void setGeometry(const QRect& rect) override;

    //!< Removes and deletes every managed widget/item.
    void clearItems();

private:
    //!< Lays out (or measures when \p testOnly) the items in \p rect; returns the used height.
    int doLayout(const QRect& rect, bool testOnly) const;

private:
    QList<QLayoutItem*> mItems;     //!< The managed items in flow order.
    int                 mHSpace;    //!< The horizontal gap between items.
    int                 mVSpace;    //!< The vertical gap between rows.
};

#endif  // LUSAN_VIEW_SM_SMFLOWLAYOUT_HPP
