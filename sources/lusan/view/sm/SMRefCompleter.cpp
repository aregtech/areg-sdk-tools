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
 *  \file        lusan/view/sm/SMRefCompleter.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard reference completer.
 *
 ************************************************************************/

#include "lusan/view/sm/SMRefCompleter.hpp"

#include <QGuiApplication>
#include <QListView>
#include <QPainter>
#include <QScreen>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QVBoxLayout>

namespace
{
    enum eRole
    {
          RoleKind      = Qt::UserRole + 1  //!< 0 header, 1 entry, 2 new-lambda.
        , RoleGlyph                          //!< The owner glyph (entries).
        , RoleColor                          //!< The owner color (entries).
        , RoleName                           //!< The display name / label.
        , RoleType                           //!< The type text (entries).
        , RoleProv                           //!< The provenance cue (entries).
        , RoleVisIndex                       //!< The mVisible index (entries).
    };

    //!< The group header for a reference kind (the completer groups by kind).
    QString groupLabel(SMGuardSymbol::eRefKind kind)
    {
        switch (kind)
        {
        case SMGuardSymbol::eRefKind::Param:    return QStringLiteral("PARAMETERS");
        case SMGuardSymbol::eRefKind::Attr:     return QStringLiteral("ATTRIBUTES");
        case SMGuardSymbol::eRefKind::Const:    return QStringLiteral("CONSTANTS");
        case SMGuardSymbol::eRefKind::Cond:
        default:                                return QStringLiteral("CONDITIONS");
        }
    }

    //!< Paints the three-column catalog row (glyph + name / type / provenance) or a header.
    class SMRefDelegate : public QStyledItemDelegate
    {
    public:
        using QStyledItemDelegate::QStyledItemDelegate;

        QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override
        {
            QSize size = QStyledItemDelegate::sizeHint(option, index);
            const int kind = index.data(RoleKind).toInt();
            size.setHeight((kind == 0) ? 18 : 22);
            return size;
        }

        void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
        {
            const int kind = index.data(RoleKind).toInt();
            QStyleOptionViewItem opt(option);
            initStyleOption(&opt, index);
            opt.text.clear();
            opt.widget->style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

            QRect rect = option.rect.adjusted(6, 0, -6, 0);
            painter->save();

            if (kind == 0)
            {
                QFont font = option.font;
                font.setBold(true);
                font.setPointSizeF(font.pointSizeF() * 0.85);
                painter->setFont(font);
                painter->setPen(option.palette.color(QPalette::Disabled, QPalette::WindowText));
                painter->drawText(rect, Qt::AlignVCenter | Qt::AlignLeft, index.data(RoleName).toString());
                painter->restore();
                return;
            }

            // Glyph cell (owner hue).
            const QString glyph = index.data(RoleGlyph).toString();
            const QColor color = index.data(RoleColor).value<QColor>();
            QRect glyphRect(rect.left(), rect.top(), 22, rect.height());
            painter->setPen(color.isValid() ? color : option.palette.color(QPalette::WindowText));
            painter->drawText(glyphRect, Qt::AlignVCenter | Qt::AlignLeft, glyph);

            QRect textRect = rect.adjusted(26, 0, 0, 0);
            painter->setPen(option.palette.color(QPalette::WindowText));
            painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, index.data(RoleName).toString());

            // Right side: provenance (italic dim) then type (dim), right-aligned.
            painter->setPen(option.palette.color(QPalette::Disabled, QPalette::WindowText));
            const QString prov = index.data(RoleProv).toString();
            const QString type = index.data(RoleType).toString();
            if (prov.isEmpty() == false)
            {
                QFont italic = option.font;
                italic.setItalic(true);
                painter->setFont(italic);
                painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignRight, prov);
            }
            if (type.isEmpty() == false)
            {
                painter->setFont(option.font);
                QRect typeRect = textRect.adjusted(0, 0, -110, 0);
                painter->drawText(typeRect, Qt::AlignVCenter | Qt::AlignRight, type);
            }

            painter->restore();
        }
    };
}

SMRefCompleter::SMRefCompleter(QWidget* parent /*= nullptr*/)
    : QWidget (parent)
    , mView   (nullptr)
    , mModel  (nullptr)
{
    setObjectName(QStringLiteral("smRefCompleter"));
    // A top-level, frameless, non-activating window: the field keeps focus and forwards keys,
    // so nothing here can swallow a keystroke. Being top-level, its width is
    // irrelevant to the Properties dock.
    setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_ShowWithoutActivating);

    QVBoxLayout* box = new QVBoxLayout(this);
    box->setContentsMargins(1, 1, 1, 1);

    mModel = new QStandardItemModel(this);
    mView = new QListView(this);
    mView->setObjectName(QStringLiteral("smRefCompleterList"));
    mView->setModel(mModel);
    mView->setItemDelegate(new SMRefDelegate(mView));
    mView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mView->setSelectionMode(QAbstractItemView::SingleSelection);
    mView->setUniformItemSizes(false);
    mView->setFocusPolicy(Qt::NoFocus);
    box->addWidget(mView);

    connect(mView, &QListView::clicked, this, [this](const QModelIndex& index)
    {
        const int kind = index.data(RoleKind).toInt();
        if (kind == 1)
        {
            mView->setCurrentIndex(index);
            acceptCurrent();
        }
        else if (kind == 2)
        {
            hide();
            emit newLambdaRequested();
        }
    });
}

void SMRefCompleter::setSymbols(const QList<SMGuardSymbol>& symbols)
{
    mAll = symbols;
}

int SMRefCompleter::rebuild(const QSet<SMGuardSymbol::eRefKind>& kindFilter, const QString& nameFilter)
{
    mModel->clear();
    mVisible.clear();

    const QString needle = nameFilter.trimmed();
    bool firstGroup = true;
    SMGuardSymbol::eRefKind currentGroup = SMGuardSymbol::eRefKind::Param;

    int entries = 0;
    for (const SMGuardSymbol& sym : mAll)
    {
        if ((kindFilter.isEmpty() == false) && (kindFilter.contains(sym.refkind) == false))
        {
            continue;
        }
        if ((needle.isEmpty() == false) && (sym.name.contains(needle, Qt::CaseInsensitive) == false))
        {
            continue;
        }

        if (firstGroup || (sym.refkind != currentGroup))
        {
            currentGroup = sym.refkind;
            firstGroup = false;
            QStandardItem* header = new QStandardItem(groupLabel(sym.refkind));
            header->setData(0, RoleKind);
            header->setData(groupLabel(sym.refkind), RoleName);
            header->setFlags(Qt::NoItemFlags);
            mModel->appendRow(header);
        }

        const int visIndex = mVisible.size();
        mVisible.append(sym);

        QStandardItem* item = new QStandardItem();
        item->setData(1, RoleKind);
        item->setData(sym.glyph, RoleGlyph);
        item->setData(NEGuardStyle::ownerColor(sym.owner), RoleColor);
        item->setData(sym.display(), RoleName);
        item->setData(sym.typeText, RoleType);
        item->setData(sym.provenance, RoleProv);
        item->setData(visIndex, RoleVisIndex);
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        mModel->appendRow(item);
        ++entries;
    }

    // The New-lambda row (only in the all-kinds view): inserts an island at the caret.
    if (kindFilter.isEmpty())
    {
        QStandardItem* sep = new QStandardItem();
        sep->setData(0, RoleKind);
        sep->setData(QStringLiteral("--"), RoleName);
        sep->setFlags(Qt::NoItemFlags);
        mModel->appendRow(sep);

        QStandardItem* lambda = new QStandardItem();
        lambda->setData(2, RoleKind);
        lambda->setData(QStringLiteral("{}"), RoleGlyph);
        lambda->setData(NEGuardStyle::ownerColor(NEGuardStyle::eOwner::Fsm), RoleColor);
        lambda->setData(QStringLiteral("New lambda here..."), RoleName);
        lambda->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        mModel->appendRow(lambda);
    }

    return entries;
}

bool SMRefCompleter::showFor(const QSet<SMGuardSymbol::eRefKind>& kindFilter, const QString& nameFilter, const QRect& caretGlobal)
{
    const int entries = rebuild(kindFilter, nameFilter);
    if ((entries == 0) && kindFilter.isEmpty() == false)
    {
        hide();
        return false;
    }

    moveCurrent(0);
    if (currentEntryIndex() < 0)
    {
        moveCurrent(1);
    }

    const int rows = mModel->rowCount();
    int height = 6;
    for (int i = 0; i < rows; ++i)
    {
        height += mView->sizeHintForRow(i);
    }
    height = qMin(height, 260);

    const int width = 320;
    resize(width, height + 2);

    // Position at the caret, then CLAMP to the screen and FLIP above on vertical overflow.
    const QScreen* screen = QGuiApplication::screenAt(caretGlobal.bottomLeft());
    const QRect avail = (screen != nullptr) ? screen->availableGeometry()
                                            : QGuiApplication::primaryScreen()->availableGeometry();
    int x = caretGlobal.left();
    int y = caretGlobal.bottom() + 2;
    if ((x + width) > avail.right())        { x = avail.right() - width; }
    if (x < avail.left())                   { x = avail.left(); }
    if ((y + height + 2) > avail.bottom())   { y = caretGlobal.top() - (height + 2) - 2; }  // flip above
    if (y < avail.top())                    { y = avail.top(); }
    move(x, y);

    if (isVisible() == false)
    {
        show();
    }

    return true;
}

int SMRefCompleter::currentEntryIndex() const
{
    const QModelIndex index = mView->currentIndex();
    if ((index.isValid() == false) || (index.data(RoleKind).toInt() != 1))
    {
        return -1;
    }

    return index.data(RoleVisIndex).toInt();
}

void SMRefCompleter::moveCurrent(int delta)
{
    const int rows = mModel->rowCount();
    if (rows == 0)
    {
        return;
    }

    int row = mView->currentIndex().isValid() ? mView->currentIndex().row() : -1;
    const int step = (delta >= 0) ? 1 : -1;
    int guard = 0;

    // Find the next selectable row (a symbol entry or the new-lambda row) in direction.
    row += (delta == 0) ? 1 : step;
    while ((row >= 0) && (row < rows) && (guard <= rows))
    {
        const int kind = mModel->item(row)->data(RoleKind).toInt();
        if ((kind == 1) || (kind == 2))
        {
            mView->setCurrentIndex(mModel->index(row, 0));
            return;
        }

        row += step;
        ++guard;
    }
}

const SMGuardSymbol* SMRefCompleter::currentSymbol() const
{
    const int index = currentEntryIndex();
    return ((index >= 0) && (index < mVisible.size())) ? &mVisible.at(index) : nullptr;
}

void SMRefCompleter::acceptCurrent()
{
    const QModelIndex index = mView->currentIndex();
    if (index.isValid() && (index.data(RoleKind).toInt() == 2))
    {
        hide();
        emit newLambdaRequested();
        return;
    }

    const SMGuardSymbol* symbol = currentSymbol();
    if (symbol != nullptr)
    {
        const SMGuardSymbol copy = *symbol;
        hide();
        emit accepted(copy);
    }
}
