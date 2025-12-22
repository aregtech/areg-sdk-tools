/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]areg.tech.
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
#include <QPainter>
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

QIcon NELusanCommon::mergeIcons(const QIcon& icon1, double scale1, const QIcon& icon2, double scale2, const QSize& size)
{
    // Step 1: Create a transparent pixmap of the target size
    QPixmap result(size);
    result.fill(Qt::transparent);

    // Step 2: Calculate scaled sizes for both icons
    QSize size1(static_cast<int>(size.width() * scale1), static_cast<int>(size.height() * scale1));
    QSize size2(static_cast<int>(size.width() * scale2), static_cast<int>(size.height() * scale2));

    // Step 3: Calculate positions to center the icons
    QPoint pos1((size.width() - size1.width()) / 2, (size.height() - size1.height()) / 2);
    QPoint pos2((size.width() - size2.width()) / 2, (size.height() - size2.height()) / 2);

    // Step 4: Paint icon1, then icon2 (icon2 overlays icon1, keeping transparency)
    QPainter painter(&result);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QPixmap pixmap1 = icon1.pixmap(size1);
    QPixmap pixmap2 = icon2.pixmap(size2);

    painter.drawPixmap(pos1, pixmap1);
    painter.drawPixmap(pos2, pixmap2);

    painter.end();

    // Step 5: Return the merged icon
    return QIcon(result);
}

