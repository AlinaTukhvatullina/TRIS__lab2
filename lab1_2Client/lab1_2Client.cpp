// lab1_2Client.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "pch.h"
#include "framework.h"
#include "lab1_2Client.h"
#include "Message.h"
#include <string>
#include <fstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma warning(disable : 4996)

// Единственный объект приложения

CWinApp theApp;

int ClientId=0;
mutex hMutex;
bool connection;

vector<string> lastMessages;

template <typename T>
T GetValue(T min, T max)
{
    T x;

    while (!(std::cin >> x) || (x < min) || (x > max))
    {
        std::cin.clear();
        std::cin.ignore(255, '\n');
        std::cout << "Oшибка ввода" << std::endl;
    }
    return x;
}


void getData() {
    while (true) {
        CSocket client;
        Message m;
        Message::SendMessage(client, 0, ClientId, M_GETDATA);
        MsgHeader h_message;
        h_message = m.Receive(client);
        if (h_message.m_Type == M_TEXT) {

            //НА БУДУЩЕЕ (СОХРАНЕНИЕ ПОСЛЕДНИХ 20 СООБЩЕНИЙ)
            time_t seconds = time(NULL);
            tm* timeinfo = localtime(&seconds);

            lastMessages.push_back("Сообщение от клиента" + to_string(m.getM_Header().m_From) + ": " + m.getM_Data()+"::"+asctime(timeinfo));
            
            if (lastMessages.size() == 20) {
                lastMessages.erase(lastMessages.begin() + 0);
            }

            hMutex.lock();
            cout << "Сообщение от клиента"<<m.getM_Header().m_From<<": "<<m.getM_Data() << endl;
            hMutex.unlock();
        }

            
        Sleep(1000);
    }
}

void ConnectToServer(Message& m, MsgHeader& h_message, CSocket& client) {
    ifstream fin("logs.l");
    if (fin.is_open()) {
        fin >> ClientId;
    }


    AfxSocketInit();

    Message::SendMessage(client, 0, ClientId, M_INIT);
    h_message = m.Receive(client);

    if (h_message.m_Type == M_CONFIRM) {
        if (ClientId != h_message.m_To) {
            ClientId = h_message.m_To;
        }
        hMutex.lock();
        cout << "Ваш ID " << ClientId << endl;
        hMutex.unlock();
        thread t(getData);
        t.detach();
        
    }
    else {
        cout << "Ошибка. Клиент не подключен." << endl;
        return;
    }

    cout << "Последние сообщения: \n";
    if (fin.is_open()) {
        do {
            string s;
            getline(fin, s);
            cout << s << endl;
            fin.peek();
        } while (!fin.eof());
    }


}

void start() {
    cout << "Выберите: 1 - подключиться к серверу. \n 0 - выйти.\n";
    int answer = GetValue(0, 1);
    if (answer == 0)
        return;
    MsgHeader h_message;
    CSocket client;
    Message m;
    ConnectToServer(m, h_message, client);
    while (true) {

        cout << "Выберите: 0 - отправить сообщение. \n 1 - выйти.\n";
        answer = GetValue(0, 1);

        switch (answer)
        {
        case 0: {

            string str;
            int ID = A_ALL;
            cout << "Выберите: 0 - одному клиенту.  \n1 - всем клиентам." << endl;
            int choice = GetValue(0, 1);
            if (choice == 0) {
                cout << "Введите ID клиента\n";
                cin >> ID;
            }
            cout << "Введите сообщение" << endl;
            cin.ignore();
            getline(cin, str);
            Message::SendMessage(client, ID, ClientId, M_TEXT, str);
            h_message = m.Receive(client);
            hMutex.lock();
            if (h_message.m_Type == M_CONFIRM) {
                cout << "Сообщение отправлено." << endl;
            }
            else {
                cout << "Ошибка. Сообщение не отправлено." << endl;
            }
            hMutex.unlock();
            break;
        }




        case 1: {
            ofstream fout("logs.l");
            fout << ClientId << endl;
            for (auto i : lastMessages) {
                fout << i << endl;
            }
            fout.close();


            Message::SendMessage(client, 0, ClientId, M_EXIT0);
            h_message = m.Receive(client);
            hMutex.lock();
            if (h_message.m_Type == M_CONFIRM)
            {
                cout << "Успешно." << endl;
                hMutex.unlock();
                return;
            }
            else {
                cout << "Ошибка." << endl;
                hMutex.unlock();
            }


            break;
        }

        }
    }

}

int main()
{
    setlocale(LC_ALL, "rus");
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // инициализировать MFC, а также печать и сообщения об ошибках про сбое
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: вставьте сюда код для приложения.
            wprintf(L"Критическая ошибка: сбой при инициализации MFC\n");
            nRetCode = 1;
        }
        else
        {
            HWND hwnd = GetConsoleWindow();
            HMENU hmenu = GetSystemMenu(hwnd, FALSE);
            EnableMenuItem(hmenu, SC_CLOSE, MF_GRAYED);
            start();
            // TODO: вставьте сюда код для приложения.
        }
    }
    else
    {
        // TODO: измените код ошибки в соответствии с потребностями
        wprintf(L"Критическая ошибка: сбой GetModuleHandle\n");
        nRetCode = 1;
    }

    return nRetCode;
}
