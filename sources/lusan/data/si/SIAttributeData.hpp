#ifndef LUSAN_DATA_SI_SIATTRIBUTEDATA_HPP
#define LUSAN_DATA_SI_SIATTRIBUTEDATA_HPP
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
 *  \file        lusan/data/si/SIAttributeData.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Attribute Data.
 *
 ************************************************************************/

/************************************************************************
 * Include files
 ************************************************************************/
#include "lusan/data/common/TEDataContainer.hpp"

#include "lusan/data/common/AttributeEntry.hpp"
#include <QList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

/**
 * \class   SIAttributeData
 * \brief   Manages attribute data for service interfaces.
 **/
class SIAttributeData   : public TEDataContainer< AttributeEntry, ElementBase >
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     **/
    SIAttributeData(ElementBase * parent = nullptr);

    /**
     * \brief   Constructor with initialization.
     * \param   entries     The list of attributes.
     **/
    SIAttributeData(const QList<AttributeEntry>& entries, ElementBase* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Reads attribute data from an XML stream.
     * \param   xml         The XML stream reader.
     * \return  True if the attribute data was successfully read, false otherwise.
     **/
    bool readFromXml(QXmlStreamReader& xml);

    /**
     * \brief   Writes attribute data to an XML stream.
     * \param   xml         The XML stream writer.
     **/
    void writeToXml(QXmlStreamWriter& xml) const;
};

#endif  // LUSAN_DATA_SI_SIATTRIBUTEDATA_HPP
