# ---------------------------------------------------------------------------
# Lusan unit tests (opt-in: -DLUSAN_BUILD_TESTS=ON).
#
# This is additive and OFF by default, so the normal `lusan` application build
# is unchanged. The SM-01 test builds a small, self-contained console executable
# from the headless data layer (common + data/common + data/sm) plus the test
# driver; it links the areg targets only to inherit their include directories
# (the data layer uses areg headers for platform macros, no linked symbols).
# ---------------------------------------------------------------------------

set(LUSAN_TEST_SUPPORT_SRC
    # common
    ${LUSAN}/common/ElementBase.cpp
    ${LUSAN}/common/VersionNumber.cpp
    ${LUSAN}/common/NELusanCommon.cpp
    ${LUSAN}/common/XmlSI.cpp
    # data/common (reused by the FSM model)
    ${LUSAN}/data/common/DocumentElem.cpp
    ${LUSAN}/data/common/ParamBase.cpp
    ${LUSAN}/data/common/ParamType.cpp
    ${LUSAN}/data/common/MethodBase.cpp
    ${LUSAN}/data/common/MethodParameter.cpp
    ${LUSAN}/data/common/ConstantEntry.cpp
    ${LUSAN}/data/common/IncludeEntry.cpp
    ${LUSAN}/data/common/DataTypeBase.cpp
    ${LUSAN}/data/common/DataTypeCustom.cpp
    ${LUSAN}/data/common/DataTypeEnum.cpp
    ${LUSAN}/data/common/DataTypeStructure.cpp
    ${LUSAN}/data/common/DataTypeImported.cpp
    ${LUSAN}/data/common/DataTypePrimitive.cpp
    ${LUSAN}/data/common/DataTypeBasic.cpp
    ${LUSAN}/data/common/DataTypeContainer.cpp
    ${LUSAN}/data/common/DataTypeEmpty.cpp
    ${LUSAN}/data/common/DataTypeFactory.cpp
    ${LUSAN}/data/common/EnumEntry.cpp
    ${LUSAN}/data/common/FieldEntry.cpp
    # data/sm (the model under test)
    ${LUSAN}/data/sm/SMAttributeData.cpp
    ${LUSAN}/data/sm/SMCondition.cpp
    ${LUSAN}/data/sm/SMConstantData.cpp
    ${LUSAN}/data/sm/SMDataTypeData.cpp
    ${LUSAN}/data/sm/SMEventData.cpp
    ${LUSAN}/data/sm/SMImportData.cpp
    ${LUSAN}/data/sm/SMIncludeData.cpp
    ${LUSAN}/data/sm/SMLayoutData.cpp
    ${LUSAN}/data/sm/SMMethodData.cpp
    ${LUSAN}/data/sm/SMOperation.cpp
    ${LUSAN}/data/sm/SMOverviewData.cpp
    ${LUSAN}/data/sm/SMState.cpp
    ${LUSAN}/data/sm/SMTimerData.cpp
    ${LUSAN}/data/sm/SMTransition.cpp
    ${LUSAN}/data/sm/StateMachineData.cpp
)

qt_add_executable(lusan_sm_tests
    ${LUSAN_TEST_SUPPORT_SRC}
    ${LUSAN_ROOT}/tests/sm/SMModelTests.cpp
)

target_include_directories(lusan_sm_tests PRIVATE ${LUSAN_BASE} ${LUSAN_THIRDPARTY})
target_compile_definitions(lusan_sm_tests PRIVATE ${COMMON_COMPILE_DEF} IMP_LOGGER_DLL)
target_link_libraries(lusan_sm_tests PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    areg::areg
    areg::aregextend
    areg::areglogger
    aregsqlite3
)

# A console test, not a windowed app — keep stdout for the harness.
set_target_properties(lusan_sm_tests PROPERTIES WIN32_EXECUTABLE OFF)

enable_testing()
add_test(NAME sm_model_tests COMMAND lusan_sm_tests)
