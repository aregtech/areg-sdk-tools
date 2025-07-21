list(APPEND LUSAN_SRC
    ${LUSAN}/model/log/LiveLogsModel.cpp
    ${LUSAN}/model/log/LiveScopesModel.cpp
    ${LUSAN}/model/log/LoggingModelBase.cpp
    ${LUSAN}/model/log/LoggingScopesModelBase.cpp
    ${LUSAN}/model/log/LogIconFactory.cpp
    ${LUSAN}/model/log/LogSearchModel.cpp
    ${LUSAN}/model/log/LogViewerFilterProxy.cpp
    ${LUSAN}/model/log/OfflineLogsModel.cpp
    ${LUSAN}/model/log/OfflineScopesModel.cpp
)

list(APPEND LUSAN_HDR
    ${LUSAN}/model/log/LiveLogsModel.hpp
    ${LUSAN}/model/log/LiveScopesModel.hpp
    ${LUSAN}/model/log/LoggingModelBase.hpp
    ${LUSAN}/model/log/LoggingScopesModelBase.hpp
    ${LUSAN}/model/log/LogIconFactory.hpp
    ${LUSAN}/model/log/LogSearchModel.hpp
    ${LUSAN}/model/log/LogViewerFilterProxy.hpp
    ${LUSAN}/model/log/OfflineLogsModel.hpp
    ${LUSAN}/model/log/OfflineScopesModel.hpp
)
