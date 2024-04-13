using System;
using System.Collections.ObjectModel;
using System.Reactive.Linq;
using System.Threading.Tasks;
using System.Windows.Input;
using AvaloniaUI.ViewModels.Msg;
using ReactiveUI;

namespace AvaloniaUI.ViewModels;

public sealed class LoggerItem(string message)
{
    public DateTime Timestamp { get; init; } = DateTime.Now;

    public string Message { get; init; } = message;
}

public class LoggerViewModel : ViewModelBase, ILoggerViewModel
{
    private readonly ObservableCollection<LoggerItem> _loggerItemsCollection = [];
    private LoggerItem? _selectedLoggerItem;

    public ReadOnlyObservableCollection<LoggerItem> LoggerItems { get; }

    public LoggerItem? SelectedLoggerItem
    {
        get => _selectedLoggerItem;
        set => this.RaiseAndSetIfChanged(ref _selectedLoggerItem, value);
    }

    public ICommand ClearCommand { get; }

    public LoggerViewModel()
    {
        LoggerItems = new ReadOnlyObservableCollection<LoggerItem>(_loggerItemsCollection);
        EventAggregator.GetEvent<LogMessage>().Subscribe(OnNewMessage);
        
        // create pipeline to activate / de-activate the Clear command
        var hasSelectedErrorItem = this.WhenAnyValue(x => x.SelectedLoggerItem)
            .Select(x => x != null);
        ClearCommand = ReactiveCommand.CreateFromTask(ClearItems, hasSelectedErrorItem);
    }

    private void OnNewMessage(string message)
    {
        _loggerItemsCollection.Add(new LoggerItem(message));
        SelectedLoggerItem = _loggerItemsCollection[^1];
    }

    private Task ClearItems()
    {
        _loggerItemsCollection.Clear();
        SelectedLoggerItem = null;
        return Task.CompletedTask;
    }
}