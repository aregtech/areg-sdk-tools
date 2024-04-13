using System.Collections.ObjectModel;
using System.Windows.Input;

namespace AvaloniaUI.ViewModels;

public interface ILoggerViewModel
{
    ReadOnlyObservableCollection<LoggerItem> LoggerItems { get; }
    
    LoggerItem? SelectedLoggerItem { get; set; }
    
    public ICommand ClearCommand { get; }
}