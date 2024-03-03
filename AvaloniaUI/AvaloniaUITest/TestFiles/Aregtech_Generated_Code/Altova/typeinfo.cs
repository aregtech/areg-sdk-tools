// typeinfo.cs
// This file contains generated code and will be overwritten when you rerun code generation.
using System;

namespace Altova.TypeInfo
{
	public enum WhitespaceType {Unknown, Preserve, Replace, Collapse}
	public enum FacetCheckResult {Fail, Success, EnumFail, EnumSuccess}

	public interface InfoBinderInterface
	{
		NamespaceInfo[] Namespaces {get;}
		TypeInfo[] Types {get;}
		MemberInfo[] Members {get;}
	}

	public abstract class FacetCheckInterface
	{
		static FacetCheckResult Check(string value, FacetInfo facet, WhitespaceType whitespaceNormalization) 
		{
			return FacetCheckResult.Fail;
		}
	}

	public class TypeInfo
	{
		private int namespaceIndex;
		private int baseTypeIndex;
		private InfoBinderInterface binder;
		private ValueFormatter formatter;

		public TypeInfo(InfoBinderInterface binder, int namespaceIndex, string localName, int baseTypeIndex, int memberBegin,
					 int memberEnd, Lazy<FacetInfo[]> facets, WhitespaceType whitespace, ValueFormatter frmatter)
		{
			this.namespaceIndex = namespaceIndex;
			this.localName = localName;
			this.baseTypeIndex = baseTypeIndex;
			this.memberBegin = memberBegin;
			this.memberEnd = memberEnd;
			this.facets = facets;
			this.whitespace = whitespace;
			this.binder = binder;
			this.formatter = frmatter;
		}
		
		public TypeInfo(InfoBinderInterface binder, int namespaceIndex, string localName, int baseTypeIndex, int memberBegin,
					 int memberEnd, Lazy<FacetInfo[]> facets, WhitespaceType whitespace)
		{
			this.namespaceIndex = namespaceIndex;
			this.localName = localName;
			this.baseTypeIndex = baseTypeIndex;
			this.memberBegin = memberBegin;
			this.memberEnd = memberEnd;
			this.facets = facets;
			this.whitespace = whitespace;
			this.binder = binder;
			this.formatter = null;
		}

		public MemberInfo[] Members
		{
			get { 
				MemberInfo[] result = new MemberInfo[memberEnd - memberBegin];
				System.Array.Copy(binder.Members, memberBegin, result, 0, result.Length);
				return result; 
			}
		}

		public NamespaceInfo Namespace
		{
			get { return binder.Namespaces[namespaceIndex]; }
		}

		public TypeInfo BaseType
		{
			get { return binder.Types[baseTypeIndex]; }
		}
		
		public ValueFormatter Formatter
		{
			get
			{
				return formatter;
			}
		}

		public string localName;
		public int memberBegin;
		public int memberEnd;
		public Lazy<FacetInfo[]> facets;
		public WhitespaceType whitespace;
	}

	[System.Flags]
	public enum MemberFlags 
	{
		None = 0,
		SpecialName = 1 << 0,
		IsAttribute = 1 << 1,
	}


	public class MemberInfo
	{
		private readonly InfoBinderInterface binder;
		private readonly int containingTypeIndex;
		private readonly int dataTypeIndex;
		private readonly string namespaceURI;
		private readonly string localName;
		private readonly MemberFlags flags;
		private readonly int minOccurs;
		private readonly int maxOccurs;

		public MemberInfo(InfoBinderInterface binder, string namespaceURI, string localName, int containingTypeIndex, int dataTypeIndex, MemberFlags flags, int minOccurs, int maxOccurs)
		{
			this.namespaceURI = namespaceURI;
			this.localName = localName;
			this.binder = binder;
			this.containingTypeIndex = containingTypeIndex;
			this.dataTypeIndex =dataTypeIndex;
			this.flags = flags;
			this.minOccurs = minOccurs;
			this.maxOccurs = maxOccurs;
		}

		public string NamespaceURI { get { return namespaceURI == null ? "" : namespaceURI; } }
		public string LocalName { get { return localName == null ? "" : localName; } }
		public TypeInfo ContainingType { get { return binder.Types[containingTypeIndex]; } }
		public TypeInfo DataType { get { return binder.Types[dataTypeIndex]; } }
		public MemberFlags Flags { get { return flags; } } 

		public int MinOccurs { get { return minOccurs; } }
		public int MaxOccurs { get { return maxOccurs; } }
	} // class MemberInfo

	public class NamespaceInfo
	{
		private readonly InfoBinderInterface binder;

		public NamespaceInfo(InfoBinderInterface binder, string namespaceURI, string prefix, int typeBegin, int typeEnd)
		{
			this.namespaceURI = namespaceURI;
			this.prefix = prefix;
			this.binder = binder;
			this.typeBegin = typeBegin;
			this.typeEnd = typeEnd;
		}

		public TypeInfo[] Types { 
			get 
			{ 
				TypeInfo[] result = new TypeInfo[typeEnd - typeBegin];
				System.Array.Copy(binder.Types, typeBegin, result, 0, result.Length);
				return result; 
			} 
		}
		public readonly string namespaceURI;
		public readonly string prefix;
		public readonly int typeBegin;
		public readonly int typeEnd;
	} // class NamespaceInfo

	public class FacetInfo
	{
		public FacetInfo(FacetCheckInterface check, string facetName, string stringValue, int intValue)
		{
			this.check = check;
			this.facetName = facetName;
			this.stringValue = stringValue;
			this.intValue = intValue;
		}

		public readonly FacetCheckInterface check;
		public readonly string facetName;
		public readonly string stringValue;
		public readonly int intValue;
	} // class FacetInfo
	
	public class ValueFormatter
	{
	}
} // namespace Altova.TypeInfo
