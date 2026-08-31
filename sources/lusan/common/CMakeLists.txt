list(APPEND LUSAN_SRC
    ${LUSAN}/common/DocElementTable.cpp
    ${LUSAN}/common/DocReservedNames.cpp
    ${LUSAN}/common/DocSchemaReader.cpp
    ${LUSAN}/common/ElementBase.cpp
    ${LUSAN}/common/LogCollectorClient.cpp
    ${LUSAN}/common/NELogPalette.cpp
    ${LUSAN}/common/NELusanCommon.cpp
    ${LUSAN}/common/NETimeUnits.cpp
    ${LUSAN}/common/VersionNumber.cpp
    ${LUSAN}/common/XmlSI.cpp
)

list(APPEND LUSAN_HDR
    ${LUSAN}/common/DocElementTable.hpp
    ${LUSAN}/common/DocReservedNames.hpp
    ${LUSAN}/common/DocSchemaReader.hpp
    ${LUSAN}/common/ElementBase.hpp
    ${LUSAN}/common/LogCollectorClient.hpp
    ${LUSAN}/common/NELogPalette.hpp
    ${LUSAN}/common/NELusanCommon.hpp
    ${LUSAN}/common/NETimeUnits.hpp
    ${LUSAN}/common/VersionNumber.hpp
    ${LUSAN}/common/XmlSI.hpp
    ${LUSAN}/common/XmlSM.hpp
)
