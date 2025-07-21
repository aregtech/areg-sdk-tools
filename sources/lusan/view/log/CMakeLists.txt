include(${LUSAN}/view/log/ui/CMakeLists.txt)

list(APPEND LUSAN_SRC
    ${LUSAN}/view/log/LogHeaderItem.cpp
    ${LUSAN}/view/log/LogTableHeader.cpp
    ${LUSAN}/view/log/LiveLogViewer.cpp
    ${LUSAN}/view/log/LogViewerBase.cpp
    ${LUSAN}/view/log/OfflineLogViewer.cpp
)

list(APPEND LUSAN_HDR
    ${LUSAN}/view/log/LogHeaderItem.hpp
    ${LUSAN}/view/log/LogTableHeader.hpp
    ${LUSAN}/view/log/LiveLogViewer.hpp
    ${LUSAN}/view/log/LogViewerBase.hpp
    ${LUSAN}/view/log/OfflineLogViewer.hpp
)
