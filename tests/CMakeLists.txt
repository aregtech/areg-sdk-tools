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

# A console test, not a windowed app - keep stdout for the harness.
set_target_properties(lusan_sm_tests PROPERTIES WIN32_EXECUTABLE OFF)

# SM-02: deterministic XML round-trip + parse robustness. Reuses the same headless
# support sources; the reference documents live next to the spec in .claude/.
qt_add_executable(lusan_sm_serialization_tests
    ${LUSAN_TEST_SUPPORT_SRC}
    ${LUSAN_ROOT}/tests/sm/SMSerializationTests.cpp
)

target_include_directories(lusan_sm_serialization_tests PRIVATE ${LUSAN_BASE} ${LUSAN_THIRDPARTY})
target_compile_definitions(lusan_sm_serialization_tests PRIVATE
    ${COMMON_COMPILE_DEF}
    IMP_LOGGER_DLL
    LUSAN_TEST_DATA_DIR="${LUSAN_ROOT}/.claude"
)
target_link_libraries(lusan_sm_serialization_tests PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    areg::areg
    areg::aregextend
    areg::areglogger
    aregsqlite3
)
set_target_properties(lusan_sm_serialization_tests PROPERTIES WIN32_EXECUTABLE OFF)

# ---------------------------------------------------------------------------
# SM-04: undo/redo command framework (FSM) + the same shared framework applied to
# the Service Interface editor. The command layer lives in model/common (shared),
# model/sm (FSM specifics) and model/si (SI specifics); each test links the headless
# data layer of its editor plus that shared command framework.
# ---------------------------------------------------------------------------

set(LUSAN_CMD_COMMON_SRC
    ${LUSAN}/model/common/DocCommand.cpp
    ${LUSAN}/model/common/DocModelNotifier.cpp
)

qt_add_executable(lusan_sm_command_tests
    ${LUSAN_TEST_SUPPORT_SRC}
    ${LUSAN_CMD_COMMON_SRC}
    ${LUSAN}/model/sm/SMCommand.cpp
    ${LUSAN}/model/sm/SMLayoutCommands.cpp
    ${LUSAN}/model/sm/SMStateCommands.cpp
    ${LUSAN}/model/sm/SMOverviewModel.cpp
    ${LUSAN}/model/sm/SMDataTypeModel.cpp
    ${LUSAN}/model/sm/SMAttributeModel.cpp
    ${LUSAN}/model/sm/SMEventModel.cpp
    ${LUSAN}/model/sm/SMTimerModel.cpp
    ${LUSAN}/model/sm/SMMethodModel.cpp
    ${LUSAN}/model/sm/SMConstantModel.cpp
    ${LUSAN}/model/sm/SMIncludeModel.cpp
    ${LUSAN}/model/sm/SMSelectionModel.cpp
    ${LUSAN}/model/sm/StateMachineModel.cpp
    ${LUSAN_ROOT}/tests/sm/SMCommandTests.cpp
    # Container tests need the real basic-container catalog (Array/HashMap/Map/...): it is
    # loaded from the "Predefined Types" resource at runtime (DataTypeFactory::_initPredefined),
    # not hardcoded, so the .qrc must be compiled in or DataTypeContainer::canHaveKey() sees an
    # empty catalog and every container looks non-keyed.
    ${LUSAN}/res/lusan.qrc
)
target_include_directories(lusan_sm_command_tests PRIVATE ${LUSAN_BASE} ${LUSAN_THIRDPARTY})
target_compile_definitions(lusan_sm_command_tests PRIVATE ${COMMON_COMPILE_DEF} IMP_LOGGER_DLL)
target_link_libraries(lusan_sm_command_tests PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    areg::areg
    areg::aregextend
    areg::areglogger
    aregsqlite3
)
set_target_properties(lusan_sm_command_tests PROPERTIES WIN32_EXECUTABLE OFF)

# ---------------------------------------------------------------------------
# SM-13: design-canvas smoke test. Drives the real SMDesign/SMScene/SMStateItem
# stack with synthetic mouse/key events on the Qt offscreen platform, so the
# canvas interactions are verifiable without a display or input injection.
# ---------------------------------------------------------------------------
qt_add_executable(lusan_sm_canvas_tests
    ${LUSAN_TEST_SUPPORT_SRC}
    ${LUSAN_CMD_COMMON_SRC}
    ${LUSAN}/model/sm/SMCommand.cpp
    ${LUSAN}/model/sm/SMLayoutCommands.cpp
    ${LUSAN}/model/sm/SMStateCommands.cpp
    ${LUSAN}/model/sm/SMTransitionCommands.cpp
    ${LUSAN}/model/sm/SMOverviewModel.cpp
    ${LUSAN}/model/sm/SMDataTypeModel.cpp
    ${LUSAN}/model/sm/SMAttributeModel.cpp
    ${LUSAN}/model/sm/SMEventModel.cpp
    ${LUSAN}/model/sm/SMTimerModel.cpp
    ${LUSAN}/model/sm/SMMethodModel.cpp
    ${LUSAN}/model/sm/SMConstantModel.cpp
    ${LUSAN}/model/sm/SMIncludeModel.cpp
    ${LUSAN}/model/sm/SMSelectionModel.cpp
    ${LUSAN}/model/sm/StateMachineModel.cpp
    ${LUSAN}/view/sm/NESMDesign.cpp
    ${LUSAN}/view/sm/SMAutoPlacer.cpp
    ${LUSAN}/view/sm/SMCanvasItem.cpp
    ${LUSAN}/view/sm/SMCanvasTool.cpp
    ${LUSAN}/view/sm/SMDesign.cpp
    ${LUSAN}/view/sm/SMEdgeItem.cpp
    ${LUSAN}/view/sm/SMGraphicsView.cpp
    ${LUSAN}/view/sm/SMNoteEditor.cpp
    ${LUSAN}/view/sm/SMNoteItem.cpp
    ${LUSAN}/view/sm/SMScene.cpp
    ${LUSAN}/view/sm/SMSceneManager.cpp
    ${LUSAN}/view/sm/SMStateItem.cpp
    ${LUSAN}/view/sm/SMToolIcons.cpp
    ${LUSAN_ROOT}/tests/sm/SMCanvasSmokeTests.cpp
    ${LUSAN}/res/lusan.qrc
)
target_include_directories(lusan_sm_canvas_tests PRIVATE ${LUSAN_BASE} ${LUSAN_THIRDPARTY})
target_compile_definitions(lusan_sm_canvas_tests PRIVATE ${COMMON_COMPILE_DEF} IMP_LOGGER_DLL)
target_link_libraries(lusan_sm_canvas_tests PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    areg::areg
    areg::aregextend
    areg::areglogger
    aregsqlite3
)
set_target_properties(lusan_sm_canvas_tests PROPERTIES WIN32_EXECUTABLE OFF)

# The Service Interface headless data layer.
set(LUSAN_SI_SUPPORT_SRC
    ${LUSAN}/common/ElementBase.cpp
    ${LUSAN}/common/VersionNumber.cpp
    ${LUSAN}/common/NELusanCommon.cpp
    ${LUSAN}/common/XmlSI.cpp
    ${LUSAN}/data/common/DocumentElem.cpp
    ${LUSAN}/data/common/ParamBase.cpp
    ${LUSAN}/data/common/ParamType.cpp
    ${LUSAN}/data/common/MethodBase.cpp
    ${LUSAN}/data/common/MethodParameter.cpp
    ${LUSAN}/data/common/AttributeEntry.cpp
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
    ${LUSAN}/data/si/SIAttributeData.cpp
    ${LUSAN}/data/si/SIConstantData.cpp
    ${LUSAN}/data/si/SIDataTypeData.cpp
    ${LUSAN}/data/si/SIIncludeData.cpp
    ${LUSAN}/data/si/SIMethodBase.cpp
    ${LUSAN}/data/si/SIMethodBroadcast.cpp
    ${LUSAN}/data/si/SIMethodData.cpp
    ${LUSAN}/data/si/SIMethodRequest.cpp
    ${LUSAN}/data/si/SIMethodResponse.cpp
    ${LUSAN}/data/si/SIOverviewData.cpp
    ${LUSAN}/data/si/SIResponseLink.cpp
    ${LUSAN}/data/si/ServiceInterfaceData.cpp
)

qt_add_executable(lusan_si_command_tests
    ${LUSAN_SI_SUPPORT_SRC}
    ${LUSAN_CMD_COMMON_SRC}
    ${LUSAN}/model/si/SICommand.cpp
    ${LUSAN_ROOT}/tests/si/SICommandTests.cpp
)
target_include_directories(lusan_si_command_tests PRIVATE ${LUSAN_BASE} ${LUSAN_THIRDPARTY})
target_compile_definitions(lusan_si_command_tests PRIVATE ${COMMON_COMPILE_DEF} IMP_LOGGER_DLL)
target_link_libraries(lusan_si_command_tests PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    areg::areg
    areg::aregextend
    areg::areglogger
    aregsqlite3
)
set_target_properties(lusan_si_command_tests PROPERTIES WIN32_EXECUTABLE OFF)

enable_testing()
add_test(NAME sm_model_tests COMMAND lusan_sm_tests)
add_test(NAME sm_serialization_tests COMMAND lusan_sm_serialization_tests)
add_test(NAME sm_command_tests COMMAND lusan_sm_command_tests)
add_test(NAME si_command_tests COMMAND lusan_si_command_tests)
