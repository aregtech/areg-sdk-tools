include("${LUSAN}/view/si/ui/CMakeLists.txt")

list(APPEND LUSAN_SRC
    ${LUSAN}/view/si/ServiceInterface.cpp
    ${LUSAN}/view/si/SIAttribute.cpp
    ${LUSAN}/view/si/SIAttributeDetails.cpp
    ${LUSAN}/view/si/SIAttributeList.cpp
    ${LUSAN}/view/si/SIConstant.cpp
    ${LUSAN}/view/si/SIDataType.cpp
    ${LUSAN}/view/si/SIInclude.cpp
    ${LUSAN}/view/si/SIIncludeDetails.cpp
    ${LUSAN}/view/si/SIIncludeList.cpp
    ${LUSAN}/view/si/SIMethod.cpp
    ${LUSAN}/view/si/SIMethodDetails.cpp
    ${LUSAN}/view/si/SIMethodList.cpp
    ${LUSAN}/view/si/SIMethodParamDetails.cpp
    ${LUSAN}/view/si/SIOverview.cpp
    ${LUSAN}/view/si/SIOverviewDetails.cpp
    ${LUSAN}/view/si/SIOverviewLinks.cpp
)

list(APPEND LUSAN_HDR
    ${LUSAN}/view/si/ServiceInterface.hpp
    ${LUSAN}/view/si/SIAttribute.hpp
    ${LUSAN}/view/si/SIAttributeDetails.hpp
    ${LUSAN}/view/si/SIAttributeList.hpp
    ${LUSAN}/view/si/SIConstant.hpp
    ${LUSAN}/view/si/SIDataType.hpp
    ${LUSAN}/view/si/SIInclude.hpp
    ${LUSAN}/view/si/SIIncludeDetails.hpp
    ${LUSAN}/view/si/SIIncludeList.hpp
    ${LUSAN}/view/si/SIMethod.hpp
    ${LUSAN}/view/si/SIMethodDetails.hpp
    ${LUSAN}/view/si/SIMethodList.hpp
    ${LUSAN}/view/si/SIMethodParamDetails.hpp
    ${LUSAN}/view/si/SIOverview.hpp
    ${LUSAN}/view/si/SIOverviewDetails.hpp
    ${LUSAN}/view/si/SIOverviewLinks.hpp
)
