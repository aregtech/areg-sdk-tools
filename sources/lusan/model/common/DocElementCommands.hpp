#ifndef LUSAN_MODEL_COMMON_DOCELEMENTCOMMANDS_HPP
#define LUSAN_MODEL_COMMON_DOCELEMENTCOMMANDS_HPP
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
 *  \file        lusan/model/common/DocElementCommands.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, generic element mutation commands shared by both editors.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/model/common/DocCommand.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"

#include "lusan/common/ElementBase.hpp"
#include "lusan/data/common/TEDataContainer.hpp"

#include <functional>
#include <type_traits>

/**
 * \class   TDocElementCommand
 * \brief   Shared machinery to add/remove one element of a TEDataContainer section. Works
 *          for value entries (AttributeEntry, ConstantEntry, SMTimerEntry) and for
 *          owning-pointer entries (DataTypeCustom*, SIMethodBase*, SMStateEntry*): while
 *          the element sits outside the model between undo/redo, this command owns it, so
 *          its subtree is captured for undo and re-inserted under its original ID.
 *
 *          IDs are allocated by the first insertion and reused verbatim afterwards:
 *          re-inserting the same element at its recorded index restores the section's exact
 *          ID layout, so history navigation allocates nothing and never duplicates an ID.
 *
 * \tparam  Data     The container's element type — a value type or an owning pointer type.
 * \tparam  ElemBase The container's element base (DocumentElem or ElementBase), matching
 *                   the concrete section's TEDataContainer instantiation.
 **/
template<typename Data, typename ElemBase>
class TDocElementCommand : public DocCommand
{
protected:
    using Container = TEDataContainer<Data, ElemBase>;

    TDocElementCommand(DocModelNotifier& notifier, Container& container, eDocElementKind kind, const QString& text, QUndoCommand* parent)
        : DocCommand (notifier, text, parent)
        , mContainer (container)
        , mKind      (kind)
    {
    }

    ~TDocElementCommand() override
    {
        if constexpr (std::is_pointer_v<Data>)
        {
            if (mOwned)
            {
                delete mElement;
            }
        }
    }

    //!< The element as its ID-bearing base, whether stored by value or by pointer.
    ElementBase* elem() const
    {
        if constexpr (std::is_pointer_v<Data>)
        {
            return mElement;
        }
        else
        {
            return const_cast<Data*>(&mElement);
        }
    }

    void doInsert()
    {
        if (mIndex < 0)
        {
            mContainer.addElement(mElement);
        }
        else
        {
            mContainer.insertElement(mIndex, mElement);
        }

        mOwned = false;
        mId    = elem()->getId();
        mIndex = mContainer.findIndex(mId);
        notifier().notifyElementAdded(mId, mKind);
    }

    void doRemove()
    {
        mIndex = mContainer.findIndex(mId);
        mContainer.removeElement(mId, &mElement);
        mOwned = true;
        notifier().notifyElementRemoved(mId, mKind);
    }

protected:
    Container&      mContainer;
    eDocElementKind mKind;
    Data            mElement {};            //!< Owned copy/pointer while the element is out of the model.
    uint32_t        mId      { 0 };
    int             mIndex   { -1 };        //!< Recorded list position (-1 = append).
    bool            mOwned   { false };     //!< True when this command, not the model, owns the element.
};

/**
 * \class   TDocAddCommand
 * \brief   Adds a pre-built element to a section (appended in document order). Redo inserts;
 *          undo removes and captures the element for the next redo.
 **/
template<typename Data, typename ElemBase>
class TDocAddCommand : public TDocElementCommand<Data, ElemBase>
{
    using Base = TDocElementCommand<Data, ElemBase>;
public:
    TDocAddCommand(DocModelNotifier& notifier, typename Base::Container& container, Data element, eDocElementKind kind, const QString& text, QUndoCommand* parent = nullptr)
        : Base(notifier, container, kind, text, parent)
    {
        this->mElement = std::move(element);
        this->mOwned   = true;
    }

    void redo() override { this->doInsert(); }
    void undo() override { this->doRemove(); }
};

/**
 * \class   TDocRemoveCommand
 * \brief   Removes an element (with its owned subtree) from a section. Redo removes and
 *          captures; undo re-inserts it under its original ID at its original position.
 **/
template<typename Data, typename ElemBase>
class TDocRemoveCommand : public TDocElementCommand<Data, ElemBase>
{
    using Base = TDocElementCommand<Data, ElemBase>;
public:
    TDocRemoveCommand(DocModelNotifier& notifier, typename Base::Container& container, uint32_t id, eDocElementKind kind, const QString& text, QUndoCommand* parent = nullptr)
        : Base(notifier, container, kind, text, parent)
    {
        this->mId = id;
    }

    void redo() override { this->doRemove(); }
    void undo() override { this->doInsert(); }
};

/**
 * \class   TDocSetPropertyCommand
 * \brief   Sets one property of an element through a getter/setter pair, capturing the
 *          previous value for undo. Kept generic so every scalar field edit is one command
 *          shape rather than one class per field.
 **/
template<typename Value>
class TDocSetPropertyCommand : public DocCommand
{
public:
    using Getter = std::function<Value()>;
    using Setter = std::function<void(const Value&)>;

    TDocSetPropertyCommand(DocModelNotifier& notifier, uint32_t id, eDocElementKind kind, Getter getter, Setter setter, Value newValue, const QString& text, QUndoCommand* parent = nullptr)
        : DocCommand(notifier, text, parent)
        , mGet      (std::move(getter))
        , mSet      (std::move(setter))
        , mOld      ( )
        , mNew      (std::move(newValue))
        , mId       (id)
        , mKind     (kind)
    {
    }

    void redo() override
    {
        mOld = mGet();
        mSet(mNew);
        notifier().notifyElementChanged(mId, mKind);
    }

    void undo() override
    {
        mSet(mOld);
        notifier().notifyElementChanged(mId, mKind);
    }

private:
    Getter          mGet;
    Setter          mSet;
    Value           mOld;
    Value           mNew;
    uint32_t        mId;
    eDocElementKind mKind;
};

/**
 * \class   TDocReorderCommand
 * \brief   Swaps two entries of a section — the priority/model-order gesture. The container
 *          keeps IDs in list order, so redo and undo are the same swap; each is its own
 *          inverse.
 **/
template<typename Data, typename ElemBase>
class TDocReorderCommand : public DocCommand
{
public:
    TDocReorderCommand(DocModelNotifier& notifier, TEDataContainer<Data, ElemBase>& container, int index1, int index2, uint32_t ownerId, eDocElementKind kind, const QString& text, QUndoCommand* parent = nullptr)
        : DocCommand (notifier, text, parent)
        , mContainer (container)
        , mIndex1    (index1)
        , mIndex2    (index2)
        , mOwnerId   (ownerId)
        , mKind      (kind)
    {
    }

    void redo() override { apply(); }
    void undo() override { apply(); }

private:
    void apply()
    {
        mContainer.swapElements(mIndex1, mIndex2);
        notifier().notifyListReordered(mOwnerId, mKind);
    }

private:
    TEDataContainer<Data, ElemBase>&    mContainer;
    int                                 mIndex1;
    int                                 mIndex2;
    uint32_t                            mOwnerId;
    eDocElementKind                     mKind;
};

/**
 * \brief   Builds a composite that inserts a pre-built element at an explicit position
 *          without ever renumbering siblings. A direct TEDataContainer::insertElement()
 *          mid-list calls reorderIds(), which reassigns every sibling's ID by its new list
 *          position — harmless for a fresh redo, but that broadcast renumbering is not
 *          itself undone by removing the inserted element, so undo would leave siblings
 *          with the wrong IDs. This builds the safe equivalent instead: append (a no-op for
 *          reorderIds(), since the appended element already sorts last) and then bubble it
 *          down to \p position via a chain of adjacent TDocReorderCommand swaps, each of
 *          which is its own exact inverse and touches only the two swapped entries.
 * \param   position    The target index; the composite is a plain append if position is at
 *                       or past \p appendIndex.
 * \param   appendIndex The index the append will actually land at when this composite's Add
 *                       child runs its redo(). Equal to the container's current element count
 *                       when this is the only command touching the container; the caller must
 *                       pass the count as it will be *after* any sibling command that removes
 *                       from the same container runs first (e.g. a category-conversion
 *                       composite's remove-then-insert), since that sibling has not run yet
 *                       at the point this composite is built.
 **/
template<typename Data, typename ElemBase>
QUndoCommand* buildInsertCommandAt(DocModelNotifier& notifier, TEDataContainer<Data, ElemBase>& container, Data element, int position, int appendIndex, uint32_t ownerId, eDocElementKind kind, const QString& text, QUndoCommand* parent = nullptr)
{
    DocCompositeCommand* composite = new DocCompositeCommand(notifier, text, parent);
    new TDocAddCommand<Data, ElemBase>(notifier, container, std::move(element), kind, text, composite);
    for (int i = appendIndex; i > position; --i)
    {
        new TDocReorderCommand<Data, ElemBase>(notifier, container, i, i - 1, ownerId, kind, text, composite);
    }

    return composite;
}

/**
 * \brief   Convenience over buildInsertCommandAt() for the common case: no sibling command in
 *          the same composite mutates \p container first, so the append index is simply the
 *          container's current element count.
 **/
template<typename Data, typename ElemBase>
QUndoCommand* buildInsertCommand(DocModelNotifier& notifier, TEDataContainer<Data, ElemBase>& container, Data element, int position, uint32_t ownerId, eDocElementKind kind, const QString& text, QUndoCommand* parent = nullptr)
{
    return buildInsertCommandAt(notifier, container, std::move(element), position, container.getElementCount(), ownerId, kind, text, parent);
}

#endif  // LUSAN_MODEL_COMMON_DOCELEMENTCOMMANDS_HPP
