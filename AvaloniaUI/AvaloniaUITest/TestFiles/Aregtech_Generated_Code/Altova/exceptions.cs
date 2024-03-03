// exceptions.cs
// This file contains generated code and will be overwritten when you rerun code generation.


using System;
using Altova;

namespace Altova.Types
{
	[Serializable]
	public class ConversionException : Exception
	{
		public ConversionException(string text)
			: base(text)
		{
		}

		public ConversionException(string text, Exception inner)
			: base(text, inner)
		{
		}
	}

	/// <summary>
	/// The given string could not be parsed to the value of the given schema-type.
	/// There are formating (or other) errors.
	/// </summary>
	[Serializable]
	public class StringParseException : ConversionException
	{
		public StringParseException(string text) 
			: base( text )
		{
		}

		public StringParseException(string text, Exception other) 
			: base( text, other )
		{
		}
	}


	[Serializable]
	public class DataSourceUnavailableException : Exception
	{
		public DataSourceUnavailableException(string message)
			: base(message) { }
		public DataSourceUnavailableException(string message, Exception inner)
			: base(message, inner) { }
	}

	[Serializable]
	public class DataTargetUnavailableException : Exception
	{
		public DataTargetUnavailableException(string message)
			: base(message) { }
		public DataTargetUnavailableException(string message, Exception inner)
			: base(message, inner) { }
	}

	[Serializable]
	public class TargetUpdateFailureException : Exception
	{
		public TargetUpdateFailureException(string message)
			: base(message) { }
		public TargetUpdateFailureException(string message, Exception inner)
			: base(message, inner) { }

	}
}