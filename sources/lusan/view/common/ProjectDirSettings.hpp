#ifndef PROJECTDIRSETTINGS_H
#define PROJECTDIRSETTINGS_H

#include <QWidget>
#include <QString>

namespace Ui {
class projectDirSettingsDlg;
}

class ProjectDirSettings : public QWidget
{
    Q_OBJECT

public:
    explicit ProjectDirSettings(QWidget *parent = nullptr);
    virtual ~ProjectDirSettings() override;

    ProjectDirSettings(ProjectDirSettings const&) = delete;
    ProjectDirSettings& operator=(ProjectDirSettings const&) = delete;

    QString getRootDirectory() const;
    QString getSourceDirectory() const;
    QString getIncludeDirectory() const;
    QString getDeliveryDirectory() const;
    QString getLogDirectory() const;
    QString getWorkspaceDescription() const;

private slots:
    void onRootDirBrowseBtnClicked();
    void onSourceDirBrowseBtnClicked();
    void onIncludeDirBrowseBtnClicked();
    void onDeliveryDirBrowseBtnClicked();
    void onLogDirBrowseBtnClicked();

private:
    void connectSignalHandlers() const;
    void initialisePathsWithCurrentWorkspaceData() const;

    Ui::projectDirSettingsDlg* ui{};
};

#endif // PROJECTDIRSETTINGS_H
