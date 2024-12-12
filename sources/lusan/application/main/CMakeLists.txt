include("${LUSAN}/application/main/ui/CMakeLists.txt")

list(APPEND LUSAN_SRC
    ${LUSAN}/application/main/LusanApplication.cpp
    ${LUSAN}/application/main/main.cpp
    ${LUSAN}/application/main/MdiChild.cpp
    ${LUSAN}/application/main/MdiMainWindow.cpp
    ${LUSAN}/application/main/Workspace.cpp
)
