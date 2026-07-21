include("${LUSAN}/view/si/ui/CMakeLists.txt")

list(APPEND LUSAN_SRC
    ${LUSAN}/view/si/ServiceInterface.cpp
    ${LUSAN}/view/si/SIAttribute.cpp
    ${LUSAN}/view/si/SIConstant.cpp
    ${LUSAN}/view/si/SIDataType.cpp
    ${LUSAN}/view/si/SIInclude.cpp
    ${LUSAN}/view/si/SIMethod.cpp
    ${LUSAN}/view/si/SIOverview.cpp
    ${LUSAN}/view/si/SIOverviewDetails.cpp
    ${LUSAN}/view/si/SIOverviewLinks.cpp
)

list(APPEND LUSAN_HDR
    ${LUSAN}/view/si/ServiceInterface.hpp
    ${LUSAN}/view/si/SIAttribute.hpp
    ${LUSAN}/view/si/SIConstant.hpp
    ${LUSAN}/view/si/SIDataType.hpp
    ${LUSAN}/view/si/SIInclude.hpp
    ${LUSAN}/view/si/SIMethod.hpp
    ${LUSAN}/view/si/SIOverview.hpp
    ${LUSAN}/view/si/SIOverviewDetails.hpp
    ${LUSAN}/view/si/SIOverviewLinks.hpp
)
