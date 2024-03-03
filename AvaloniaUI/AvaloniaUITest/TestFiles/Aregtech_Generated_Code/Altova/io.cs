// io.cs
// This file contains generated code and will be overwritten when you rerun code generation.

using System.IO;
using System.Xml;
using System.Text;
using System.Net;
using System.Net.Http;
using System;

namespace Altova.IO
{
	public abstract class Input : IDisposable
	{
		public enum InputType {Reader = 0, Stream = 1, XmlDocument = 2}
		private InputType type;

		public Input(InputType t) { type = t; }
		public InputType Type { get { return type; } }
		public abstract TextReader Reader { get; }
		public abstract Stream Stream { get; }
		public abstract XmlDocument Document { get; }
		public virtual void Close() {}
		public void Dispose()
		{
			Dispose(true);
			GC.SuppressFinalize(this);
		}
		protected virtual void Dispose(bool disposing)
		{
			Close();
		}
		public virtual string Filename { get { return ""; } }
	};

	public class StreamInput : Input
	{
		private Stream stream;
		public StreamInput(Stream s) : base(InputType.Stream) { stream = s; }

		public override Stream Stream { get { return stream;} }
		public override TextReader Reader { get {throw new System.InvalidOperationException("This is not a reader"); } }
		public override XmlDocument Document { get {throw new System.InvalidOperationException("This is not an XML Dom tree"); } }
		public override void Close()
		{
			if (stream != null)
			{
				stream.Close();
				stream = null;
			}
		}
		public static StreamInput createInput(string url) 
		{ 
			String u = url.Trim();
			if (u.IndexOf(':') > 1)
				return new UrlInput(u);
			return new FileInput(u);
		}
	};

	public class FileInput : StreamInput
	{
		private readonly string filename;
		public FileInput(string f) : base(System.IO.File.OpenRead(f)) { filename = f; }
		public override string Filename { get { return filename; } }
	};
	
	public class UrlInput : StreamInput
	{
		private readonly string url;
		public UrlInput(string u) : base(OpenFromUrl(u)) { url = u; }
		public override string Filename { get { return url; } }
		private static Stream OpenFromUrl(string u)
		{
			Uri uri = new Uri(u);
			if (uri.Scheme == "file" || uri.Scheme == "ftp")
				return WebRequest.Create(uri).GetResponse().GetResponseStream();

			return Web.HttpClient.GetStreamAsync(uri).Result;
		}
	}

	public class ReaderInput : Input
	{
		private TextReader reader;
		public ReaderInput(TextReader r) : base(InputType.Reader)
		{
			reader = r;
		}

		public override Stream Stream { get { throw new System.InvalidOperationException("This is not a stream"); } }
		public override TextReader Reader { get { return reader; } }
		public override XmlDocument Document { get { throw new System.InvalidOperationException("This is not an XML Dom tree"); } }
		public override void Close()
		{
			if (reader != null)
			{
				reader.Close();
				reader = null;
			}
		}
	};

	public class StringInput : ReaderInput
	{
		private string content;
		public StringInput(string c) : base(new System.IO.StringReader(c))
		{
			content = c;
		}

		public string Content { get { return content; } }
	};

	public class DocumentInput : Input
	{
		private XmlDocument document;
		public DocumentInput(XmlDocument d) : base(InputType.XmlDocument) { document = d; }

		public override TextReader Reader { get { throw new System.InvalidOperationException("This is not a reader"); } }
		public override Stream Stream { get { throw new System.InvalidOperationException("This is not a stream"); } }
		public override XmlDocument Document { get { return document; } }
	};

	public abstract class Output : IDisposable
	{
		public enum OutputType {Writer = 0, Stream = 1, XmlDocument = 2}
		public OutputType type;

		public Output(OutputType t) { type = t; }
		public OutputType Type { get { return type; } }
		public abstract TextWriter Writer { get; }
		public abstract Stream Stream { get; }
		public abstract XmlDocument Document { get; }
		public virtual void Close() {}
		public void Dispose()
		{
			Dispose(true);
			GC.SuppressFinalize(this);
		}
		protected virtual void Dispose(bool disposing)
		{
			Close();
		}
	};

	public class StreamOutput : Output
	{
		private Stream stream = null;
		private System.IO.FileMode filemode =System.IO.FileMode.Create;
		protected string filename = null;
			
		public StreamOutput(Stream s) : base(OutputType.Stream) { stream = s; }
		public StreamOutput(string f, System.IO.FileMode fm) : base(OutputType.Stream) { filename = f; filemode = fm; }

		public override Stream Stream 
		{ 
			get 
			{ 
				if (stream == null) stream = new FileStream(filename, filemode);
				return stream; 
			} 
		}
		
		public override TextWriter Writer { get { throw new System.InvalidOperationException("This is not a writer"); } }
		public override XmlDocument Document { get { throw new System.InvalidOperationException("This is not an XML Dom tree"); } }
		public override void Close()
		{
			if (stream != null)
			{
				stream.Close();
				stream = null;
			}
		}
	};

	public class FileOutput : StreamOutput
	{
		public FileOutput(string f) : this(f, System.IO.FileMode.Create) {}
		public FileOutput(string f, System.IO.FileMode fm) : base(f, fm) {}
		public string Fielname { get { return filename; } }
	};

	public class WriterOutput : Output
	{
		private TextWriter writer;
		public WriterOutput(TextWriter w) : base (OutputType.Writer) { writer = w; }

		public override Stream Stream { get { throw new System.InvalidOperationException("This is not a stream"); } }
		public override TextWriter Writer { get { return writer; } }
		public override XmlDocument Document { get { throw new System.InvalidOperationException("This is not an XML Dom tree"); } }
		public override void Close()
		{
			if (writer != null)
		{
				writer.Close();
				writer = null;
			}
		}
	};

	public class StringOutput : WriterOutput
	{
		private StringBuilder content;
		public StringOutput(StringBuilder s) : base (new System.IO.StringWriter(s)) { content = s; }

		public StringBuilder Content { get { return content; } }
	};

	public class DocumentOutput : Output
	{
		private XmlDocument document;
		public DocumentOutput(XmlDocument d) : base(OutputType.XmlDocument) { document = d; }

		public override Stream Stream { get { throw new System.InvalidOperationException("This is not a stream"); } }
		public override TextWriter Writer { get { throw new System.InvalidOperationException("This is not a writer"); } }
		public override XmlDocument Document { get { return document; } }
	};

	public static class Web
	{
		// The one and only HttpClient instance, timeout handling is done on individual requests.
		private static readonly HttpClient httpClient = new System.Net.Http.HttpClient() { Timeout = System.Threading.Timeout.InfiniteTimeSpan };
		public static HttpClient HttpClient { get  { return httpClient; } }
	}
}
