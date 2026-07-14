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
 *  \file        lusan/view/sm/SMSymbolPopup.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard completion catalog popup (v7 B4).
 *
 ************************************************************************/

#include "lusan/view/sm/SMSymbolPopup.hpp"

#include <QListView>
#include <QPainter>
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

    QString groupLabel(NEGuardStyle::eOwner owner)
    {
        switch (owner)
        {
        case NEGuardStyle::eOwner::Stimulus:    return QStringLiteral("STIMULUS");
        case NEGuardStyle::eOwner::Handler:     return QStringLiteral("HANDLER");
        case NEGuardStyle::eOwner::Fsm:
        default:                                return QStringLiteral("FSM");
        }
    }

    //!< Paints the three-column catalog row (glyph + name / type / provenance) or a header.
    class SMSymbolDelegate : public QStyledItemDelegate
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
            const bool enabled = (index.flags() & Qt::ItemIsEnabled);
            painter->setPen(enabled ? option.palette.color(QPalette::WindowText)
                                    : option.palette.color(QPalette::Disabled, QPalette::WindowText));
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

SMSymbolPopup::SMSymbolPopup(QWidget* parent /*= nullptr*/)
    : QWidget (parent)
    , mView   (nullptr)
    , mModel  (nullptr)
{
    setObjectName(QStringLiteral("smSymbolPopup"));
    setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_ShowWithoutActivating);

    QVBoxLayout* box = new QVBoxLayout(this);
    box->setContentsMargins(1, 1, 1, 1);

    mModel = new QStandardItemModel(this);
    mView = new QListView(this);
    mView->setObjectName(QStringLiteral("smSymbolPopupList"));
    mView->setModel(mModel);
    mView->setItemDelegate(new SMSymbolDelegate(mView));
    mView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mView->setSelectionMode(QAbstractItemView::SingleSelection);
    mView->setUniformItemSizes(false);
    mView->setFocusPolicy(Qt::NoFocus);
    box->addWidget(mView);

    connect(mView, &QListView::clicked, this, [this](const QModelIndex& index)
    {
        if (index.data(RoleKind).toInt() == 1)
        {
            mView->setCurrentIndex(index);
            acceptCurrent();
        }
    });
}

void SMSymbolPopup::setSymbols(const QList<SMGuardSymbol>& symbols)
{
    mAll = symbols;
}

int SMSymbolPopup::rebuild(const QString& prefix)
{
    mModel->clear();
    mVisible.clear();

    const QString needle = prefix.trimmed();
    bool firstGroup = true;
    NEGuardStyle::eOwner currentGroup = NEGuardStyle::eOwner::Operator;

    int entries = 0;
    for (const SMGuardSymbol& sym : mAll)
    {
        if ((needle.isEmpty() == false) && (sym.name.contains(needle, Qt::CaseInsensitive) == false))
        {
            continue;
        }

        if (firstGroup || (sym.owner != currentGroup))
        {
            currentGroup = sym.owner;
            firstGroup = false;
            QStandardItem* header = new QStandardItem(groupLabel(sym.owner));
            header->setData(0, RoleKind);
            header->setData(groupLabel(sym.owner), RoleName);
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

    // The New-lambda row: present, disabled (Session U3).
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
    lambda->setToolTip(QStringLiteral("Session U3"));
    lambda->setFlags(Qt::ItemIsSelectable);   // selectable but not enabled -> disabled look
    mModel->appendRow(lambda);

    return entries;
}

bool SMSymbolPopup::filterAndShow(const QString& prefix, const QPoint& globalTopLeft)
{
    const int entries = rebuild(prefix);
    if (entries == 0)
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

    resize(320, height + 2);
    move(globalTopLeft);
    if (isVisible() == false)
    {
        show();
    }

    return true;
}

int SMSymbolPopup::currentEntryIndex() const
{
    const QModelIndex index = mView->currentIndex();
    if ((index.isValid() == false) || (index.data(RoleKind).toInt() != 1))
    {
        return -1;
    }

    return index.data(RoleVisIndex).toInt();
}

void SMSymbolPopup::moveCurrent(int delta)
{
    const int rows = mModel->rowCount();
    if (rows == 0)
    {
        return;
    }

    int row = mView->currentIndex().isValid() ? mView->currentIndex().row() : -1;
    const int step = (delta >= 0) ? 1 : -1;
    int guard = 0;

    // Find the next selectable entry row (kind == 1) in the requested direction.
    row += (delta == 0) ? 1 : step;
    while ((row >= 0) && (row < rows) && (guard <= rows))
    {
        if (mModel->item(row)->data(RoleKind).toInt() == 1)
        {
            mView->setCurrentIndex(mModel->index(row, 0));
            return;
        }

        row += step;
        ++guard;
    }
}

const SMGuardSymbol* SMSymbolPopup::currentSymbol() const
{
    const int index = currentEntryIndex();
    return ((index >= 0) && (index < mVisible.size())) ? &mVisible.at(index) : nullptr;
}

void SMSymbolPopup::acceptCurrent()
{
    const SMGuardSymbol* symbol = currentSymbol();
    if (symbol != nullptr)
    {
        const SMGuardSymbol copy = *symbol;
        hide();
        emit accepted(copy);
    }
}
