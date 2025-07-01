/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]aregtech.com.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        examples/offline-scope-navigation-example.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Example usage of LogOfflineScopesModel for offline scope navigation.
 *
 ************************************************************************/

/*

EXAMPLE USAGE OF LogOfflineScopesModel:

This example demonstrates how to use the LogOfflineScopesModel to provide
scope navigation functionality for offline log data stored in database files.

// 1. Create the offline model and open a database
LogOfflineModel* offlineModel = new LogOfflineModel(this);
if (offlineModel->openDatabase("/path/to/logfile.db")) {
    
    // 2. Create the offline scopes model
    LogOfflineScopesModel* scopesModel = new LogOfflineScopesModel(this);
    
    // 3. Initialize the scopes model with offline data
    if (scopesModel->initialize(offlineModel)) {
        
        // 4. Use the model with a QTreeView for navigation
        QTreeView* scopeTreeView = new QTreeView(this);
        scopeTreeView->setModel(scopesModel);
        
        // 5. Connect signals to handle scope selection
        connect(scopeTreeView->selectionModel(), &QItemSelectionModel::currentChanged,
                this, &MyWidget::onScopeSelectionChanged);
                
        // 6. The model will automatically build the scope tree from database:
        //    - Root nodes represent different service instances
        //    - Child nodes represent scope hierarchies (e.g., "MyService::Component::Scope")
        //    - Leaf nodes represent actual log scopes with priority information
        
        // 7. Example tree structure that would be built:
        //    📁 Service Instance 1 (1001)
        //    ├── 📁 MyService
        //    │   ├── 📁 Component1
        //    │   │   ├── 📄 InitScope (Priority: Debug)
        //    │   │   └── 📄 WorkScope (Priority: Info)
        //    │   └── 📁 Component2
        //    │       └── 📄 ProcessScope (Priority: Warning)
        //    └── 📄 GlobalScope (Priority: Error)
        //    📁 Service Instance 2 (1002)
        //    └── 📁 AnotherService
        //        └── 📄 MainScope (Priority: Info)
        
    } else {
        // Handle initialization failure
        qWarning() << "Failed to initialize offline scopes model";
    }
} else {
    // Handle database open failure
    qWarning() << "Failed to open offline database";
}

// Example slot to handle scope selection
void MyWidget::onScopeSelectionChanged(const QModelIndex& current, const QModelIndex& previous) {
    if (current.isValid()) {
        ScopeNodeBase* node = static_cast<ScopeNodeBase*>(current.internalPointer());
        if (node != nullptr) {
            QString scopePath = node->makePath();
            qDebug() << "Selected scope:" << scopePath;
            
            // You can now filter the LogOfflineModel to show only messages from this scope
            // or perform other navigation operations
        }
    }
}

KEY FEATURES:

1. **Offline Navigation**: Works with local database files without requiring live connection
2. **Tree Structure**: Organizes scopes in a hierarchical tree for easy navigation
3. **Instance Separation**: Shows different service instances as separate root nodes
4. **Scope Hierarchy**: Preserves the original scope naming hierarchy (e.g., Service::Component::Scope)
5. **Priority Information**: Displays logging priority for each scope
6. **Qt Integration**: Fully compatible with Qt's model/view framework

INTEGRATION WITH EXISTING CODE:

The LogOfflineScopesModel is designed to work alongside the existing LogOfflineModel:
- LogOfflineModel provides the data access layer for log messages and metadata
- LogOfflineScopesModel provides the navigation layer for scope organization
- Both can be used together to create a complete offline log viewing experience

This model is similar to LogScopesModel but specifically designed for offline mode:
- LogScopesModel: For live/online log scope navigation with real-time updates
- LogOfflineScopesModel: For offline log scope navigation from database files

*/