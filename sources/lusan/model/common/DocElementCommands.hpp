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

    virtual ~TDocElementCommand(void) override
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
    ElementBase* elem(void) const
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

    void doInsert(void)
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

    void doRemove(void)
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

    virtual void redo(void) override { this->doInsert(); }
    virtual void undo(void) override { this->doRemove(); }
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

    virtual void redo(void) override { this->doRemove(); }
    virtual void undo(void) override { this->doInsert(); }
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
    using Getter = std::function<Value(void)>;
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

    virtual void redo(void) override
    {
        mOld = mGet();
        mSet(mNew);
        notifier().notifyElementChanged(mId, mKind);
    }

    virtual void undo(void) override
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

    virtual void redo(void) override { apply(); }
    virtual void undo(void) override { apply(); }

private:
    void apply(void)
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

#endif  // LUSAN_MODEL_COMMON_DOCELEMENTCOMMANDS_HPP
