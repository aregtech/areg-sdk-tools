# Log Search Usage Guide

## Overview

The Log Viewer now includes comprehensive search functionality to help you quickly find specific log entries. This guide covers all available search features and how to use them effectively.

## Basic Search

### Quick Search
1. **Focus the search field**: Click in the search text field or press `Ctrl+F`
2. **Enter search text**: Type the text you want to find in log entries
3. **Execute search**: Press `Enter` or click the "Search" button
4. **Find next**: Press `F3` or click "Search" again to find the next occurrence

### Search Controls
- **Search Field**: Located in the toolbar, accepts any text input
- **Search Button**: Executes the search and finds next occurrence
- **Clear Search**: Press `Escape` to clear the search field

## Advanced Search Options

### Case Sensitivity
- **Default**: Search is case-insensitive
- **Enable**: Right-click on search field → Check "Case Sensitive"
- **Disable**: Right-click on search field → Uncheck "Case Sensitive"

### Column-Specific Search
- **All Columns** (default): Searches across all visible columns
- **Specific Column**: Right-click on search field → "Search In" → Select column
- **Available Columns**:
  - Priority: Log message priority level
  - Timestamp: Log message timestamp
  - Source: Log message source name
  - Source ID: Log message source identifier
  - Thread: Thread name that generated the log
  - Thread ID: Thread identifier
  - Scope ID: Log scope identifier
  - Message: The actual log message text

## Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| `Ctrl+F` | Focus search field and select all text |
| `Enter` | Execute search (when search field is focused) |
| `F3` | Find next occurrence |
| `Escape` | Clear search field and focus log table |

## Visual Feedback

### Search Results
- **Found**: Matching row is automatically selected and centered in view
- **Not Found**: Search field background turns light red
- **Continuing Search**: Next occurrence is found when pressing F3 or clicking Search again

### Search State
- **Active Search**: Search field retains the last search term
- **Reset**: Background color resets when typing new text
- **Clear**: All search state is cleared when logs are cleared

## Tips and Best Practices

### Effective Searching
1. **Use specific terms**: More specific search terms yield better results
2. **Column filtering**: Limit search to specific columns for faster and more precise results
3. **Case sensitivity**: Use case-sensitive search for exact matches when needed
4. **Iterative search**: Use F3 to quickly navigate through multiple matches

### Common Use Cases
- **Error Investigation**: Search in "Priority" column for "Error" or "Fatal"
- **Thread Analysis**: Search in "Thread" column for specific thread names
- **Time-based Search**: Search in "Message" column for specific error codes or keywords
- **Source Debugging**: Search in "Source" column for specific component names

### Workflow Integration
1. **Quick Filter**: Use search to quickly locate relevant log entries
2. **Context Navigation**: Once found, use standard table navigation to examine surrounding entries
3. **Multi-criteria**: Combine column-specific search with case sensitivity for precise filtering
4. **Continuous Monitoring**: Search remains active while new logs arrive

## Troubleshooting

### No Results Found
- **Check spelling**: Verify the search term is spelled correctly
- **Try different case**: Toggle case sensitivity if needed
- **Expand scope**: Switch from column-specific to "All Columns" search
- **Check filters**: Ensure the log entries you're looking for are visible

### Performance
- **Large datasets**: Column-specific search performs better on large log files
- **Complex terms**: Simple text search is faster than complex patterns
- **Regular updates**: Search state persists as new log entries arrive

## Future Enhancements

The search functionality is designed to be extensible. Planned enhancements include:
- Regular expression support
- Search result highlighting
- Search history and bookmarks
- Advanced filtering combinations
- Search result statistics

## Related Features

- **Column Filtering**: Use existing header filters in combination with search
- **Log Priority Controls**: Combine search with priority-level filtering
- **Table Navigation**: Use standard table selection and scrolling after finding results