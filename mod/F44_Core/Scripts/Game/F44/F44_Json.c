// Arma & Heroes - tiny JSON DOM parser (no engine serializer dependency).
// Supports objects, arrays, strings (\" \\ \/ \n \t \r, \u -> '?'), numbers, true/false/null.

enum F44_EJsonType
{
	JNULL,
	JBOOL,
	JNUMBER,
	JSTRING,
	JARRAY,
	JOBJECT
}

class F44_JsonNode
{
	F44_EJsonType m_eType;
	string m_sValue;
	float m_fValue;
	bool m_bValue;
	ref array<ref F44_JsonNode> m_aItems;
	ref array<string> m_aKeys;

	//------------------------------------------------------------------------------------------------
	bool IsObject() { return m_eType == F44_EJsonType.JOBJECT; }
	bool IsArray() { return m_eType == F44_EJsonType.JARRAY; }
	bool IsNull() { return m_eType == F44_EJsonType.JNULL; }

	int Count()
	{
		if (!m_aItems)
			return 0;
		return m_aItems.Count();
	}

	F44_JsonNode At(int i)
	{
		if (!m_aItems || i < 0 || i >= m_aItems.Count())
			return null;
		return m_aItems[i];
	}

	string KeyAt(int i)
	{
		if (!m_aKeys || i < 0 || i >= m_aKeys.Count())
			return string.Empty;
		return m_aKeys[i];
	}

	F44_JsonNode Get(string key)
	{
		if (!m_aKeys)
			return null;
		int idx = m_aKeys.Find(key);
		if (idx < 0)
			return null;
		return m_aItems[idx];
	}

	//------------------------------------------------------------------------------------------------
	string GetString(string key, string def = string.Empty)
	{
		F44_JsonNode n = Get(key);
		if (!n || n.m_eType == F44_EJsonType.JNULL)
			return def;
		if (n.m_eType == F44_EJsonType.JSTRING)
			return n.m_sValue;
		if (n.m_eType == F44_EJsonType.JNUMBER)
			return n.m_fValue.ToString();
		return def;
	}

	int GetInt(string key, int def = 0)
	{
		F44_JsonNode n = Get(key);
		if (!n)
			return def;
		if (n.m_eType == F44_EJsonType.JNUMBER)
			return Math.Round(n.m_fValue);
		if (n.m_eType == F44_EJsonType.JSTRING)
			return n.m_sValue.ToInt();
		return def;
	}

	//! String value of this node (for array items)
	string AsString()
	{
		if (m_eType == F44_EJsonType.JSTRING)
			return m_sValue;
		if (m_eType == F44_EJsonType.JNUMBER)
			return m_fValue.ToString();
		return string.Empty;
	}
}

//------------------------------------------------------------------------------------------------
class F44_Json
{
	protected string m_sText;
	protected int m_iPos;
	protected int m_iLen;
	protected bool m_bError;

	//------------------------------------------------------------------------------------------------
	static F44_JsonNode Parse(string text)
	{
		F44_Json p = new F44_Json();
		p.m_sText = text;
		p.m_iLen = text.Length();
		p.m_iPos = 0;
		F44_JsonNode root = p.ParseValue(0);
		if (p.m_bError)
			return null;
		return root;
	}

	//------------------------------------------------------------------------------------------------
	static F44_JsonNode ParseFile(string path)
	{
		// NOTE: FileIO.FileExists() hangs on addon-relative paths (seen in Workbench): only use it on $profile
		if (path.StartsWith("$profile:") && !FileIO.FileExists(path))
			return null;
		// addon-relative paths are resolved inside this addon's file system (a bare relative
		// path made FileIO scan every mounted addon and froze Workbench)
		if (!path.StartsWith("$"))
			path = "$F44_Core:" + path;
		FileHandle fh = FileIO.OpenFile(path, FileMode.READ);
		if (!fh)
			return null;
		int len = fh.GetLength();
		string text;
		if (len > 0 && len < 4000000)
			fh.Read(text, len);
		fh.Close();
		return Parse(text);
	}

	//------------------------------------------------------------------------------------------------
	protected string Ch()
	{
		if (m_iPos >= m_iLen)
			return string.Empty;
		return m_sText.Get(m_iPos);
	}

	//------------------------------------------------------------------------------------------------
	protected void SkipWs()
	{
		while (m_iPos < m_iLen)
		{
			string c = m_sText.Get(m_iPos);
			if (c == " " || c == "\n" || c == "\r" || c == "\t")
				m_iPos++;
			else
				return;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected F44_JsonNode Fail(string msg)
	{
		if (!m_bError)
			PrintFormat("[F44] JSON error at %1: %2", m_iPos, msg, level: LogLevel.ERROR);
		m_bError = true;
		m_iPos = m_iLen;
		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected F44_JsonNode ParseValue(int depth)
	{
		if (depth > 64)
			return Fail("too deep");
		SkipWs();
		string c = Ch();
		if (c == "{")
			return ParseObject(depth);
		if (c == "[")
			return ParseArray(depth);
		if (c == "\"")
		{
			F44_JsonNode s = new F44_JsonNode();
			s.m_eType = F44_EJsonType.JSTRING;
			s.m_sValue = ParseString();
			return s;
		}
		if (c == "t" || c == "f" || c == "n")
			return ParseLiteral();
		if (c != string.Empty && "-0123456789".Contains(c))
			return ParseNumber();
		return Fail("unexpected char '" + c + "'");
	}

	//------------------------------------------------------------------------------------------------
	protected F44_JsonNode ParseObject(int depth)
	{
		F44_JsonNode n = new F44_JsonNode();
		n.m_eType = F44_EJsonType.JOBJECT;
		n.m_aItems = {};
		n.m_aKeys = {};
		m_iPos++; // {
		SkipWs();
		if (Ch() == "}")
		{
			m_iPos++;
			return n;
		}
		while (!m_bError && m_iPos < m_iLen)
		{
			SkipWs();
			if (Ch() != "\"")
				return Fail("key expected");
			string key = ParseString();
			SkipWs();
			if (Ch() != ":")
				return Fail("':' expected");
			m_iPos++;
			F44_JsonNode v = ParseValue(depth + 1);
			if (m_bError)
				return null;
			n.m_aKeys.Insert(key);
			n.m_aItems.Insert(v);
			SkipWs();
			string c = Ch();
			m_iPos++;
			if (c == "}")
				return n;
			if (c != ",")
				return Fail("',' or '}' expected");
		}
		return Fail("unterminated object");
	}

	//------------------------------------------------------------------------------------------------
	protected F44_JsonNode ParseArray(int depth)
	{
		F44_JsonNode n = new F44_JsonNode();
		n.m_eType = F44_EJsonType.JARRAY;
		n.m_aItems = {};
		m_iPos++; // [
		SkipWs();
		if (Ch() == "]")
		{
			m_iPos++;
			return n;
		}
		while (!m_bError && m_iPos < m_iLen)
		{
			F44_JsonNode v = ParseValue(depth + 1);
			if (m_bError)
				return null;
			n.m_aItems.Insert(v);
			SkipWs();
			string c = Ch();
			m_iPos++;
			if (c == "]")
				return n;
			if (c != ",")
				return Fail("',' or ']' expected");
		}
		return Fail("unterminated array");
	}

	//------------------------------------------------------------------------------------------------
	protected string ParseString()
	{
		m_iPos++; // opening quote
		string result;
		while (m_iPos < m_iLen)
		{
			string c = m_sText.Get(m_iPos);
			m_iPos++;
			if (c == "\"")
				return result;
			if (c == "\\")
			{
				if (m_iPos >= m_iLen)
					break;
				string e = m_sText.Get(m_iPos);
				m_iPos++;
				if (e == "n")
					result += "\n";
				else if (e == "t")
					result += "\t";
				else if (e == "r")
					result += "\r";
				else if (e == "u")
				{
					m_iPos += 4;
					result += "?";
				}
				else
					result += e;
				continue;
			}
			result += c;
		}
		Fail("unterminated string");
		return result;
	}

	//------------------------------------------------------------------------------------------------
	protected F44_JsonNode ParseNumber()
	{
		int start = m_iPos;
		while (m_iPos < m_iLen)
		{
			string c = m_sText.Get(m_iPos);
			if ("0123456789+-.eE".Contains(c))
				m_iPos++;
			else
				break;
		}
		F44_JsonNode n = new F44_JsonNode();
		n.m_eType = F44_EJsonType.JNUMBER;
		n.m_sValue = m_sText.Substring(start, m_iPos - start);
		n.m_fValue = n.m_sValue.ToFloat();
		return n;
	}

	//------------------------------------------------------------------------------------------------
	protected F44_JsonNode ParseLiteral()
	{
		F44_JsonNode n = new F44_JsonNode();
		if (m_iPos + 4 <= m_iLen && m_sText.Substring(m_iPos, 4) == "true")
		{
			n.m_eType = F44_EJsonType.JBOOL;
			n.m_bValue = true;
			m_iPos += 4;
			return n;
		}
		if (m_iPos + 5 <= m_iLen && m_sText.Substring(m_iPos, 5) == "false")
		{
			n.m_eType = F44_EJsonType.JBOOL;
			m_iPos += 5;
			return n;
		}
		if (m_iPos + 4 <= m_iLen && m_sText.Substring(m_iPos, 4) == "null")
		{
			n.m_eType = F44_EJsonType.JNULL;
			m_iPos += 4;
			return n;
		}
		return Fail("bad literal");
	}
}
