#ifndef PROJECTSETTINGS_H
#define PROJECTSETTINGS_H

#include "ProjectDirSettings.hpp"
#include <QDialog>
#include <QStackedWidget>
#include <QStringListModel>

namespace Ui {
class ProjectSettingsDlg;
}

class QAbstractButton;


class ProjectSettings : public QDialog
{
    Q_OBJECT

public:
    explicit ProjectSettings(QWidget *parent = nullptr);
    virtual ~ProjectSettings() override;

    ProjectSettings(ProjectSettings const&) = delete;
    ProjectSettings& operator=(ProjectSettings const&) = delete;

private slots:
    void settingsListSelectionChanged(QModelIndex const&);
    void buttonClicked(QAbstractButton*);
private:
    void setupDialog();
    void connectSignals() const;
    void addSettings();
    void selectSetting(int index);

    Ui::ProjectSettingsDlg *ui;
    QStackedWidget* settingsStackedWidget = new QStackedWidget{this};
    ProjectDirSettings* mDirSettings{new ProjectDirSettings(this)};
    QStringListModel model{this};
};

#endif // PROJECTSETTINGS_H
