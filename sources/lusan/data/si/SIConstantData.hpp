#ifndef LUSAN_DATA_SI_SICONSTANTDATA_HPP
#define LUSAN_DATA_SI_SICONSTANTDATA_HPP
/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]aregtech.com.
 *
 *  \copyright   � 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/data/si/SIConstantData.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Constant Data.
 *
 ************************************************************************/

#include <QList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "lusan/data/common/ConstantEntry.hpp"

/**
 * \class   SIConstantData
 * \brief   Manages constant data for service interfaces.
 **/
class SIConstantData
{
//////////////////////////////////////////////////////////////////////////
// Local types and static members
//////////////////////////////////////////////////////////////////////////
private:
    static const ConstantEntry InvalidConstant; //!< Invalid constant object

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     **/
    SIConstantData(void);

    /**
     * \brief   Constructor with initialization.
     * \param   entries     The list of constants.
     **/
    SIConstantData(const QList<ConstantEntry>& entries);

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Gets the list of constants.
     * \return  The list of constants.
     **/
    const QList<ConstantEntry>& getConstants(void) const;

    /**
     * \brief   Sets the list of constants.
     * \param   entries     The list of constants.
     **/
    void setConstants(const QList<ConstantEntry>& entries);

    /**
     * \brief   Searches for a constant in the list.
     * \param   entry   The constant to search for.
     * \return  The index of the constant, or -1 if not found.
     **/
    int findConstant(const ConstantEntry& entry) const;

    /**
     * \brief   Adds a constant to the list.
     * \param   entry   The constant to add.
     **/
    void addConstant(const ConstantEntry& entry);

    /**
     * \brief   Removes a constant from the list.
     * \param   entry   The constant to remove.
     * \return  True if the constant was removed, false otherwise.
     **/
    bool removeConstant(const ConstantEntry& entry);

    /**
     * \brief   Replaces a constant in the list.
     * \param   oldEntry    The constant to replace.
     * \param   newEntry    The new constant.
     * \return  True if the constant was replaced, false otherwise.
     **/
    bool replaceConstant(const ConstantEntry& oldEntry, const ConstantEntry& newEntry);

    /**
     * \brief   Reads constant data from an XML stream.
     * \param   xml         The XML stream reader.
     * \return  True if the constant data was successfully read, false otherwise.
     **/
    bool readFromXml(QXmlStreamReader& xml);

    /**
     * \brief   Writes constant data to an XML stream.
     * \param   xml         The XML stream writer.
     **/
    void writeToXml(QXmlStreamWriter& xml) const;

    /**
     * \brief   Finds a constant by name.
     * \param   name    The name to search for.
     * \return  The index of the constant, or -1 if not found.
     **/
    int findConstant(const QString& name) const;

    /**
     * \brief   Checks if a constant exists by name.
     * \param   name    The name to check.
     * \return  True if the constant exists, false otherwise.
     **/
    bool exists(const QString& name) const;

    /**
     * \brief   Gets a constant by name.
     * \param   name    The name to search for.
     * \return  The constant if found, otherwise an invalid constant.
     **/
    const ConstantEntry& getConstant(const QString& name) const;

    /**
     * \brief   Removes a constant by name.
     * \param   name    The name to remove.
     * \return  True if the constant was removed, false otherwise.
     **/
    bool removeConstant(const QString& name);

    /**
     * \brief   Adds a constant to the list and sorts the list.
     * \param   entry       The constant to add.
     * \param   ascending   If true, sorts in ascending order, otherwise in descending order.
     **/
    void addConstant(const ConstantEntry& entry, bool ascending);

    /**
     * \brief   Sorts the list of constants.
     * \param   ascending   If true, sorts in ascending order, otherwise in descending order.
     **/
    void sortConstants(bool ascending = true);

    /**
     * \brief   Removes all constants from the list.
     **/
    void removeAll(void);

//////////////////////////////////////////////////////////////////////////
// Hidden member variables.
//////////////////////////////////////////////////////////////////////////
private:
    QList<ConstantEntry> mConstants; //!< The list of constants.
};

#endif  // LUSAN_DATA_SI_SICONSTANTDATA_HPP

