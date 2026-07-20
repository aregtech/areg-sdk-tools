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
    ${LUSAN}/data/sm/SMClipboard.cpp
    ${LUSAN}/data/sm/SMCondition.cpp
    ${LUSAN}/data/sm/SMConstantData.cpp
    ${LUSAN}/data/sm/SMDataTypeData.cpp
    ${LUSAN}/data/sm/SMEventData.cpp
    ${LUSAN}/data/sm/SMGuardTree.cpp
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
    # StateMachineModel's load path applies the legacy->draft guard shim (SM-21 U4).
    ${LUSAN}/model/sm/SMConditionText.cpp
    ${LUSAN}/model/sm/SMGuardParser.cpp
    ${LUSAN}/model/sm/SMGuardSymbols.cpp
    ${LUSAN}/model/sm/SMLayoutCommands.cpp
    ${LUSAN}/model/sm/SMPasteCommand.cpp
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
# SM-22/SM-21-01: the shared argument-mapping table. Drives the REAL SMArgMapTable widget offscreen
# over an ActionCall's argument list -- source-kind legality, compatibility + widening,
# resolution order, and the undoable byte-exact round-trip. Reuses the command-tests link
# set (the full model facade) plus the mapping helpers and the widget under test.
# ---------------------------------------------------------------------------
qt_add_executable(lusan_sm_mapping_tests
    ${LUSAN_TEST_SUPPORT_SRC}
    ${LUSAN_CMD_COMMON_SRC}
    ${LUSAN}/model/sm/SMCommand.cpp
    ${LUSAN}/model/sm/SMConditionText.cpp
    ${LUSAN}/model/sm/SMGuardParser.cpp
    ${LUSAN}/model/sm/SMGuardSymbols.cpp
    ${LUSAN}/model/sm/SMLayoutCommands.cpp
    ${LUSAN}/model/sm/SMPasteCommand.cpp
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
    ${LUSAN}/model/sm/SMArgumentCommands.cpp
    ${LUSAN}/model/sm/SMMappingSources.cpp
    ${LUSAN}/model/sm/SMTypeCompat.cpp
    ${LUSAN}/model/sm/SMLiteralValidator.cpp
    ${LUSAN}/view/sm/NEGuardStyle.cpp
    ${LUSAN}/view/sm/SMArgMapTable.cpp
    ${LUSAN}/view/sm/SMArgSinkOperation.cpp
    ${LUSAN_ROOT}/tests/sm/SMArgMapTableTests.cpp
    ${LUSAN}/res/lusan.qrc
)
target_include_directories(lusan_sm_mapping_tests PRIVATE ${LUSAN_BASE} ${LUSAN_THIRDPARTY})
target_compile_definitions(lusan_sm_mapping_tests PRIVATE ${COMMON_COMPILE_DEF} IMP_LOGGER_DLL)
target_link_libraries(lusan_sm_mapping_tests PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    areg::areg
    areg::aregextend
    areg::areglogger
    aregsqlite3
)
set_target_properties(lusan_sm_mapping_tests PROPERTIES WIN32_EXECUTABLE OFF)

# ---------------------------------------------------------------------------
# SM-21 (U1): the deterministic guard core. Headless: the data layer + shared command
# framework + the guard model (tree/parser/render/codegen preview/commands + symbols)
# and the legacy condition renderer used by the read-shim.
# ---------------------------------------------------------------------------
qt_add_executable(lusan_sm_guard_tests
    ${LUSAN_TEST_SUPPORT_SRC}
    ${LUSAN_CMD_COMMON_SRC}
    ${LUSAN}/model/sm/SMCommand.cpp
    ${LUSAN}/model/sm/SMConditionText.cpp
    ${LUSAN}/model/sm/SMGuardCodegenPreview.cpp
    ${LUSAN}/model/sm/SMGuardCommands.cpp
    ${LUSAN}/model/sm/SMGuardParser.cpp
    ${LUSAN}/model/sm/SMGuardRender.cpp
    ${LUSAN}/model/sm/SMGuardSymbols.cpp
    ${LUSAN_ROOT}/tests/sm/SMGuardTests.cpp
)
target_include_directories(lusan_sm_guard_tests PRIVATE ${LUSAN_BASE} ${LUSAN_THIRDPARTY})
target_compile_definitions(lusan_sm_guard_tests PRIVATE ${COMMON_COMPILE_DEF} IMP_LOGGER_DLL)
target_link_libraries(lusan_sm_guard_tests PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    areg::areg
    areg::aregextend
    areg::areglogger
    aregsqlite3
)
set_target_properties(lusan_sm_guard_tests PROPERTIES WIN32_EXECUTABLE OFF)

# ---------------------------------------------------------------------------
# SM-21 (U3): the promotion ladder composites, grid<->tree edit semantics, node
# spans, type ranking, guard where-used, and the island round-trip. Headless: the
# data layer + shared command framework + the guard model and the ladder.
# ---------------------------------------------------------------------------
qt_add_executable(lusan_sm_guard_ladder_tests
    ${LUSAN_TEST_SUPPORT_SRC}
    ${LUSAN_CMD_COMMON_SRC}
    ${LUSAN}/model/sm/SMCommand.cpp
    ${LUSAN}/model/sm/SMConditionText.cpp
    ${LUSAN}/model/sm/SMGuardCodegenPreview.cpp
    ${LUSAN}/model/sm/SMGuardCommands.cpp
    ${LUSAN}/model/sm/SMGuardLadder.cpp
    ${LUSAN}/model/sm/SMGuardParser.cpp
    ${LUSAN}/model/sm/SMGuardRender.cpp
    ${LUSAN}/model/sm/SMGuardSymbols.cpp
    ${LUSAN}/model/sm/SMGuardWhereUsed.cpp
    ${LUSAN}/model/sm/SMSymbolIndex.cpp
    ${LUSAN}/model/sm/SMTypeCompat.cpp
    ${LUSAN_ROOT}/tests/sm/SMGuardLadderTests.cpp
)
target_include_directories(lusan_sm_guard_ladder_tests PRIVATE ${LUSAN_BASE} ${LUSAN_THIRDPARTY})
target_compile_definitions(lusan_sm_guard_ladder_tests PRIVATE ${COMMON_COMPILE_DEF} IMP_LOGGER_DLL)
target_link_libraries(lusan_sm_guard_ladder_tests PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    areg::areg
    areg::aregextend
    areg::areglogger
    aregsqlite3
)
set_target_properties(lusan_sm_guard_ladder_tests PROPERTIES WIN32_EXECUTABLE OFF)

# ---------------------------------------------------------------------------
# SM-21-06: verbatim-code symbol assistance. Headless: the data layer plus the
# symbol-index helper (known symbols, used-symbols scan, verbatim where-used).
# ---------------------------------------------------------------------------
qt_add_executable(lusan_sm_symbol_tests
    ${LUSAN_TEST_SUPPORT_SRC}
    ${LUSAN}/model/sm/SMSymbolIndex.cpp
    ${LUSAN_ROOT}/tests/sm/SMSymbolIndexTests.cpp
)
target_include_directories(lusan_sm_symbol_tests PRIVATE ${LUSAN_BASE} ${LUSAN_THIRDPARTY})
target_compile_definitions(lusan_sm_symbol_tests PRIVATE ${COMMON_COMPILE_DEF} IMP_LOGGER_DLL)
target_link_libraries(lusan_sm_symbol_tests PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    areg::areg
    areg::aregextend
    areg::areglogger
    aregsqlite3
)
set_target_properties(lusan_sm_symbol_tests PROPERTIES WIN32_EXECUTABLE OFF)

# ---------------------------------------------------------------------------
# SM-21 (U2): the guard editing-surface symbol catalog + the did-you-mean
# edit-distance helper. Headless: the catalog builder and nearestName are
# model-facing (no widgets), so they compile against the data layer only.
# ---------------------------------------------------------------------------
qt_add_executable(lusan_sm_guard_catalog_tests
    ${LUSAN_TEST_SUPPORT_SRC}
    ${LUSAN}/model/sm/SMConditionText.cpp
    ${LUSAN}/model/sm/SMGuardParser.cpp
    ${LUSAN}/model/sm/SMGuardRender.cpp
    ${LUSAN}/model/sm/SMGuardSymbols.cpp
    ${LUSAN}/view/sm/NEGuardStyle.cpp
    ${LUSAN}/view/sm/SMGuardCatalog.cpp
    ${LUSAN}/view/sm/SMGuardCatalogModel.cpp
    ${LUSAN_ROOT}/tests/sm/SMGuardCatalogTests.cpp
)
target_include_directories(lusan_sm_guard_catalog_tests PRIVATE ${LUSAN_BASE} ${LUSAN_THIRDPARTY})
target_compile_definitions(lusan_sm_guard_catalog_tests PRIVATE ${COMMON_COMPILE_DEF} IMP_LOGGER_DLL)
target_link_libraries(lusan_sm_guard_catalog_tests PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    areg::areg
    areg::aregextend
    areg::areglogger
    aregsqlite3
)
set_target_properties(lusan_sm_guard_catalog_tests PROPERTIES WIN32_EXECUTABLE OFF)

# ---------------------------------------------------------------------------
# SM-21 (U4): the guard what-if evaluator (Try-it strip semantics). Headless:
# the data layer + parser/symbols (to build trees) + the evaluator under test.
# ---------------------------------------------------------------------------
qt_add_executable(lusan_sm_guard_eval_tests
    ${LUSAN_TEST_SUPPORT_SRC}
    ${LUSAN}/model/sm/SMConditionText.cpp
    ${LUSAN}/model/sm/SMGuardEval.cpp
    ${LUSAN}/model/sm/SMGuardParser.cpp
    ${LUSAN}/model/sm/SMGuardRender.cpp
    ${LUSAN}/model/sm/SMGuardSymbols.cpp
    ${LUSAN}/model/sm/SMGuardValidation.cpp
    ${LUSAN_ROOT}/tests/sm/SMGuardEvalTests.cpp
)
target_include_directories(lusan_sm_guard_eval_tests PRIVATE ${LUSAN_BASE} ${LUSAN_THIRDPARTY})
target_compile_definitions(lusan_sm_guard_eval_tests PRIVATE ${COMMON_COMPILE_DEF} IMP_LOGGER_DLL)
target_link_libraries(lusan_sm_guard_eval_tests PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    areg::areg
    areg::aregextend
    areg::areglogger
    aregsqlite3
)
set_target_properties(lusan_sm_guard_eval_tests PROPERTIES WIN32_EXECUTABLE OFF)

# ---------------------------------------------------------------------------
# SM-13: design-canvas smoke test. Drives the real SMDesign/SMScene/SMStateItem
# stack with synthetic mouse/key events on the Qt offscreen platform, so the
# canvas interactions are verifiable without a display or input injection.
# ---------------------------------------------------------------------------
qt_add_executable(lusan_sm_canvas_tests
    ${LUSAN_TEST_SUPPORT_SRC}
    ${LUSAN_CMD_COMMON_SRC}
    ${LUSAN}/model/sm/SMCommand.cpp
    ${LUSAN}/model/sm/SMConditionText.cpp
    ${LUSAN}/model/sm/SMLayoutCommands.cpp
    ${LUSAN}/model/sm/SMLiteralValidator.cpp
    ${LUSAN}/model/sm/SMPasteCommand.cpp
    ${LUSAN}/model/sm/SMStateCommands.cpp
    ${LUSAN}/model/sm/SMTransitionCommands.cpp
    ${LUSAN}/model/sm/SMTypeCompat.cpp
    ${LUSAN}/model/sm/SMOverviewModel.cpp
    ${LUSAN}/model/sm/SMDataTypeModel.cpp
    ${LUSAN}/model/sm/SMAttributeModel.cpp
    ${LUSAN}/model/sm/SMEventModel.cpp
    ${LUSAN}/model/sm/SMTimerModel.cpp
    ${LUSAN}/model/sm/SMMethodModel.cpp
    ${LUSAN}/model/sm/SMConstantModel.cpp
    ${LUSAN}/model/sm/SMIncludeModel.cpp
    ${LUSAN}/model/sm/SMSelectionModel.cpp
    ${LUSAN}/model/sm/SMSymbolIndex.cpp
    ${LUSAN}/model/sm/StateMachineModel.cpp
    ${LUSAN}/view/common/CppSyntaxHighlighter.cpp
    ${LUSAN}/view/sm/NESMDesign.cpp
    ${LUSAN}/view/sm/SMAutoPlacer.cpp
    ${LUSAN}/view/sm/SMCanvasItem.cpp
    ${LUSAN}/view/sm/SMCanvasTool.cpp
    ${LUSAN}/view/sm/SMCodeEditor.cpp
    ${LUSAN}/view/sm/NEGuardStyle.cpp
    ${LUSAN}/view/sm/SMArgSinkGuard.cpp
    ${LUSAN}/view/sm/SMFixBar.cpp
    ${LUSAN}/view/sm/SMFlowLayout.cpp
    ${LUSAN}/view/sm/SMGuardBar.cpp
    ${LUSAN}/view/sm/SMGuardCallsOutline.cpp
    ${LUSAN}/view/sm/SMGuardCatalog.cpp
    ${LUSAN}/view/sm/SMGuardCatalogModel.cpp
    ${LUSAN}/view/sm/SMGuardCatalogView.cpp
    ${LUSAN}/view/sm/SMGuardField.cpp
    ${LUSAN}/view/sm/SMGuardHelpCard.cpp
    ${LUSAN}/view/sm/SMGuardHighlighter.cpp
    ${LUSAN}/view/sm/SMGuardStatusLine.cpp
    ${LUSAN}/view/sm/SMHoverCard.cpp
    ${LUSAN}/view/sm/SMInlineToken.cpp
    ${LUSAN}/view/sm/SMIslandEditor.cpp
    ${LUSAN}/view/sm/SMOperandField.cpp
    ${LUSAN}/view/sm/SMSignatureCard.cpp
    ${LUSAN}/view/sm/SMStructureLens.cpp
    ${LUSAN}/view/sm/SMRefCompleter.cpp
    ${LUSAN}/model/sm/SMGuardCodegenPreview.cpp
    ${LUSAN}/model/sm/SMGuardCommands.cpp
    ${LUSAN}/model/sm/SMGuardEval.cpp
    ${LUSAN}/model/sm/SMGuardLadder.cpp
    ${LUSAN}/model/sm/SMGuardParser.cpp
    ${LUSAN}/model/sm/SMGuardRender.cpp
    ${LUSAN}/model/sm/SMGuardSymbols.cpp
    ${LUSAN}/model/sm/SMGuardValidation.cpp
    ${LUSAN}/model/sm/SMGuardWhereUsed.cpp
    ${LUSAN}/view/sm/SMTryStrip.cpp
    ${LUSAN}/view/sm/SMValidationPanel.cpp
    ${LUSAN}/view/sm/SMDesign.cpp
    ${LUSAN}/view/sm/SMEdgeItem.cpp
    ${LUSAN}/view/sm/SMGraphicsView.cpp
    ${LUSAN}/view/sm/SMNoteEditor.cpp
    ${LUSAN}/view/sm/SMNoteItem.cpp
    ${LUSAN}/view/sm/SMOutlinePanel.cpp
    ${LUSAN}/view/sm/SMPropertiesPanel.cpp
    ${LUSAN}/view/sm/SMScene.cpp
    ${LUSAN}/view/sm/SMSceneManager.cpp
    ${LUSAN}/view/sm/SMStateItem.cpp
    ${LUSAN}/view/sm/SMToolIcons.cpp
    ${LUSAN}/model/sm/SMArgumentCommands.cpp
    ${LUSAN}/model/sm/SMMappingSources.cpp
    ${LUSAN}/model/sm/SMOperationCommands.cpp
    ${LUSAN}/model/sm/SMOperationSummary.cpp
    ${LUSAN}/view/sm/SMArgMapTable.cpp
    ${LUSAN}/view/sm/SMArgSinkOperation.cpp
    ${LUSAN}/view/sm/SMOperationsEditor.cpp
    ${LUSAN}/view/sm/SMOperationsDialog.cpp
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

# ---------------------------------------------------------------------------
# SM-21 (U3): the guard-bar UI verification harness. Drives the REAL SMGuardBar
# and Methods page widgets programmatically (no input injection), asserts the
# field invariants (23a/23b, form-first parity) and saves QWidget::grab() shots.
# Run manually: lusan_sm_guard_ui_shots <doc.fsml> [grab dir] -- not a ctest
# entry, because it opens visible windows.
# ---------------------------------------------------------------------------
set(LUSAN_GUARD_UI_SRC
    ${LUSAN_TEST_SUPPORT_SRC}
    ${LUSAN_CMD_COMMON_SRC}
    ${LUSAN}/model/sm/SMCommand.cpp
    ${LUSAN}/model/sm/SMConditionText.cpp
    ${LUSAN}/model/sm/SMLayoutCommands.cpp
    ${LUSAN}/model/sm/SMLiteralValidator.cpp
    ${LUSAN}/model/sm/SMPasteCommand.cpp
    ${LUSAN}/model/sm/SMStateCommands.cpp
    ${LUSAN}/model/sm/SMTransitionCommands.cpp
    ${LUSAN}/model/sm/SMTypeCompat.cpp
    ${LUSAN}/model/sm/SMOverviewModel.cpp
    ${LUSAN}/model/sm/SMDataTypeModel.cpp
    ${LUSAN}/model/sm/SMAttributeModel.cpp
    ${LUSAN}/model/sm/SMEventModel.cpp
    ${LUSAN}/model/sm/SMTimerModel.cpp
    ${LUSAN}/model/sm/SMMethodModel.cpp
    ${LUSAN}/model/sm/SMConstantModel.cpp
    ${LUSAN}/model/sm/SMIncludeModel.cpp
    ${LUSAN}/model/sm/SMSelectionModel.cpp
    ${LUSAN}/model/sm/SMSymbolIndex.cpp
    ${LUSAN}/model/sm/StateMachineModel.cpp
    ${LUSAN}/model/sm/SMGuardCodegenPreview.cpp
    ${LUSAN}/model/sm/SMGuardCommands.cpp
    ${LUSAN}/model/sm/SMGuardEval.cpp
    ${LUSAN}/model/sm/SMGuardLadder.cpp
    ${LUSAN}/model/sm/SMGuardParser.cpp
    ${LUSAN}/model/sm/SMGuardRender.cpp
    ${LUSAN}/model/sm/SMGuardSymbols.cpp
    ${LUSAN}/model/sm/SMGuardWhereUsed.cpp
    ${LUSAN}/model/sm/SMMappingSources.cpp
    ${LUSAN}/view/common/CppSyntaxHighlighter.cpp
    ${LUSAN}/view/common/EditCancelFilter.cpp
    ${LUSAN}/view/common/MethodDetailsView.cpp
    ${LUSAN}/view/common/MethodListView.cpp
    ${LUSAN}/view/common/MethodParamDetailsView.cpp
    ${LUSAN}/view/common/TableCell.cpp
    ${LUSAN}/view/sm/NEGuardStyle.cpp
    ${LUSAN}/view/sm/SMArgMapTable.cpp
    ${LUSAN}/view/sm/SMArgSinkGuard.cpp
    ${LUSAN}/view/sm/SMCodeEditor.cpp
    ${LUSAN}/view/sm/SMEventParamDetails.cpp
    ${LUSAN}/view/sm/SMFixBar.cpp
    ${LUSAN}/view/sm/SMFlowLayout.cpp
    ${LUSAN}/view/sm/SMGuardBar.cpp
    ${LUSAN}/view/sm/SMGuardCallsOutline.cpp
    ${LUSAN}/view/sm/SMGuardCatalog.cpp
    ${LUSAN}/view/sm/SMGuardCatalogModel.cpp
    ${LUSAN}/view/sm/SMGuardCatalogView.cpp
    ${LUSAN}/view/sm/SMGuardField.cpp
    ${LUSAN}/view/sm/SMGuardHelpCard.cpp
    ${LUSAN}/view/sm/SMGuardHighlighter.cpp
    ${LUSAN}/view/sm/SMGuardStatusLine.cpp
    ${LUSAN}/view/sm/SMHoverCard.cpp
    ${LUSAN}/view/sm/SMInlineToken.cpp
    ${LUSAN}/view/sm/SMIslandEditor.cpp
    ${LUSAN}/view/sm/SMMethod.cpp
    ${LUSAN}/view/sm/SMOperandField.cpp
    ${LUSAN}/view/sm/SMSignatureCard.cpp
    ${LUSAN}/view/sm/SMStructureLens.cpp
    ${LUSAN}/view/sm/SMRefCompleter.cpp
    ${LUSAN}/view/sm/SMTryStrip.cpp
)

qt_add_executable(lusan_sm_guard_ui_shots
    ${LUSAN_GUARD_UI_SRC}
    ${LUSAN_ROOT}/tests/sm/SMGuardUiShots.cpp
    ${LUSAN}/res/lusan.qrc
)
target_include_directories(lusan_sm_guard_ui_shots PRIVATE ${LUSAN_BASE} ${LUSAN_THIRDPARTY})
target_compile_definitions(lusan_sm_guard_ui_shots PRIVATE ${COMMON_COMPILE_DEF} IMP_LOGGER_DLL)
target_link_libraries(lusan_sm_guard_ui_shots PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    areg::areg
    areg::aregextend
    areg::areglogger
    aregsqlite3
)
set_target_properties(lusan_sm_guard_ui_shots PROPERTIES WIN32_EXECUTABLE OFF)

# ---------------------------------------------------------------------------
# SM-21 (U4): the acceptance sweep harness -- items 16-18/20-22 end-to-end through
# the real widgets + file + validation path, the Try-it live checks, and the
# item-24 light/dark theme grabs. Run manually:
# lusan_sm_guard_u4_sweep <doc.fsml> [grab dir] -- opens visible windows.
# ---------------------------------------------------------------------------
qt_add_executable(lusan_sm_guard_u4_sweep
    ${LUSAN_GUARD_UI_SRC}
    ${LUSAN}/model/sm/SMGuardValidation.cpp
    ${LUSAN}/view/sm/SMPropertiesPanel.cpp
    ${LUSAN}/view/sm/SMValidationPanel.cpp
    ${LUSAN}/model/sm/SMArgumentCommands.cpp
    ${LUSAN}/model/sm/SMMappingSources.cpp
    ${LUSAN}/model/sm/SMOperationCommands.cpp
    ${LUSAN}/model/sm/SMOperationSummary.cpp
    ${LUSAN}/view/sm/SMArgMapTable.cpp
    ${LUSAN}/view/sm/SMArgSinkOperation.cpp
    ${LUSAN}/view/sm/SMOperationsEditor.cpp
    ${LUSAN_ROOT}/tests/sm/SMGuardU4Sweep.cpp
    ${LUSAN}/res/lusan.qrc
)
target_include_directories(lusan_sm_guard_u4_sweep PRIVATE ${LUSAN_BASE} ${LUSAN_THIRDPARTY})
target_compile_definitions(lusan_sm_guard_u4_sweep PRIVATE ${COMMON_COMPILE_DEF} IMP_LOGGER_DLL)
target_link_libraries(lusan_sm_guard_u4_sweep PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    areg::areg
    areg::aregextend
    areg::areglogger
    aregsqlite3
)
set_target_properties(lusan_sm_guard_u4_sweep PROPERTIES WIN32_EXECUTABLE OFF)

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
add_test(NAME sm_guard_tests COMMAND lusan_sm_guard_tests)
add_test(NAME sm_guard_catalog_tests COMMAND lusan_sm_guard_catalog_tests)
add_test(NAME sm_guard_ladder_tests COMMAND lusan_sm_guard_ladder_tests)
add_test(NAME sm_guard_eval_tests COMMAND lusan_sm_guard_eval_tests)
add_test(NAME sm_symbol_tests COMMAND lusan_sm_symbol_tests)
add_test(NAME sm_command_tests COMMAND lusan_sm_command_tests)
add_test(NAME sm_mapping_tests COMMAND lusan_sm_mapping_tests)
add_test(NAME si_command_tests COMMAND lusan_si_command_tests)
