include("${LUSAN}/view/si/ui/CMakeLists.txt")

list(APPEND LUSAN_SRC
    ${LUSAN}/view/si/ServiceInterface.cpp
    ${LUSAN}/view/si/SIOverview.cpp
    ${LUSAN}/view/si/SIOverviewDetails.cpp
    ${LUSAN}/view/si/SIOverviewLinks.cpp
)

list(APPEND LUSAN_HDR
    ${LUSAN}/view/si/ServiceInterface.hpp
    ${LUSAN}/view/si/SIOverview.hpp
    ${LUSAN}/view/si/SIOverviewDetails.hpp
    ${LUSAN}/view/si/SIOverviewLinks.hpp
)
