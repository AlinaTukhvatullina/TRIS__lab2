using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.IO;

namespace CSharpClient
{
    class Program
    {
        static int ClientID;
        static string path = "logs.l";
        static List<string> lastMessages = new List<string>();

        static int GetValue(int min, int max)
        {
            while (true)
            {
                try
                {
                    int x = Convert.ToInt32(Console.ReadLine());
                    if ((x < min) || (x > max))
                    {
                        Console.WriteLine("Oшибка ввода\n");
                    }
                    else
                    {
                        return x;
                    }
                }
                catch
                {
                    Console.WriteLine("Oшибка ввода\n");
                }
            }

        }
        static void SendMessage(Socket s, IPEndPoint endPoint, int To, int From,  Messages Type = Messages.M_TEXT, string Data = "")
        {
            Message m = new Message(To, From, Type, Data);
            m.Send(s, endPoint);
        }

        static Message ReceiveMessage(Socket s)
        {
            Message m=new Message();
            m.Receive(s);
            return m;
        }

        static void getData()
        {

            while (true)
            {
                Socket s = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                IPEndPoint endPoint = new IPEndPoint(IPAddress.Parse("127.0.0.1"), 11111);
                
                SendMessage(s, endPoint, 0, ClientID, Messages.M_GETDATA);
                Message m = ReceiveMessage(s);
                if (m.GetM_Header().m_Type == (int)Messages.M_TEXT)
                {
                    Console.WriteLine("Сообщение от клиента " + m.GetM_Header().m_From + ": " + m.getM_Data() + '\n');
                    lastMessages.Add("Сообщение от клиента " + m.GetM_Header().m_From + ": " + m.getM_Data()+"::"+ DateTime.Now);
                    if (lastMessages.Count == 20)
                    {
                        lastMessages.RemoveAt(0);
                    }
                }
                Thread.Sleep(2000);
            }
        }

        static void ConnectToServer()
        {
            
            try
            {
                StreamReader fstream = new StreamReader(path);
                
                string buffer=fstream.ReadLine();
                ClientID = Convert.ToInt32(buffer);
                Console.WriteLine("Последние сообщения:");

                while (buffer != null)
                {
                    buffer = fstream.ReadLine();
                    Console.WriteLine(buffer);
                }
                fstream.Close();

            }
            catch
            {
                ClientID = 0;
            }


            IPEndPoint endPoint = new IPEndPoint(IPAddress.Parse("127.0.0.1"), 11111);
            Socket s = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

            SendMessage(s, endPoint, 0, ClientID, Messages.M_INIT);
            Message m = ReceiveMessage(s);

            if (m.GetM_Header().m_Type == (int)Messages.M_CONFIRM)
            {
                if (ClientID != m.GetM_Header().m_To)
                {
                    ClientID = m.GetM_Header().m_To;
                }
                Console.WriteLine("Ваш ID: " + ClientID);
                //НАдо добавить поток для GetDATA
                Thread getDataThread = new Thread(getData);
                getDataThread.Start();
            }
        }

        static void Main(string[] args)
        {

            Console.WriteLine("Выберите: 1 - подключиться к серверу. \n 0 - выйти.\n");
            int answer = GetValue(0, 1);
            if (answer == 0)
                Environment.Exit(0);
            IPEndPoint endPoint;
            Socket s;

            ConnectToServer();

            while (true)
            {
                endPoint = new IPEndPoint(IPAddress.Parse("127.0.0.1"), 11111);
                s = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

                Console.WriteLine("Выберите: 0 - отправить сообщение. \n 1 - выйти.\n");
                answer = GetValue(0, 1);
                switch (answer)
                {
                    case 0:
                        {
                            int ID = (int)Adresses.A_ALL;
                            Console.WriteLine("Выберите: 0 - одному клиенту.  \n1 - всем клиентам.\n");
                            int choice = GetValue(0, 1);
                            if (choice == 0)
                            {
                                Console.WriteLine("Введите ID клиента");
                                ID = Convert.ToInt32(Console.ReadLine());
                            }
                            Console.WriteLine("Введите сообщение");
                            string str = Console.ReadLine().ToString();
                            SendMessage(s, endPoint, ID, ClientID, Messages.M_TEXT, str);
                            
                            if (ReceiveMessage(s).GetM_Header().m_Type == (int)Messages.M_CONFIRM)
                            {
                                Console.WriteLine("Сообщение отправлено.");
                            }
                            else
                            {
                                Console.WriteLine("Ошибка. Сообщение не отправлено.");
                            }

                            break;
                        }
                    case 1:
                        {
                            //НАДО ДОБАВИТЬ РАБОТУ С ФАЙЛОМ
                            StreamWriter fstream = new StreamWriter(path);
                            fstream.WriteLine(ClientID.ToString());

                            foreach (var i in lastMessages)
                            {
                                fstream.WriteLine(i);
                            }
                            fstream.Close();


                            SendMessage(s, endPoint, (int)Adresses.A_BROCKER, ClientID, Messages.M_EXIT0);
                            if (ReceiveMessage(s).GetM_Header().m_Type == (int)Messages.M_CONFIRM)
                            {
                                Console.WriteLine("Успешно.");
                            }
                            else
                            {
                                Console.WriteLine("Ошибка.");
                            }
                            Environment.Exit(0);
                            break;
                        }
                }
            }

        }
    }
}
