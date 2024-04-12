using Avalonia.Controls;

namespace AvaloniaUI.Views;

/// <summary>
/// A control that visualizes a list of errors.
/// </summary>
public partial class LoggerView : UserControl
{
    public LoggerView() => this.InitializeComponent();

    private void ErrorListDataGrid_OnSelectionChanged(object? sender, SelectionChangedEventArgs e)
    {
        if (sender is DataGrid dataGrid && e.AddedItems.Count > 0)
        {
            dataGrid.ScrollIntoView(e.AddedItems[0], null);
        }
    }
}
