list(APPEND LUSAN_SRC
    ${LUSAN}/model/common/DataTypesModel.cpp
    ${LUSAN}/model/common/DocCommand.cpp
    ${LUSAN}/model/common/DocModelNotifier.cpp
    ${LUSAN}/model/common/FileSystemEntry.cpp
    ${LUSAN}/model/common/FileSystemFilter.cpp
    ${LUSAN}/model/common/FileSystemModel.cpp
    ${LUSAN}/model/common/TableModelBase.cpp
    ${LUSAN}/model/common/ReplyMethodModel.cpp
    ${LUSAN}/model/common/WorkspaceModel.cpp
)

list(APPEND LUSAN_HDR
    ${LUSAN}/model/common/DataTypesModel.hpp
    ${LUSAN}/model/common/DocCommand.hpp
    ${LUSAN}/model/common/DocElementCommands.hpp
    ${LUSAN}/model/common/DocModelNotifier.hpp
    ${LUSAN}/model/common/FileSystemEntry.hpp
    ${LUSAN}/model/common/FileSystemFilter.hpp
    ${LUSAN}/model/common/FileSystemModel.hpp
    ${LUSAN}/model/common/TableModelBase.hpp
    ${LUSAN}/model/common/ReplyMethodModel.hpp
    ${LUSAN}/model/common/WorkspaceModel.hpp
)
