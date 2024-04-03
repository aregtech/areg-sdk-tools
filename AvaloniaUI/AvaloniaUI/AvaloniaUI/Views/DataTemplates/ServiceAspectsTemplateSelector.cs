using System;
using System.Collections.Generic;
using Avalonia.Controls;
using Avalonia.Controls.Templates;
using Avalonia.Metadata;
using AvaloniaUI.ViewModels;

namespace AvaloniaUI.Views.DataTemplates
{
    public class ServiceAspectsTemplateSelector : IDataTemplate
    {
        // This Dictionary should store our aspects. We mark this as [Content],
        // so we can directly add elements to it later.
        // ReSharper disable once CollectionNeverUpdated.Global
        [Content] public Dictionary<string, IDataTemplate> AvailableTemplates { get; } = new();

        // Build the DataTemplate here
        public Control Build(object? param)
        {
            // Our Keys in the dictionary are strings, so we call .ToString() to get the key to look up
            var key = param?.ToString();
            if (key is null) // If the key is null, we throw an ArgumentNullException
            {
                throw new ArgumentNullException(nameof(param));
            }

            return
                // finally we look up the provided key and let the System build the DataTemplate for us
                AvailableTemplates[key]
                    .Build(param) ?? throw new InvalidOperationException(); 
        }

        // Check if we can accept the provided data
        public bool Match(object? data)
        {
            // Our Keys in the dictionary are strings, so we call .ToString() to get the key to look up
            var key = data?.ToString();

            return data is ServiceAspectType // the provided data needs to be our enum type
                   && !string.IsNullOrEmpty(key) // and the key must not be null or empty
                   && AvailableTemplates.ContainsKey(key); // and the key must be found in our Dictionary
        }
    }
}