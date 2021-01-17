// Lab1_2_ClientServer.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "pch.h"
#include "framework.h"
#include "Lab1_2_ClientServer.h"
#include "Message.h"
#include "Session.h"
#include <vector>
#include <cstdio>
#include <ctime>
#include <chrono>
#include <fstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Единственный объект приложения

CWinApp theApp;

int gMaxID = 0;
map<int, shared_ptr<Session>> gSessions;


template <typename T>
void removeInVector(std::vector<T>& vec, size_t pos)
{
    vec.erase(vec.begin() + pos);
    vector<int>(vec).swap(vec);

}

void TimeOut() {

    while (true)
    {
        /*if(!gSessions.empty())
        for (const auto& i : gSessions) {
            double worktime = clock() - i.second->getTime();
            if (worktime > 15000) {
                cout << "КЛиент " << i.first << "был отключен из-за долгой неактивности." << endl;
                gSessions.erase(i.first);
                freeIDs.push_back(i.first);
            }
        }*/

        for (int i = A_ALL+1; i <= gMaxID; i++)
        {
            if (gSessions.find(i) != gSessions.end())
            {
                double  workTime = clock() - gSessions[i]->getTime();
                if (workTime > 100000)
                {
                    cout << "Клиент " << i << " был отключен из-за долгой неактивности." << endl;
                    gSessions.erase(i);

                }
            }
        }
        Sleep(1000);

        
    }
}

void ProcessClient(SOCKET hSock) {
    CSocket s;
    s.Attach(hSock);
    Message m;
    
        switch (m.Receive(s)) {
        case M_INIT: {
            int clientID;
            if (m.getM_Header().m_From==0){
                gMaxID++;
                clientID = gMaxID;
            }
            else {
                clientID = m.getM_Header().m_From;
            }


            ofstream fout("logs.l", ios::out | ios::trunc);
            fout << gMaxID<<endl;
            fout.close();


            auto pSession = make_shared<Session>(clientID, clock());
            cout << "Клиент " << clientID << " подключен." << endl;
            gSessions[pSession->getM_ID()] = pSession;

            Message::SendMessage(s, pSession->getM_ID(), A_BROCKER, M_CONFIRM);
            break;
        }

        case M_EXIT0: {
            
            gSessions.erase(m.getM_Header().m_From);
            cout << "Клиент " << m.getM_Header().m_From << " отключен." << endl;
            Message::SendMessage(s, m.getM_Header().m_From, A_BROCKER, M_CONFIRM);
            break;
        }
        case M_GETDATA: {
            if (gSessions.find(m.getM_Header().m_From) != gSessions.end()) {

                gSessions[m.getM_Header().m_From]->setTime(clock());
                gSessions[m.getM_Header().m_From]->Send(s);
                
            }
            break;
        }
        default: {
            if (gSessions.find(m.getM_Header().m_From) != gSessions.end())
            {
                if (gSessions.find(m.getM_Header().m_To) != gSessions.end())
                {
                    gSessions[m.getM_Header().m_To]->Add(m);
                }
                else if (m.getM_Header().m_To == A_ALL)
                {
                    
                    for (const auto& i : gSessions) {
                        if (i.first != m.getM_Header().m_From) {
                            i.second->Add(m);
                        }
                    }

                }
                Message::SendMessage(s, m.getM_Header().m_From, A_BROCKER, M_CONFIRM);
                gSessions[m.getM_Header().m_From]->setTime(clock());
            }
            break;
        }
        }
    

}


void start() {
    AfxSocketInit();
    CSocket Server;
    Server.Create(11111);

    thread tt(TimeOut);
    tt.detach();

    ifstream fin("logs.l");
    if (fin.is_open()) {
        fin >> gMaxID;
    }
    fin.close();

    while (true)
    {
        Server.Listen();
        CSocket s;
        Server.Accept(s);
        thread t(ProcessClient, s.Detach());
        t.detach();
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
            start();
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
