include("${LUSAN}/view/si/ui/CMakeLists.txt")

list(APPEND LUSAN_SRC
    ${LUSAN}/view/si/ServiceInterface.cpp
    ${LUSAN}/view/si/SIConstant.cpp
    ${LUSAN}/view/si/SIConstantDetails.cpp
    ${LUSAN}/view/si/SIConstantList.cpp
    ${LUSAN}/view/si/SIDataTopic.cpp
    ${LUSAN}/view/si/SIDataTopicDetails.cpp
    ${LUSAN}/view/si/SIDataTopicList.cpp
    ${LUSAN}/view/si/SIDataType.cpp
    ${LUSAN}/view/si/SIDataTypeDetails.cpp
    ${LUSAN}/view/si/SIDataTypeFieldDetails.cpp
    ${LUSAN}/view/si/SIDataTypeList.cpp
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
    ${LUSAN}/view/si/SIConstant.hpp
    ${LUSAN}/view/si/SIConstantDetails.hpp
    ${LUSAN}/view/si/SIConstantList.hpp
    ${LUSAN}/view/si/SIDataTopic.hpp
    ${LUSAN}/view/si/SIDataTopicDetails.hpp
    ${LUSAN}/view/si/SIDataTopicList.hpp
    ${LUSAN}/view/si/SIDataType.hpp
    ${LUSAN}/view/si/SIDataTypeDetails.hpp
    ${LUSAN}/view/si/SIDataTypeFieldDetails.hpp
    ${LUSAN}/view/si/SIDataTypeList.hpp
    ${LUSAN}/view/si/SIMethod.hpp
    ${LUSAN}/view/si/SIMethodDetails.hpp
    ${LUSAN}/view/si/SIMethodList.hpp
    ${LUSAN}/view/si/SIMethodParamDetails.hpp
    ${LUSAN}/view/si/SIOverview.hpp
    ${LUSAN}/view/si/SIOverviewDetails.hpp
    ${LUSAN}/view/si/SIOverviewLinks.hpp
)
