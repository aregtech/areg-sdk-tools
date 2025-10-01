include(${LUSAN}/view/log/ui/CMakeLists.txt)

list(APPEND LUSAN_SRC
    ${LUSAN}/view/log/LiveLogViewer.cpp
    ${LUSAN}/view/log/LogFilterWidgets.cpp
    ${LUSAN}/view/log/LogHeaderItem.cpp
    ${LUSAN}/view/log/LogTableHeader.cpp
    ${LUSAN}/view/log/LogTextHighlight.cpp
    ${LUSAN}/view/log/LogViewerBase.cpp
    ${LUSAN}/view/log/OfflineLogViewer.cpp
    ${LUSAN}/view/log/ScopeOutputViewer.cpp
)

list(APPEND LUSAN_HDR
    ${LUSAN}/view/log/LiveLogViewer.hpp
    ${LUSAN}/view/log/LogFilterWidgets.hpp
    ${LUSAN}/view/log/LogHeaderItem.hpp
    ${LUSAN}/view/log/LogTableHeader.hpp
    ${LUSAN}/view/log/LogTextHighlight.hpp
    ${LUSAN}/view/log/LogViewerBase.hpp
    ${LUSAN}/view/log/OfflineLogViewer.hpp
    ${LUSAN}/view/log/ScopeOutputViewer.hpp
)
