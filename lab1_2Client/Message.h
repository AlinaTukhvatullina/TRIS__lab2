#pragma once

enum Addresses {
	A_BROCKER = 0,
	A_ALL = -1
};

enum Messages {
	M_INIT,
	M_EXIT0,
	M_GETDATA,
	M_NODATA,
	M_TEXT,
	M_CONFIRM
};

struct MsgHeader
{
	unsigned int m_To;
	unsigned int m_From;
	unsigned int m_Type;
	unsigned int m_Size;
};

void canalStart(CSocket& hS) {
	hS.Create();
	hS.Connect(_T("127.0.0.1"), 11111);
}

void canalStop(CSocket& hS) {
	hS.Close();
}

class Message {
private:
	MsgHeader m_header;
	string m_data;
public:
	MsgHeader getM_Header() {
		return m_header;
	}

	void setM_Header(MsgHeader h) {
		m_header = h;
	}

	string getM_Data() {
		return m_data;
	}

	void setM_Data(string d) {
		m_data = d;
	}


	Message() {
		m_header = { 0 };
	}
	Message(unsigned int to, unsigned int from, unsigned int type = M_TEXT, const string& data = "") :m_data(data) {
		m_header.m_To = to;
		m_header.m_From = from;
		m_header.m_Type = type;
		m_header.m_Size = data.length();
	}

	void Send(CSocket& s) {
		canalStart(s);
		s.Send(&m_header, sizeof(MsgHeader));
		if (m_header.m_Size) {
			s.Send(m_data.c_str(), m_header.m_Size + 1);
		}
	}

	MsgHeader Receive(CSocket& s) {
		s.Receive(&m_header, sizeof(MsgHeader));
		if (m_header.m_Size)
		{
			char* pBuff = new char[m_header.m_Size + 1];
			s.Receive(pBuff, m_header.m_Size + 1);
			pBuff[m_header.m_Size] = '\0';
			m_data = pBuff;
			delete[] pBuff;
		}
		canalStop(s);
		return m_header;
	}

	static void SendMessage(CSocket& s, unsigned int To, unsigned int From, unsigned int Type = M_TEXT, const string& Data = "")
	{
		Message msg(To, From, Type, Data);
		msg.Send(s);
	}
};