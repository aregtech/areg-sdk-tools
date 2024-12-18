include("${LUSAN}/view/si/ui/CMakeLists.txt")

list(APPEND LUSAN_SRC
    ${LUSAN}/view/si/ServiceInterface.cpp
    ${LUSAN}/view/si/SIDataType.cpp
    ${LUSAN}/view/si/SIDataTypeDetails.cpp
    ${LUSAN}/view/si/SIDataTypeList.cpp
    ${LUSAN}/view/si/SIOverview.cpp
)

list(APPEND LUSAN_HDR
    ${LUSAN}/view/si/ServiceInterface.hpp
    ${LUSAN}/view/si/SIDataType.hpp
    ${LUSAN}/view/si/SIDataTypeDetails.hpp
    ${LUSAN}/view/si/SIDataTypeList.hpp
    ${LUSAN}/view/si/SIOverview.hpp
)
