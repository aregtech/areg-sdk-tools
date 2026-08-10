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
 *  \file        lusan/data/dt/DataTypeDocumentData.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Data Type document data.
 *
 ************************************************************************/

#include "lusan/data/dt/DataTypeDocumentData.hpp"

#include "lusan/common/XmlDT.hpp"

#include <QFile>
#include <QFileInfo>

namespace
{
    //!< An empty description is left out of a `.dtml`, the way a `.fsml` leaves it out.
    constexpr bool OMIT_EMPTY_DESCRIPTION { true };
}

DataTypeDocumentData::DataTypeDocumentData(const QString& filePath /*= QString()*/)
    : ElementBase   (MINIMUM_ID, nullptr)
    , mFilePath     ( )
    , mXmlVersion   ( )
    , mOverviewData (OMIT_EMPTY_DESCRIPTION, this)
    , mDataTypeData (this)
    , mIncludeData  (this)
    , mOpenSuccess  (false)
{
    if (filePath.isEmpty() || (readFromFile(filePath) == false))
    {
        mOverviewData.setId(getNextId());
    }
}

bool DataTypeDocumentData::readFromFile(const QString& filePath)
{
    mOpenSuccess = false;
    mFilePath.clear();
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly))
    {
        mFilePath = filePath;
        mOverviewData.setName(QFileInfo(filePath).baseName());

        QXmlStreamReader xml(&file);
        while (!xml.atEnd() && !xml.hasError())
        {
            if (xml.readNextStartElement())
            {
                if (readFromXml(xml) == false)
                {
                    xml.raiseError(tr("Invalid XML format"));
                }
            }
        }

        file.close();

        if (xml.hasError() == false)
        {
            mOpenSuccess = true;
            mDataTypeData.validate(mDataTypeData);
        }
    }

    return mOpenSuccess;
}

bool DataTypeDocumentData::writeToFile(const QString& filePath /*= QString()*/)
{
    const QString path = filePath.isEmpty() ? mFilePath : filePath;
    if (path.isEmpty())
        return false;

    QFile file(path);
    if (file.open(QFile::WriteOnly | QFile::Text) == false)
        return false;

    // The document is named by the file it lives in, so a save under another name renames it.
    mOverviewData.setName(QFileInfo(path).baseName());
    mFilePath = path;

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.writeStartDocument("1.0", true);
    writeToXml(xml);
    file.close();
    return true;
}

bool DataTypeDocumentData::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() != XmlDT::xmlDTElementDocument)
        return false;

    QString version = xml.attributes().value(XmlDT::xmlDTAttributeFormatVersion).toString();
    if (VersionNumber(version).isCompatible(VersionNumber(XML_FORMAT_DEFAULT)) == false)
    {
        version = XML_FORMAT_DEFAULT;
    }

    mXmlVersion = version;

    while (xml.readNextStartElement())
    {
        if (xml.name() == XmlDT::xmlDTElementOverview)
        {
            mOverviewData.readFromXml(xml);
        }
        else if (xml.name() == XmlDT::xmlDTElementDataTypeList)
        {
            mDataTypeData.readFromXml(xml);
        }
        else if (xml.name() == XmlDT::xmlDTElementIncludeList)
        {
            mIncludeData.readFromXml(xml);
        }
        else
        {
            xml.skipCurrentElement();
        }
    }

    return true;
}

void DataTypeDocumentData::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlDT::xmlDTElementDocument);
    xml.writeAttribute(XmlDT::xmlDTAttributeFormatVersion, XML_FORMAT_DEFAULT);

    mOverviewData.writeToXml(xml);
    mDataTypeData.writeToXml(xml);
    mIncludeData.writeToXml(xml);

    xml.writeEndElement();
}
