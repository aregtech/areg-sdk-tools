include("${LUSAN}/view/common//ui/CMakeLists.txt")

list(APPEND LUSAN_SRC
    ${LUSAN}/view/common/DirSelectionDialog.cpp
    ${LUSAN}/view/common/IEDataTypeConsumer.cpp
    ${LUSAN}/view/common/LogExplorer.cpp
    ${LUSAN}/view/common/MdiArea.cpp
    ${LUSAN}/view/common/MdiChild.cpp
    ${LUSAN}/view/common/MdiMainWindow.cpp
    ${LUSAN}/view/common/NaviFileSystem.cpp
    ${LUSAN}/view/common/Navigation.cpp
    ${LUSAN}/view/common/NavigationWindow.cpp
    ${LUSAN}/view/common/TableCell.cpp
    ${LUSAN}/view/common/Workspace.cpp
    ${LUSAN}/view/common/WorkspaceFileDialog.cpp
    ${LUSAN}/view/common/ProjectDirSettings.cpp
    ${LUSAN}/view/common/ProjectSettings.cpp
    ${LUSAN}/view/common/WorkspaceManager.cpp
)

list(APPEND LUSAN_HDR
    ${LUSAN}/view/common/DirSelectionDialog.hpp
    ${LUSAN}/view/common/IEDataTypeConsumer.hpp
    ${LUSAN}/view/common/LogExplorer.hpp
    ${LUSAN}/view/common/MdiArea.hpp
    ${LUSAN}/view/common/MdiChild.hpp
    ${LUSAN}/view/common/MdiMainWindow.hpp
    ${LUSAN}/view/common/NaviFileSystem.hpp
    ${LUSAN}/view/common/Navigation.hpp
    ${LUSAN}/view/common/NavigationWindow.hpp
    ${LUSAN}/view/common/TableCell.hpp
    ${LUSAN}/view/common/Workspace.hpp
    ${LUSAN}/view/common/WorkspaceFileDialog.hpp
    ${LUSAN}/view/common/ProjectDirSettings.hpp
    ${LUSAN}/view/common/ProjectSettings.hpp
    ${LUSAN}/view/common/WorkspaceManager.hpp
)
