#ifndef LUSAN_DATA_COMMON_METHODDATASECTION_HPP
#define LUSAN_DATA_COMMON_METHODDATASECTION_HPP
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
 *  \file        lusan/data/common/MethodDataSection.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the `MethodList` section shared by every document editor.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/DocumentElem.hpp"
#include "lusan/data/common/MethodEntry.hpp"
#include "lusan/data/common/TEDataContainer.hpp"

#include <QList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

/************************************************************************
 * Dependencies
 ************************************************************************/
class DataTypeDataSection;

namespace NEMethod
{
    /**
     * \brief   The service interface kinds, in the order the editor offers them.
     **/
    constexpr int SiRequest     { 0 };
    constexpr int SiResponse    { 1 };
    constexpr int SiBroadcast   { 2 };

    /**
     * \brief   The state machine kinds, in the order the editor offers them.
     **/
    constexpr int SmTrigger     { 0 };
    constexpr int SmAction      { 1 };
    constexpr int SmCondition   { 2 };

    /**
     * \brief   A service interface declares requests, the responses that answer them, and
     *          broadcasts. A request names its response; nothing returns a value.
     **/
    const MethodConfig& serviceInterface();

    /**
     * \brief   A state machine declares triggers, actions and conditions. A condition returns a
     *          value and says whether it is implemented here or by a handler.
     **/
    const MethodConfig& stateMachine();
}

/**
 * \class   MethodDataSection
 * \brief   The `MethodList` section: the document's declared methods, in document order. Entries
 *          are held by owning pointer so their addresses -- and with them their parameters'
 *          parent chain for ID allocation -- stay stable while the list grows.
 *
 *          A name is unique per kind, not per document: a request and a broadcast may both be
 *          called `start`, and so may a trigger and an action, because each becomes a member of
 *          a different generated class.
 *
 *          The section is told once what its document's methods are, and stamps every entry it
 *          creates or reads with it, so an entry writes the shape its document expects wherever
 *          it is serialized.
 **/
class MethodDataSection : public TEDataContainer<MethodEntry*, DocumentElem>
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Constructor with initialization.
     * \param   config      What the document's methods are.
     * \param   parent      The parent element.
     **/
    MethodDataSection(const MethodConfig& config, ElementBase* parent = nullptr);

    virtual ~MethodDataSection();

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    virtual bool isValid() const override;

    /**
     * \brief   Reads the `MethodList` element and every `Method` row in it.
     **/
    virtual bool readFromXml(QXmlStreamReader& xml) override;

    /**
     * \brief   Writes the `MethodList` element, or nothing when the section is empty.
     **/
    virtual void writeToXml(QXmlStreamWriter& xml) const override;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns what this document's methods are.
     **/
    inline const MethodConfig& getConfig() const;

    /**
     * \brief   Appends a method row.
     * \param   name    The name of the new method, unique among the methods of its kind.
     * \param   kind    The index of the kind to declare it as.
     * \return  The created entry, or nullptr when the name is already taken by that kind.
     **/
    MethodEntry* createMethod(const QString& name, int kind);

    /**
     * \brief   Inserts a method row at the given position.
     * \param   position    The position to insert at.
     * \param   name        The name of the new method, unique among the methods of its kind.
     * \param   kind        The index of the kind to declare it as.
     * \return  The created entry, or nullptr when the name is already taken by that kind.
     **/
    MethodEntry* insertMethod(int position, const QString& name, int kind);

    /**
     * \brief   Finds the first method with the given name, whatever kind it is.
     **/
    MethodEntry* findMethod(const QString& name) const;

    /**
     * \brief   Finds a method by document ID.
     **/
    MethodEntry* findMethod(uint32_t id) const;

    /**
     * \brief   Finds the method of the given name and kind. The name space is per kind, so a
     *          lookup without the kind returns whichever entry happens to come first and reports
     *          the other kinds as undeclared.
     **/
    MethodEntry* findMethod(const QString& name, int kind) const;

    /**
     * \brief   Every method declared as the given kind, in document order.
     **/
    QList<MethodEntry*> methodsOfKind(int kind) const;

    /**
     * \brief   Resolves every parameter's declared type against the document's data types, so
     *          the type-based icons and checks reflect the current registry right after a file
     *          load. A freshly parsed parameter only holds the type name.
     **/
    void validate(const DataTypeDataSection& dataTypes);

    /**
     * \brief   Repoints every parameter declared with the old data type to the new one.
     * \return  The IDs of the methods that changed.
     **/
    QList<uint32_t> replaceDataType(DataTypeBase* oldDataType, DataTypeBase* newDataType);

    /**
     * \brief   Deletes and removes every method.
     **/
    void removeAll();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    MethodConfig    mConfig;    //!< What this document's methods are.
};

//////////////////////////////////////////////////////////////////////////
// MethodDataSection inline methods
//////////////////////////////////////////////////////////////////////////

inline const MethodConfig& MethodDataSection::getConfig() const
{
    return mConfig;
}

#endif  // LUSAN_DATA_COMMON_METHODDATASECTION_HPP
