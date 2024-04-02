using System;
using System.Collections.Generic;
using System.Globalization;
using Avalonia.Data.Converters;

namespace AvaloniaUI.Views.Converters;

public class IsEqualConverter : IMultiValueConverter
{
    public static IsEqualConverter Instance { get; } = new();

    public object? Convert(IList<object?> values, Type targetType, object? parameter, CultureInfo culture)
    {
        return values.Count == 2 && values[0] == values[1];
    }
}