#ifndef LUSAN_MODEL_SM_SMSELECTIONMODEL_HPP
#define LUSAN_MODEL_SM_SMSELECTIONMODEL_HPP
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
 *  \file        lusan/model/sm/SMSelectionModel.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM document-wide selection model.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QList>
#include <QObject>
#include <cstdint>

/**
 * \class   SMSelectionModel
 * \brief   The single per-document selection: the selected element IDs and the machine
 *          level being edited. The canvas, outline, properties, and search results all
 *          read and drive this one object instead of mirroring each other pairwise.
 **/
class SMSelectionModel : public QObject
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMSelectionModel(QObject* parent = nullptr);
    virtual ~SMSelectionModel() = default;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns the selected element IDs in selection order.
     **/
    inline const QList<uint32_t>& getSelection() const;

    /**
     * \brief   Returns true if the given element is selected.
     **/
    inline bool isSelected(uint32_t id) const;

    /**
     * \brief   Returns true if nothing is selected.
     **/
    inline bool isEmpty() const;

    /**
     * \brief   Returns the ID of the machine level being edited
     *          (the Overview ID for the root level).
     **/
    inline uint32_t getActiveLevel() const;

    /**
     * \brief   Replaces the whole selection. Emits signalSelectionChanged()
     *          only when the effective selection differs.
     **/
    void setSelection(const QList<uint32_t>& ids);

    /**
     * \brief   Adds an element to the selection.
     **/
    void select(uint32_t id);

    /**
     * \brief   Removes an element from the selection.
     **/
    void deselect(uint32_t id);

    /**
     * \brief   Toggles an element's selection.
     **/
    void toggle(uint32_t id);

    /**
     * \brief   Clears the selection.
     **/
    void clearSelection();

    /**
     * \brief   Switches the edited machine level; the selection is cleared,
     *          it never spans levels.
     **/
    void setActiveLevel(uint32_t levelId);

    /**
     * \brief   Resets to the initial empty state (no selection, no level).
     *          Used when the document is replaced.
     **/
    void reset();

//////////////////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////////////////////
signals:
    /**
     * \brief   Emitted when the set of selected elements changed.
     * \param   selected    The complete new selection.
     **/
    void signalSelectionChanged(const QList<uint32_t>& selected);

    /**
     * \brief   Emitted when the edited machine level changed.
     * \param   levelId     The new level owner ID.
     **/
    void signalActiveLevelChanged(uint32_t levelId);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QList<uint32_t> mSelection;     //!< The selected element IDs, in selection order.
    uint32_t        mActiveLevel;   //!< The edited level's owner element ID.
};

//////////////////////////////////////////////////////////////////////////
// SMSelectionModel inline methods
//////////////////////////////////////////////////////////////////////////

inline const QList<uint32_t>& SMSelectionModel::getSelection() const
{
    return mSelection;
}

inline bool SMSelectionModel::isSelected(uint32_t id) const
{
    return mSelection.contains(id);
}

inline bool SMSelectionModel::isEmpty() const
{
    return mSelection.isEmpty();
}

inline uint32_t SMSelectionModel::getActiveLevel() const
{
    return mActiveLevel;
}

#endif  // LUSAN_MODEL_SM_SMSELECTIONMODEL_HPP
