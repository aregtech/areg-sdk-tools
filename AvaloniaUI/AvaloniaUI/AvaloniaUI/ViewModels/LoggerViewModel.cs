using System;
using System.Collections.ObjectModel;
using AvaloniaUI.ViewModels.Msg;

namespace AvaloniaUI.ViewModels;

public sealed class LoggerItem(string message)
{
    public DateTime Timestamp { get; init; } = DateTime.Now;

    public string Message { get; init; } = message;
}

public class LoggerViewModel : ViewModelBase, ILoggerViewModel
{
    private readonly ObservableCollection<LoggerItem> _loggerItemsCollection = [];
    
    public ReadOnlyObservableCollection<LoggerItem> LoggerItems { get; }
    
    public LoggerItem? SelectedLoggerItem { get; set; }

    public LoggerViewModel()
    {
        LoggerItems = new ReadOnlyObservableCollection<LoggerItem>(_loggerItemsCollection);
        EventAggregator.GetEvent<LogMessage>().Subscribe(OnNewMessage);
    }

    private void OnNewMessage(string message)
    {
        _loggerItemsCollection.Add(new LoggerItem(message));
        SelectedLoggerItem = _loggerItemsCollection[^1];
    }
}