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
 *  \file        lusan/view/common/MdiChild.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application Multi-document interface (MDI) child window.
 *
 ************************************************************************/

#include "lusan/view/common/MdiChild.hpp"
#include "lusan/app/LusanApplication.hpp"
#include "lusan/app/NEAppThemes.hpp"
#include "lusan/common/NELusanCommon.hpp"
#include "lusan/data/common/OverviewDataSection.hpp"
#include "lusan/model/common/IEDocumentModel.hpp"
#include "lusan/view/common/MdiMainWindow.hpp"

#include <QCloseEvent>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGuiApplication>
#include <QIconEngine>
#include <QKeySequence>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QSaveFile>
#include <QScrollBar>
#include <QShortcut>
#include <QTabBar>
#include <QTabWidget>
#include <QTextDocument>

#include <algorithm>

namespace
{
    //!< Paints an icon with a round marker in its bottom right corner.
    class UnsavedMarkIconEngine : public QIconEngine
    {
    public:
        explicit UnsavedMarkIconEngine(const QIcon& base)
            : QIconEngine()
            , mBase(base)
        {
        }

        void paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state) override
        {
            mBase.paint(painter, rect, Qt::AlignCenter, mode, state);

            const QRectF area(rect);
            const qreal side{ std::min(area.width(), area.height()) };
            const qreal ring{ std::max(1.0, side / 16.0) };
            const qreal diameter{ side / 2.0 };
            const QRectF dot{ area.right() - diameter, area.bottom() - diameter, diameter, diameter };

            painter->save();
            painter->setRenderHint(QPainter::RenderHint::Antialiasing, true);
            painter->setPen(QPen(QGuiApplication::palette().color(QPalette::ColorRole::Base), ring));
            painter->setBrush(NEAppThemes::unsavedMarkColor());
            painter->drawEllipse(dot.adjusted(ring / 2.0, ring / 2.0, -ring / 2.0, -ring / 2.0));
            painter->restore();
        }

        QPixmap pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state) override
        {
            return scaledPixmap(size, mode, state, 1.0);
        }

        QPixmap scaledPixmap(const QSize& size, QIcon::Mode mode, QIcon::State state, qreal scale) override
        {
            QPixmap result(size * scale);
            result.setDevicePixelRatio(scale);
            result.fill(Qt::GlobalColor::transparent);
            QPainter painter(&result);
            paint(&painter, QRect(QPoint(0, 0), size), mode, state);
            return result;
        }

        QSize actualSize(const QSize& size, QIcon::Mode mode, QIcon::State state) override
        {
            return (mBase.isNull() ? size : mBase.actualSize(size, mode, state));
        }

        QIconEngine* clone() const override
        {
            return new UnsavedMarkIconEngine(mBase);
        }

        QString key() const override
        {
            return QStringLiteral("UnsavedMarkIconEngine");
        }

    private:
        QIcon   mBase;  //!< The icon the marker is painted on.
    };
}

MdiChild::MdiChild(MdiChild::eMdiWindow windowType, MdiMainWindow* wndMain, QWidget* parent /*= nullptr*/)
    : QWidget       (parent)

    , mMdiWindowType(windowType)
    , mCurFile      ( )
    , mDocName      ( )
    , mIsUntitled   ( true )
    , mIsModified   ( false )
    , mIsClosing    ( false )
    , mFileTime     ( )
    , mFileSize     ( -1 )
    , mReloadAsked  ( false )
    , mFileMissing  ( false )
    , mTabMarked    ( false )
    , mTabIcon      ( )
    , mMdiSubWindow ( nullptr )
    , mMainWindow   (wndMain)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    Q_ASSERT(wndMain != nullptr);
    emit wndMain->signalMdiWindowCreated(this);

    // Ctrl+PageDown and Ctrl+PageUp cycle the document's own pages, leaving Ctrl+Tab for the MDI
    // windows. The children scope keeps them local, and the call is inert without page tabs.
    QShortcut* nextPage = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_PageDown), this);
    nextPage->setContext(Qt::WidgetWithChildrenShortcut);
    connect(nextPage, &QShortcut::activated, this, [this]() { switchToAdjacentPage(1); });

    QShortcut* prevPage = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_PageUp), this);
    prevPage->setContext(Qt::WidgetWithChildrenShortcut);
    connect(prevPage, &QShortcut::activated, this, [this]() { switchToAdjacentPage(-1); });
}

MdiChild::~MdiChild()
{
}

bool MdiChild::openSucceeded() const
{
    return false;
}

QTabWidget* MdiChild::pageTabWidget()
{
    return nullptr;
}

void MdiChild::switchToAdjacentPage(int delta)
{
    QTabWidget* tabs = pageTabWidget();
    if (tabs == nullptr)
    {
        return;
    }

    const int count = tabs->count();
    if (count <= 1)
    {
        return;
    }

    int index = tabs->currentIndex() + delta;
    if (index < 0)
    {
        index = count - 1;
    }
    else if (index >= count)
    {
        index = 0;
    }

    tabs->setCurrentIndex(index);
}

QString MdiChild::newDocumentName()
{
    static uint32_t _seqNr{0};
    mDocName = newDocument() + QString::number(++_seqNr);
    return (mDocName + newDocumentExt());
}

const QString& MdiChild::newDocument() const
{
    static const QString _newDoc{"document"};
    return _newDoc;
}

const QString& MdiChild::newDocumentExt() const
{
    static const QString _newExt("");
    return _newExt;
}

const QString& MdiChild::fileSuffix() const
{
    static const QString _suffix("");
    return _suffix;
}

const QString& MdiChild::fileFilter() const
{
    static const QString _filter("All Files (*.*)");
    return _filter;
}

void MdiChild::newFile()
{
    mIsUntitled = true;
    mCurFile = newDocumentName();
    mIsModified = true;
    // The document opens under the generated name, so it is never nameless. The file it is
    // first saved into replaces that name, unless the author has already chosen one.
    setDocumentName(mDocName);
    setWindowTitle(mCurFile + "[*]");
    setWindowModified(true);
#if 0
    connect(document(), &QTextDocument::contentsChanged, this, &MdiChild::onDocumentModified);
#endif
}

bool MdiChild::loadFile(const QString& fileName)
{
#if 0
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::warning(this, tr("MDI"), tr("Cannot read file %1:\n%2.").arg(fileName).arg(file.errorString()));
        return false;
    }

    QTextStream in(&file);
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    setPlainText(in.readAll());
    QGuiApplication::restoreOverrideCursor();
    connect(document(), &QTextDocument::contentsChanged, this, &MdiChild::onDocumentModified);
#endif // 0
    
    setCurrentFile(fileName);
    return true;
}

bool MdiChild::save()
{
    return ((mIsUntitled || mFileMissing) ? saveAs() : saveFile(mCurFile));
}

bool MdiChild::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"), suggestedSaveName(), fileFilter());
    return (fileName.isEmpty() ? false : saveFile(fileName));
}

QString MdiChild::suggestedSaveName() const
{
    return mCurFile;
}

bool MdiChild::saveFile(const QString& fileName)
{
    bool saved { false };
    commitPendingEdits();
    seedNameFromFile(fileName);
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    if (writeToFile(fileName) )
    {
        mIsModified = false;
        mIsUntitled = false;
        setCurrentFile(fileName);
        saved = true;
    }
    
    QGuiApplication::restoreOverrideCursor();
    return saved;
}

bool MdiChild::writeToFile(const QString& filePath)
{
    return true;
}

void MdiChild::seedNameFromFile(const QString& filePath)
{
    IEDocumentModel* model = documentModel();
    if ((model == nullptr) || filePath.isEmpty())
        return;

    const QString current{ model->getOverviewSection().getName() };
    if ((current.isEmpty() == false) && (current != mDocName))
        return;     // the author named it, so the file name has nothing to say

    setDocumentName(NELusanCommon::toDocumentName(QFileInfo(filePath).completeBaseName()));
}

void MdiChild::setDocumentName(const QString& name)
{
    IEDocumentModel* model = documentModel();
    if ((model == nullptr) || name.isEmpty())
        return;

    OverviewDataSection& overview = model->getOverviewSection();
    if (overview.getName() == name)
        return;

    // Written straight into the section rather than through a command: naming a new document and
    // naming a saved one are not edits the author made, and neither belongs in the history.
    overview.setName(name);
    model->getNotifier().notifyElementChanged(overview.getId(), eDocElementKind::Overview);
}

void MdiChild::commitPendingEdits(void)
{
}

QString MdiChild::userFriendlyCurrentFile()
{
    return strippedName(mCurFile);
}

void MdiChild::closeEvent(QCloseEvent* event)
{
    if (maybeSave())
    {
        mIsClosing = true;
        onWindowClosing(isActiveWindow());
        emit signalMdiChildClosed(this);
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void MdiChild::rememberFileState()
{
    const QFileInfo info(mCurFile);
    mFileTime = (mCurFile.isEmpty() == false) && info.exists() ? info.lastModified() : QDateTime();
    mFileSize = (mCurFile.isEmpty() == false) && info.exists() ? info.size() : -1;
}

void MdiChild::checkFileChangedOnDisk()
{
    if (mIsClosing || mReloadAsked || mCurFile.isEmpty() || (mMainWindow == nullptr))
    {
        return;
    }

    // An absent file is not a change to reload: a text editor that saves by replacing the file
    // makes it disappear for a moment, and the notification for the replacement follows.
    const QFileInfo info(mCurFile);
    if (info.exists() == false)
    {
        return;
    }

    if ((info.lastModified() == mFileTime) && (info.size() == mFileSize))
    {
        return;     // this is the editor's own save coming back
    }

    rememberFileState();

    QMessageBox box(this);
    box.setWindowTitle(tr("File Changed on Disk"));
    box.setIcon(QMessageBox::Warning);
    box.setText(tr("The file '%1' has been changed by another program.").arg(userFriendlyCurrentFile()));
    box.setInformativeText(isModified()
                            ? tr("Reload it from disk and lose the changes you made here, or ignore the change and keep the document as it is?")
                            : tr("Reload it from disk, or ignore the change and keep the document as it is?"));
    QPushButton* reload = box.addButton(tr("Reload"), QMessageBox::AcceptRole);
    QPushButton* ignore = box.addButton(tr("Ignore"), QMessageBox::RejectRole);
    box.setDefaultButton(isModified() ? ignore : reload);
    // Size it before showing it; see the recovery prompt in MdiMainWindow for the same call.
    box.adjustSize();

    mReloadAsked = true;
    box.exec();
    mReloadAsked = false;

    if (box.clickedButton() == reload)
    {
        mMainWindow->reopenDocument(*this);
    }
}

void MdiChild::checkFileOnDisk()
{
    if (mIsClosing || mReloadAsked || mIsUntitled || mCurFile.isEmpty())
    {
        return;
    }

    if (QFileInfo::exists(mCurFile))
    {
        mFileMissing = false;
        checkFileChangedOnDisk();
        return;
    }

    if (mFileMissing)
    {
        return;
    }

    mFileMissing = true;
    QMessageBox box(this);
    box.setWindowTitle(tr("File Not Found"));
    box.setIcon(QMessageBox::Warning);
    box.setText(tr("The file '%1' is no longer available in the file system.").arg(QDir::toNativeSeparators(mCurFile)));
    box.setInformativeText(isEditableDocument()
                            ? tr("The file or one of its folders was deleted, renamed or moved. Keep the document open and save it later, or close it?")
                            : tr("The file or one of its folders was deleted, renamed or moved. Keep the window open, or close it?"));
    QPushButton* keep = box.addButton(tr("Keep"), QMessageBox::AcceptRole);
    QPushButton* close = box.addButton(tr("Close"), QMessageBox::RejectRole);
    box.setDefaultButton(keep);
    box.adjustSize();

    mReloadAsked = true;
    box.exec();
    mReloadAsked = false;

    if (box.clickedButton() == close)
    {
        setModified(false);
        if (mMdiSubWindow != nullptr)
        {
            mMdiSubWindow->close();
        }
        else
        {
            this->close();
        }
    }
    else if (isEditableDocument())
    {
        setModified(true);
    }
}

void MdiChild::onWindowClosing(bool /*isActive*/)
{
}

void MdiChild::onWindowActivated()
{
}

void MdiChild::onWindowCreated()
{
    
}

void MdiChild::onDocumentModified()
{
    // setWindowModified(document()->isModified());
}

bool MdiChild::maybeSave()
{
    if (mIsModified == false)
        return true;

    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this, tr("lusan"),
            tr("'%1' has been modified.\nDo you want to save your changes?")
            .arg(userFriendlyCurrentFile()),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret)
    {
    case QMessageBox::Save:
        return save();
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }

    return true;
}

void MdiChild::setModified(bool modified)
{
    mIsModified = modified;
    if (mMdiSubWindow != nullptr)
    {
        mMdiSubWindow->setWindowModified(modified);
        bool showWarning{ (mIsUntitled == false) && (mCurFile.isEmpty() == false) && (LusanApplication::isWorkpacePath(mCurFile) == false) };
        QString title{ QString("%1%2%3").arg(showWarning ? "[!] " : "", userFriendlyCurrentFile(), mIsUntitled || mIsModified ? "[*]" : "") };
        mMdiSubWindow->setWindowTitle(title);
        updateTabMark();
    }
}

void MdiChild::updateTabMark()
{
    if ((mMdiSubWindow == nullptr) || (mTabMarked == mIsModified))
        return;

    if (mTabMarked == false)
    {
        mTabIcon = mMdiSubWindow->windowIcon();
    }

    mTabMarked = mIsModified;
    mMdiSubWindow->setWindowIcon(mTabMarked ? QIcon(new UnsavedMarkIconEngine(mTabIcon)) : mTabIcon);
}

void MdiChild::setCurrentFile(const QString& fileName)
{
    mCurFile = fileName.isEmpty() ? QString() : QFileInfo(fileName).canonicalFilePath();
    mIsUntitled = false;
    mFileMissing = false;
    rememberFileState();
    if (mMainWindow != nullptr)
    {
        mMainWindow->refreshDocumentWatch();
    }

    if (mMdiSubWindow != nullptr)
    {
        mMdiSubWindow->setWindowModified(false);
        mMdiSubWindow->setWindowFilePath(mCurFile);
        // Plain ASCII marker; emoji in titles triggers slow DirectWrite font fallback on first use.
        bool showWarning{ (mCurFile.isEmpty() == false) && (LusanApplication::isWorkpacePath(mCurFile) == false) };
        QString title{ QString("%1%2%3").arg( showWarning ? "[!] " : ""
                                            , userFriendlyCurrentFile()
                                            , mIsUntitled || mIsModified ? "[*]" : "") };
        mMdiSubWindow->setWindowTitle(title);
        updateTabMark();
        if (mMainWindow != nullptr)
            mMainWindow->setTabBarTooltip(mMdiSubWindow, mCurFile);
    }
}

QString MdiChild::strippedName(const QString& fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MdiChild::cut()
{
    // Implement cut functionality
#if 0
    if (textCursor().hasSelection())
    {
        textCursor().removeSelectedText();
    }
#endif
}

void MdiChild::copy()
{
#if 0
    // Implement copy functionality
    if (textCursor().hasSelection())
    {
        QApplication::clipboard()->setText(textCursor().selectedText());
    }
#endif
}

void MdiChild::paste()
{
    // Implement paste functionality
    // textCursor().insertText(QApplication::clipboard()->text());
}

void MdiChild::undo()
{
    // Implement undo functionality
    // document()->undo();
}

void MdiChild::redo()
{
    // Implement redo functionality
    // document()->redo();
}

void MdiChild::find()
{
    // No search facility on the base document; overridden by StateMachine.
}

void MdiChild::findUsages()
{
    // No where-used facility on the base document; overridden by StateMachine.
}

void MdiChild::gotoDefinition()
{
    // No go-to-declaration facility on the base document; overridden by StateMachine.
}

bool MdiChild::canUndo() const
{
    return false;
}

bool MdiChild::canRedo() const
{
    return false;
}

void MdiChild::setToolbarVisible(bool /*visible*/)
{
}

bool MdiChild::isToolbarVisible() const
{
    return true;
}

IEDocumentModel* MdiChild::documentModel()
{
    return nullptr;
}

void MdiChild::navigateToIssue(uint32_t /*elementId*/, eDocElementKind /*kind*/, int /*rule*/)
{
}

void MdiChild::clear()
{
    // Implement clear functionality
    // document()->clear();
}

void MdiChild::selectAll()
{
    // Implement select all functionality
    // textCursor().select(QTextCursor::Document);
}

void MdiChild::zoomIn(int range)
{
    // Implement zoom in functionality
    QFont font = this->font();
    font.setPointSize(font.pointSize() + range);
    setFont(font);
}

void MdiChild::zoomOut(int range)
{
    // Implement zoom out functionality
    QFont font = this->font();
    font.setPointSize(font.pointSize() - range);
    setFont(font);
}
