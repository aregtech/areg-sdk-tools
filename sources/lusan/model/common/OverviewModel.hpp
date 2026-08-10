#ifndef LUSAN_MODEL_COMMON_OVERVIEWMODEL_HPP
#define LUSAN_MODEL_COMMON_OVERVIEWMODEL_HPP
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
 *  \file        lusan/model/common/OverviewModel.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the Overview page model shared by every document editor.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/OverviewDataSection.hpp"
#include "lusan/model/common/DocElementCommands.hpp"
#include "lusan/model/common/IEDocumentModel.hpp"

#include <QObject>
#include <functional>

/**
 * \class   OverviewModel
 * \brief   The Overview page model. It reads the document's `Overview` section and routes every
 *          edit through an undo command, so the page never mutates the section directly.
 *
 *          The section is asked for on every access: a document that opens or reloads a file
 *          swaps the data object underneath, and a reference kept from construction would dangle.
 *          What a document declares beyond the shared rows -- a service category, a threading
 *          mode -- is added by its own model over this one.
 **/
class OverviewModel
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit OverviewModel(IEDocumentModel& document);

    virtual ~OverviewModel(void) = default;

//////////////////////////////////////////////////////////////////////////
// Reads
//////////////////////////////////////////////////////////////////////////
public:
    const QString& getName(void) const;
    const VersionNumber& getVersion(void) const;
    const QString& getDescription(void) const;
    bool getIsDeprecated(void) const;
    const QString& getDeprecateHint(void) const;

    //!< The document ID of the Overview element, which every change notification carries.
    uint32_t getOverviewId(void) const;

    DocModelNotifier& getNotifier(void) const;
    inline IEDocumentModel& getDocument(void) const;

//////////////////////////////////////////////////////////////////////////
// Mutations
//////////////////////////////////////////////////////////////////////////
public:
    void setName(const QString& name);
    void setVersion(const VersionNumber& version);
    void setDescription(const QString& description);

    /**
     * \brief   Marks the document deprecated, or takes the mark off and the hint with it. Both
     *          are one undo step, because unchecking the box and clearing the hint is one gesture.
     **/
    void setIsDeprecated(bool isDeprecated);
    void setDeprecateHint(const QString& hint);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
protected:
    //!< The live Overview section of whichever document is open now.
    inline OverviewDataSection& section(void) const;

    /**
     * \brief   Pushes one property edit onto the document's undo stack, reported as a change of
     *          the Overview element. The getter and the setter reach the section themselves, so
     *          the command survives a document reload the same way the model does.
     **/
    template<typename Value>
    void pushProperty( std::function<Value()> getter
                     , std::function<void(const Value&)> setter
                     , const Value& newValue
                     , const QString& text);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    IEDocumentModel&    mDocument;  //!< The document being edited.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    OverviewModel(void) = delete;
    OverviewModel(const OverviewModel& /*src*/) = delete;
    OverviewModel& operator = (const OverviewModel& /*src*/) = delete;
};

//////////////////////////////////////////////////////////////////////////
// OverviewModel inline methods
//////////////////////////////////////////////////////////////////////////

inline IEDocumentModel& OverviewModel::getDocument(void) const
{
    return mDocument;
}

inline OverviewDataSection& OverviewModel::section(void) const
{
    return mDocument.getOverviewSection();
}

template<typename Value>
void OverviewModel::pushProperty( std::function<Value()> getter
                                , std::function<void(const Value&)> setter
                                , const Value& newValue
                                , const QString& text)
{
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<Value>( getNotifier(), getOverviewId()
                                                                  , eDocElementKind::Overview
                                                                  , getter, setter, newValue, text));
}

#endif  // LUSAN_MODEL_COMMON_OVERVIEWMODEL_HPP
