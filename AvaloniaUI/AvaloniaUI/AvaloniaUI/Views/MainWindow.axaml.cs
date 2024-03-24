using Avalonia;
using Avalonia.Controls;
using Avalonia.Styling;

namespace AvaloniaUI.Views;

public partial class MainWindow : Window
{
    public MainWindow()
    {
        InitializeComponent();

        // For switching between Light & Dark themes:
        ThemeVariants.SelectedItem = Application.Current!.RequestedThemeVariant;
        ThemeVariants.SelectionChanged += (_, _) =>
        {
            if (ThemeVariants.SelectedItem is ThemeVariant themeVariant)
            {
                Application.Current.RequestedThemeVariant = themeVariant;
            }
        };
    }
}