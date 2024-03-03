// document.cs
// This file contains generated code and will be overwritten when you rerun code generation.

using System.Xml;
using Altova.TypeInfo;

namespace Altova.Xml
{
	public class Document
	{
		static public XmlNode Load(string filename)
		{
			XmlDocument doc = new XmlDocument();
			doc.Load(filename);
			return doc;
		}
		
		static public XmlNode Create()
		{
			return new XmlDocument();
		}

		static public void Save(TypeBase doc, string filename)
		{
			Save(doc, filename, "UTF-8", false, false);
		}

		static public void SetSchemaLocation(TypeBase node, string schLoc)
		{
			XmlElement root = null;
			// if this is a doc, set it on the DocumentElement
			if (node.Node is XmlDocument)
			{
				XmlDocument doc = (XmlDocument) node.Node;
				root = doc.DocumentElement;
			}
			else
			{
				root = (XmlElement) node.Node;
			}
			
			// 
		}
		
		static public void Save( TypeBase doc, string filename, string encoding )
		{
			Save( doc, filename, encoding, System.String.Compare(encoding, "UTF-16BE", true) == 0, System.String.Compare(encoding, "UTF-16", true) == 0 );
		}

		static public void Save( TypeBase doc, string filename, string encoding, bool bBigEndian, bool bBOM )
		{

			using (XmlTextWriter writer = new XmlTextWriter(filename, GetEncodingObject(encoding, bBigEndian, bBOM)))
			{
				writer.Formatting = Formatting.Indented;
				XmlDocument dDoc = (XmlDocument)doc.Node;
				dDoc.Save(writer);
			}
		}

		private static System.Text.Encoding GetEncodingObject( string encoding, bool bBigEndian, bool bBOM )
		{
			int unisize = GetUnicodeSizeFromEncodingName( encoding );

			if( unisize == 1 )
				return new System.Text.UTF8Encoding( bBOM );

			if( unisize == 2 )
				return new UTF16Encoding( bBigEndian, bBOM );

			return System.Text.Encoding.GetEncoding( encoding );
		}

		private static int GetUnicodeSizeFromEncodingName( string encoding )
		{
			if( encoding == null ) return 0;
			encoding = encoding.ToUpper();

			if( encoding.IndexOf("UTF-8") >= 0 )
				return 1;

			if( encoding.IndexOf("UTF-16") >= 0 || encoding.IndexOf("UCS-2") >= 0 )
				return 2;

			return 0;
		}
	}
}