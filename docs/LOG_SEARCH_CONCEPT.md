# Log Search Concept

## Overview

This document outlines the comprehensive log search functionality for the Lusan log viewer application. The log search concept encompasses multiple features designed to provide powerful and intuitive search capabilities for analyzing log data.

## Features and Sub-tasks

### 1. Basic Text Search
**Description**: Fundamental text search functionality within log messages.

#### Sub-tasks:
- [x] **1.1 Simple Text Search**
  - Search for exact text matches in log messages
  - Case-sensitive and case-insensitive search options
  - Real-time search as user types (with debouncing)

- [ ] **1.2 Regular Expression Search**
  - Support for regex pattern matching
  - Regex validation and error handling
  - Common regex pattern shortcuts

- [x] **1.3 Search Input UI**
  - Search input field with search/clear buttons
  - Search options checkboxes (case sensitivity via context menu)
  - Keyboard shortcuts (Ctrl+F to open search, Escape to close, F3 for find next, Enter for search)

### 2. Advanced Filtering
**Description**: Multi-criteria filtering system for precise log data analysis.

#### Sub-tasks:
- [ ] **2.1 Priority-based Filtering**
  - Filter logs by priority levels (Error, Warning, Info, Debug, etc.)
  - Multiple priority selection
  - Quick priority filter buttons

- [ ] **2.2 Timestamp Range Filtering**
  - Date/time range picker interface
  - Predefined time ranges (Last hour, Today, Last 24h, etc.)
  - Custom timestamp format support

- [ ] **2.3 Source/Instance Filtering**
  - Filter by log source/instance names
  - Multi-select dropdown with search
  - Integration with existing instance query methods

- [ ] **2.4 Thread-based Filtering**
  - Filter logs by thread names/IDs
  - Thread selection interface
  - Integration with existing thread query methods

- [ ] **2.5 Scope-based Filtering**
  - Filter by log scope IDs
  - Hierarchical scope selection
  - Integration with existing scope query methods

### 3. Search Navigation
**Description**: Tools for navigating through search results efficiently.

#### Sub-tasks:
- [x] **3.1 Find Next/Previous**
  - Navigation buttons and keyboard shortcuts (F3/Shift+F3)
  - Wrap-around search from end to beginning
  - Current match highlighting

- [x] **3.2 Results Counter**
  - Display current match position and total matches
  - Real-time update as search criteria changes
  - "No results found" indication (visual feedback via background color)

- [ ] **3.3 Jump to Result**
  - Jump to specific search result by number
  - Jump to first/last match shortcuts
  - Auto-scroll to bring matches into view

### 4. Search Highlighting
**Description**: Visual highlighting of search matches for better visibility.

#### Sub-tasks:
- [ ] **4.1 Text Highlighting**
  - Highlight search matches in log message text
  - Customizable highlight colors
  - Multiple match highlighting in single log entry

- [ ] **4.2 Row Highlighting**
  - Highlight entire rows containing matches
  - Alternating colors for better visual separation
  - Current match vs. other matches distinction

- [ ] **4.3 Highlight Management**
  - Clear highlighting option
  - Persistent highlighting during navigation
  - Highlight performance optimization

### 5. Search History and Bookmarks
**Description**: Remember and manage search patterns and frequently accessed log entries.

#### Sub-tasks:
- [ ] **5.1 Search History**
  - Remember recent search terms
  - Search history dropdown
  - Clear search history option

- [ ] **5.2 Search Bookmarks**
  - Save frequently used search patterns
  - Named bookmarks for complex search criteria
  - Bookmark management (add, edit, delete)

- [ ] **5.3 Quick Filters**
  - Predefined common search patterns
  - One-click application of frequent filters
  - Customizable quick filter buttons

### 6. Search Performance and Optimization
**Description**: Ensure search functionality performs well with large log datasets.

#### Sub-tasks:
- [ ] **6.1 Search Indexing**
  - Background indexing of log content for faster searches
  - Incremental index updates for new log entries
  - Index management and optimization

- [ ] **6.2 Progressive Search**
  - Show partial results while search is in progress
  - Cancel long-running searches
  - Search progress indication

- [ ] **6.3 Search Result Caching**
  - Cache search results for repeated queries
  - Cache invalidation on data changes
  - Memory management for cached results

## Implementation Architecture

### Core Components

1. **LogSearchEngine**: Central search logic and algorithms
2. **SearchFilter**: Individual filter implementations
3. **SearchHighlighter**: Text and row highlighting functionality
4. **SearchNavigator**: Navigation through search results
5. **SearchHistory**: Storage and management of search history
6. **SearchUI**: User interface components for search

### Integration Points

- **LogViewerModel**: Integration with existing log data model
- **LogHeaderItem**: Extension of existing filtering infrastructure
- **LogViewer**: UI integration with main log viewer window
- **LogObserver**: Real-time search updates for live log data

## Technical Considerations

### Performance Requirements
- Search should complete within 100ms for datasets up to 10,000 log entries
- UI should remain responsive during search operations
- Memory usage should scale appropriately with dataset size

### Usability Requirements
- Search interface should be intuitive and discoverable
- Keyboard shortcuts should follow standard conventions
- Search state should persist during session

### Compatibility Requirements
- Maintain compatibility with existing filtering mechanisms
- Support all existing log data formats and sources
- Preserve existing log viewer functionality

## Implementation Priority

1. **Phase 1**: Basic Text Search (Features 1.1, 1.3) ✅ **COMPLETED**
2. **Phase 2**: Search Navigation and Highlighting (Features 3.1, 3.2, 4.1) ✅ **PARTIALLY COMPLETED**
3. **Phase 3**: Advanced Filtering (Features 2.1, 2.2, 2.3)
4. **Phase 4**: Search History and Additional Features (Feature 5, remaining items)
5. **Phase 5**: Performance Optimization (Feature 6)

## Success Criteria

- Users can quickly find specific log entries using text search
- Complex filtering scenarios are supported and perform well
- Search functionality integrates seamlessly with existing log viewer
- Search operations provide immediate visual feedback
- Large log datasets can be searched efficiently