#pragma once

class Session {
private:
	int s_id;
	queue<Message> m_messages;
	CRITICAL_SECTION m_CS;
	clock_t  time;
public:

	

	clock_t getTime() {
		return time;
	}

	void setTime(clock_t t) {
		EnterCriticalSection(&m_CS);
		time = t;
		LeaveCriticalSection(&m_CS);
	}

	int getM_ID() {
		return s_id;
	}

	void setM_ID(int id) {
		s_id = id;
	}

	

	Session(int ID, clock_t t)
		:s_id(ID), time(t)
	{
		InitializeCriticalSection(&m_CS);
	}
	~Session()
	{
		DeleteCriticalSection(&m_CS);
	}

	void Add(Message& msg)
	{
		EnterCriticalSection(&m_CS);
		m_messages.push(msg);
		LeaveCriticalSection(&m_CS);
	}
	void Send(CSocket& s)
	{
		EnterCriticalSection(&m_CS);
		if (m_messages.empty())
		{
			Message::SendMessage(s, s_id, A_BROCKER, M_NODATA);
		}
		else
		{
			m_messages.front().Send(s);
			m_messages.pop();
		}
		LeaveCriticalSection(&m_CS);
	}


};