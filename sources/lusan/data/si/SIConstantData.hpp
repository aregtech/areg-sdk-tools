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
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/data/si/SIConstantData.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Constant Data.
 *
 ************************************************************************/

/************************************************************************
 * Include files
 ************************************************************************/
#include "lusan/data/common/TEDataContainer.hpp"

#include "lusan/data/common/ConstantEntry.hpp"
#include <QList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

/************************************************************************
 * Dependencies
 ************************************************************************/
class SIDataTypeData;

/**
 * \class   SIConstantData
 * \brief   Manages constant data for service interfaces.
 **/
class SIConstantData    : public TEDataContainer< ConstantEntry, ElementBase>
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     * \param   parent  The parent element.
     **/
    SIConstantData(ElementBase * parent = nullptr);

    /**
     * \brief   Constructor with initialization.
     * \param   entries     The list of constants.
     * \param   parent      The parent element.
     **/
    SIConstantData(const QList<ConstantEntry>& entries, ElementBase* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
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
};

#endif  // LUSAN_DATA_SI_SICONSTANTDATA_HPP

