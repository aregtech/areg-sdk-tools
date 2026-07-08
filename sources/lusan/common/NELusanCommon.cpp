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
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/common/NELusanCommon.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Dialog to select folder.
 *
 ************************************************************************/

#include "lusan/common/NELusanCommon.hpp"

#include <QColor>
#include <QDateTime>
#include <QFileInfo>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QStandardPaths>
#include <QToolButton>
#include <QMenu>

const QStringList NELusanCommon::FILTERS
{
      QStringLiteral("Service Interface Files (*.siml)")
    , QStringLiteral("State Machine Files (*.fsml)")
    , QStringLiteral("Log Files (*.logs)")
    , QStringLiteral("All Files (*.*)")
};

const QString    NELusanCommon::APPLICATION    { "lusan" };
const QString    NELusanCommon::ORGANIZATION   { "Aregtech" };
const QString    NELusanCommon::VERSION        { "1.0.0" };
const QString    NELusanCommon::OPTIONS        { "lusan.opt" };
const QString    NELusanCommon::INIT_FILE      { "./config/lusan.init" };


namespace
{
    bool _darkThemeIcons{ false };

    //!< Lightens dark strokes so monochrome icons stay visible on dark backgrounds.
    QImage adaptImageToDark(QImage image)
    {
        image = image.convertToFormat(QImage::Format_ARGB32);
        const int height = image.height();
        const int width  = image.width();
        for (int y = 0; y < height; ++y)
        {
            QRgb* line = reinterpret_cast<QRgb*>(image.scanLine(y));
            for (int x = 0; x < width; ++x)
            {
                const QRgb rgba = line[x];
                if (qAlpha(rgba) == 0)
                    continue;

                QColor color = QColor::fromRgba(rgba);
                int hue{ 0 }, sat{ 0 }, light{ 0 }, alpha{ 0 };
                color.getHsl(&hue, &sat, &light, &alpha);
                if (light < 128)
                {
                    color.setHsl(hue, sat, 255 - light, alpha);
                    line[x] = color.rgba();
                }
            }
        }

        return image;
    }
}

QIcon NELusanCommon::loadIcon(const QString & fileName, const QSize & size /*= QSize{32, 32}*/)
{
    if (_darkThemeIcons == false)
    {
        QIcon icon;
        icon.addFile(fileName, size, QIcon::Mode::Normal, QIcon::State::On);
        return icon;
    }

    QIcon source;
    source.addFile(fileName, size, QIcon::Mode::Normal, QIcon::State::On);
    QPixmap pixmap = source.pixmap(size);
    if (pixmap.isNull())
        return source;

    return QIcon(QPixmap::fromImage(adaptImageToDark(pixmap.toImage())));
}

void NELusanCommon::setIconsForDarkTheme(bool isDark)
{
    _darkThemeIcons = isDark;
}

bool NELusanCommon::iconsForDarkTheme()
{
    return _darkThemeIcons;
}

QString NELusanCommon::getOptionsFile()
{
    return getUserProfileFile(OPTIONS);
}

QString NELusanCommon::getUserProfileFile(const QString& fileName)
{
    return QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation), fileName);
}

uint32_t NELusanCommon::getId()
{
    static uint32_t _id = 0;
    return (++_id != 0 ? _id : ++_id);
}


uint64_t NELusanCommon::getTimestamp()
{
    return static_cast<uint64_t>(QDateTime::currentMSecsSinceEpoch());
}

const QString& NELusanCommon::getStyleToolbutton()
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

QToolButton* NELusanCommon::createToolButton(QWidget* parent, const QString& iconName, const QString& toolTip, const QKeySequence& shortcut)
{
    QToolButton* button = new QToolButton(parent);
    button->setMaximumSize(24, 24);
    button->setCursor(Qt::PointingHandCursor);
    button->setMouseTracking(true);
    button->setToolTip(toolTip);
    button->setIcon(QIcon(iconName));
    button->setIconSize(QSize(25, 25));
    button->setShortcut(shortcut);
    return button;
}

void NELusanCommon::decorateToolButton(QToolButton* button)
{
    if (button == nullptr)
        return;

    button->setMenu(nullptr);
    button->setPopupMode(QToolButton::DelayedPopup);
    button->setIconSize(QSize(24, 24));
    button->setMaximumSize(24, 24);
}

void NELusanCommon::decorateToolButton(QToolButton* button, QMenu* menu)
{
    if ((button == nullptr) || (menu == nullptr))
    {
        decorateToolButton(button);
        return;
    }

    button->setMenu(menu);
    button->setPopupMode(QToolButton::MenuButtonPopup);
    // Keep a dedicated right-side drop-down zone so the menu arrow never crowds the icon.
    button->setIconSize(QSize(20, 20));
    button->setFixedSize(48, 24);
}
