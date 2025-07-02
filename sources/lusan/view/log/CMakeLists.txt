include(${LUSAN}/view/log/ui/CMakeLists.txt)

list(APPEND LUSAN_SRC
    ${LUSAN}/view/log/LogHeaderItem.cpp
    ${LUSAN}/view/log/LogTableHeader.cpp
    ${LUSAN}/view/log/LogViewer.cpp
    ${LUSAN}/view/log/LogOfflineViewer.cpp
)

list(APPEND LUSAN_HDR
    ${LUSAN}/view/log/LogHeaderItem.hpp
    ${LUSAN}/view/log/LogTableHeader.hpp
    ${LUSAN}/view/log/LogViewer.hpp
    ${LUSAN}/view/log/LogOfflineViewer.hpp
)
