// xstypes.cs 
// This file contains generated code and will be overwritten when you rerun code generation.

using Altova.TypeInfo;
using Altova.Types;
using System.Xml;
using System.Text;
using System.Collections.Generic;

namespace Altova.Xml
{
	
	public class XmlFormatter : ValueFormatter
	{
		public virtual string Format(DateTime v)
		{
			return CoreTypes.CastToString(v);
		}
		
		public virtual string Format(Duration v)
		{
			return CoreTypes.CastToString(v);
		}
		
		public virtual string Format(long v)
		{
			return CoreTypes.CastToString(v);
		}
		
		public virtual string Format(ulong v)
		{
			return CoreTypes.CastToString(v);
		}
		
		public virtual string Format(double v)
		{
			return CoreTypes.CastToString(v);
		}
		
		public virtual string Format(string v)
		{
			return CoreTypes.CastToString(v);
		}
		
		public virtual string Format(byte[] v)
		{
			return System.Convert.ToBase64String(v);
		}
		
		public virtual string Format(bool v)
		{
			return CoreTypes.CastToString(v);
		}
		
		public virtual string Format(decimal v)
		{
			return CoreTypes.CastToString(v);
		}
		
		public virtual byte[] ParseBinary(string v)
		{
			return System.Convert.FromBase64String(v);
		}
	}
	
	public class XmlTimeFormatter : XmlFormatter
	{
		public override string Format(DateTime dt)
		{
			return dt.ToString(Altova.Types.DateTimeFormat.W3_time);
		}
	}
	
	public class XmlDateFormatter : XmlFormatter
	{
		public override string Format(DateTime dt)
		{
			return dt.ToString(Altova.Types.DateTimeFormat.W3_date);
		}
	}
	
	class XmlGYearFormatter : XmlFormatter
	{
		public override string Format(DateTime dt)
		{
			return dt.ToString(Altova.Types.DateTimeFormat.W3_gYear);
		}
	}
	
	class XmlGMonthFormatter : XmlFormatter
	{
		public override string Format(DateTime dt)
		{
			return dt.ToString(Altova.Types.DateTimeFormat.W3_gMonth);
		}
	}


	class XmlGDayFormatter : XmlFormatter
	{
		public override string Format(DateTime dt)
		{
			return dt.ToString(Altova.Types.DateTimeFormat.W3_gDay);
		}
	}
	
	class XmlGYearMonthFormatter : XmlFormatter
	{
		public override string Format(DateTime dt)
		{
			return dt.ToString(Altova.Types.DateTimeFormat.W3_gYearMonth);
		}
	}
	
	class XmlGMonthDayFormatter : XmlFormatter
	{
		public override string Format(DateTime dt)
		{
			return dt.ToString(Altova.Types.DateTimeFormat.W3_gMonthDay);
		}
	}
	
	class XmlHexBinaryFormatter : XmlFormatter
	{
		public override string Format(byte[] v)
		{
			return Altova.HexBinary.decode(v);
		}

		public override byte[] ParseBinary(string s)
		{
			return Altova.HexBinary.encode(s);
		}
	}
	
	class XmlIntegerFormatter : XmlFormatter
	{
		public override string Format(double v)
		{
			return CoreTypes.CastToString((long) v);
		}
	}
	public class Xs
	{
		public readonly static ValueFormatter StandardFormatter = new XmlFormatter();
		public readonly static ValueFormatter TimeFormatter = new XmlTimeFormatter();
		public readonly static ValueFormatter DateFormatter = new XmlDateFormatter();
		public readonly static ValueFormatter DateTimeFormatter = StandardFormatter;
		public readonly static ValueFormatter GYearFormatter = new XmlGYearFormatter();
		public readonly static ValueFormatter GMonthFormatter = new XmlGMonthFormatter();
		public readonly static ValueFormatter GDayFormatter = new XmlGDayFormatter();
		public readonly static ValueFormatter GYearMonthFormatter = new XmlGYearMonthFormatter();
		public readonly static ValueFormatter GMonthDayFormatter = new XmlGMonthDayFormatter();
		public readonly static ValueFormatter HexBinaryFormatter = new XmlHexBinaryFormatter();
		public readonly static ValueFormatter IntegerFormatter = new XmlIntegerFormatter();
		public readonly static ValueFormatter DecimalFormatter = StandardFormatter;
		public readonly static ValueFormatter AnySimpleTypeFormatter = StandardFormatter;
		public readonly static ValueFormatter DurationFormatter = StandardFormatter;
		public readonly static ValueFormatter DoubleFormatter = StandardFormatter;
		public readonly static ValueFormatter Base64BinaryFormatter = StandardFormatter;
	}
		
	public class XsValidation
	{
		public static string ReplaceWhitespace(string input)
		{
			return input; 
		}
		
		public static string CollapseWhitespace(string input)
		{
			return input; 
		}
		
		public static bool Validate(string input, TypeInfo.TypeInfo info)
		{
			return true; 
		}
		
		private class LengthFacetCheckHelper
		{
			public static bool IsWhitespace(char c) { return c == '\t' || c =='\n' || c == '\r' || c == ' '; }
			
			public static int ComputeLength(string value, WhitespaceType whitespaceNormalization)
			{
				if (whitespaceNormalization == WhitespaceType.Collapse)
				{
					int length = 0;
					bool pendingSpace = false;
					for (int i=0; i< value.Length; i++)
					{
						if (IsWhitespace(value[i]))
						{
							if (length != 0)
								pendingSpace=true;
						}
						else
						{
							if (pendingSpace)
							{
								length += 1;
								pendingSpace=false;
							}
							length += 1;
						}
					}
					return length;
				}
				return value.Length;
			}
			
			public static bool IsEqual(string value, string normalizedCompare, WhitespaceType whitespaceNormalization)
			{
				if (whitespaceNormalization == WhitespaceType.Collapse)
				{
					bool flag =false;
					bool pendingSpace = false;
					for (int i=0, j=0; i< value.Length; ++i)
					{
						if (IsWhitespace(value[i]))
						{
							if (flag)
								pendingSpace = true;
						}
						else
						{
							flag = true;
							if (j == normalizedCompare.Length)
								return false;
							if (pendingSpace)
							{
								if (normalizedCompare[j] != ' ')
									return false;
								++j;
								if (j == normalizedCompare.Length)
									return false;
								pendingSpace = false;
							}
							if (value[i] != normalizedCompare[j])
								return false;
							++j;
						}
					}
					return true;
				}
				else if (whitespaceNormalization == WhitespaceType.Replace)
				{
					int i=0, j=0;
					for (; i< value.Length && j<normalizedCompare.Length; ++i, ++j)
					{
						if(IsWhitespace(value[i]))
						{
							if (normalizedCompare[j] != ' ')
								return false;
						}
						else
						{
							if (value[i] != normalizedCompare[j])
								return false;
						}
					}
					if((i == value.Length) != (j == normalizedCompare.Length))
						return false;
					return true;
				}
				return value == normalizedCompare;
			}
		}
		
		public class FacetCheck_Success : FacetCheckInterface
		{
			public static FacetCheckResult Check(string s, FacetInfo facet, WhitespaceType whitespace)
			{
				return FacetCheckResult.Success;
			}
		}
		
		public class FacetCheck_string_length : FacetCheckInterface
		{
			public static FacetCheckResult Check(string value, FacetInfo facet, WhitespaceType whitespaceNormalization)
			{
				if (LengthFacetCheckHelper.ComputeLength(value, whitespaceNormalization) == facet.intValue)
					return FacetCheckResult.Success;
				return FacetCheckResult.Fail;
			}
		}

		public class FacetCheck_string_minLength : FacetCheckInterface
		{
			public static FacetCheckResult Check(string value, FacetInfo facet, WhitespaceType whitespaceNormalization)
			{
				if (LengthFacetCheckHelper.ComputeLength(value, whitespaceNormalization) >= facet.intValue)
					return FacetCheckResult.Success;
				return FacetCheckResult.Fail;
			}
		}
		
		public class FacetCheck_string_maxLength : FacetCheckInterface
		{
			public static FacetCheckResult Check(string value, FacetInfo facet, WhitespaceType whitespaceNormalization)
			{
				if (LengthFacetCheckHelper.ComputeLength(value, whitespaceNormalization) <= facet.intValue)
					return FacetCheckResult.Success;
				return FacetCheckResult.Fail;
			}
		}

		public class FacetCheck_string_enumeration : FacetCheckInterface
		{
			public static FacetCheckResult Check (string value, FacetInfo facet, WhitespaceType whitespaceNormalization)
			{
				if (LengthFacetCheckHelper.IsEqual(value, facet.stringValue, whitespaceNormalization))
					return FacetCheckResult.EnumSuccess;
				return FacetCheckResult.EnumFail;
			}
		}
		
		public class FacetCheck_hexBinary_length : FacetCheckInterface
		{
			public static FacetCheckResult Check(string value, FacetInfo facet, WhitespaceType whitespaceNormalization)
			{
				if (LengthFacetCheckHelper.ComputeLength(value, whitespaceNormalization) == facet.intValue * 2)
					return FacetCheckResult.Success;
				return FacetCheckResult.Fail;
			}
		}
		
		public class FacetCheck_hexBinary_minLength : FacetCheckInterface
		{
			public static FacetCheckResult Check(string value, FacetInfo facet, WhitespaceType whitespaceNormalization)
			{
				if (LengthFacetCheckHelper.ComputeLength(value, whitespaceNormalization) >= facet.intValue * 2)
					return FacetCheckResult.Success;
				return FacetCheckResult.Fail;
			}
		}
		
		public class FacetCheck_hexBinary_maxLength : FacetCheckInterface
		{
			public static FacetCheckResult Check(string value, FacetInfo facet, WhitespaceType whitespaceNormalization)
			{
				if (LengthFacetCheckHelper.ComputeLength(value, whitespaceNormalization) <= facet.intValue * 2)
					return FacetCheckResult.Success;
				return FacetCheckResult.Fail;
			}
		}
		
		public readonly static FacetCheckInterface facetCheck_Success = new FacetCheck_Success();
			
		public readonly static FacetCheckInterface facetCheck_string_length = new FacetCheck_string_length();
		public readonly static FacetCheckInterface facetCheck_string_minLength = new FacetCheck_string_minLength();
		public readonly static FacetCheckInterface facetCheck_string_maxLength = new FacetCheck_string_maxLength();
		public readonly static FacetCheckInterface facetCheck_string_enumeration = new FacetCheck_string_enumeration();
			
		public readonly static FacetCheckInterface facetCheck_hexBinary_length = new FacetCheck_hexBinary_length();
		public readonly static FacetCheckInterface facetCheck_hexBinary_minLength = new FacetCheck_hexBinary_minLength();
		public readonly static FacetCheckInterface facetCheck_hexBinary_maxLength = new FacetCheck_hexBinary_maxLength();
	} // class XsValidation
} // namespace

namespace Altova.Xml.Meta
{
	/// <summary>
	/// Information object for a simple type.
	/// </summary>
	public class SimpleType
	{
		TypeInfo.TypeInfo typeInfo;

		/// <summary>
		/// Constructs a new instance of the information object.
		/// </summary>
		/// <param name="typeInfo">The core typeinfo object.</param>
		public SimpleType(TypeInfo.TypeInfo typeInfo)
		{			
			this.typeInfo = typeInfo;
		}

		/// <summary>
		/// Returns the base type of this type or null if no base type.
		/// </summary>
		public SimpleType BaseType
		{
			get
			{
				return new SimpleType(typeInfo.BaseType);
			}
		}

		/// <summary>
		/// Returns the namespace URI of this type.
		/// </summary>
		public string NamespaceURI
		{
			get
			{
				return typeInfo.Namespace.namespaceURI;
			}
		}

		/// <summary>
		/// Returns the local name of this type.
		/// </summary>
		public string LocalName
		{
			get
			{
				return typeInfo.localName;
			}
		}

		/// <summary>
		/// Returns the qualified name of this type.
		/// </summary>
		public XmlQualifiedName QualifiedName
		{
			get
			{
				return new XmlQualifiedName(LocalName, NamespaceURI);
			}
		}

		/// <summary>
		/// Checks if two info objects refer to the same type, based on 
		/// qualified name comparison.
		/// </summary>
		/// <param name="obj">The object to compare to.</param>
		/// <returns>True if the type has the same qualified name.</returns>
		public override bool Equals(object obj)
		{
			SimpleType s = obj as SimpleType;
			if (s == null)
				return false;
			return NamespaceURI == s.NamespaceURI &&
				LocalName == s.LocalName;
		}

		/// <summary>
		/// Returns the hash code of the type.
		/// </summary>
		/// <returns></returns>
		public override int GetHashCode()
		{
			return QualifiedName.GetHashCode();
		}

		/// <summary>
		/// Converts the type info to a reasonable string representation.
		/// </summary>
		/// <returns></returns>
		public override string ToString()
		{
			if (NamespaceURI.Length > 0)
				return "{" + NamespaceURI + "}" + LocalName;
			return LocalName;
		}

		/// <summary>
		/// Comparison operator.
		/// </summary>
		/// <param name="a"></param>
		/// <param name="b"></param>
		/// <returns></returns>
		public static bool operator ==(SimpleType a, SimpleType b)
		{
			bool an = object.ReferenceEquals(a, null);
			bool bn = object.ReferenceEquals(b, null);
			if (an || bn)
				return an == bn;
			return a.Equals(b);
		}

		/// <summary>
		/// Comparison operator.
		/// </summary>
		/// <param name="a"></param>
		/// <param name="b"></param>
		/// <returns></returns>
		public static bool operator !=(SimpleType a, SimpleType b)
		{
			bool an = object.ReferenceEquals(a, null);
			bool bn = object.ReferenceEquals(b, null);
			if (an || bn)
				return an != bn;
			return !a.Equals(b);
		}

		static int GetFacetIntFallback(FacetInfo[] facets, string facetName, string fallbackName, int defaultValue)
		{
			if (facets == null)
				return defaultValue;
			int value = defaultValue;
			foreach (FacetInfo facet in facets)
			{
				if (facet.facetName == facetName)
					return facet.intValue;
				if (fallbackName != null && facet.facetName == fallbackName)
					value = facet.intValue;
			}
			return value;
		}

		static string GetFacetString(FacetInfo[] facets, string facetName)
		{
			if (facets == null)
				return null;
			foreach (FacetInfo facet in facets)
			{
				if (facet.facetName == facetName)
					return facet.stringValue;
			}
			return null;
		}


		/// <summary>
		/// Returns the minLength facet value, or 0 if no minimum length is specified.
		/// </summary>
		/// <remarks>
		/// The value returned is the value after resolving all constraining facets, therefore
		/// a nonzero value may be returned even though no minLength facet was present 
		/// originally.
		/// </remarks>
		public int MinLength
		{
			get
			{
				return GetFacetIntFallback(typeInfo.facets.Value, "minLength", "length", 0);
			}
		}

		/// <summary>
		/// Returns the maxLength facet value, or -1 if no maximum length is specified.
		/// </summary>
		/// <remarks>
		/// The value returned is the value after resolving all constraining facets, therefore
		/// a nonnegative value may be returned even though no maxLength facet was present 
		/// originally.
		/// </remarks>
		public int MaxLength
		{
			get
			{
				return GetFacetIntFallback(typeInfo.facets.Value, "maxLength", "length", -1);
			}
		}

		/// <summary>
		/// Returns the length facet value, or -1 if no length is specified.
		/// </summary>
		/// <remarks>
		/// The value returned is the value after resolving all constraining facets, therefore
		/// a nonnegative value may be returned even though no length facet was present 
		/// originally.
		/// </remarks>
		public int Length
		{
			get
			{
				return GetFacetIntFallback(typeInfo.facets.Value, "length", null, -1);
			}
		}

		/// <summary>
		/// Returns the totalDigits facet value, or -1 if total digits is unspecified.
		/// </summary>
		/// <remarks>
		/// The number of digits returned is the maximum number of digits, not the exact
		/// number.
		/// </remarks>
		public int TotalDigits
		{
			get
			{
				return GetFacetIntFallback(typeInfo.facets.Value, "totalDigits", null, -1);
			}
		}

		/// <summary>
		/// Returns the fractionDigits facet value, or -1 if fraction digits is unspecified.
		/// </summary>
		/// <remarks>
		/// The number of digits returned is the maximum number of fraction digits, not the
		/// exact number.
		/// </remarks>
		public int FractionDigits
		{
			get
			{
				return GetFacetIntFallback(typeInfo.facets.Value, "fractionDigits", null, -1);
			}
		}

		/// <summary>
		/// Returns the minInclusive facet value, or null if minInclusive is unspecified.
		/// </summary>
		/// <remarks>
		/// Value space constraints are returned as string values, since the interpretation
		/// of the value depends on the data type.
		/// </remarks>
		public string MinInclusive
		{
			get
			{
				return GetFacetString(typeInfo.facets.Value, "minInclusive");
			}
		}

		/// <summary>
		/// Returns the minExclusive facet value, or null if minExclusive is unspecified.
		/// </summary>
		/// <remarks>
		/// Value space constraints are returned as string values, since the interpretation
		/// of the value depends on the data type.
		/// </remarks>
		public string MinExclusive
		{
			get
			{
				return GetFacetString(typeInfo.facets.Value, "minExclusive");
			}
		}

		/// <summary>
		/// Returns the maxInclusive facet value, or null if maxInclusive is unspecified.
		/// </summary>
		/// <remarks>
		/// Value space constraints are returned as string values, since the interpretation
		/// of the value depends on the data type.
		/// </remarks>
		public string MaxInclusive
		{
			get
			{
				return GetFacetString(typeInfo.facets.Value, "maxInclusive");
			}
		}

		/// <summary>
		/// Returns the maxExclusive facet value, or null if maxExclusive is unspecified.
		/// </summary>
		/// <remarks>
		/// Value space constraints are returned as string values, since the interpretation
		/// of the value depends on the data type.
		/// </remarks>
		public string MaxExclusive
		{
			get
			{
				return GetFacetString(typeInfo.facets.Value, "maxExclusive");
			}
		}

		string[] GetAllFacets(FacetInfo[] facets, string name)
		{
			System.Collections.ArrayList result = new System.Collections.ArrayList();
			if (facets != null)
			{
				foreach (FacetInfo facet in facets)
				{
					if (facet.facetName == name)
						result.Add(facet.stringValue);
				}
			}
			if (result.Count > 0)
				return (string[]) result.ToArray(typeof(string));
			return null;
		}

		/// <summary>
		/// Returns the enumeration facets, or null if no enumerations are specified.
		/// </summary>
		/// <remarks>
		/// Value space constraints are returned as string values, since the interpretation
		/// of the value depends on the data type.
		/// </remarks>
		public string[] Enumerations
		{
			get
			{
				return GetAllFacets(typeInfo.facets.Value, "enumeration");
			}					
		}

		/// <summary>
		/// Returns the pattern facets, or null if no patterns are specified.
		/// </summary>
		public string[] Patterns
		{
			get
			{
				return GetAllFacets(typeInfo.facets.Value, "pattern");
			}
		}

		/// <summary>
		/// Returns the whitespace normalization facet.
		/// </summary>
		public WhitespaceType Whitespace
		{
			get
			{
				return typeInfo.whitespace;
			}
		}
	}


	/// <summary>
	/// Information object for a complex type.
	/// </summary>
	public class ComplexType
	{
		Altova.TypeInfo.TypeInfo typeInfo;

		/// <summary>
		/// Constructs a new instance of complex type from a type info object.
		/// </summary>
		/// <param name="typeInfo"></param>
		public ComplexType(Altova.TypeInfo.TypeInfo typeInfo)
		{
			this.typeInfo = typeInfo;
		}

		/// <summary>
		/// Returns the base type of this type or null if no base type.
		/// </summary>
		public ComplexType BaseType
		{
			get
			{
				return new ComplexType(typeInfo.BaseType);
			}
		}

		/// <summary>
		/// Returns the namespace URI of this type.
		/// </summary>
		public string NamespaceURI
		{
			get
			{
				return typeInfo.Namespace.namespaceURI;
			}
		}

		/// <summary>
		/// Returns the local name of this type.
		/// </summary>
		public string LocalName
		{
			get
			{
				return typeInfo.localName;
			}
		}

		/// <summary>
		/// Returns the qualified name of this type.
		/// </summary>
		public XmlQualifiedName QualifiedName
		{
			get
			{
				return new XmlQualifiedName(LocalName, NamespaceURI);
			}
		}

		/// <summary>
		/// Checks if two info objects refer to the same type, based on 
		/// qualified name comparison.
		/// </summary>
		/// <param name="obj">The object to compare to.</param>
		/// <returns>True if the type has the same qualified name.</returns>
		public override bool Equals(object obj)
		{
			ComplexType s = obj as ComplexType;
			if (s == null)
				return false;
			return NamespaceURI == s.NamespaceURI &&
				LocalName == s.LocalName;
		}

		/// <summary>
		/// Returns the hash code of the type.
		/// </summary>
		/// <returns></returns>
		public override int GetHashCode()
		{
			return QualifiedName.GetHashCode();
		}

		/// <summary>
		/// Converts the type info to a reasonable string representation.
		/// </summary>
		/// <returns></returns>
		public override string ToString()
		{
			if (NamespaceURI.Length > 0)
				return "{" + NamespaceURI + "}" + LocalName;
			return LocalName;
		}

		/// <summary>
		/// Comparison operator.
		/// </summary>
		/// <param name="a"></param>
		/// <param name="b"></param>
		/// <returns></returns>
		public static bool operator ==(ComplexType a, ComplexType b)
		{
			bool an = object.ReferenceEquals(a, null);
			bool bn = object.ReferenceEquals(b, null);
			if (an || bn)
				return an == bn;
			return a.Equals(b);
		}

		/// <summary>
		/// Comparison operator.
		/// </summary>
		/// <param name="a"></param>
		/// <param name="b"></param>
		/// <returns></returns>
		public static bool operator !=(ComplexType a, ComplexType b)
		{
			bool an = object.ReferenceEquals(a, null);
			bool bn = object.ReferenceEquals(b, null);
			if (an || bn)
				return an != bn;
			return !a.Equals(b);
		}


		Altova.TypeInfo.MemberInfo GetMember(string localName, string namespaceURI, MemberFlags mask, MemberFlags check)
		{
			foreach (MemberInfo member in typeInfo.Members)
			{
				if ((member.Flags & mask) != check)
					continue;
				if (member.LocalName != localName || member.NamespaceURI != namespaceURI)
					continue;
				return member;
			}
			return null;
		}


		SimpleType contentType = null;
		volatile bool contentTypeComputed = false;

		/// <summary>
		/// Returns the content type of this complex type, which is a simple type.
		/// </summary>
		/// <remarks>
		/// The content type of element-only types is null, the content type of 
		/// extensions of simple types is the simple base type, the content type
		/// of mixed content types is string.
		/// </remarks>
		public SimpleType ContentType
		{
			get
			{
				if (!contentTypeComputed)
				{
					lock (this)
					{
						if (!contentTypeComputed)
						{
							foreach (MemberInfo member in typeInfo.Members)
							{
								if (member.LocalName == "")
									contentType = new SimpleType(member.DataType);
							}
							// do not set earlier, otherwise there is a race condition
							contentTypeComputed = true;
						}
					}
				}
				return contentType;
			}
		}

		// attributes are cached
		volatile Attribute[] attributes = null;
		void buildAttributes()
		{
			lock (this)
			{
				if (attributes == null)
				{
					System.Collections.ArrayList result = new System.Collections.ArrayList();
					foreach (MemberInfo info in typeInfo.Members)
					{
						if ((info.Flags & MemberFlags.IsAttribute) == MemberFlags.IsAttribute &&
							info.LocalName.Length > 0)
							result.Add(new Attribute(info));
					}
					attributes = (Attribute[]) result.ToArray(typeof(Attribute));
				}
			}
		}

		/// <summary>
		/// Returns all attributes.
		/// </summary>
		/// <param name="localName"></param>
		/// <param name="namespaceURI"></param>
		/// <returns></returns>
		public Attribute[] Attributes
		{
			get
			{
				if (attributes == null)
					buildAttributes();
				return attributes;
			}
		}
			

		// elements are cached
		volatile Element[] elements = null;
		void buildElements()
		{
			lock (this)
			{
				if (elements == null)
				{
					System.Collections.ArrayList result = new System.Collections.ArrayList();
					foreach (MemberInfo info in typeInfo.Members)
					{
						if ((info.Flags & MemberFlags.IsAttribute) != MemberFlags.IsAttribute)
							result.Add(new Element(info));
					}
					elements = (Element[])result.ToArray(typeof(Element));
				}
			}
		}

		/// <summary>
		/// Returns all child elements of the type in source schema order.
		/// </summary>
		/// <returns>An array containing all child elements.</returns>
		/// <remarks>
		/// If the content model contains multiple references to the same element (identified
		/// by full name) then the first occurrence is used for ordering purposes, and the
		/// occurrence counts are cumulative.
		/// </remarks>
		public Element[] Elements
		{
			get
			{
				if (elements == null)
					buildElements();
				return elements;
			}
		}

		static bool CompareWildcard(string a, string b)
		{
			if (b == null)
				return true;
			return a == b;
		}

		/// <summary>
		/// Finds an element by name.
		/// </summary>
		/// <param name="localName">The local name to search for, or null to return the first 
		/// match by namespace.</param>
		/// <param name="namespaceURI">The namespace to search for, or null to return the first
		/// match by local name. Use the empty string to specify the absent namespace.</param>
		/// <returns>The matching element, or null if no such element exists.</returns>
		public Element FindElement(string localName, string namespaceURI)
		{
			foreach (Element e in Elements)
			{
				if (CompareWildcard(e.LocalName, localName) && CompareWildcard(e.NamespaceURI, namespaceURI))
					return e;
			}
			return null;
		}

		/// <summary>
		/// Finds an attribute by name. 
		/// </summary>
		/// <param name="localName">The local name to search for, or null to return the first
		/// match by namespace.</param>
		/// <param name="namespaceURI">The namespace to search for, or null to return the first
		/// match by local name. Use the empty string to specify the absent namespace.</param>
		/// <returns>The matching attribute, or null if no such attribute exists.</returns>
		public Attribute FindAttribute(string localName, string namespaceURI)
		{
			foreach (Attribute a in Attributes)
			{
				if (CompareWildcard(a.LocalName, localName) && CompareWildcard(a.NamespaceURI, namespaceURI))
					return a;
			}
			return null;
		}
	}

	/// <summary>
	/// Represents a child element within a complex type.
	/// </summary>
	public class Element
	{
		Altova.TypeInfo.MemberInfo memberInfo;

		/// <summary>
		/// Constructs an instance of this class from the given member information.
		/// </summary>
		/// <param name="info">The member information object.</param>
		public Element(Altova.TypeInfo.MemberInfo info)
		{
			this.memberInfo = info;
		}

		/// <summary>
		/// Gets the minimum occurrence count.
		/// </summary>
		public int MinOccurs
		{
			get
			{
				return memberInfo.MinOccurs;
			}
		}

		/// <summary>
		/// Gets the maximum occurrence count.
		/// </summary>
		public int MaxOccurs
		{
			get
			{
				return memberInfo.MaxOccurs;
			}
		}

		/// <summary>
		/// Gets the element local name.
		/// </summary>
		public string LocalName
		{
			get { return memberInfo.LocalName; }
		}

		/// <summary>
		/// Gets the element namespace URI. The empty string represents the absent namespace.
		/// </summary>
		public string NamespaceURI
		{
			get { return memberInfo.NamespaceURI; }
		}

		/// <summary>
		/// Gets the element name as qualified name.
		/// </summary>
		public XmlQualifiedName QualifiedName
		{
			get { return new XmlQualifiedName(LocalName, NamespaceURI); }
		}

		/// <summary>
		/// Gets the element data type.
		/// </summary>
		public ComplexType DataType
		{
			get
			{
				return new ComplexType(memberInfo.DataType);
			}
		}
	}

	/// <summary>
	/// Represents an attribute of a complex type.
	/// </summary>
	public class Attribute
	{
		Altova.TypeInfo.MemberInfo memberInfo;

		public Attribute(Altova.TypeInfo.MemberInfo info)
		{
			this.memberInfo = info;
		}

		/// <summary>
		/// Returns true if the attribute is required.
		/// </summary>
		public bool Required
		{
			get
			{
				return memberInfo.MinOccurs > 0;
			}
		}

		/// <summary>
		/// Returns the local name of the attribute.
		/// </summary>
		public string LocalName
		{
			get { return memberInfo.LocalName; }
		}

		/// <summary>
		/// Returns the namespace URI of the attribute. The absent namespace is designated by the
		/// empty string.
		/// </summary>
		public string NamespaceURI
		{
			get { return memberInfo.NamespaceURI; }
		}

		/// <summary>
		/// Returns the name of attribute as qualified name.
		/// </summary>
		public XmlQualifiedName QualifiedName
		{
			get { return new XmlQualifiedName(LocalName, NamespaceURI); }
		}

		/// <summary>
		/// Returns the data type of the attribute.
		/// </summary>
		public SimpleType DataType
		{
			get
			{
				return new SimpleType(memberInfo.DataType);
			}
		}
	}
} // namespace