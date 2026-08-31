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
 *  \file        lusan/view/common/SourceViewer.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, read-only viewer for plain text and source files.
 *
 ************************************************************************/

#include "lusan/view/common/SourceViewer.hpp"

#include "lusan/view/common/CppSyntaxHighlighter.hpp"

#include <QByteArray>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QFontDatabase>
#include <QFontMetricsF>
#include <QGuiApplication>
#include <QPlainTextEdit>
#include <QStringConverter>
#include <QStringList>
#include <QTextCursor>
#include <QTextDocument>
#include <QVBoxLayout>

namespace
{
    //!< How much of the head is inspected to decide whether the file is text at all.
    constexpr qint64    _sniffSize{ 8 * 1024 };
}

SourceViewer::SourceViewer(MdiMainWindow* wndMain, const QString& filePath, QWidget* parent /*= nullptr*/)
    : MdiChild      (MdiChild::eMdiWindow::MdiSourceViewer, wndMain, parent)

    , mText         (nullptr)
    , mHighlighter  (nullptr)
    , mError        ( )
{
    setupWidgets();
    if (filePath.isEmpty() == false)
    {
        readFile(filePath);
    }
}

bool SourceViewer::openSucceeded() const
{
    return mError.isEmpty();
}

bool SourceViewer::loadFile(const QString& fileName)
{
    return (readFile(fileName) ? MdiChild::loadFile(fileName) : false);
}

bool SourceViewer::save()
{
    return false;
}

bool SourceViewer::saveAs()
{
    return false;
}

bool SourceViewer::saveFile(const QString& /*fileName*/)
{
    return false;
}

void SourceViewer::cut()
{
}

void SourceViewer::copy()
{
    mText->copy();
}

void SourceViewer::paste()
{
}

void SourceViewer::setupWidgets()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    const QFont mono = QFontDatabase::systemFont(QFontDatabase::FixedFont);

    mText = new QPlainTextEdit(this);
    mText->setReadOnly(true);
    mText->setFont(mono);
    mText->setLineWrapMode(QPlainTextEdit::NoWrap);
    mText->setTabStopDistance(4 * QFontMetricsF(mono).horizontalAdvance(QLatin1Char(' ')));

    // The caret is what tells the reader where they are while scrolling with the keyboard, and a
    // read-only QPlainTextEdit hides it by default.
    mText->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

    layout->addWidget(mText);
}

bool SourceViewer::readFile(const QString& filePath)
{
    mError.clear();
    mText->clear();

    const QFileInfo info(filePath);
    if (info.isFile() == false)
    {
        mError = tr("This is not a file.");
        return false;
    }

    if (info.size() > SourceViewer::MaxViewSize)
    {
        mError = tr("The file is %1 MB. Lusan shows text files up to %2 MB.")
                    .arg(info.size() / (1024 * 1024))
                    .arg(SourceViewer::MaxViewSize / (1024 * 1024));
        return false;
    }

    QFile file(filePath);
    if (file.open(QFile::ReadOnly) == false)
    {
        mError = file.errorString();
        return false;
    }

    // A NUL byte in the head is the practical test for "not text". It is no proof, but showing a
    // binary as mojibake is worse than saying plainly that it cannot be shown.
    const QByteArray head = file.peek(_sniffSize);
    if (head.contains('\0'))
    {
        mError = tr("The file is not a text file.");
        return false;
    }

    const QByteArray raw = file.readAll();
    file.close();

    // The decoder consumes a byte order mark when there is one. Latin-1 is the fallback for bytes
    // that are not valid UTF-8: it never fails, so the file still opens and the code stays legible.
    QStringDecoder decoder(QStringConverter::Utf8);
    QString text = decoder(raw);
    if (decoder.hasError())
    {
        text = QString::fromLatin1(raw);
    }

    if ((mHighlighter == nullptr) && SourceViewer::isCppSuffix(info.suffix()))
    {
        mHighlighter = new CppSyntaxHighlighter(mText->document());
    }

    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    mText->setPlainText(text);
    mText->moveCursor(QTextCursor::Start);
    QGuiApplication::restoreOverrideCursor();

    // Nothing here can modify the document, so the window must never grow the "[*]" marker or
    // ask about unsaved changes on close.
    mText->document()->setModified(false);
    setModified(false);

    return true;
}

bool SourceViewer::isCppSuffix(const QString& suffix)
{
    static const QStringList _cppSuffixes
    {
          "c", "cc", "cpp", "cxx", "c++", "cppm", "ixx"
        , "h", "h++", "hh", "hpp", "hxx"
        , "inl", "ipp", "inc", "tlh", "tli"
    };

    return _cppSuffixes.contains(suffix, Qt::CaseInsensitive);
}
