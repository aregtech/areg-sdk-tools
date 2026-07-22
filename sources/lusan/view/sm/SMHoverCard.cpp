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
 *  \file        lusan/view/sm/SMHoverCard.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard hover cards.
 *
 ************************************************************************/

#include "lusan/view/sm/SMHoverCard.hpp"

#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"

#include "lusan/model/sm/SMGuardCodegenPreview.hpp"
#include "lusan/model/sm/SMGuardRender.hpp"
#include "lusan/model/sm/SMGuardSymbols.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"

#include "lusan/view/sm/NEGuardStyle.hpp"

#include <QFontDatabase>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

namespace
{
    //!< The node reached by the child-index \p path, or nullptr.
    const SMGuardNode* nodeAt(const SMGuardNode* root, const QList<int>& path)
    {
        const SMGuardNode* node = root;
        for (int index : path)
        {
            if (node == nullptr) { return nullptr; }
            node = node->childAt(index);
        }

        return node;
    }
}

SMHoverCard::SMHoverCard(QWidget* parent /*= nullptr*/)
    : QFrame        (parent)
    , mContent      (nullptr)
    , mWhereUsed    (nullptr)
    , mMapArgs      (nullptr)
    , mButtonRow    (nullptr)
    , mHideTimer    (nullptr)
    , mSymbolId     (0u)
{
    setObjectName(QStringLiteral("smHoverCard"));
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Raised);

    QVBoxLayout* outer = new QVBoxLayout(this);
    outer->setContentsMargins(10, 8, 10, 8);
    outer->setSpacing(2);

    mContent = new QVBoxLayout();
    mContent->setSpacing(2);
    outer->addLayout(mContent);

    mButtonRow = new QWidget(this);
    QHBoxLayout* buttons = new QHBoxLayout(mButtonRow);
    buttons->setContentsMargins(0, 4, 0, 0);
    mWhereUsed = new QPushButton(tr("where used"), mButtonRow);
    mWhereUsed->setFlat(true);
    mMapArgs = new QPushButton(tr("map args"), mButtonRow);
    mMapArgs->setFlat(true);
    buttons->addWidget(mWhereUsed);
    buttons->addWidget(mMapArgs);
    buttons->addStretch(1);
    outer->addWidget(mButtonRow);

    mHideTimer = new QTimer(this);
    mHideTimer->setSingleShot(true);
    mHideTimer->setInterval(300);

    connect(mHideTimer, &QTimer::timeout, this, [this]()
    {
        if (underMouse() == false)
        {
            hide();
        }
    });
    connect(mWhereUsed, &QPushButton::clicked, this, [this]()
    {
        hide();
        emit whereUsedRequested(mSymbolId);
    });
    connect(mMapArgs, &QPushButton::clicked, this, [this]()
    {
        hide();
        emit mapArgsRequested(mSymbolId);
    });
}

//////////////////////////////////////////////////////////////////////////
// Faces
//////////////////////////////////////////////////////////////////////////

void SMHoverCard::showSymbol(StateMachineModel& model, uint32_t /*transitionId*/, const SMGuardSymbol& symbol, const QPoint& globalPos)
{
    clearContent();
    mSymbolId = symbol.symbolId;

    const StateMachineData& data = model.getData();
    const SMMethodEntry* method = symbol.isCall ? SMGuardSymbols::method(data, symbol.symbolId) : nullptr;

    QString kindLine;
    QString declared;
    QString generated;
    switch (symbol.owner)
    {
    case NEGuardStyle::eOwner::Stimulus:
        kindLine = tr("a  stimulus parameter");
        declared = tr("declared: the trigger's payload");
        generated = symbol.name;
        break;

    case NEGuardStyle::eOwner::Handler:
        kindLine = tr("h  condition method");
        declared = tr("declared: Methods page");
        generated = QString::fromLatin1(SMGuardCodegenPreview::HANDLER_ACCESSOR) + QLatin1Char('.') + symbol.name + QStringLiteral("(...)");
        break;

    case NEGuardStyle::eOwner::Fsm:
    default:
        if ((method != nullptr) && method->isLambdaCondition())
        {
            kindLine = tr("{} named lambda");
            declared = tr("declared: Methods page -- body written in Lusan");
            generated = QString::fromLatin1(SMGuardCodegenPreview::LAMBDA_MEMBER_PREFIX) + symbol.name + QStringLiteral("(...)");
        }
        else if (symbol.glyph == QStringLiteral("K"))
        {
            kindLine = tr("K  FSM constant");
            declared = tr("declared: Constants page");
            generated = QString::fromLatin1(SMGuardCodegenPreview::FSM_DATA_QUALIFIER) + QStringLiteral("::") + symbol.name;
        }
        else
        {
            kindLine = tr("#  FSM attribute");
            declared = tr("declared: Attributes page");
            generated = symbol.name + QStringLiteral("()");
        }
        break;
    }

    addLine(kindLine);
    addLine(symbol.display() + QStringLiteral(" -> ") + (symbol.typeText.isEmpty() ? QStringLiteral("bool") : symbol.typeText), true);
    addLine(declared);
    if ((method != nullptr) && method->isHandlerCondition())
    {
        addLine(tr("IMPLEMENTED BY YOUR HANDLER"));
    }
    if ((method != nullptr) && method->isLambdaCondition())
    {
        addLine(tr("generated as std::function member %1%2")
                .arg(QString::fromLatin1(SMGuardCodegenPreview::LAMBDA_MEMBER_PREFIX), symbol.name));
    }

    addLine(tr("called as %1").arg(generated), true);

    mButtonRow->setVisible(true);
    mMapArgs->setVisible(symbol.isCall);

    placeAt(globalPos);
}

void SMHoverCard::showCall(StateMachineModel& model, uint32_t transitionId, const QList<int>& callPath, const QPoint& globalPos)
{
    const StateMachineData& data = model.getData();
    const SMTransitionEntry* transition = data.findTransitionById(transitionId);
    if ((transition == nullptr) || (transition->getGuard().isOk() == false))
    {
        return;
    }

    const SMGuardNode* call = nodeAt(transition->getGuard().getTree(), callPath);
    if ((call == nullptr) || (call->getKind() != SMGuardNode::eKind::Call))
    {
        return;
    }

    const SMMethodEntry* method = SMGuardSymbols::method(data, call->getSymbolId());
    if (method == nullptr)
    {
        return;
    }

    clearContent();
    mSymbolId = call->getSymbolId();

    const bool isLambda = method->isLambdaCondition();
    addLine((isLambda ? QStringLiteral("{}  ") : QStringLiteral("h  ")) + method->getName()
            + QStringLiteral(" -- ") + (isLambda ? tr("lambda") : tr("handler")));

    const QList<MethodParameter>& params = method->getElements();
    for (int i = 0; i < params.size(); ++i)
    {
        const QString argText = (i < call->getCount())
                                ? SMGuardRender::text(data, transitionId, *call->childAt(i))
                                : QString();
        addLine(QStringLiteral("  ") + params.at(i).getName() + QStringLiteral(" : ") + params.at(i).getType()
                + QStringLiteral(" <- ") + argText, true);
    }

    addLine(tr("generated:"));
    addLine(QStringLiteral("  ") + SMGuardCodegenPreview::expression(data, transitionId, *call), true);

    mButtonRow->setVisible(false);
    placeAt(globalPos);
}

//////////////////////////////////////////////////////////////////////////
// Show / hide plumbing
//////////////////////////////////////////////////////////////////////////

void SMHoverCard::scheduleHide()
{
    mHideTimer->start();
}

void SMHoverCard::cancelHide()
{
    mHideTimer->stop();
}

void SMHoverCard::enterEvent(QEnterEvent* event)
{
    cancelHide();
    QFrame::enterEvent(event);
}

void SMHoverCard::leaveEvent(QEvent* event)
{
    scheduleHide();
    QFrame::leaveEvent(event);
}

void SMHoverCard::clearContent()
{
    while (mContent->count() > 0)
    {
        QLayoutItem* item = mContent->takeAt(0);
        delete item->widget();
        delete item;
    }
}

QLabel* SMHoverCard::addLine(const QString& text, bool monospace /*= false*/)
{
    QLabel* label = new QLabel(text, this);
    if (monospace)
    {
        label->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    }

    mContent->addWidget(label);
    return label;
}

void SMHoverCard::placeAt(const QPoint& globalPos)
{
    adjustSize();
    move(globalPos);
    show();
    raise();
}
