#ifndef LUSAN_VIEW_SM_SMCANVASITEM_HPP
#define LUSAN_VIEW_SM_SMCANVASITEM_HPP
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
 *  \file        lusan/view/sm/SMCanvasItem.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM design canvas item base.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QGraphicsItem>

#include <cstdint>

/**
 * \class   SMCanvasItem
 * \brief   The base of every diagram item. It links the item to its document element
 *          by ID (the only model link an item may hold), keeps the scene's ID lookup
 *          up to date, and snaps interactive position changes to the grid.
 **/
class SMCanvasItem : public QGraphicsItem
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Creates the item of a document element.
     * \param   elementId   The document element ID the item renders.
     * \param   parent      The parent graphics item.
     **/
    explicit SMCanvasItem(uint32_t elementId, QGraphicsItem* parent = nullptr);
    virtual ~SMCanvasItem();

//////////////////////////////////////////////////////////////////////////
// Attributes
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns the ID of the document element the item renders.
     **/
    inline uint32_t getElementId() const;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    const uint32_t  mElementId; //!< The rendered document element's ID.
};

//////////////////////////////////////////////////////////////////////////
// SMCanvasItem inline methods
//////////////////////////////////////////////////////////////////////////

inline uint32_t SMCanvasItem::getElementId() const
{
    return mElementId;
}

#endif  // LUSAN_VIEW_SM_SMCANVASITEM_HPP
