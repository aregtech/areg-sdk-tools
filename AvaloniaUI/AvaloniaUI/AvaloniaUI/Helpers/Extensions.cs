using System;

namespace AvaloniaUI.Helpers;

public static class Extensions
{
    public static T ToEnum<T>(this string value)
    {
        return (T) Enum.Parse(typeof(T), value, true);
    }
}