include(${LUSAN}/view/log/ui/CMakeLists.txt)

list(APPEND LUSAN_SRC
    ${LUSAN}/view/log/LogTableHeader.cpp
    ${LUSAN}/view/log/LogViewer.cpp
)

list(APPEND LUSAN_HDR
    ${LUSAN}/view/log/LogTableHeader.hpp
    ${LUSAN}/view/log/LogViewer.hpp
)
