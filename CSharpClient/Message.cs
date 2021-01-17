using System;
using System.Collections.Generic;
using System.Text;
using System.Net;
using System.Net.Sockets;

namespace CSharpClient
{
    public enum Messages
    {
        M_INIT,
        M_EXIT0,
        M_GETDATA,
        M_NODATA,
        M_TEXT,
        M_CONFIRM
    }

    public enum Adresses
    {
        A_BROCKER = 0,
        A_ALL = -1
    }

    public struct MsgHeader
    {
        public int m_To;
        public int m_From;
        public int m_Type;
        public int m_Size;
    }

    class Message
    {
        private MsgHeader m_Header;
        private string m_Data;

        static void canalStart(Socket s, IPEndPoint endPoint)
        {
            s.Connect(endPoint);
        }

        static void canalStop(Socket s)
        {
            s.Shutdown(SocketShutdown.Both);
            s.Close();
        }


        public MsgHeader GetM_Header()
        {
            return m_Header;
        }
        public string getM_Data()
        {
            return m_Data;
        }
        public void setM_Data(string d)
        {
            m_Data = d;
        }

        public Message()
        {
            m_Header.m_To = 0;
            m_Header.m_From = 0;
            m_Header.m_Size = 0;
            m_Header.m_Type = -1;
        }

        public Message(int To, int From, Messages Type = Messages.M_TEXT, string Data = "")
        {
            m_Header = new MsgHeader();
            m_Header.m_To = To;
            m_Header.m_From = From;
            m_Header.m_Type = (int)Type;
            m_Data = Data;
            m_Header.m_Size = Data.Length;
        }

        public void Send(Socket s, IPEndPoint endPoint)
        {
            canalStart(s, endPoint);
            s.Send(BitConverter.GetBytes(m_Header.m_To), sizeof(int), SocketFlags.None);
            s.Send(BitConverter.GetBytes(m_Header.m_From), sizeof(int), SocketFlags.None);
            s.Send(BitConverter.GetBytes(m_Header.m_Type), sizeof(int), SocketFlags.None);
            s.Send(BitConverter.GetBytes(m_Header.m_Size), sizeof(int), SocketFlags.None);

            if (m_Header.m_Size != 0)
            {
                s.Send(Encoding.UTF8.GetBytes(m_Data), m_Header.m_Size, SocketFlags.None);
            }

        }

        public void Receive(Socket s)
        {
            byte[]  b = new byte[4];
            s.Receive(b, sizeof(int), SocketFlags.None);
            m_Header.m_To = BitConverter.ToInt32(b, 0);

            b = new byte[4];
            s.Receive(b, sizeof(int), SocketFlags.None);
            m_Header.m_From = BitConverter.ToInt32(b, 0);

            b = new byte[4];
            s.Receive(b, sizeof(int), SocketFlags.None);
            m_Header.m_Type = BitConverter.ToInt32(b, 0);

            b = new byte[4];
            s.Receive(b, sizeof(int), SocketFlags.None);
            m_Header.m_Size = BitConverter.ToInt32(b, 0);

            if (m_Header.m_Size != 0)
            {
                b = new byte[m_Header.m_Size];
                s.Receive(b, m_Header.m_Size, SocketFlags.None);
                m_Data = Encoding.UTF8.GetString(b, 0, m_Header.m_Size);
            }
            canalStop(s);

        }

    }
}
