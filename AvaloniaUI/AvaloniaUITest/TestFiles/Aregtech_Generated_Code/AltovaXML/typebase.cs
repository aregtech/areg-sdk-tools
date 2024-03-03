// typebase.cs
// This file contains generated code and will be overwritten when you rerun code generation.

using System.Xml;
using Altova.TypeInfo;

namespace Altova.Xml
{
	public class TypeBase
	{
		protected internal XmlNode node;

		public TypeBase(XmlNode node)
		{
			this.node = node;
		}

		public XmlNode Node 
		{ 
			get { return node; } 
		}
 
		public XmlNode GetAttribute(MemberInfo member)
		{
			XmlElement element = (XmlElement) node;
			if (member.LocalName == "")
			{
				for (XmlNode child = element.FirstChild; child != null; child = child.NextSibling)
				{
					if (child.NodeType == XmlNodeType.Text || child.NodeType == XmlNodeType.CDATA)
						return child;
				}
				return null;
			}
			else
				return element.GetAttributeNode(member.LocalName, member.NamespaceURI);
		}

		public XmlNode CreateAttribute(MemberInfo member)
		{
			XmlElement element = (XmlElement) node;
			if (member.LocalName == "")
			{
				XmlNode existing = GetAttribute(member);
				if (existing == null)
				{
					XmlText text = element.OwnerDocument.CreateTextNode("");
					element.AppendChild(text);
					return text;
				}
				return existing;
			}
			else
			{
				XmlAttribute att = element.OwnerDocument.CreateAttribute(member.LocalName, member.NamespaceURI);
				element.SetAttributeNode(att);
				return att;
			}
		}

		public void RemoveAttribute(MemberInfo member)
		{
			XmlElement element = (XmlElement) node;
			if (member.LocalName == "")
			{
				for (XmlNode child = element.FirstChild; child != null;)
				{
					XmlNode keep = child;
					child = child.NextSibling;
					if (keep.NodeType == XmlNodeType.Text || keep.NodeType == XmlNodeType.CDATA)
						element.RemoveChild(keep);
				}
			}
			else
			{
				element.RemoveAttribute(member.LocalName, member.NamespaceURI);
			}
		}

		public XmlNode GetElementFirst(MemberInfo member)
		{
			return GetElementAt(member, 0);
		}

		public XmlNode GetElementAt(MemberInfo member, int index)
		{
			for (XmlNode child = node.FirstChild; child != null; child = child.NextSibling)
			{
				if (MemberEqualsNode(member, child))
				{
					if (index-- == 0)
						return child;
				}
			}
			return null;
		}

		public XmlNode GetElementLast(MemberInfo member)
		{
			for (XmlNode child = node.LastChild; child != null; child = child.PreviousSibling)
			{
				if (MemberEqualsNode(member, child))
				{
					return child;
				}
			}
			return null;
		}

		public XmlNode CreateElement(MemberInfo member)
		{
			return Altova.Xml.XmlTreeOperations.AddElement(node, member);
		}

		public XmlNode CreateElement(string prefix, string localName, string namespaceURI)
		{
			return XmlTreeOperations.AddElement(node, localName, prefix, namespaceURI);
		}

		public int CountElement(MemberInfo member)
		{
			int count = 0;
			for (XmlNode child=node.FirstChild; child != null; child = child.NextSibling)
			{
				if (MemberEqualsNode(member, child))
					++count;
			}
			return count;
		}

		public void RemoveElement(MemberInfo member)
		{
			for (XmlNode child = node.FirstChild; child != null;)
			{
				XmlNode keep = child;
				child = child.NextSibling;
				if (MemberEqualsNode(member, keep))
					node.RemoveChild(keep);
			}
		}
	
		public void RemoveElementAt(MemberInfo member, int index)
		{
			for (XmlNode child = node.FirstChild; child != null;)
			{
				XmlNode keep = child;
				child = child.NextSibling;
				if (MemberEqualsNode(member, keep) && index-- == 0)
				{
					node.RemoveChild(keep);
					break;
				}
			}
		}
	
		internal static bool MemberEqualsNode(MemberInfo member, XmlNode node)
		{
			string nodeURI = node.NamespaceURI == null ? "" : node.NamespaceURI;
			string nodeLocalName = node.LocalName == null ? "" : node.LocalName;
			string memberURI = member.NamespaceURI == null ? "" : member.NamespaceURI;
			string memberLocalName = member.LocalName == null ? "" : member.LocalName;
				
			if (nodeURI == memberURI && nodeLocalName == memberLocalName)
				return true;
			return false;
		}
		
		public static int GetEnumerationIndex(MemberInfo member, string sValue, int enumOffset, int enumCount)
		{
			for( int i = enumOffset; i < (enumOffset + enumCount); i++)
			{
				if( member.DataType.facets.Value[i].facetName == "enumeration" && 
						member.DataType.facets.Value[i].stringValue == sValue)
					return i - enumOffset;
			}
			return -1;
		}
	}

	public class ElementType : TypeBase
	{
		public ElementType(XmlNode node) : base(node)	{ }

		public void DeclareNamespace(string prefix, string namespaceURI)
		{
			if (namespaceURI == null || namespaceURI.Length == 0)
				throw new System.InvalidOperationException("DeclareNamespace requires non-empty namespaceURI.");
			string localName = "xmlns";
			if (prefix != null && prefix.Length > 0)
				localName = localName + ":" + prefix;

			XmlTreeOperations.SetAttribute(node, localName, "http://www.w3.org/2000/xmlns/", namespaceURI);
		}
	}
}
