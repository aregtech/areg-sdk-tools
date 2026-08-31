# ---------------------------------------------------------------------------
# Lusan unit tests (opt-in: -DLUSAN_BUILD_TESTS=ON).
#
# This is additive and OFF by default, so the normal `lusan` application build
# is unchanged. The SM-01 test builds a small, self-contained console executable
# from the headless data layer (common + data/common + data/sm) plus the test
# driver; it links the areg targets only to inherit their include directories
# (the data layer uses areg headers for platform macros, no linked symbols).
# ---------------------------------------------------------------------------

# A service interface and a state machine both resolve their `.dtml` includes as they load, so
# every headless target that carries one of them carries the data type document with it.
set(LUSAN_DT_DATA_SRC
    ${LUSAN}/data/dt/DTDocumentCache.cpp
    ${LUSAN}/data/dt/DataTypeDocumentData.cpp
    ${LUSAN}/data/dt/DataTypeImportResolver.cpp
)

set(LUSAN_TEST_SUPPORT_SRC
    ${LUSAN_DT_DATA_SRC}
    # common
    ${LUSAN}/common/DocElementTable.cpp
    ${LUSAN}/common/DocReservedNames.cpp
    ${LUSAN}/common/DocSchemaReader.cpp
    ${LUSAN}/common/ElementBase.cpp
    ${LUSAN}/common/VersionNumber.cpp
    ${LUSAN}/common/NELusanCommon.cpp
    ${LUSAN}/common/XmlSI.cpp
    ${LUSAN}/model/common/DocUnknownScan.cpp
    # data/common (reused by the FSM model)
    ${LUSAN}/data/common/DocumentElem.cpp
    ${LUSAN}/data/common/ParamBase.cpp
    ${LUSAN}/data/common/ParamType.cpp
    ${LUSAN}/data/common/MethodBase.cpp
    ${LUSAN}/data/common/MethodParameter.cpp
    ${LUSAN}/data/common/MethodEntry.cpp
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
    ${LUSAN}/data/common/AttributeEntry.cpp
    # data/sm (the model under test)
    ${LUSAN}/data/common/AttributeDataSection.cpp
    ${LUSAN}/data/sm/SMClipboard.cpp
    ${LUSAN}/data/sm/SMCondition.cpp
    ${LUSAN}/data/common/ConstantDataSection.cpp
    ${LUSAN}/data/common/DataTypeDataSection.cpp
    ${LUSAN}/data/sm/SMEventData.cpp
    ${LUSAN}/data/sm/SMGuardTree.cpp
    ${LUSAN}/data/sm/SMDocumentCache.cpp
    ${LUSAN}/data/sm/SMImportResolver.cpp
    ${LUSAN}/data/common/IncludeDataSection.cpp
    ${LUSAN}/data/sm/SMLayoutData.cpp
    ${LUSAN}/data/common/MethodDataSection.cpp
    ${LUSAN}/data/sm/SMOperation.cpp
    ${LUSAN}/data/common/OverviewDataSection.cpp
    ${LUSAN}/data/sm/SMOverviewData.cpp
    ${LUSAN}/data/sm/SMReferences.cpp
    ${LUSAN}/data/sm/SMState.cpp
    ${LUSAN}/data/sm/SMTimerData.cpp
    ${LUSAN}/data/sm/SMTransition.cpp
    ${LUSAN}/data/sm/StateMachineData.cpp
    # model/sm (headless reference glue)
    ${LUSAN}/model/sm/SMDocumentIndex.cpp
    ${LUSAN}/model/sm/SMGoToDef.cpp
    # The element table reads the compiled-in schemas, so every headless target carries them.
    ${LUSAN}/res/lusan.qrc
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

# SM-26: the reference walker
qt_add_executable(lusan_sm_reference_tests
    ${LUSAN_TEST_SUPPORT_SRC}
    ${LUSAN_ROOT}/tests/sm/SMReferenceTests.cpp
)
target_include_directories(lusan_sm_reference_tests PRIVATE ${LUSAN_BASE} ${LUSAN_THIRDPARTY})
target_compile_definitions(lusan_sm_reference_tests PRIVATE ${COMMON_COMPILE_DEF} IMP_LOGGER_DLL)
target_link_libraries(lusan_sm_reference_tests PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    areg::areg
    areg::aregextend
    areg::areglogger
    aregsqlite3
)
set_target_properties(lusan_sm_reference_tests PROPERTIES WIN32_EXECUTABLE OFF)

# deterministic XML round-trip + parse robustness
qt_add_executable(lusan_sm_serialization_tests
    ${LUSAN_TEST_SUPPORT_SRC}
    ${LUSAN_ROOT}/tests/sm/SMSerializationTests.cpp
)

target_include_directories(lusan_sm_serialization_tests PRIVATE ${LUSAN_BASE} ${LUSAN_THIRDPARTY})
target_compile_definitions(lusan_sm_serialization_tests PRIVATE
    ${COMMON_COMPILE_DEF}
    IMP_LOGGER_DLL
    LUSAN_TEST_DATA_DIR="${LUSAN_ROOT}/tests/data"
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
# Structural and reference validation engine. The engine is headless, so it reuses
# the same data-layer support sources plus SMValidator.cpp; the QObject scheduler
# (DocValidationController) is exercised by the app, not this headless driver.
# ---------------------------------------------------------------------------
qt_add_executable(lusan_sm_validation_tests
    ${LUSAN_TEST_SUPPORT_SRC}
    ${LUSAN}/model/sm/SMValidator.cpp
    ${LUSAN}/model/sm/SMGuardValidation.cpp
    ${LUSAN}/model/sm/SMOperationValidation.cpp
    ${LUSAN}/model/sm/SMGuardSymbols.cpp
    ${LUSAN}/model/sm/SMTypeCompat.cpp
    ${LUSAN}/model/common/DocRuleChecks.cpp
    ${LUSAN}/model/common/LiteralValidator.cpp
    ${LUSAN_ROOT}/tests/sm/SMValidationTests.cpp
)
target_include_directories(lusan_sm_validation_tests PRIVATE ${LUSAN_BASE} ${LUSAN_THIRDPARTY})
target_compile_definitions(lusan_sm_validation_tests PRIVATE
    ${COMMON_COMPILE_DEF}
    IMP_LOGGER_DLL
    # The reference-fixture sweep validates the documents that ship next to the spec.
    LUSAN_TEST_DATA_DIR="${LUSAN_ROOT}/tests/data"
)
target_link_libraries(lusan_sm_validation_tests PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    areg::areg
    areg::aregextend
    areg::areglogger
    aregsqlite3
)
set_target_properties(lusan_sm_validation_tests PROPERTIES WIN32_EXECUTABLE OFF)


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
    ${LUSAN}/model/sm/SMRenameCommands.cpp
    # StateMachineModel's load path applies the legacy->draft guard shim (SM-21 U4).
    ${LUSAN}/model/sm/SMConditionText.cpp
    ${LUSAN}/model/sm/SMGuardParser.cpp
    ${LUSAN}/model/sm/SMGuardRender.cpp
    ${LUSAN}/model/sm/SMGuardSymbols.cpp
    ${LUSAN}/model/sm/SMLayoutCommands.cpp
    ${LUSAN}/model/sm/SMPasteCommand.cpp
    ${LUSAN}/model/sm/SMStateCommands.cpp
    ${LUSAN}/model/sm/SMTransitionCommands.cpp
    ${LUSAN}/model/common/OverviewModel.cpp
    ${LUSAN}/model/sm/SMOverviewModel.cpp
    ${LUSAN}/model/common/DataTypeModel.cpp
    ${LUSAN}/model/common/AttributeModel.cpp
    ${LUSAN}/model/sm/SMEventModel.cpp
    ${LUSAN}/model/sm/SMTimerModel.cpp
    ${LUSAN}/model/common/MethodModel.cpp
    ${LUSAN}/model/common/ConstantModel.cpp
    ${LUSAN}/model/common/IncludeModel.cpp
    ${LUSAN}/model/sm/SMIncludeModel.cpp
    ${LUSAN}/model/sm/SMSelectionModel.cpp
    ${LUSAN}/model/sm/StateMachineModel.cpp
    ${LUSAN}/model/common/DocValidationController.cpp
    ${LUSAN}/model/sm/SMValidator.cpp
    ${LUSAN}/model/sm/SMGuardValidation.cpp
    ${LUSAN}/model/sm/SMOperationValidation.cpp
    ${LUSAN}/model/sm/SMTypeCompat.cpp
    ${LUSAN}/model/common/DocRuleChecks.cpp
    ${LUSAN}/model/common/LiteralValidator.cpp
    ${LUSAN_ROOT}/tests/sm/SMCommandTests.cpp
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
    ${LUSAN}/model/sm/SMRenameCommands.cpp
    ${LUSAN}/model/sm/SMConditionText.cpp
    ${LUSAN}/model/sm/SMGuardParser.cpp
    ${LUSAN}/model/sm/SMGuardRender.cpp
    ${LUSAN}/model/sm/SMGuardSymbols.cpp
    ${LUSAN}/model/sm/SMLayoutCommands.cpp
    ${LUSAN}/model/sm/SMPasteCommand.cpp
    ${LUSAN}/model/sm/SMStateCommands.cpp
    ${LUSAN}/model/common/OverviewModel.cpp
    ${LUSAN}/model/sm/SMOverviewModel.cpp
    ${LUSAN}/model/common/DataTypeModel.cpp
    ${LUSAN}/model/common/AttributeModel.cpp
    ${LUSAN}/model/sm/SMEventModel.cpp
    ${LUSAN}/model/sm/SMTimerModel.cpp
    ${LUSAN}/model/common/MethodModel.cpp
    ${LUSAN}/model/common/ConstantModel.cpp
    ${LUSAN}/model/common/IncludeModel.cpp
    ${LUSAN}/model/sm/SMIncludeModel.cpp
    ${LUSAN}/model/sm/SMSelectionModel.cpp
    ${LUSAN}/model/sm/StateMachineModel.cpp
    ${LUSAN}/model/common/DocValidationController.cpp
    ${LUSAN}/model/sm/SMValidator.cpp
    ${LUSAN}/model/sm/SMGuardValidation.cpp
    ${LUSAN}/model/sm/SMArgumentCommands.cpp
    ${LUSAN}/model/sm/SMMappingSources.cpp
    ${LUSAN}/model/sm/SMTypeCompat.cpp
    ${LUSAN}/model/common/DocRuleChecks.cpp
    ${LUSAN}/model/common/LiteralValidator.cpp
    ${LUSAN}/model/sm/SMOperationValidation.cpp
    ${LUSAN}/view/sm/NEGuardStyle.cpp
    ${LUSAN}/view/sm/SMAccordion.cpp
    ${LUSAN}/view/sm/SMArgMapTable.cpp
    ${LUSAN}/view/sm/SMArgSinkOperation.cpp
    ${LUSAN_ROOT}/tests/sm/SMArgMapTableTests.cpp
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
    ${LUSAN}/model/sm/SMRenameCommands.cpp
    ${LUSAN}/model/sm/SMConditionText.cpp
    ${LUSAN}/model/sm/SMGuardCodegenPreview.cpp
    ${LUSAN}/model/sm/SMGuardCommands.cpp
    ${LUSAN}/model/sm/SMGuardParser.cpp
    ${LUSAN}/model/sm/SMGuardRender.cpp
    ${LUSAN}/model/sm/SMGuardSymbols.cpp
    ${LUSAN}/model/sm/SMTypeCompat.cpp
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
    ${LUSAN}/model/sm/SMRenameCommands.cpp
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
    ${LUSAN}/model/sm/SMTypeCompat.cpp
    ${LUSAN}/view/sm/NEGuardStyle.cpp
    ${LUSAN}/view/sm/SMGuardCatalog.cpp
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
    ${LUSAN}/model/sm/SMTypeCompat.cpp
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
# The design-page source set: everything a harness needs to construct a real SMDesign.
# Shared by the canvas smoke tests and the SM-27 responsiveness gate so the two can
# never drift into building different halves of the page.
set(LUSAN_DESIGN_PAGE_SRC
    ${LUSAN_TEST_SUPPORT_SRC}
    ${LUSAN_CMD_COMMON_SRC}
    ${LUSAN}/model/sm/SMCommand.cpp
    ${LUSAN}/model/sm/SMRenameCommands.cpp
    ${LUSAN}/model/sm/SMConditionText.cpp
    ${LUSAN}/model/sm/SMLayoutCommands.cpp
    ${LUSAN}/model/common/DocRuleChecks.cpp
    ${LUSAN}/model/common/LiteralValidator.cpp
    ${LUSAN}/model/sm/SMPasteCommand.cpp
    ${LUSAN}/model/sm/SMStateCommands.cpp
    ${LUSAN}/model/sm/SMTransitionCommands.cpp
    ${LUSAN}/model/sm/SMTypeCompat.cpp
    ${LUSAN}/model/common/OverviewModel.cpp
    ${LUSAN}/model/sm/SMOverviewModel.cpp
    ${LUSAN}/model/common/DataTypeModel.cpp
    ${LUSAN}/model/common/AttributeModel.cpp
    ${LUSAN}/model/sm/SMEventModel.cpp
    ${LUSAN}/model/sm/SMTimerModel.cpp
    ${LUSAN}/model/common/MethodModel.cpp
    ${LUSAN}/model/common/ConstantModel.cpp
    ${LUSAN}/model/common/IncludeModel.cpp
    ${LUSAN}/model/sm/SMIncludeModel.cpp
    ${LUSAN}/model/sm/SMSelectionModel.cpp
    ${LUSAN}/model/sm/SMSymbolIndex.cpp
    ${LUSAN}/model/sm/StateMachineModel.cpp
    ${LUSAN}/model/common/DocValidationController.cpp
    ${LUSAN}/model/sm/SMValidator.cpp
    ${LUSAN}/view/common/CppSyntaxHighlighter.cpp
    ${LUSAN}/view/common/SearchLineEdit.cpp
    ${LUSAN}/view/common/PendingEditWatcher.cpp
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
    ${LUSAN}/view/sm/SMGuardDataPanel.cpp
    ${LUSAN}/view/sm/SMGuardCatalog.cpp
    ${LUSAN}/view/sm/SMGuardField.cpp
    ${LUSAN}/view/sm/SMGuardHelpCard.cpp
    ${LUSAN}/view/sm/SMGuardHighlighter.cpp
    ${LUSAN}/view/sm/SMGuardPopout.cpp
    ${LUSAN}/view/sm/SMGuardStatusLine.cpp
    ${LUSAN}/view/sm/SMHoverCard.cpp
    ${LUSAN}/view/sm/SMInlineToken.cpp
    ${LUSAN}/view/sm/SMIslandEditor.cpp
    ${LUSAN}/view/sm/SMOperandField.cpp
    ${LUSAN}/view/sm/SMSignatureCard.cpp
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
    ${LUSAN}/view/common/DocValidationPanel.cpp
    ${LUSAN}/data/sm/SMReferences.cpp
    ${LUSAN}/model/sm/SMGoToDef.cpp
    ${LUSAN}/model/sm/SMWhereUsed.cpp
    ${LUSAN}/view/sm/SMWhereUsedMenu.cpp
    ${LUSAN}/view/sm/SMDesign.cpp
    ${LUSAN}/view/sm/SMEdgeItem.cpp
    ${LUSAN}/view/sm/SMGraphicsView.cpp
    ${LUSAN}/view/sm/SMKindGlyph.cpp
    ${LUSAN}/view/sm/SMNoteEditor.cpp
    ${LUSAN}/view/sm/SMNoteItem.cpp
    ${LUSAN}/view/sm/SMOutlinePanel.cpp
    ${LUSAN}/view/sm/SMPropertiesPanel.cpp
    ${LUSAN}/view/sm/SMScene.cpp
    ${LUSAN}/view/sm/SMSceneManager.cpp
    ${LUSAN}/view/sm/SMStateItem.cpp
    ${LUSAN}/view/sm/SMSubmachinePeek.cpp
    ${LUSAN}/view/sm/SMToolIcons.cpp
    ${LUSAN}/model/sm/SMArgumentCommands.cpp
    ${LUSAN}/model/sm/SMMappingSources.cpp
    ${LUSAN}/model/sm/SMOperationCommands.cpp
    ${LUSAN}/model/sm/SMOperationSummary.cpp
    ${LUSAN}/model/sm/SMOperationValidation.cpp
    ${LUSAN}/view/sm/SMAccordion.cpp
    ${LUSAN}/view/sm/SMArgMapTable.cpp
    ${LUSAN}/view/sm/SMArgSinkOperation.cpp
    ${LUSAN}/view/sm/SMSectionChrome.cpp
    ${LUSAN}/view/sm/SMOperationsEditor.cpp
    ${LUSAN}/view/sm/SMInternalDialog.cpp
    ${LUSAN}/view/sm/SMInternalEditor.cpp
    ${LUSAN}/view/sm/SMStimulusPicker.cpp
    ${LUSAN}/view/sm/SMOperationsDialog.cpp
)

qt_add_executable(lusan_sm_canvas_tests
    ${LUSAN_DESIGN_PAGE_SRC}
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
# The modified mark: it must follow the first keystroke, not the commit that comes
# when the field loses the focus, and after a save it must follow the distance to
# the point the file was written from. Offscreen, real document, real panel:
#   lusan_sm_dirty_mark <TrafficLight.fsml> [output directory]
# ---------------------------------------------------------------------------

qt_add_executable(lusan_sm_dirty_mark
    ${LUSAN_DESIGN_PAGE_SRC}
    ${LUSAN_ROOT}/tests/sm/SMDirtyMarkTests.cpp
    ${LUSAN}/res/lusan.qrc
)
target_include_directories(lusan_sm_dirty_mark PRIVATE ${LUSAN_BASE} ${LUSAN_THIRDPARTY})
target_compile_definitions(lusan_sm_dirty_mark PRIVATE ${COMMON_COMPILE_DEF} IMP_LOGGER_DLL)
target_link_libraries(lusan_sm_dirty_mark PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    areg::areg
    areg::aregextend
    areg::areglogger
    aregsqlite3
)
set_target_properties(lusan_sm_dirty_mark PROPERTIES WIN32_EXECUTABLE OFF)

# ---------------------------------------------------------------------------
# L4 acceptance: edits the internal transition of TRAFFIC_LIGHT_RED through both
# routes the task names, saves, reloads, and grabs the pictures. Deliberately NOT
# registered with ctest and deliberately NOT offscreen -- it is run by hand against
# a real document so its PNGs carry real text:
#   lusan_sm_l4_proof <TrafficLight.fsml> <output directory>
# ---------------------------------------------------------------------------

qt_add_executable(lusan_sm_l4_proof
    ${LUSAN_DESIGN_PAGE_SRC}
    ${LUSAN_ROOT}/tests/sm/SML4ProofTests.cpp
    ${LUSAN}/res/lusan.qrc
)
target_include_directories(lusan_sm_l4_proof PRIVATE ${LUSAN_BASE} ${LUSAN_THIRDPARTY})
target_compile_definitions(lusan_sm_l4_proof PRIVATE ${COMMON_COMPILE_DEF} IMP_LOGGER_DLL)
target_link_libraries(lusan_sm_l4_proof PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    areg::areg
    areg::aregextend
    areg::areglogger
    aregsqlite3
)
set_target_properties(lusan_sm_l4_proof PROPERTIES WIN32_EXECUTABLE OFF)

# ---------------------------------------------------------------------------
# L8 acceptance: the canvas draws the document's TransitionList and nothing else,
# and no gesture can draw a transition into a Start. Registered as the ctest entry
# sm_l8_proof (offscreen QPA); can also be run by hand for the pictures:
#   lusan_sm_l8_proof <FullFeature.fsml> <output directory>
# ---------------------------------------------------------------------------

qt_add_executable(lusan_sm_l8_proof
    ${LUSAN_DESIGN_PAGE_SRC}
    ${LUSAN_ROOT}/tests/sm/SML8ProofTests.cpp
    ${LUSAN}/res/lusan.qrc
)
target_include_directories(lusan_sm_l8_proof PRIVATE ${LUSAN_BASE} ${LUSAN_THIRDPARTY})
target_compile_definitions(lusan_sm_l8_proof PRIVATE ${COMMON_COMPILE_DEF} IMP_LOGGER_DLL)
target_link_libraries(lusan_sm_l8_proof PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    areg::areg
    areg::aregextend
    areg::areglogger
    aregsqlite3
)
set_target_properties(lusan_sm_l8_proof PROPERTIES WIN32_EXECUTABLE OFF)

# ---------------------------------------------------------------------------
# SM-27: the responsiveness gate. Generates a large synthetic document and times the
# interactions spec 9.9 requires to feel immediate. Registered as the ctest entry
# sm_perf (offscreen QPA); can also be run manually: lusan_sm_perf [output dir].
# ---------------------------------------------------------------------------

qt_add_executable(lusan_sm_perf
    ${LUSAN_DESIGN_PAGE_SRC}
    ${LUSAN_ROOT}/tests/sm/SMPerformanceTests.cpp
    ${LUSAN}/res/lusan.qrc
)
target_include_directories(lusan_sm_perf PRIVATE ${LUSAN_BASE} ${LUSAN_THIRDPARTY})
target_compile_definitions(lusan_sm_perf PRIVATE ${COMMON_COMPILE_DEF} IMP_LOGGER_DLL)
target_link_libraries(lusan_sm_perf PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    areg::areg
    areg::aregextend
    areg::areglogger
    aregsqlite3
)
set_target_properties(lusan_sm_perf PROPERTIES WIN32_EXECUTABLE OFF)

# ---------------------------------------------------------------------------
# SM-21 (U3): the guard-bar UI verification harness. Drives the REAL SMGuardBar
# and Methods page widgets programmatically (no input injection), asserts the
# field invariants (23a/23b, form-first parity) and saves QWidget::grab() shots.
# Registered as the ctest entry sm_guard_ui_shots (offscreen QPA); can also be
# run manually: lusan_sm_guard_ui_shots <doc.fsml> [grab dir].
# ---------------------------------------------------------------------------
set(LUSAN_GUARD_UI_SRC
    ${LUSAN_TEST_SUPPORT_SRC}
    ${LUSAN_CMD_COMMON_SRC}
    ${LUSAN}/model/sm/SMCommand.cpp
    ${LUSAN}/model/sm/SMRenameCommands.cpp
    ${LUSAN}/model/sm/SMConditionText.cpp
    ${LUSAN}/model/sm/SMLayoutCommands.cpp
    ${LUSAN}/model/common/DocRuleChecks.cpp
    ${LUSAN}/model/common/LiteralValidator.cpp
    ${LUSAN}/model/sm/SMPasteCommand.cpp
    ${LUSAN}/model/sm/SMStateCommands.cpp
    ${LUSAN}/model/sm/SMTransitionCommands.cpp
    ${LUSAN}/model/sm/SMTypeCompat.cpp
    ${LUSAN}/model/common/OverviewModel.cpp
    ${LUSAN}/model/sm/SMOverviewModel.cpp
    ${LUSAN}/model/common/DataTypeModel.cpp
    ${LUSAN}/model/common/AttributeModel.cpp
    ${LUSAN}/model/sm/SMEventModel.cpp
    ${LUSAN}/model/sm/SMTimerModel.cpp
    ${LUSAN}/model/common/MethodModel.cpp
    ${LUSAN}/model/common/ConstantModel.cpp
    ${LUSAN}/model/common/IncludeModel.cpp
    ${LUSAN}/model/sm/SMIncludeModel.cpp
    ${LUSAN}/model/sm/SMSelectionModel.cpp
    ${LUSAN}/model/sm/SMSymbolIndex.cpp
    ${LUSAN}/model/sm/StateMachineModel.cpp
    ${LUSAN}/model/common/DocValidationController.cpp
    ${LUSAN}/model/sm/SMValidator.cpp
    ${LUSAN}/model/sm/SMGuardValidation.cpp
    ${LUSAN}/model/sm/SMOperationValidation.cpp
    ${LUSAN}/model/sm/SMGuardCodegenPreview.cpp
    ${LUSAN}/model/sm/SMGuardCommands.cpp
    ${LUSAN}/model/sm/SMGuardEval.cpp
    ${LUSAN}/model/sm/SMGuardLadder.cpp
    ${LUSAN}/model/sm/SMGuardParser.cpp
    ${LUSAN}/model/sm/SMGuardRender.cpp
    ${LUSAN}/model/sm/SMGuardSymbols.cpp
    ${LUSAN}/model/sm/SMGuardWhereUsed.cpp
    ${LUSAN}/model/sm/SMWhereUsed.cpp
    ${LUSAN}/model/sm/SMMappingSources.cpp
    ${LUSAN}/view/common/CppSyntaxHighlighter.cpp
    ${LUSAN}/view/common/EditCancelFilter.cpp
    ${LUSAN}/view/common/IEDataTypeConsumer.cpp
    ${LUSAN}/view/common/MethodDetailsView.cpp
    ${LUSAN}/view/common/MethodListView.cpp
    ${LUSAN}/view/common/MethodParamDetailsView.cpp
    ${LUSAN}/view/common/PendingEditWatcher.cpp
    ${LUSAN}/view/common/TableCell.cpp
    ${LUSAN}/view/common/WidgetHighlight.cpp
    ${LUSAN}/view/sm/NEGuardStyle.cpp
    ${LUSAN}/view/sm/SMAccordion.cpp
    ${LUSAN}/view/sm/SMSectionChrome.cpp
    ${LUSAN}/view/sm/SMArgMapTable.cpp
    ${LUSAN}/view/sm/SMArgSinkGuard.cpp
    ${LUSAN}/view/sm/SMCodeEditor.cpp
    ${LUSAN}/view/sm/SMEventParamDetails.cpp
    ${LUSAN}/view/sm/SMFixBar.cpp
    ${LUSAN}/view/sm/SMFlowLayout.cpp
    ${LUSAN}/view/sm/SMGuardBar.cpp
    ${LUSAN}/view/sm/SMGuardCallsOutline.cpp
    ${LUSAN}/view/sm/SMGuardDataPanel.cpp
    ${LUSAN}/view/sm/SMGuardCatalog.cpp
    ${LUSAN}/view/sm/SMGuardField.cpp
    ${LUSAN}/view/sm/SMGuardHelpCard.cpp
    ${LUSAN}/view/sm/SMGuardHighlighter.cpp
    ${LUSAN}/view/sm/SMGuardPopout.cpp
    ${LUSAN}/view/sm/SMGuardStatusLine.cpp
    ${LUSAN}/view/sm/SMHoverCard.cpp
    ${LUSAN}/view/sm/SMInlineToken.cpp
    ${LUSAN}/view/sm/SMIslandEditor.cpp
    ${LUSAN}/view/common/MethodPage.cpp
    ${LUSAN}/view/sm/SMMethod.cpp
    ${LUSAN}/view/sm/SMOperandField.cpp
    ${LUSAN}/view/sm/SMSignatureCard.cpp
    ${LUSAN}/view/sm/SMRefCompleter.cpp
    ${LUSAN}/view/sm/SMTryStrip.cpp
    ${LUSAN}/view/sm/SMToolIcons.cpp
    ${LUSAN}/view/sm/SMWhereUsedMenu.cpp
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
# item-24 light/dark theme grabs. Registered as the ctest entry sm_guard_u4_sweep
# (offscreen QPA); can also be run manually:
# lusan_sm_guard_u4_sweep <doc.fsml> [grab dir].
# ---------------------------------------------------------------------------
qt_add_executable(lusan_sm_guard_u4_sweep
    ${LUSAN_GUARD_UI_SRC}
    ${LUSAN}/model/sm/SMGuardValidation.cpp
    ${LUSAN}/view/sm/SMPropertiesPanel.cpp
    ${LUSAN}/view/sm/SMKindGlyph.cpp
    ${LUSAN}/view/sm/SMInternalDialog.cpp
    ${LUSAN}/view/sm/SMInternalEditor.cpp
    ${LUSAN}/view/sm/SMStimulusPicker.cpp
    ${LUSAN}/view/common/DocValidationPanel.cpp
    ${LUSAN}/model/sm/SMArgumentCommands.cpp
    ${LUSAN}/model/sm/SMMappingSources.cpp
    ${LUSAN}/model/sm/SMOperationCommands.cpp
    ${LUSAN}/model/sm/SMOperationSummary.cpp
    ${LUSAN}/view/sm/SMAccordion.cpp
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

# ---------------------------------------------------------------------------
# The shared editor pages, built over both document kinds. LUSAN_DESIGN_PAGE_SRC
# carries the two models and the canvas but none of the page views, so this list
# adds them, the service interface data and model layer, and the three state
# machine subclasses. Registered as the ctest entry design_pages (offscreen QPA);
# can also be run manually for the pictures:
#   lusan_design_pages <TrafficLight.fsml> [output directory]
# ---------------------------------------------------------------------------
set(LUSAN_DT_MODEL_SRC
    ${LUSAN}/model/dt/DTValidator.cpp
    ${LUSAN}/model/dt/DataTypeDocumentModel.cpp
)

set(LUSAN_SI_MODEL_SRC
    ${LUSAN}/data/common/OverviewDataSection.cpp
    ${LUSAN}/data/si/SIOverviewData.cpp
    ${LUSAN}/data/si/ServiceInterfaceData.cpp
    ${LUSAN}/model/si/SICommand.cpp
    ${LUSAN}/model/common/OverviewModel.cpp
    ${LUSAN}/model/si/SIOverviewModel.cpp
    ${LUSAN}/model/si/SIValidator.cpp
    ${LUSAN}/model/si/ServiceInterfaceModel.cpp
)

set(LUSAN_EDITOR_PAGE_SRC
    ${LUSAN}/view/common/AttributeDetailsView.cpp
    ${LUSAN}/view/common/AttributeListView.cpp
    ${LUSAN}/view/common/AttributePage.cpp
    ${LUSAN}/view/common/ConstantDetailsView.cpp
    ${LUSAN}/view/common/ConstantListView.cpp
    ${LUSAN}/view/common/ConstantPage.cpp
    ${LUSAN}/view/common/DataTypeDetailsView.cpp
    ${LUSAN}/view/common/DataTypeFieldDetailsView.cpp
    ${LUSAN}/view/common/DataTypeListView.cpp
    ${LUSAN}/view/common/DataTypePage.cpp
    ${LUSAN}/view/common/EditCancelFilter.cpp
    ${LUSAN}/view/common/IEDataTypeConsumer.cpp
    ${LUSAN}/view/common/IncludeDetailsView.cpp
    ${LUSAN}/view/common/IncludeListView.cpp
    ${LUSAN}/view/common/IncludePage.cpp
    ${LUSAN}/view/common/MethodDetailsView.cpp
    ${LUSAN}/view/common/MethodListView.cpp
    ${LUSAN}/view/common/MethodPage.cpp
    ${LUSAN}/view/common/MethodParamDetailsView.cpp
    ${LUSAN}/view/common/OverviewPage.cpp
    ${LUSAN}/view/common/TableCell.cpp
    ${LUSAN}/view/common/WidgetHighlight.cpp
    ${LUSAN}/view/common/WorkspaceFileDialog.cpp
    ${LUSAN}/view/si/SIOverview.cpp
    ${LUSAN}/view/sm/SMAttribute.cpp
    ${LUSAN}/view/sm/SMConstant.cpp
    ${LUSAN}/view/sm/SMInclude.cpp
    ${LUSAN}/view/sm/SMMethod.cpp
    ${LUSAN}/view/sm/SMOverview.cpp
)

qt_add_executable(lusan_design_pages
    ${LUSAN_DESIGN_PAGE_SRC}
    ${LUSAN_SI_MODEL_SRC}
    ${LUSAN_DT_MODEL_SRC}
    ${LUSAN_EDITOR_PAGE_SRC}
    ${LUSAN_ROOT}/tests/common/AppEnvStub.cpp
    ${LUSAN_ROOT}/tests/common/DesignPageTests.cpp
    ${LUSAN}/res/lusan.qrc
)
target_include_directories(lusan_design_pages PRIVATE ${LUSAN_BASE} ${LUSAN_THIRDPARTY})
target_compile_definitions(lusan_design_pages PRIVATE ${COMMON_COMPILE_DEF} IMP_LOGGER_DLL)
target_link_libraries(lusan_design_pages PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    areg::areg
    areg::aregextend
    areg::areglogger
    aregsqlite3
)
set_target_properties(lusan_design_pages PROPERTIES WIN32_EXECUTABLE OFF)

# The Service Interface headless data layer.
set(LUSAN_SI_SUPPORT_SRC
    ${LUSAN_DT_DATA_SRC}
    ${LUSAN}/common/DocElementTable.cpp
    ${LUSAN}/common/DocSchemaReader.cpp
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
    ${LUSAN}/data/common/AttributeDataSection.cpp
    ${LUSAN}/data/common/ConstantDataSection.cpp
    ${LUSAN}/data/common/DataTypeDataSection.cpp
    ${LUSAN}/data/common/IncludeDataSection.cpp
    ${LUSAN}/data/common/MethodDataSection.cpp
    ${LUSAN}/data/common/MethodEntry.cpp
    ${LUSAN}/data/common/OverviewDataSection.cpp
    ${LUSAN}/data/si/SIOverviewData.cpp
    ${LUSAN}/data/si/ServiceInterfaceData.cpp
    ${LUSAN}/model/common/DocUnknownScan.cpp
)

qt_add_executable(lusan_si_command_tests
    ${LUSAN_SI_SUPPORT_SRC}
    ${LUSAN_CMD_COMMON_SRC}
    ${LUSAN}/model/si/SICommand.cpp
    ${LUSAN}/model/si/SIValidator.cpp
    ${LUSAN}/model/common/DocRuleChecks.cpp
    ${LUSAN}/model/common/LiteralValidator.cpp
    ${LUSAN_ROOT}/tests/si/SICommandTests.cpp
    # Resolving a declared type against the primitive and basic-container catalogs needs the real
    # ones: DataTypeFactory loads them from the "Predefined Types" resource at runtime, so without
    # the .qrc compiled in every uint32 or bool looks like an undeclared type.
    ${LUSAN}/res/lusan.qrc
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

# ---------------------------------------------------------------------------
# The Data Type document (.dtml): the file format, the name that follows the file,
# and the validation engine. Headless -- the document is the shared data sections
# plus a root, so it reuses the service interface support list and adds its own.
# ---------------------------------------------------------------------------
qt_add_executable(lusan_dt_document_tests
    ${LUSAN_SI_SUPPORT_SRC}
    ${LUSAN}/model/dt/DTValidator.cpp
    ${LUSAN}/model/common/DocRuleChecks.cpp
    ${LUSAN}/model/common/LiteralValidator.cpp
    ${LUSAN_ROOT}/tests/dt/DTDocumentTests.cpp
    # Resolving uint32 and the basic containers needs the real predefined-type catalog,
    # which DataTypeFactory loads from the "Predefined Types" resource at runtime.
    ${LUSAN}/res/lusan.qrc
)
target_include_directories(lusan_dt_document_tests PRIVATE ${LUSAN_BASE} ${LUSAN_THIRDPARTY})
target_compile_definitions(lusan_dt_document_tests PRIVATE
    ${COMMON_COMPILE_DEF}
    IMP_LOGGER_DLL
    # The reference document that ships beside the spec is validated by the same run.
    LUSAN_TEST_DATA_DIR="${LUSAN_ROOT}/tests/data"
)
target_link_libraries(lusan_dt_document_tests PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    areg::areg
    areg::aregextend
    areg::areglogger
    aregsqlite3
)
set_target_properties(lusan_dt_document_tests PROPERTIES WIN32_EXECUTABLE OFF)

# ---------------------------------------------------------------------------
# A service interface reading types out of an included data type document:
# qualified resolution, shadowing, the duplicate namespace, a broken include,
# and the re-read after the included file changes. Headless.
# ---------------------------------------------------------------------------
qt_add_executable(lusan_dt_import_tests
    ${LUSAN_SI_SUPPORT_SRC}
    ${LUSAN}/model/si/SIValidator.cpp
    ${LUSAN}/model/common/DocRuleChecks.cpp
    ${LUSAN}/model/common/LiteralValidator.cpp
    ${LUSAN_ROOT}/tests/dt/DTImportTests.cpp
    # Resolving uint32 and the basic containers needs the real predefined-type catalog,
    # which DataTypeFactory loads from the "Predefined Types" resource at runtime.
    ${LUSAN}/res/lusan.qrc
)
target_include_directories(lusan_dt_import_tests PRIVATE ${LUSAN_BASE} ${LUSAN_THIRDPARTY})
target_compile_definitions(lusan_dt_import_tests PRIVATE ${COMMON_COMPILE_DEF} IMP_LOGGER_DLL)
target_link_libraries(lusan_dt_import_tests PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    areg::areg
    areg::aregextend
    areg::areglogger
    aregsqlite3
)
set_target_properties(lusan_dt_import_tests PROPERTIES WIN32_EXECUTABLE OFF)

# ---------------------------------------------------------------------------
# The element table each document format is validated against: what the reader makes
# of a schema, that the copy delivered with the SDK is preferred over the compiled-in
# one, and that an element none of them defines is reported and dropped on save.
# ---------------------------------------------------------------------------
qt_add_executable(lusan_doc_schema_tests
    ${LUSAN_SI_SUPPORT_SRC}
    ${LUSAN_ROOT}/tests/common/DocSchemaTests.cpp
    ${LUSAN}/res/lusan.qrc
)
target_include_directories(lusan_doc_schema_tests PRIVATE ${LUSAN_BASE} ${LUSAN_THIRDPARTY})
target_compile_definitions(lusan_doc_schema_tests PRIVATE ${COMMON_COMPILE_DEF} IMP_LOGGER_DLL)
target_link_libraries(lusan_doc_schema_tests PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    areg::areg
    areg::aregextend
    areg::areglogger
    aregsqlite3
)
set_target_properties(lusan_doc_schema_tests PROPERTIES WIN32_EXECUTABLE OFF)

# ---------------------------------------------------------------------------
# An icon keeps up with the theme after it has been handed to a control, and a
# measured time is written in the unit that is configured.
# ---------------------------------------------------------------------------
qt_add_executable(lusan_theme_icon_tests
    ${LUSAN}/common/NELusanCommon.cpp
    ${LUSAN}/common/NETimeUnits.cpp
    ${LUSAN_ROOT}/tests/common/ThemeIconTests.cpp
    ${LUSAN}/res/lusan.qrc
)
target_include_directories(lusan_theme_icon_tests PRIVATE ${LUSAN_BASE} ${LUSAN_THIRDPARTY})
target_compile_definitions(lusan_theme_icon_tests PRIVATE ${COMMON_COMPILE_DEF} IMP_LOGGER_DLL)
target_link_libraries(lusan_theme_icon_tests PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    areg::areg
    areg::aregextend
    areg::areglogger
    aregsqlite3
)
set_target_properties(lusan_theme_icon_tests PROPERTIES WIN32_EXECUTABLE OFF)

enable_testing()
add_test(NAME doc_schema_tests COMMAND lusan_doc_schema_tests)
add_test(NAME theme_icon_tests COMMAND lusan_theme_icon_tests)
set_tests_properties(theme_icon_tests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
add_test(NAME sm_model_tests COMMAND lusan_sm_tests)
add_test(NAME sm_reference_tests COMMAND lusan_sm_reference_tests)
add_test(NAME sm_serialization_tests COMMAND lusan_sm_serialization_tests)
add_test(NAME sm_guard_tests COMMAND lusan_sm_guard_tests)
add_test(NAME sm_guard_catalog_tests COMMAND lusan_sm_guard_catalog_tests)
add_test(NAME sm_guard_ladder_tests COMMAND lusan_sm_guard_ladder_tests)
add_test(NAME sm_guard_eval_tests COMMAND lusan_sm_guard_eval_tests)
add_test(NAME sm_symbol_tests COMMAND lusan_sm_symbol_tests)
add_test(NAME sm_command_tests COMMAND lusan_sm_command_tests)
add_test(NAME sm_mapping_tests COMMAND lusan_sm_mapping_tests)
add_test(NAME sm_validation_tests COMMAND lusan_sm_validation_tests)
add_test(NAME si_command_tests COMMAND lusan_si_command_tests)
add_test(NAME dt_document_tests COMMAND lusan_dt_document_tests)
add_test(NAME dt_import_tests COMMAND lusan_dt_import_tests)

# The two standalone guard-editor harnesses run to completion (no app.exec) and
# return 0 on success, so they are safe ctest entries. Force the offscreen QPA
# platform so they never need a real display on CI or a headless run.
add_test(NAME sm_guard_ui_shots COMMAND lusan_sm_guard_ui_shots
         ${LUSAN_ROOT}/tests/data/GuardDemo.fsml ${CMAKE_CURRENT_BINARY_DIR}/guard-shots)
add_test(NAME sm_guard_u4_sweep COMMAND lusan_sm_guard_u4_sweep
         ${LUSAN_ROOT}/tests/data/GuardDemo.fsml ${CMAKE_CURRENT_BINARY_DIR}/guard-sweep)

# The canvas harness is the widest regression guard in the tree (toolbar/menu parity,
# panel placement, tools, search, kind marks). It has to be in the gate, or the features
# it protects regress unnoticed.
add_test(NAME sm_canvas_tests COMMAND lusan_sm_canvas_tests
         ${LUSAN_ROOT}/tests/data/TrafficLight.fsml ${CMAKE_CURRENT_BINARY_DIR}/canvas-shots)

# The drawn graph must be the document's graph. FullFeature.fsml is the acceptance document
# because eight of its eleven transitions carry no persisted route, so the audit lands on the
# auto-routed case.
add_test(NAME sm_l8_proof COMMAND lusan_sm_l8_proof
         ${LUSAN_ROOT}/tests/data/FullFeature.fsml ${CMAKE_CURRENT_BINARY_DIR}/l8-shots)

# The title must say "changed" from the first keystroke on, and say it again exactly when the
# document stands away from the point it was last written from.
add_test(NAME sm_dirty_mark COMMAND lusan_sm_dirty_mark
         ${LUSAN_ROOT}/tests/data/TrafficLight.fsml ${CMAKE_CURRENT_BINARY_DIR}/dirty-mark)

# The responsiveness gate. Its budgets are generous enough for a debug build on a loaded
# machine, so a failure means a real algorithmic regression, not timing noise.
add_test(NAME sm_perf COMMAND lusan_sm_perf ${CMAKE_CURRENT_BINARY_DIR}/perf)

# The four merged pages, over both document kinds. Guards the merges no window was ever
# free to look at, and leaves a picture of each page behind.
add_test(NAME design_pages COMMAND lusan_design_pages
         ${LUSAN_ROOT}/tests/data/TrafficLight.fsml ${CMAKE_CURRENT_BINARY_DIR}/design-pages)

set_tests_properties(sm_guard_ui_shots sm_guard_u4_sweep sm_canvas_tests sm_l8_proof sm_dirty_mark sm_perf design_pages PROPERTIES
                     ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# ---------------------------------------------------------------------------
# The log subsystem tests.
#
# The log data layer and the log controls carry few dependencies, so both targets
# build from a short source set rather than from the whole application. The model
# tests feed synthetic log entries, so neither a collector nor an archive is needed.
# ---------------------------------------------------------------------------
set(LUSAN_LOG_CORE_SRC
    ${LUSAN}/common/NELogPalette.cpp
    ${LUSAN}/common/NELusanCommon.cpp
    ${LUSAN}/common/NETimeUnits.cpp
    ${LUSAN}/model/common/TableModelBase.cpp
    ${LUSAN}/model/log/LogClockSkew.cpp
    ${LUSAN}/model/log/LogIconFactory.cpp
    ${LUSAN}/model/log/LogViewerFilter.cpp
    ${LUSAN}/model/log/LoggingModelBase.cpp
    ${LUSAN}/model/log/ScopeLogViewerFilter.cpp
    ${LUSAN}/data/log/ScopeNodeBase.cpp
    ${LUSAN}/data/log/ScopeNodes.cpp
)

qt_add_executable(lusan_log_model_tests
    ${LUSAN_LOG_CORE_SRC}
    ${LUSAN_ROOT}/tests/log/LogModelTests.cpp
    ${LUSAN}/res/lusan.qrc
)
target_include_directories(lusan_log_model_tests PRIVATE ${LUSAN_BASE} ${LUSAN_THIRDPARTY} ${LUSAN_ROOT})
target_compile_definitions(lusan_log_model_tests PRIVATE ${COMMON_COMPILE_DEF} IMP_LOGGER_DLL)
target_link_libraries(lusan_log_model_tests PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    areg::areg
    areg::aregextend
    areg::areglogger
    aregsqlite3
)
set_target_properties(lusan_log_model_tests PROPERTIES WIN32_EXECUTABLE OFF)

qt_add_executable(lusan_log_ui_tests
    ${LUSAN_LOG_CORE_SRC}
    ${LUSAN}/app/NEAppThemes.cpp
    ${LUSAN}/view/common/SearchLineEdit.cpp
    ${LUSAN}/view/log/LogEmptyState.cpp
    ${LUSAN}/view/log/LogPriorityBar.cpp
    ${LUSAN_ROOT}/tests/log/LogUiTests.cpp
    ${LUSAN}/res/lusan.qrc
)
target_include_directories(lusan_log_ui_tests PRIVATE ${LUSAN_BASE} ${LUSAN_THIRDPARTY} ${LUSAN_ROOT})
target_compile_definitions(lusan_log_ui_tests PRIVATE ${COMMON_COMPILE_DEF} IMP_LOGGER_DLL)
target_link_libraries(lusan_log_ui_tests PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    areg::areg
    areg::aregextend
    areg::areglogger
    aregsqlite3
)
set_target_properties(lusan_log_ui_tests PROPERTIES WIN32_EXECUTABLE OFF)

# The colour table, the column keys the workspace stores, the scopes the tree keeps out
# of the view, the view filter and the clock-skew measure.
add_test(NAME log_model_tests COMMAND lusan_log_model_tests)

# The log controls judged on what they paint: the ladder in both roles, the empty states
# and the charger. It leaves a picture of each behind.
add_test(NAME log_ui_tests COMMAND lusan_log_ui_tests ${CMAKE_CURRENT_BINARY_DIR}/log-shots)

set_tests_properties(log_model_tests log_ui_tests PROPERTIES
                     ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# ---------------------------------------------------------------------------
# The assembled log windows.
#
# LogViewerBase reaches the main window and the application, so this target is built
# from the application's own source list with the entry point taken out, plus the test
# driver. It is the only way to see the real windows rather than a widget on its own.
# ---------------------------------------------------------------------------
set(LUSAN_APP_NO_MAIN ${LUSAN_SRC})
list(REMOVE_ITEM LUSAN_APP_NO_MAIN "${LUSAN}/app/main.cpp")

# The header list of the application names main.cpp as well, so it has to lose it too,
# or the entry point comes back through the second list.
set(LUSAN_APP_HDR_NO_MAIN ${LUSAN_HDR})
list(REMOVE_ITEM LUSAN_APP_HDR_NO_MAIN "${LUSAN}/app/main.cpp")

qt_add_executable(lusan_log_window_shots
    ${LUSAN_APP_NO_MAIN}
    ${LUSAN_APP_HDR_NO_MAIN}
    ${LUSAN_UI}
    ${LUSAN_ROOT}/tests/log/LogWindowShots.cpp
    ${LUSAN}/res/lusan.qrc
)
target_include_directories(lusan_log_window_shots PRIVATE ${LUSAN_BASE} ${LUSAN_THIRDPARTY} ${LUSAN_ROOT})
target_compile_definitions(lusan_log_window_shots PRIVATE ${COMMON_COMPILE_DEF} IMP_LOGGER_DLL)
target_link_libraries(lusan_log_window_shots PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    areg::areg
    areg::aregextend
    areg::areglogger
    aregsqlite3
    ${LUSAN_ADS_TARGET}
)
set_target_properties(lusan_log_window_shots PROPERTIES WIN32_EXECUTABLE OFF)

# The parts the assembled windows must carry, and a picture of each window.
add_test(NAME log_window_shots COMMAND lusan_log_window_shots ${CMAKE_CURRENT_BINARY_DIR}/log-window-shots)
set_tests_properties(log_window_shots PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# The guarantees the assembled main window gives its document and log windows.
qt_add_executable(lusan_mdi_window_tests
    ${LUSAN_APP_NO_MAIN}
    ${LUSAN_APP_HDR_NO_MAIN}
    ${LUSAN_UI}
    ${LUSAN_ROOT}/tests/common/MdiWindowTests.cpp
    ${LUSAN}/res/lusan.qrc
)
target_include_directories(lusan_mdi_window_tests PRIVATE ${LUSAN_BASE} ${LUSAN_THIRDPARTY} ${LUSAN_ROOT})
target_compile_definitions(lusan_mdi_window_tests PRIVATE ${COMMON_COMPILE_DEF} IMP_LOGGER_DLL)
target_link_libraries(lusan_mdi_window_tests PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    areg::areg
    areg::aregextend
    areg::areglogger
    aregsqlite3
    ${LUSAN_ADS_TARGET}
)
set_target_properties(lusan_mdi_window_tests PROPERTIES WIN32_EXECUTABLE OFF)

add_test(NAME mdi_window_tests COMMAND lusan_mdi_window_tests)
set_tests_properties(mdi_window_tests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

