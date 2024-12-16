include("${LUSAN}/view/common//ui/CMakeLists.txt")

list(APPEND LUSAN_SRC
    ${LUSAN}/view/common/DirSelectionDialog.cpp
    ${LUSAN}/view/common/MdiArea.cpp
    ${LUSAN}/view/common/MdiChild.cpp
    ${LUSAN}/view/common/MdiMainWindow.cpp
    ${LUSAN}/view/common/NaviFileSystem.cpp
    ${LUSAN}/view/common/Navigation.cpp
    ${LUSAN}/view/common/Workspace.cpp
)

list(APPEND LUSAN_HDR
    ${LUSAN}/view/common/DirSelectionDialog.hpp
    ${LUSAN}/view/common/MdiArea.hpp
    ${LUSAN}/view/common/MdiChild.hpp
    ${LUSAN}/view/common/MdiMainWindow.hpp
    ${LUSAN}/view/common/NaviFileSystem.hpp
    ${LUSAN}/view/common/Navigation.hpp
    ${LUSAN}/view/common/Workspace.hpp
)
