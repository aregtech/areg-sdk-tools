#ifndef LUSAN_DATA_DT_DATATYPEDOCUMENTDATA_HPP
#define LUSAN_DATA_DT_DATATYPEDOCUMENTDATA_HPP
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
 *  \file        lusan/data/dt/DataTypeDocumentData.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Data Type document data.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/common/ElementBase.hpp"

#include "lusan/data/common/DataTypeDataSection.hpp"
#include "lusan/data/common/IncludeDataSection.hpp"
#include "lusan/data/common/OverviewDataSection.hpp"
#include "lusan/model/common/DocUnknownScan.hpp"

#include <QObject>

/**
 * \class   DataTypeDocumentData
 * \brief   A `.dtml` document: the data types it declares, what it says about itself, and the
 *          C++ headers those types need. There is nothing else in it -- a data type document
 *          declares no attribute, method or constant.
 *
 *          It is named by the file it lives in, the way a service interface is: the name is read
 *          from the file on open and written back on every save, because that name becomes the
 *          namespace the generated types sit in.
 **/
class DataTypeDocumentData  : protected QObject
                            , public    ElementBase
{
//////////////////////////////////////////////////////////////////////////
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
public:
    static constexpr const char* const  XML_VERSION_100     { "1.0.0" };            //!< The first format version.
    static constexpr const char* const  XML_FORMAT_DEFAULT  { XML_VERSION_100 };    //!< The format version written.

private:
    static constexpr const uint32_t     MINIMUM_ID          { 50u };                //!< The first ID handed out.

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Creates the document, reading \p filePath when one is given.
     **/
    DataTypeDocumentData(const QString& filePath = QString());

    virtual ~DataTypeDocumentData(void) = default;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns the format version this build writes.
     **/
    inline QString getFileFormatVersion(void) const;

    /**
     * \brief   Reads the document from a file. The document takes its name from the file.
     **/
    bool readFromFile(const QString& filePath);

    /**
     * \brief   Writes the document to a file, naming it after that file.
     **/
    bool writeToFile(const QString& filePath = QString());

    /**
     * \brief   Reads the document from an XML stream positioned at the document element.
     **/
    bool readFromXml(QXmlStreamReader& xml);

    /**
     * \brief   Writes the document to an XML stream.
     **/
    void writeToXml(QXmlStreamWriter& xml) const;

    inline const QString& getFilePath(void) const;

    inline const OverviewDataSection& getOverviewData(void) const;
    inline OverviewDataSection& getOverviewData(void);

    inline const DataTypeDataSection& getDataTypeData(void) const;
    inline DataTypeDataSection& getDataTypeData(void);

    inline const IncludeDataSection& getIncludeData(void) const;
    inline IncludeDataSection& getIncludeData(void);

    /**
     * \brief   True when the file was read.
     **/
    inline bool openSucceeded(void) const;

    /**
     * \brief   The format version the document declared.
     **/
    inline const VersionNumber& getCurrentDocumentVersion(void) const;

    /**
     * \brief   Every element of the opened document that the format does not define, in
     *          document order. Empty for a document written by this build.
     **/
    inline const QList<DocUnknownElement>& getUnknownElements(void) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QString             mFilePath;      //!< The file the document was read from or written to.
    VersionNumber       mXmlVersion;    //!< The format version the document declared.
    OverviewDataSection mOverviewData;  //!< What the document says about itself.
    DataTypeDataSection mDataTypeData;  //!< The data types it declares.
    IncludeDataSection  mIncludeData;   //!< The C++ headers those types need.
    bool                mOpenSuccess;   //!< Whether the file was read.

    //!< The blocks of the opened file this build could not place, kept so a save puts them back.
    QList<DocUnknownElement> mUnknownElements;

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    DataTypeDocumentData(const DataTypeDocumentData& /*src*/) = delete;
    DataTypeDocumentData& operator = (const DataTypeDocumentData& /*src*/) = delete;
};

//////////////////////////////////////////////////////////////////////////
// DataTypeDocumentData inline methods
//////////////////////////////////////////////////////////////////////////

inline QString DataTypeDocumentData::getFileFormatVersion(void) const
{
    return DataTypeDocumentData::XML_FORMAT_DEFAULT;
}

inline const QString& DataTypeDocumentData::getFilePath(void) const
{
    return mFilePath;
}

inline const OverviewDataSection& DataTypeDocumentData::getOverviewData(void) const
{
    return mOverviewData;
}

inline OverviewDataSection& DataTypeDocumentData::getOverviewData(void)
{
    return mOverviewData;
}

inline const DataTypeDataSection& DataTypeDocumentData::getDataTypeData(void) const
{
    return mDataTypeData;
}

inline DataTypeDataSection& DataTypeDocumentData::getDataTypeData(void)
{
    return mDataTypeData;
}

inline const IncludeDataSection& DataTypeDocumentData::getIncludeData(void) const
{
    return mIncludeData;
}

inline IncludeDataSection& DataTypeDocumentData::getIncludeData(void)
{
    return mIncludeData;
}

inline bool DataTypeDocumentData::openSucceeded(void) const
{
    return mOpenSuccess;
}

inline const VersionNumber& DataTypeDocumentData::getCurrentDocumentVersion(void) const
{
    return mXmlVersion;
}

inline const QList<DocUnknownElement>& DataTypeDocumentData::getUnknownElements(void) const
{
    return mUnknownElements;
}

#endif  // LUSAN_DATA_DT_DATATYPEDOCUMENTDATA_HPP
