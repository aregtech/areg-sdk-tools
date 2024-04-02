using System;
using Avalonia.Controls;
using Avalonia.Controls.Templates;
using AvaloniaUI.ViewModels;

namespace AvaloniaUI.Views.DataTemplates;

/// <summary>
/// Part of the solution for the TabControl sharing its content template amongst all tabs.
/// Got it from "AvaloniaInterop.zip", <see href="https://github.com/AvaloniaUI/Avalonia/discussions/11837"/>.
/// Currently not used.
/// </summary>
public class ViewLocatorForTabControl : IDataTemplate
{
    public Control Build(object? data)
    {
        var name = data!.GetType().FullName!.Replace("ViewModel", "View");
        var type = Type.GetType(name);

        if (type == null) 
            return new TextBlock {Text = "Not Found: " + name};
        
        var view = (Control)Activator.CreateInstance(type)!; // Create View
        
        // Set ViewModel as DataContext. NOTE: Setting this differs from other View-Locator versions!
        view.DataContext = data;
        return view;
    }

    public bool Match(object? data) => data is ViewModelBase;
}