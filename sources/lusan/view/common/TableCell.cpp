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
    , mParent   (parent)
    , mTable    (tableHelper)
    , mWaitEnd  (waitEndEdit)
    , mNewText  ( )
    , mSelIndex ( )
{
}

TableCell::TableCell(const QList<QAbstractItemModel*>& models, const QList<int>& columns, QWidget* parent, IETableHelper * tableHelper, bool waitEndEdit)
    : QStyledItemDelegate(parent)
    , mModels   (models)
    , mColumns  (columns)
    , mValidation( )
    , mEditable ( )
    , mModelOf  ( )
    , mValidOf  ( )
    , mParent   (parent)
    , mTable    (tableHelper)
    , mWaitEnd  (waitEndEdit)
    , mNewText  ( )
    , mSelIndex ( )
{
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

bool TableCell::isComboWidget(int col) const
{
    if (isValidColumn(col))
    {
        for (int i = 0; i < static_cast<int>(mColumns.size()); ++i)
        {
            if (mColumns[i] == col)
            {
                return true;
            }
        }
    }

    return false;
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

    // A heterogeneous tree (e.g. Data Types) suppresses editing on cells that make no sense for
    // the row's category; when no predicate is set every valid cell stays editable.
    if (mEditable && (mEditable(index) == false))
    {
        return nullptr;
    }

    // The per-cell resolver wins over the per-column model list, so one column can be a combo for
    // some rows (struct field type) and a text editor for others (imported type, enum derived).
    QAbstractItemModel* model = mModelOf ? mModelOf(index) : columnToModel(index.column());
    if (model != nullptr)
    {
        QComboBox* combo = new QComboBox(parent);
        combo->setModel(model);
        combo->setProperty("index", index);
        // Commit ONLY on user activation (mouse pick or keyboard choose), never on
        // currentTextChanged. Programmatic setCurrentText() in setEditorData() must not commit:
        // when the controller updates the cell it emits the model's dataChanged, which reopens
        // this editor mid-commit and re-seeds it with the still-stale cell text; committing on
        // that would revert the user's choice. The line editor commits on user-only textEdited
        // for the same reason, so the combo mirrors it here.
        connect(combo, &QComboBox::activated, this, &TableCell::onComboActivated);

        return combo;
    }
    else if ( isValidColumn(index.column()) )
    {
        QLineEdit* lineEdit = new QLineEdit(parent);
        // The index must travel with the editor so the change routes back to the correct
        // row/column (a missing property yields an invalid index that is silently dropped).
        lineEdit->setProperty("index", index);
        // Forbid invalid characters directly in the table, matching the details panel. The
        // per-cell resolver wins so the same column can validate differently by row category.
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
    // Key off the actual editor type, not the per-column registration: a combo may now be
    // produced by the per-cell resolver on a column that is not in the combo-column list.
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
        // A flat list row is often shorter than a comfortable edit control; grow the editor
        // vertically (centered on the cell) and a touch wider so the text is fully visible.
        QRect rect = option.rect;
        const int minHeight = qMax(editor->sizeHint().height(), 24);
        if (rect.height() < minHeight)
        {
            rect.setTop(rect.top() - ((minHeight - rect.height()) / 2));
            rect.setHeight(minHeight);
        }
        rect.setWidth(rect.width() + 8);
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
        // Route the user's choice through the same signal the line editor uses, then dismiss the
        // drop-down. The owning controller updates the model and cell text (single source of
        // truth), so the base setModelData() is intentionally not invoked.
        emit signalEditorDataChanged(combo->property("index").toModelIndex(), combo->currentText());
        emit closeEditor(combo);
    }
}

void TableCell::onEditorTextChanged(const QString & newText)
{
    QWidget *editor = qobject_cast<QWidget *>(sender());
    if (editor != nullptr)
    {
        if (mWaitEnd == false)
        {
            emit signalEditorDataChanged(editor->property("index").toModelIndex(), newText);
        }
        else
        {
            mNewText = newText;
            mSelIndex = editor->property("index").toModelIndex();
        }
    }
}

void TableCell::onEditorTextChangeFinished()
{
    if (mWaitEnd && mSelIndex.isValid() && (mNewText.isEmpty() == false))
    {
        emit signalEditorDataChanged(mSelIndex, mNewText);
        mSelIndex = QModelIndex();
        mNewText.clear();
    }
}
