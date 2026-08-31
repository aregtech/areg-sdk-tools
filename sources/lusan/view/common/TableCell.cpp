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
 *  \file        lusan/view/common/TableCell.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, ComboBox for the table cell.
 *
 ************************************************************************/

#include "lusan/view/common/TableCell.hpp"
#include "lusan/app/LusanApplication.hpp"
#include "lusan/common/NELusanCommon.hpp"

#include <QAbstractItemView>
#include <QComboBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QStandardItemModel>
#include <QLineEdit>
#include <QStyleFactory>
#include <QTableWidget>
#include <QTableWidgetItem>

TableCell::TableCell(QWidget* parent, IETableHelper * tableHelper, bool waitEndEdit)
    : QStyledItemDelegate(parent)
    , mModels   ( )
    , mColumns  ( )
    , mValidation( )
    , mEditable ( )
    , mModelOf  ( )
    , mValidOf  ( )
    , mTable    (tableHelper)
    , mWaitEnd  (waitEndEdit)
    , mNewText  ( )
    , mSelIndex ( )
    , mEditOriginal( )
{
    // Escape cancels the edit: the view reports RevertModelCache when the editor closes.
    connect(this, &QAbstractItemDelegate::closeEditor, this, &TableCell::onCloseEditor);
}

TableCell::TableCell(const QList<QAbstractItemModel*>& models, const QList<int>& columns, QWidget* parent, IETableHelper * tableHelper, bool waitEndEdit)
    : QStyledItemDelegate(parent)
    , mModels   (models)
    , mColumns  (columns)
    , mValidation( )
    , mEditable ( )
    , mModelOf  ( )
    , mValidOf  ( )
    , mTable    (tableHelper)
    , mWaitEnd  (waitEndEdit)
    , mNewText  ( )
    , mSelIndex ( )
    , mEditOriginal( )
{
    connect(this, &QAbstractItemDelegate::closeEditor, this, &TableCell::onCloseEditor);

    Q_ASSERT(models.size() == columns.size());
    for (QAbstractItemModel* model : models)
    {
        if (model != nullptr)
        {
            model->setParent(this);
        }
    }
}

void TableCell::setColumnValidation(int column, eCellValidation kind)
{
    if (kind == eCellValidation::NoValidation)
    {
        mValidation.remove(column);
    }
    else
    {
        mValidation.insert(column, kind);
    }
}

void TableCell::setEditableCheck(FuncEditable check)
{
    mEditable = std::move(check);
}

void TableCell::setEditorModelResolver(FuncEditorModel resolver)
{
    mModelOf = std::move(resolver);
}

void TableCell::setValidationResolver(FuncValidation resolver)
{
    mValidOf = std::move(resolver);
}

inline bool TableCell::isValidColumn(int col) const
{
    return (col >= 0) && (col < mTable->getColumnCount());
}

QAbstractItemModel* TableCell::columnToModel(int col) const
{
    if (isValidColumn(col))
    {
        for (int i = 0; i < static_cast<int>(mColumns.size()); ++i)
        {
            if (mColumns[i] == col)
            {
                return static_cast<QAbstractItemModel*>(mModels[i]);
            }
        }
    }

    return nullptr;
}

QWidget* TableCell::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& index) const
{
    mNewText.clear();
    mSelIndex = QModelIndex();
    mEditOriginal.clear();

    if (mEditable && (mEditable(index) == false))
    {
        return nullptr;
    }

    QAbstractItemModel* model = mModelOf ? mModelOf(index) : columnToModel(index.column());
    if (model != nullptr)
    {
        QComboBox* combo = new QComboBox(parent);
        combo->setModel(model);
        combo->setProperty("index", index);
        // The drop-down inherits the cell width and elides its entries, so it is widened here.
        if (QAbstractItemView* popup = combo->view())
        {
            popup->setTextElideMode(Qt::ElideNone);
            const int hint = popup->sizeHintForColumn(combo->modelColumn());
            if (hint > 0)
            {
                popup->setMinimumWidth(hint + 24);
            }
        }

        // Commit only on user activation, never on currentTextChanged.
        connect(combo, &QComboBox::activated, this, &TableCell::onComboActivated);

        return combo;
    }
    else if ( isValidColumn(index.column()) )
    {
        QLineEdit* lineEdit = new QLineEdit(parent);
        // The index travels with the editor. Without it the change routes nowhere and is dropped.
        lineEdit->setProperty("index", index);
        // The committed text, for the rollback an Escape needs.
        mEditOriginal = mTable->getCellText(index);
        const eCellValidation kind = mValidOf ? mValidOf(index) : mValidation.value(index.column(), eCellValidation::NoValidation);
        switch (kind)
        {
        case eCellValidation::Identifier:
            lineEdit->setValidator(NELusanCommon::createIdentifierValidator(lineEdit));
            break;
        case eCellValidation::Path:
            lineEdit->setValidator(NELusanCommon::createPathValidator(lineEdit));
            break;
        case eCellValidation::QualifiedName:
            lineEdit->setValidator(NELusanCommon::createQualifiedNameValidator(lineEdit));
            break;
        case eCellValidation::Value:
            // Enumeration value: letters, digits, '_' and '::' (e.g. Other::Value or 0x10).
            lineEdit->setValidator(new QRegularExpressionValidator(QRegularExpression(QStringLiteral("[A-Za-z0-9_:]*")), lineEdit));
            break;
        default:
            break;
        }
        connect(lineEdit, &QLineEdit::textEdited, this, &TableCell::onEditorTextChanged);
        if (mWaitEnd)
        {
            connect(lineEdit, &QLineEdit::editingFinished, this, &TableCell::onEditorTextChangeFinished);
        }

        return lineEdit;
    }
    
    return nullptr;
}

void TableCell::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    // Keyed off the editor type: the resolver can put a combo on any column.
    if (QComboBox* combo = qobject_cast<QComboBox*>(editor))
    {
        if (index.data(Qt::EditRole).isNull())
            combo->setCurrentIndex(-1);
        else
            combo->setCurrentText(index.model()->data(index, Qt::EditRole).toString());
    }
    else if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(editor))
    {
        lineEdit->setText(mTable->getCellText(index));
    }
}

void TableCell::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& /*index*/) const
{
    if (editor != nullptr)
    {
        // The editor grows vertically, centered on the cell. Its width stays inside the cell:
        // anything wider covers the neighbouring column while the edit is open.
        QRect rect = option.rect;
        const int minHeight = qMax(editor->sizeHint().height(), 24);
        if (rect.height() < minHeight)
        {
            rect.setTop(rect.top() - ((minHeight - rect.height()) / 2));
            rect.setHeight(minHeight);
        }
        editor->setGeometry(rect);
        if (QComboBox* combo = qobject_cast<QComboBox*>(editor))
        {
            combo->showPopup();
        }
    }
}

void TableCell::onComboActivated(int /*index*/)
{
    QComboBox* combo = qobject_cast<QComboBox*>(sender());
    if (combo != nullptr)
    {
        // The owning page updates the model, so the base setModelData() stays uncalled.
        emit signalEditorDataChanged(combo->property("index").toModelIndex(), combo->currentText());
        emit closeEditor(combo);
    }
}

void TableCell::onEditorTextChanged(const QString & newText)
{
    QWidget *editor = qobject_cast<QWidget *>(sender());
    if (editor != nullptr)
    {
        const QModelIndex index = editor->property("index").toModelIndex();
        emit signalEditorTextChanged(index, newText);

        if (mWaitEnd == false)
        {
            emit signalEditorDataChanged(index, newText);
        }
        else
        {
            mNewText = newText;
            mSelIndex = index;
        }
    }
}

void TableCell::onEditorTextChangeFinished()
{
    // An emptied cell is a text edit like any other; the page decides whether it is allowed.
    if (mWaitEnd && mSelIndex.isValid())
    {
        emit signalEditorDataChanged(mSelIndex, mNewText);
        mSelIndex = QModelIndex();
        mNewText.clear();
    }
}

void TableCell::onCloseEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint)
{
    if (qobject_cast<QLineEdit*>(editor) == nullptr)
        return;

    const QModelIndex index = editor->property("index").toModelIndex();
    if (hint == QAbstractItemDelegate::RevertModelCache)
    {
        // Escape. Drop the pending text before the lost focus turns it into a commit.
        mSelIndex = QModelIndex();
        mNewText.clear();

        // Live mode already applied every keystroke, so the pre-edit value is replayed.
        if ((mWaitEnd == false) && index.isValid())
        {
            emit signalEditorDataChanged(index, mEditOriginal);
        }
    }

    mEditOriginal.clear();

    // Queued, so a commit that the closing editor still has to report is dispatched first.
    if (index.isValid())
    {
        QMetaObject::invokeMethod(this, [this]() { emit signalEditorClosed(); }, Qt::QueuedConnection);
    }
}
