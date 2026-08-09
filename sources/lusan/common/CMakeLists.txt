list(APPEND LUSAN_SRC
    ${LUSAN}/common/DocElementTable.cpp
    ${LUSAN}/common/DocReservedNames.cpp
    ${LUSAN}/common/ElementBase.cpp
    ${LUSAN}/common/LogCollectorClient.cpp
    ${LUSAN}/common/NELusanCommon.cpp
    ${LUSAN}/common/VersionNumber.cpp
    ${LUSAN}/common/XmlSI.cpp
)

list(APPEND LUSAN_HDR
    ${LUSAN}/common/DocElementTable.hpp
    ${LUSAN}/common/DocReservedNames.hpp
    ${LUSAN}/common/ElementBase.hpp
    ${LUSAN}/common/LogCollectorClient.hpp
    ${LUSAN}/common/NELusanCommon.hpp
    ${LUSAN}/common/VersionNumber.hpp
    ${LUSAN}/common/XmlSI.hpp
    ${LUSAN}/common/XmlSM.hpp
)
