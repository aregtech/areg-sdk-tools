#ifndef WORKSPACEMANAGER_H
#define WORKSPACEMANAGER_H

#include <QWidget>
#include <QString>
#include <unordered_map>
#include <cstdint>
#include <optional>

namespace Ui {
class workspaceManager;
}
class WorkspaceEntry;


class WorkspaceManager : public QWidget
{
    Q_OBJECT

public:
    explicit WorkspaceManager(QWidget *parent = nullptr);
    virtual ~WorkspaceManager() override;

    WorkspaceManager(WorkspaceManager const&) = delete;
    WorkspaceManager& operator=(WorkspaceManager const&) = delete;

    void applyChanges();

private slots:
    void handleDeleteButtonClicked();
    void handleWorkspaceSelectionChanged() const;

private:
    void connectSignalHandlers() const;
    void initialisePathsWithSelectedWorkspaceData(uint32_t workspaceId) const;
    void populateListOfWorkspaces() const;
    void setupUi() const;
    std::optional<WorkspaceEntry> GetWorkspace(uint32_t workspaceId) const;
    void handleWorkspaceDescChanged();
    std::optional<uint32_t> getSelectedWorskpaceId() const;
    void selectWorkspace(int index) const;

    struct WorkspaceChangeData
    {
        bool hasDeleted = false;
        std::optional<QString> newDescription;
    };

    using WorkspaceId = uint32_t;

    Ui::workspaceManager* ui{};
    std::unordered_map<WorkspaceId, WorkspaceChangeData> mModifiedWorkspaces;
};

#endif // WORKSPACEMANAGER_H
