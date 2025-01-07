#ifndef LUSAN_DATA_SI_SIINCLUDEDATA_HPP
#define LUSAN_DATA_SI_SIINCLUDEDATA_HPP
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
 *  \file        lusan/data/si/SIIncludeData.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Include Data.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/TEDataContainer.hpp"
#include "lusan/data/common/IncludeEntry.hpp"

#include <QList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

 /**
  * \class   SIIncludeData
  * \brief   Manages include data for service interfaces.
  **/
class SIIncludeData : public TEDataContainer< IncludeEntry, ElementBase>
{
//////////////////////////////////////////////////////////////////////////
// public methods
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     **/
    SIIncludeData(ElementBase * parent = nullptr);

    /**
     * \brief   Constructor with initialization.
     * \param   entries    The list of include entries.
     **/
    SIIncludeData(const QList<IncludeEntry>& entries, ElementBase* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Reads include data from an XML stream.
     * \param   xml         The XML stream reader.
     * \return  True if the include data was successfully read, false otherwise.
     **/
    bool readFromXml(QXmlStreamReader& xml);

    /**
     * \brief   Writes include data to an XML stream.
     * \param   xml         The XML stream writer.
     **/
    void writeToXml(QXmlStreamWriter& xml) const;

};

#endif  // LUSAN_DATA_SI_SIINCLUDEDATA_HPP
