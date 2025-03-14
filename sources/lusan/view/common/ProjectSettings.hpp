#ifndef PROJECTSETTINGS_H
#define PROJECTSETTINGS_H

#include <QDialog>
#include <QStackedWidget>
#include <QStringListModel>

namespace Ui {
class ProjectSettingsDlg;
}


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

private:
    void setupDialog();
    void connectSignals() const;
    void addSettings();
    void selectSetting(int index);

    Ui::ProjectSettingsDlg *ui;
    QStackedWidget settingsStackedWidget{this};
    QStringListModel model{this};
};

#endif // PROJECTSETTINGS_H
