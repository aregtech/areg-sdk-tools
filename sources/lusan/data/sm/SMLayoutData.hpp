#ifndef LUSAN_DATA_SM_SMLAYOUTDATA_HPP
#define LUSAN_DATA_SM_SMLAYOUTDATA_HPP
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
 *  \file        lusan/data/sm/SMLayoutData.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM editor-only layout data
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/DocumentElem.hpp"

#include <QList>
#include <QPointF>
#include <QString>

/**
 * \brief   `View`: the saved zoom/scroll of a machine level, keyed by the
 *          `Owner` element ID (the level's composite state, or the Overview for level 0).
 **/
struct SMLayoutView
{
    uint32_t    owner   { 0 };       //!< The owning element ID.
    int         zoom    { 100 };     //!< The zoom percentage.
    double      x       { 0.0 };     //!< The scene point shown at the viewport center (X).
    double      y       { 0.0 };     //!< The scene point shown at the viewport center (Y).
};

/**
 * \brief   `Node`: a state's box geometry and appearance, keyed by `Owner`.
 **/
struct SMLayoutNode
{
    uint32_t    owner       { 0 };       //!< The owning state ID.
    double      x           { 0.0 };
    double      y           { 0.0 };
    double      width       { 0.0 };
    double      height      { 0.0 };
    QString     color       { };         //!< Optional fill color (empty = default).
    QString     headerColor { };         //!< Optional header color (empty = default).
    bool        hasExpanded { false };   //!< Whether the Expanded flag is present.
    bool        expanded    { true };    //!< The expanded/collapsed state (composite nodes).
};

/**
 * \brief   `Edge`: a transition's routed geometry, keyed by `Owner`.
 **/
struct SMLayoutEdge
{
    enum class eShape
    {
          Line  //!< A straight/polyline edge (default).
        , Arc   //!< A curved edge (uses `bulge`).
    };

    uint32_t        owner   { 0 };                  //!< The owning transition ID.
    eShape          shape   { eShape::Line };       //!< The edge shape.
    double          bulge   { 0.0 };                //!< The arc bulge factor.
    QString         color   { };                    //!< Optional color (empty = default).
    QList<QPointF>  points  { };                    //!< The waypoints (2 or more).
    bool            hasLabel{ false };              //!< Whether a label position is set.
    QPointF         label   { };                    //!< The label anchor position.
};

/**
 * \brief   `Note`: a diagram-only annotation with its own document ID; it
 *          belongs to a machine level (`Level` = that level's owner ID, 0 for the root).
 **/
struct SMLayoutNote
{
    uint32_t    id      { 0 };       //!< The note's own document ID.
    uint32_t    level   { 0 };       //!< The owning level's element ID (the Overview ID at the root level).
    uint32_t    owner   { 0 };       //!< The bound state/transition ID, or 0 for a free (unowned) note.
    double      x       { 0.0 };
    double      y       { 0.0 };
    double      width   { 0.0 };
    double      height  { 0.0 };
    QString     color   { };         //!< Optional color (empty = default).
    QString     text    { };         //!< The annotation text.
};

//////////////////////////////////////////////////////////////////////////
// SMLayoutData class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \class   SMLayoutData
 * \brief   The `Layout` section: editor-only geometry. It owns four keyed
 *          lists (View/Node/Edge by owner element ID; Note by its own ID). Logical
 *          elements never reference layout; layout references logical elements by ID
 *          only, so pure renames never touch it. Entries are written in ascending key
 *          order, so a saved file does not depend on the order edits created the entries.
 **/
class SMLayoutData : public DocumentElem
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    SMLayoutData(ElementBase* parent = nullptr);
    virtual ~SMLayoutData() = default;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    bool isValid() const override;
    bool readFromXml(QXmlStreamReader& xml) override;
    void writeToXml(QXmlStreamWriter& xml) const override;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    inline int getGridSize() const;
    inline void setGridSize(int gridSize);
    inline bool isGridVisible() const;
    inline void setGridVisible(bool visible);

    inline const QList<SMLayoutView>& getViews() const;
    inline const QList<SMLayoutNode>& getNodes() const;
    inline const QList<SMLayoutEdge>& getEdges() const;
    inline const QList<SMLayoutNote>& getNotes() const;

    SMLayoutView& addView(uint32_t owner);
    SMLayoutNode& addNode(uint32_t owner);
    SMLayoutEdge& addEdge(uint32_t owner);

    /**
     * \brief   Adds a note at the given level, allocating its ID from the document counter.
     * \param   level   The owning level's element ID (0 for the root level).
     * \param   owner   The bound state/transition ID, or 0 for a free (unowned) note.
     * \return  Reference to the created note.
     **/
    SMLayoutNote& addNote(uint32_t level, uint32_t owner = 0);

    SMLayoutView* findView(uint32_t owner);
    SMLayoutNode* findNode(uint32_t owner);
    SMLayoutEdge* findEdge(uint32_t owner);
    SMLayoutNote* findNote(uint32_t id);

    /**
     * \brief   Returns the note bound to the given state/transition owner ID, or nullptr.
     *          A zero owner never matches (free notes have no owner).
     **/
    SMLayoutNote* findNoteByOwner(uint32_t owner);

    const SMLayoutView* findView(uint32_t owner) const;
    const SMLayoutNode* findNode(uint32_t owner) const;
    const SMLayoutEdge* findEdge(uint32_t owner) const;
    const SMLayoutNote* findNote(uint32_t id) const;
    const SMLayoutNote* findNoteByOwner(uint32_t owner) const;

    /**
     * \brief   Removes the View entry of a level owner, if present.
     * \return  True if an entry was removed.
     **/
    bool removeView(uint32_t owner);

    /**
     * \brief   Removes the Node entry of a state, if present. A composite state owns both
     *          a Node and the View of its sublevel, so node-only edits must not use
     *          removeOwned(), which drops every entry of the owner.
     * \return  True if an entry was removed.
     **/
    bool removeNode(uint32_t owner);

    /**
     * \brief   Removes the note with the given ID, if present.
     * \return  True if a note was removed.
     **/
    bool removeNote(uint32_t id);

    /**
     * \brief   Re-inserts a previously removed note exactly as captured (its own ID
     *          included); never allocates a new ID. Used to undo a note removal.
     * \return  Reference to the re-inserted note.
     **/
    SMLayoutNote& restoreNote(const SMLayoutNote& note);

    /**
     * \brief   Removes every View/Node/Edge owned by any of the given element IDs. Used
     *          when logical elements are deleted so their layout is deleted with them
     *          (data-layer helper so every delete path stays consistent).
     * \return  The number of layout entries removed.
     **/
    int removeOwned(const QList<uint32_t>& ownerIds);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    int                     mGridSize;      //!< The grid size (editor state).
    bool                    mGridVisible;   //!< Whether the grid is visible.
    QList<SMLayoutView>     mViews;         //!< The per-level views.
    QList<SMLayoutNode>     mNodes;         //!< The state nodes.
    QList<SMLayoutEdge>     mEdges;         //!< The transition edges.
    QList<SMLayoutNote>     mNotes;         //!< The notes.
};

//////////////////////////////////////////////////////////////////////////
// SMLayoutData inline methods
//////////////////////////////////////////////////////////////////////////

inline int SMLayoutData::getGridSize() const
{
    return mGridSize;
}

inline void SMLayoutData::setGridSize(int gridSize)
{
    mGridSize = gridSize;
}

inline bool SMLayoutData::isGridVisible() const
{
    return mGridVisible;
}

inline void SMLayoutData::setGridVisible(bool visible)
{
    mGridVisible = visible;
}

inline const QList<SMLayoutView>& SMLayoutData::getViews() const
{
    return mViews;
}

inline const QList<SMLayoutNode>& SMLayoutData::getNodes() const
{
    return mNodes;
}

inline const QList<SMLayoutEdge>& SMLayoutData::getEdges() const
{
    return mEdges;
}

inline const QList<SMLayoutNote>& SMLayoutData::getNotes() const
{
    return mNotes;
}

#endif  // LUSAN_DATA_SM_SMLAYOUTDATA_HPP
