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
 *  \file        lusan/common/NELusanCommon.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Dialog to select folder.
 *
 ************************************************************************/

#include "lusan/common/NELusanCommon.hpp"

#include <QColor>
#include <QDateTime>
#include <QDir>
#include <QEvent>
#include <QFileInfo>
#include <QFontMetrics>
#include <QGuiApplication>
#include <QHash>
#include <QImage>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QScreen>
#include <QStandardPaths>
#include <QStyle>
#include <QStyleOptionToolButton>
#include <QToolButton>
#include <QMenu>

const QStringList NELusanCommon::FILTERS
{
      QStringLiteral("Service Interface Files (*.siml)")
    , QStringLiteral("State Machine Files (*.fsml)")
    , QStringLiteral("Data Type Files (*.dtml)")
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
    static QHash<QString, QIcon> _icons;
    static bool _builtForDark{ false };

    if (_builtForDark != _darkThemeIcons)
    {
        _icons.clear();
        _builtForDark = _darkThemeIcons;
    }

    const auto found = _icons.constFind(fileName);
    if (found != _icons.constEnd())
        return found.value();

    QIcon source(fileName);
    QIcon result{ source };
    if (_darkThemeIcons && (source.isNull() == false))
    {
        // A lightened mark can only be handed over as ready pixmaps, so each extent the tool
        // draws at is rendered here. A single rescaled pixmap would blur the strokes.
        QList<int> extents{ 12, 16, 20, 22, 24, 25, 32, 48, 64 };
        if (extents.contains(size.height()) == false)
            extents.append(size.height());

        const QScreen* screen = QGuiApplication::primaryScreen();
        const qreal ratio = screen != nullptr ? screen->devicePixelRatio() : 1.0;

        QIcon lightened;
        for (int extent : extents)
        {
            QPixmap pixmap = source.pixmap(QSize(extent, extent), ratio);
            if (pixmap.isNull())
                continue;

            QPixmap adapted = QPixmap::fromImage(adaptImageToDark(pixmap.toImage()));
            adapted.setDevicePixelRatio(ratio);
            lightened.addPixmap(adapted, QIcon::Mode::Normal, QIcon::State::Off);
            lightened.addPixmap(adapted, QIcon::Mode::Normal, QIcon::State::On);
        }

        if (lightened.isNull() == false)
            result = lightened;
    }

    _icons.insert(fileName, result);
    return result;
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

int NELusanCommon::inputRowHeight(const QWidget& owner)
{
    return QFontMetrics(owner.font()).height() + NELusanCommon::InputAir;
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

namespace
{
    //!< The workspace directories a relative include location is measured from, most preferred
    //!< first. Set once when a workspace activates, read by the data layer.
    QStringList _searchRoots;

    //!< File names compare the way the platform's file system compares them.
#ifdef Q_OS_WIN
    constexpr Qt::CaseSensitivity _pathCase{ Qt::CaseInsensitive };
#else
    constexpr Qt::CaseSensitivity _pathCase{ Qt::CaseSensitive };
#endif // Q_OS_WIN

    //!< True when the file lies inside the root. Both paths are already cleaned. The separator
    //!< check is what keeps "/home/dev/src2/a.dtml" out of the root "/home/dev/src".
    bool isUnderRoot(const QString& file, const QString& root)
    {
        if (root.isEmpty() || (file.size() < root.size()) || (file.startsWith(root, _pathCase) == false))
            return false;

        return (file.size() == root.size())
            || root.endsWith(QLatin1Char('/'))
            || (file.at(root.size()) == QLatin1Char('/'));
    }
}

QString NELusanCommon::relativeToRoots(const QString& absoluteFilePath, const QStringList& roots)
{
    const QString file{ QDir::cleanPath(absoluteFilePath) };
    if (file.isEmpty())
        return QString();

    for (const QString& entry : roots)
    {
        const QString root{ QDir::cleanPath(entry) };
        if (isUnderRoot(file, root))
        {
            return QDir(root).relativeFilePath(file);
        }
    }

    return file;
}

QString NELusanCommon::toStorableLocation(const QString& absoluteFilePath)
{
    return NELusanCommon::relativeToRoots(absoluteFilePath, _searchRoots);
}

void NELusanCommon::setSearchRoots(const QStringList& roots)
{
    _searchRoots = roots;
}

const QStringList& NELusanCommon::getSearchRoots(void)
{
    return _searchRoots;
}

QString NELusanCommon::resolveLocation(const QString& hostDirectory, const QString& location)
{
    if (location.isEmpty())
        return QString();

    const QFileInfo info(location);
    if (info.isAbsolute())
        return QDir::cleanPath(info.absoluteFilePath());

    // A location spelled "./" or "../" was written against the document that holds it. Anything
    // else was written against a workspace root, which is the form every document shares.
    const bool hostFirst = location.startsWith(QStringLiteral("./")) || location.startsWith(QStringLiteral("../"));
    const QString fromHost{ hostDirectory.isEmpty()
                            ? QString()
                            : QDir::cleanPath(QDir(hostDirectory).absoluteFilePath(location)) };

    QStringList candidates;
    if (hostFirst && (fromHost.isEmpty() == false))
    {
        candidates.append(fromHost);
    }

    for (const QString& root : _searchRoots)
    {
        if (root.isEmpty() == false)
        {
            candidates.append(QDir::cleanPath(QDir(root).absoluteFilePath(location)));
        }
    }

    if ((hostFirst == false) && (fromHost.isEmpty() == false))
    {
        candidates.append(fromHost);
    }

    // One candidate needs no disambiguation, and this is the hot path: resolution runs on every
    // validation sweep, so touching the disk when there is nothing to choose between costs a
    // stat per import for an answer that cannot change.
    if (candidates.size() < 2)
    {
        return (candidates.isEmpty() ? QString() : candidates.first());
    }

    for (const QString& candidate : candidates)
    {
        if (QFileInfo(candidate).isFile())
        {
            return candidate;
        }
    }

    return candidates.first();
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

const QString NELusanCommon::SPLIT_TOOLBUTTON_NAME{ QStringLiteral("lusanSplitToolButton") };

namespace
{
    /**
     * Holds a split tool button to one icon cell plus the drop-down zone the current style
     * reserves, so its icon starts exactly where a plain toolbar button's icon starts.
     *
     * The zone is not a constant: the system style and the themed one reserve different widths,
     * and picking a theme swaps the style under a window that is already open. So the width is
     * measured from the style rather than written down, and measured again after a style change.
     */
    class SplitToolButtonSizer : public QObject
    {
    public:
        explicit SplitToolButtonSizer(QToolButton* button)
            : QObject(button)
        {
            button->installEventFilter(this);
            applyWidth(button);
        }

        static void applyWidth(QToolButton* button)
        {
            QStyleOptionToolButton option;
            option.initFrom(button);
            option.iconSize         = button->iconSize();
            option.toolButtonStyle  = Qt::ToolButtonIconOnly;
            option.features         = QStyleOptionToolButton::MenuButtonPopup | QStyleOptionToolButton::HasMenu;
            option.subControls      = QStyle::SC_All;
            const int zone = button->style()->subControlRect(QStyle::CC_ToolButton, &option, QStyle::SC_ToolButtonMenu, button).width();
            button->setFixedSize(NELusanCommon::TOOLBUTTON_CELL + zone, NELusanCommon::TOOLBUTTON_CELL);
        }

    protected:
        bool eventFilter(QObject* watched, QEvent* event) override
        {
            if (event->type() == QEvent::StyleChange)
            {
                // A theme change installs the new style first and its sheet after it, so the width
                // is taken once the whole switch is through rather than half way into it.
                QToolButton* button = static_cast<QToolButton*>(watched);
                QMetaObject::invokeMethod(button, [button]() { applyWidth(button); }, Qt::QueuedConnection);
            }

            return QObject::eventFilter(watched, event);
        }
    };
}

QToolButton* NELusanCommon::createToolButton(QWidget* parent, const QString& iconName, const QString& toolTip, const QKeySequence& shortcut)
{
    QToolButton* button = new QToolButton(parent);
    button->setMaximumSize(TOOLBUTTON_CELL, TOOLBUTTON_CELL);
    button->setCursor(Qt::PointingHandCursor);
    button->setMouseTracking(true);
    button->setToolTip(toolTip);
    button->setIcon(loadIcon(iconName, QSize(TOOLBUTTON_ICON, TOOLBUTTON_ICON)));
    button->setIconSize(QSize(TOOLBUTTON_ICON, TOOLBUTTON_ICON));
    button->setShortcut(shortcut);
    return button;
}

void NELusanCommon::decorateToolButton(QToolButton* button)
{
    if (button == nullptr)
        return;

    button->setMenu(nullptr);
    button->setPopupMode(QToolButton::DelayedPopup);
    button->setIconSize(QSize(TOOLBUTTON_ICON, TOOLBUTTON_ICON));
    button->setMaximumSize(TOOLBUTTON_CELL, TOOLBUTTON_CELL);
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
    // The icon keeps the size and the cell of a plain toolbar button; only the arrow zone is
    // added, to the right of that cell. The name must be set before the width is measured,
    // because the theme sheet reserves the arrow zone through a rule that matches on it.
    button->setObjectName(SPLIT_TOOLBUTTON_NAME);
    button->setIconSize(QSize(TOOLBUTTON_ICON, TOOLBUTTON_ICON));
    new SplitToolButtonSizer(button);
}

const QString& NELusanCommon::identifierPattern()
{
    // A letter or underscore, then any number of letters, digits or underscores. Used to build
    // the shared keystroke validator, so the whole editor family enforces the same C++ rule.
    static const QString _pattern{ QStringLiteral("[A-Za-z_][A-Za-z0-9_]*") };
    return _pattern;
}

bool NELusanCommon::isValidIdentifier(const QString& name)
{
    static const QRegularExpression _re{ QStringLiteral("\\A[A-Za-z_][A-Za-z0-9_]*\\z") };
    return (name.isEmpty() == false)
        && (name.size() <= NELusanCommon::MAX_IDENTIFIER_LENGTH)
        && _re.match(name).hasMatch();
}

QString NELusanCommon::toDocumentName(const QString& fileBaseName)
{
    QString result;
    result.reserve(fileBaseName.size());
    for (const QChar ch : fileBaseName)
    {
        if (ch.isSpace())
            continue;

        // Only the characters C++ accepts survive; the test is deliberately ASCII, so an accented
        // letter is replaced rather than kept as a letter Qt would happily call one.
        const bool spellable = (ch == QLatin1Char('_'))
                            || ((ch >= QLatin1Char('a')) && (ch <= QLatin1Char('z')))
                            || ((ch >= QLatin1Char('A')) && (ch <= QLatin1Char('Z')))
                            || ((ch >= QLatin1Char('0')) && (ch <= QLatin1Char('9')));
        result.append(spellable ? ch : QLatin1Char('_'));
    }

    for (qsizetype i = 0; (i < result.size()) && (result.at(i) >= QLatin1Char('0')) && (result.at(i) <= QLatin1Char('9')); ++i)
    {
        result[i] = QLatin1Char('N');
    }

    return result;
}

QValidator* NELusanCommon::createIdentifierValidator(QObject* parent)
{
    // A full match is Acceptable and a prefix Intermediate, so the field can be cleared while
    // typing, but a space or a leading digit is rejected as it is typed.
    return new QRegularExpressionValidator(QRegularExpression(identifierPattern()), parent);
}

QValidator* NELusanCommon::createPathValidator(QObject* parent)
{
    // Characters valid in an include path: word characters plus '.', '/', '\', ':', '-', space.
    return new QRegularExpressionValidator(QRegularExpression(QStringLiteral("[A-Za-z0-9_./\\\\:\\- ]*")), parent);
}

QValidator* NELusanCommon::createQualifiedNameValidator(QObject* parent)
{
    // A qualified name is one or more identifiers joined by '::'. A partial name stays
    // Intermediate, so a namespace can be typed segment by segment.
    static const QString _pattern{ QStringLiteral("[A-Za-z_][A-Za-z0-9_]*(::[A-Za-z_][A-Za-z0-9_]*)*") };
    return new QRegularExpressionValidator(QRegularExpression(_pattern), parent);
}

QIcon NELusanCommon::chevronIcon(bool expanded, const QColor& color, const QSize& size /*= QSize{ 16, 16 }*/)
{
    const int w = (size.width()  > 0) ? size.width()  : 16;
    const int h = (size.height() > 0) ? size.height() : 16;

    QPixmap pix(w, h);
    pix.fill(Qt::transparent);

    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPen pen(color);
    pen.setWidthF(std::max(1.5, w / 8.0));
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    // Inset the glyph so the round caps stay inside the icon box.
    const double mx = w * 0.28;
    const double my = h * 0.28;
    if (expanded)
    {
        // Downward chevron: "v"
        const QPointF pts[3] = { QPointF(mx, my), QPointF(w / 2.0, h - my), QPointF(w - mx, my) };
        painter.drawPolyline(pts, 3);
    }
    else
    {
        // Rightward chevron: ">"
        const QPointF pts[3] = { QPointF(mx, my), QPointF(w - mx, h / 2.0), QPointF(mx, h - my) };
        painter.drawPolyline(pts, 3);
    }

    painter.end();
    return QIcon(pix);
}
