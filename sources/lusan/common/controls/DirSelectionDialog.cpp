#include "lusan/common/controls/DirSelectionDialog.hpp"

#include <QLabel>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QTreeView>
#include <QFileSystemModel>
#include <QPushButton>
#include <QLineEdit>

DirSelectionDialog::DirSelectionDialog(QWidget* parent /*= nullptr*/)
    : QDialog(parent)

    , mTreeViewDirs(nullptr)
    , mModel(nullptr)
    , mDirName(nullptr)
    , mButtonOK(nullptr)
{
    _initialize(QDir::homePath());
}

DirSelectionDialog::DirSelectionDialog(const QString & curDir, QWidget* parent /*= nullptr*/)
    : QDialog       (parent)

    , mTreeViewDirs ( nullptr )
    , mModel        ( nullptr )
    , mDirName      ( nullptr )
    , mButtonOK     ( nullptr )
{
    _initialize(curDir);
}

void DirSelectionDialog::_initialize(const QString & curDir)
{
    setMinimumSize(200, 300);
    resize(400, 430);
    mModel = new QFileSystemModel(this);
    mTreeViewDirs = new QTreeView(this);
    mDirName = new QLineEdit(this);
    auto mainLayout = new QVBoxLayout();
    auto pathLayout = new QHBoxLayout();
    auto label = new QLabel(tr("Folder:"));
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    mButtonOK = buttonBox->button(QDialogButtonBox::Ok);
    
    mModel->setFilter(QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot);
    QFileSystemModel::Options opt = mModel->options();
    opt &= ~QFileSystemModel::Option::DontUseCustomDirectoryIcons;
    mModel->setOptions(opt);
    QModelIndex rootIdx;
    if (curDir.isEmpty() == false)
    {
        rootIdx = mModel->setRootPath(curDir);
    }

    mTreeViewDirs->setModel(mModel);
    mTreeViewDirs->setSelectionMode(QAbstractItemView::SingleSelection);
    mTreeViewDirs->setHeaderHidden(true);
    mTreeViewDirs->setSortingEnabled(true);
    mTreeViewDirs->sortByColumn(0, Qt::AscendingOrder);
    for (int i = 1; i < mModel->columnCount(); i++)
    {
        // hide not required colums
        mTreeViewDirs->setColumnHidden(i, true);
    }
    
    if (rootIdx.isValid())
    {
        mTreeViewDirs->scrollTo(rootIdx);
        mTreeViewDirs->selectionModel()->setCurrentIndex(rootIdx, QItemSelectionModel::Current | QItemSelectionModel::Select);
    }
    
    connect(mTreeViewDirs->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DirSelectionDialog::onCurrentDirChanged);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &DirSelectionDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &DirSelectionDialog::reject);

    mDirName->setReadOnly(true);
    mDirName->setText(QFileInfo(curDir).fileName());
    
    pathLayout->addWidget(label);
    pathLayout->addSpacing(10);
    pathLayout->addWidget(mDirName);

    mainLayout->addWidget(mTreeViewDirs);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(pathLayout);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
}

void DirSelectionDialog::onCurrentDirChanged()
{
    auto fileInfo = mModel->fileInfo(mTreeViewDirs->selectionModel()->currentIndex());
    mDirName->setText(fileInfo.fileName());
    mButtonOK->setEnabled(fileInfo.isDir());
    mButtonOK->setDefault(fileInfo.isDir());
}

QDir DirSelectionDialog::getDirectory() const
{
    return QDir(mModel->fileInfo(mTreeViewDirs->selectionModel()->currentIndex()).absoluteFilePath());
}
