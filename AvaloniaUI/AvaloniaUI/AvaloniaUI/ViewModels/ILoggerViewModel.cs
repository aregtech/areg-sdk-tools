using System.Collections.ObjectModel;

namespace AvaloniaUI.ViewModels;

public interface ILoggerViewModel
{
    ReadOnlyObservableCollection<LoggerItem> LoggerItems { get; }
    
    LoggerItem? SelectedLoggerItem { get; set; }
}