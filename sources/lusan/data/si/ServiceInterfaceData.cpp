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
 *  \file        lusan/data/si/ServiceInterfaceData.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Data.
 *
 ************************************************************************/

#include "lusan/data/si/ServiceInterfaceData.hpp"

#include <QFile>
#include <QFileInfo>

ServiceInterfaceData::ServiceInterfaceData(const QString& filePath /*= QString()*/)
    : ElementBase   (MINIMUM_ID, nullptr)
    , mFilePath     ( )
    , mOverviewData (this)
    , mDataTypeData (this)
    , mAttributeData(this)
    , mMethodData   (this)
    , mConstantData (this)
    , mIncludeData  (this)
    , mOpenSuccess  (false)
{
    if (filePath.isEmpty() || (readFromFile(filePath) == false))
    {
        mOverviewData.setId(getNextId());
    }
}

bool ServiceInterfaceData::readFromFile(const QString& filePath)
{
    mOpenSuccess = false;
    mFilePath.clear();
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly))
    {
        QXmlStreamReader xml(&file);
        while (!xml.atEnd() && !xml.hasError())
        {
            if (xml.readNextStartElement())
            {
                if (readFromXml(xml))
                {
                    mFilePath = filePath;
                    QFileInfo info(filePath);
                    QString name = info.baseName();
                    mOverviewData.setName(name);
                }
                else
                {
                    xml.raiseError("Invalid XML format");
                }
            }
        }

        file.close();

        if (xml.hasError() == false)
        {
            mOpenSuccess = true;
            const QList<DataTypeCustom*>& dataTypes = mDataTypeData.getCustomDataTypes();
            mOverviewData.validate(dataTypes);
            mDataTypeData.validate(dataTypes);
            mAttributeData.validate(dataTypes);
            mMethodData.validate(dataTypes);
            mConstantData.validate(dataTypes);
            mIncludeData.validate(dataTypes);
        }
    }

    return mOpenSuccess;
}

bool ServiceInterfaceData::writeToFile(const QString& filePath /*= ""*/)
{
    QString path = filePath.isEmpty() ? mFilePath : filePath;
    if (!path.isEmpty())
    {
        QFile file(path);
        if (file.open(QFile::WriteOnly | QFile::Text))
        {
            QFileInfo info(QString(file.filesystemFileName().c_str()));
            const QString & serviceName = info.baseName();
            mOverviewData.setName(serviceName);
            
            QXmlStreamWriter xml(&file);
            xml.setAutoFormatting(true);
            xml.writeStartDocument();
            writeToXml(xml);
            file.close();
            return true;
        }
    }

    return false;
}

bool ServiceInterfaceData::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() == XmlSI::xmlSIElementServiceInterface)
    {
        QXmlStreamAttributes attributes = xml.attributes();
        QString version = attributes.value(XmlSI::xmlSIAttributeFormatVersion).toString();
        if (VersionNumber(version).isCompatible(VersionNumber(XML_FORMAT_VERSION)) == false)
        {
            return false;
        }

        while (xml.readNextStartElement())
        {
            if (xml.name() == XmlSI::xmlSIElementOverview)
            {
                mOverviewData.readFromXml(xml);
            }
            else if (xml.name() == XmlSI::xmlSIElementDataTypeList)
            {
                mDataTypeData.readFromXml(xml);
            }
            else if (xml.name() == XmlSI::xmlSIElementAttributeList)
            {
                mAttributeData.readFromXml(xml);
            }
            else if (xml.name() == XmlSI::xmlSIElementMethodList)
            {
                mMethodData.readFromXml(xml);
            }
            else if (xml.name() == XmlSI::xmlSIElementConstantList)
            {
                mConstantData.readFromXml(xml);
            }
            else if (xml.name() == XmlSI::xmlSIElementIncludeList)
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

    return false;
}

void ServiceInterfaceData::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSI::xmlSIElementServiceInterface);
    xml.writeAttribute(XmlSI::xmlSIAttributeFormatVersion, XML_FORMAT_VERSION);

    mOverviewData.writeToXml(xml);
    mDataTypeData.writeToXml(xml);
    mAttributeData.writeToXml(xml);
    mMethodData.writeToXml(xml);
    mConstantData.writeToXml(xml);
    mIncludeData.writeToXml(xml);

    xml.writeEndElement();
}
