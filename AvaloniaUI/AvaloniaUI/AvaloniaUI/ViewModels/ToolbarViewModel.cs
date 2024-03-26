using System.Collections.ObjectModel;
using Avalonia.Controls;
using ReactiveUI;

namespace AvaloniaUI.ViewModels;

public class ToolbarViewModel : ViewModelBase
{
    private ObservableCollection<object> _items = new();

    // ReSharper disable once EmptyConstructor
    public ToolbarViewModel()
    {
        for (int i = 0; i < 5; i++)
        {
            Items.Add(new Button
            {
                Content = $"Button {i + 1}"
            });
        }
    }

    public ObservableCollection<object> Items
    {
        get => _items;
        set => this.RaiseAndSetIfChanged(ref _items, value);
    }
}