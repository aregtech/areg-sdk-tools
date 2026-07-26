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
#include "lusan/model/sm/SMGuardValidation.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"

#include "lusan/view/sm/NEGuardStyle.hpp"

#include <QColor>
#include <QFont>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QLabel>
#include <QPalette>
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

    //!< What a symbol IS, in the three phrasings both explanation surfaces need.
    struct SymbolFacts
    {
        QString noun;       //!< `FSM attribute`, `condition method`, ...
        QString declared;   //!< where the user would go to change it.
        QString generated;  //!< the form the generator emits.
    };

    SymbolFacts symbolFacts(const SMGuardSymbol& symbol, const SMMethodEntry* method)
    {
        SymbolFacts facts;
        switch (symbol.owner)
        {
        case NEGuardStyle::eOwner::Stimulus:
            facts.noun = SMHoverCard::tr("stimulus parameter");
            facts.declared = SMHoverCard::tr("declared: the trigger's payload");
            facts.generated = symbol.name;
            break;

        case NEGuardStyle::eOwner::Handler:
            facts.noun = SMHoverCard::tr("condition method");
            facts.declared = SMHoverCard::tr("declared: Methods page");
            facts.generated = QString::fromLatin1(SMGuardCodegenPreview::HANDLER_ACCESSOR) + QLatin1Char('.') + symbol.name + QStringLiteral("(...)");
            break;

        case NEGuardStyle::eOwner::Fsm:
        default:
            if ((method != nullptr) && method->isLambdaCondition())
            {
                facts.noun = SMHoverCard::tr("named lambda");
                facts.declared = SMHoverCard::tr("declared on the Methods page, with the body written in Lusan");
                facts.generated = QString::fromLatin1(SMGuardCodegenPreview::LAMBDA_MEMBER_PREFIX) + symbol.name + QStringLiteral("(...)");
            }
            else if (symbol.glyph == QStringLiteral("K"))
            {
                facts.noun = SMHoverCard::tr("FSM constant");
                facts.declared = SMHoverCard::tr("declared: Constants page");
                facts.generated = QString::fromLatin1(SMGuardCodegenPreview::FSM_DATA_QUALIFIER) + QStringLiteral("::") + symbol.name;
            }
            else
            {
                facts.noun = SMHoverCard::tr("FSM attribute");
                facts.declared = SMHoverCard::tr("declared: Attributes page");
                facts.generated = symbol.name + QStringLiteral("()");
            }
            break;
        }

        return facts;
    }

    //!< One tooltip line, optionally in the code font and optionally in \p color.
    QString tipLine(const QString& text, bool monospace = false, const QColor& color = QColor())
    {
        QString body = text.toHtmlEscaped();
        if (monospace)
        {
            body = QStringLiteral("<code>") + body + QStringLiteral("</code>");
        }
        if (color.isValid())
        {
            body = QStringLiteral("<span style='color:%1'>%2</span>").arg(color.name(), body);
        }

        return body + QStringLiteral("<br>");
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

QString SMHoverCard::symbolTip(StateMachineModel& model, uint32_t transitionId, const SMGuardSymbol& symbol)
{
    const StateMachineData& data = model.getData();
    const SMMethodEntry* method = symbol.isCall ? SMGuardSymbols::method(data, symbol.symbolId) : nullptr;
    const SymbolFacts facts = symbolFacts(symbol, method);

    // Leads with the badge the user is LOOKING at -- the owner letter the chip draws -- not the
    // catalog's own glyph vocabulary. Reading `#  FSM attribute` under the pointer while the chip
    // says `[f]` is how a legend teaches the wrong letter.
    const QString glyph = NEGuardStyle::ownerGlyph(symbol.owner);
    const QString badge = glyph.isEmpty() ? facts.noun : QStringLiteral("%1 : %2").arg(glyph, facts.noun);
    QString tip = QStringLiteral("<b><span style='color:%1'>%2</span></b><br>")
                  .arg(NEGuardStyle::ownerColor(symbol.owner).name(), badge.toHtmlEscaped());

    tip += tipLine(symbol.display() + QStringLiteral(" -> ")
                   + (symbol.typeText.isEmpty() ? QStringLiteral("bool") : symbol.typeText), true);
    tip += tipLine(facts.declared);
    if ((method != nullptr) && method->isHandlerCondition())
    {
        tip += tipLine(tr("IMPLEMENTED BY YOUR HANDLER"));
    }
    if ((method != nullptr) && method->isLambdaCondition())
    {
        tip += tipLine(tr("generated as std::function member %1%2")
                       .arg(QString::fromLatin1(SMGuardCodegenPreview::LAMBDA_MEMBER_PREFIX), symbol.name));
    }

    tip += tipLine(tr("called as %1").arg(facts.generated), true);

    // Only what this ELEMENT is guilty of. The transition's other findings belong to the status
    // line and the results panel; repeating them here would make every chip of a troubled guard
    // look equally broken.
    if ((transitionId != 0u) && (symbol.symbolId != 0u))
    {
        for (const SMGuardValidation::Finding& finding : SMGuardValidation::validateTransition(data, transitionId))
        {
            if (finding.symbolId == symbol.symbolId)
            {
                tip += tipLine(finding.message, false
                              , NEGuardStyle::severityColor((finding.severity == DocIssue::eSeverity::Error)
                                                            ? NEGuardStyle::eSeverity::Err
                                                            : (finding.severity == DocIssue::eSeverity::Warning)
                                                              ? NEGuardStyle::eSeverity::Warn
                                                              : NEGuardStyle::eSeverity::Ok));
            }
        }
    }

    return tip;
}

void SMHoverCard::showSymbol(StateMachineModel& model, uint32_t transitionId, const SMGuardSymbol& symbol, const QPoint& globalPos)
{
    clearContent();
    mSymbolId = symbol.symbolId;

    const StateMachineData& data = model.getData();
    const SMMethodEntry* method = symbol.isCall ? SMGuardSymbols::method(data, symbol.symbolId) : nullptr;
    const SymbolFacts facts = symbolFacts(symbol, method);

    addBadgeLine(symbol.owner, facts.noun);
    addLine(symbol.display() + QStringLiteral(" -> ") + (symbol.typeText.isEmpty() ? QStringLiteral("bool") : symbol.typeText), true);
    addLine(facts.declared);
    if ((method != nullptr) && method->isHandlerCondition())
    {
        addLine(tr("IMPLEMENTED BY YOUR HANDLER"));
    }
    if ((method != nullptr) && method->isLambdaCondition())
    {
        addLine(tr("generated as std::function member %1%2")
                .arg(QString::fromLatin1(SMGuardCodegenPreview::LAMBDA_MEMBER_PREFIX), symbol.name));
    }

    addLine(tr("called as %1").arg(facts.generated), true);
    addValidationLines(data, transitionId, symbol.symbolId);

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
            + QStringLiteral(" : ") + (isLambda ? tr("lambda") : tr("handler")));

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

QLabel* SMHoverCard::addBadgeLine(NEGuardStyle::eOwner owner, const QString& noun)
{
    // `f : FSM attribute`, the badge and then what that badge means here. The letter is drawn in
    // the owner hue so the eye ties it to the chip it came from.
    const QString glyph = NEGuardStyle::ownerGlyph(owner);
    QLabel* label = addLine(glyph.isEmpty() ? noun
                                            : QStringLiteral("%1 : %2").arg(glyph, noun));
    QPalette palette = label->palette();
    palette.setColor(QPalette::WindowText, NEGuardStyle::ownerColor(owner));
    label->setPalette(palette);
    QFont font = label->font();
    font.setBold(true);
    label->setFont(font);
    return label;
}

void SMHoverCard::addValidationLines(const StateMachineData& data, uint32_t transitionId, uint32_t symbolId)
{
    if ((transitionId == 0u) || (symbolId == 0u))
    {
        return;
    }

    // Only what this ELEMENT is guilty of. The transition's other findings belong to the status
    // line and the results panel; repeating them here would make every chip of a troubled guard
    // look equally broken.
    for (const SMGuardValidation::Finding& finding : SMGuardValidation::validateTransition(data, transitionId))
    {
        if (finding.symbolId != symbolId)
        {
            continue;
        }

        QLabel* label = addLine(finding.message);
        label->setWordWrap(true);
        QPalette palette = label->palette();
        palette.setColor(QPalette::WindowText
                        , NEGuardStyle::severityColor((finding.severity == DocIssue::eSeverity::Error)
                                                      ? NEGuardStyle::eSeverity::Err
                                                      : (finding.severity == DocIssue::eSeverity::Warning)
                                                        ? NEGuardStyle::eSeverity::Warn
                                                        : NEGuardStyle::eSeverity::Ok));
        label->setPalette(palette);
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
