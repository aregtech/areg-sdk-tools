# LogOfflineScopesModel

## Overview

The `LogOfflineScopesModel` provides scope navigation functionality for offline log data stored in database files. It is similar to the `LogScopesModel` but specifically designed for offline mode, working with data from `LogOfflineModel` instead of live connections.

## Purpose

This model was created to address the need for navigating log scopes when working with offline/historical log data stored in database files. It enables users to browse through scope hierarchies and filter log data by specific scopes in offline mode.

## Key Features

- **Offline Navigation**: Works with local database files without requiring live connection
- **Tree Structure**: Organizes scopes in a hierarchical tree for easy navigation
- **Instance Separation**: Shows different service instances as separate root nodes
- **Scope Hierarchy**: Preserves the original scope naming hierarchy (e.g., `Service::Component::Scope`)
- **Priority Information**: Displays logging priority for each scope
- **Qt Integration**: Fully compatible with Qt's model/view framework

## Architecture

The model inherits from `QAbstractItemModel` and uses the existing ScopeNodes hierarchy:

```
ScopeNodeBase (base class)
├── ScopeLeaf (leaf nodes - actual scopes)
├── ScopeNode (intermediate nodes)
└── ScopeRoot (root nodes - service instances)
```

## Usage

```cpp
// 1. Create and open offline model
LogOfflineModel* offlineModel = new LogOfflineModel(this);
if (offlineModel->openDatabase("/path/to/logfile.db")) {
    
    // 2. Create offline scopes model
    LogOfflineScopesModel* scopesModel = new LogOfflineScopesModel(this);
    
    // 3. Initialize with offline data
    if (scopesModel->initialize(offlineModel)) {
        
        // 4. Use with QTreeView
        QTreeView* treeView = new QTreeView(this);
        treeView->setModel(scopesModel);
        
        // 5. Handle selection changes
        connect(treeView->selectionModel(), &QItemSelectionModel::currentChanged,
                this, &MyWidget::onScopeSelectionChanged);
    }
}
```

## Tree Structure Example

```
📁 Service Instance 1 (1001)
├── 📁 MyService
│   ├── 📁 Component1
│   │   ├── 📄 InitScope (Priority: Debug)
│   │   └── 📄 WorkScope (Priority: Info)
│   └── 📁 Component2
│       └── 📄 ProcessScope (Priority: Warning)
└── 📄 GlobalScope (Priority: Error)

📁 Service Instance 2 (1002)
└── 📁 AnotherService
    └── 📄 MainScope (Priority: Info)
```

## Integration

The model is designed to work alongside existing components:

- **LogOfflineModel**: Provides data access layer for log messages and metadata
- **LogOfflineScopesModel**: Provides navigation layer for scope organization
- **LogScopesModel**: Similar functionality but for live/online mode

## Implementation Details

### Data Population

1. Gets instance information using `LogOfflineModel::getLogInstanceInfos()`
2. For each instance, retrieves scopes using `LogOfflineModel::getLogInstScopes()`
3. Builds tree structure using `ScopeRoot::makeChildNode()` and `addChildNode()`

### Tree Navigation

- Root level: Service instances from the database
- Child levels: Scope hierarchy parsed from scope names (e.g., "Service::Component::Scope")
- Leaf nodes: Actual log scopes with priority information

### Signals

- `signalRootUpdated(const QModelIndex& root)`: Emitted when root is updated
- `signalScopesInserted(const QModelIndex& parent)`: Emitted when scopes are inserted
- `signalScopesUpdated(const QModelIndex& parent)`: Emitted when scopes are updated

## Files

- `sources/lusan/model/log/LogOfflineScopesModel.hpp` - Header file
- `sources/lusan/model/log/LogOfflineScopesModel.cpp` - Implementation file
- `examples/offline-scope-navigation-example.cpp` - Usage example

## Building

The model is automatically included in the build when building the main Lusan application. It has been added to the CMakeLists.txt in the log model directory.

## Testing

The model syntax has been validated and compiles correctly. For full testing, build the complete Lusan application and test with offline log database files.