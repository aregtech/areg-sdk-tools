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
 *  \file        lusan/common/NELusanCommon.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Dialog to select folder.
 *
 ************************************************************************/

#include "lusan/common/NELusanCommon.hpp"

#include <QDateTime>
#include <QFileInfo>
#include <QStandardPaths>

const QStringList NELusanCommon::FILTERS
{
      QStringLiteral("Service Interface Files (*.siml)")
    , QStringLiteral("Log Files (*.logs)")
    , QStringLiteral("All Files (*.*)")
};

const QString    NELusanCommon::APPLICATION    { "lusan" };
const QString    NELusanCommon::ORGANIZATION   { "Aregtech" };
const QString    NELusanCommon::VERSION        { "1.0.0" };
const QString    NELusanCommon::OPTIONS        { "lusan.opt" };
const QString    NELusanCommon::INIT_FILE      { "./config/lusan.init" };


QString NELusanCommon::getOptionsFile(void)
{
    return getUserProfileFile(OPTIONS);
}

QString NELusanCommon::getUserProfileFile(const QString& fileName)
{
    return QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation), fileName);
}

uint32_t NELusanCommon::getId(void)
{
    static uint32_t _id = 0;
    return (++_id != 0 ? _id : ++_id);
}


uint64_t NELusanCommon::getTimestamp(void)
{
    return static_cast<uint64_t>(QDateTime::currentMSecsSinceEpoch());
}

const QString& NELusanCommon::getStyleToolbutton(void)
{
    static const QString& _style(QString::fromUtf8(NELusanCommon::StyleToolbuttonChecked.data(), NELusanCommon::StyleToolbuttonChecked.length()));
    return _style;
}

QString NELusanCommon::fixPath(const QString& path)
{
    if (path.isEmpty())
        return path;
    
    QString filePath{ QString::fromStdString(std::filesystem::absolute(path.toStdString()).string()) };
    QFileInfo fi(filePath);
    return fi.absoluteFilePath();
}
