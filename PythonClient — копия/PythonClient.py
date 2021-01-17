import socket, struct
from enum import IntEnum
from threading import Thread
from time import sleep
import sys
import datetime
import os


HOST = '127.0.0.1'
PORT = 11111

def GetValue(min, max):
    while True:
        try:
            x=int(input())
            if (x<min or x>max):
                print('Ошибка ввода')
            else:
                return x
        except IOError:
            print('Ошибка ввода')


class Addresses(IntEnum):
    A_BROCKER = 0
    A_ALL = -1

class Messages(IntEnum):
    M_INIT=0
    M_EXIT0=1
    M_GETDATA=2
    M_NODATA=3
    M_TEXT=4
    M_CONFIRM=5


class MsgHeader():
    def __init__(self, m_To=0, m_From=0, m_Type=0, m_Size=0):
        self.m_To=m_To
        self.m_From=m_From
        self.m_Type=m_Type
        self.m_Size=m_Size
    def HeaderInit(self, header):
        self.m_To=header[0]
        self.m_From=header[1]
        self.m_Type=header[2]
        self.m_Size=header[3]

class Message():
    def __init__(self, To=0, From=0, Type=Messages.M_TEXT, Data=''):
        self.m_Header=MsgHeader()
        self.m_Header.m_To=To;
        self.m_Header.m_From=From;
        self.m_Header.m_Type=Type;
        self.m_Header.m_Size=int(len(Data))
        self.m_Data=Data
    def SendData(self, s):
        s.send(struct.pack('i', self.m_Header.m_To))
        s.send(struct.pack('i', self.m_Header.m_From))
        s.send(struct.pack('i', self.m_Header.m_Type))
        s.send(struct.pack('i', self.m_Header.m_Size))
        if self.m_Header.m_Size>0:
            s.send(struct.pack(f'{self.m_Header.m_Size}s', self.m_Data.encode('utf-8')))
    def ReceiveData(self, s):
        self.m_Header=MsgHeader()
        self.m_Header.HeaderInit(struct.unpack('iiii', s.recv(16)))
        if self.m_Header.m_Size>0:
            self.m_Data=struct.unpack(f'{self.m_Header.m_Size}s', s.recv(self.m_Header.m_Size))[0]


def connect(Socket):
    Socket.connect((HOST, PORT))

def disconnect(Socket):
    Socket.close()


def SendMessage(Socket, To, From, Type=Messages.M_TEXT, Data=''):
    connect(Socket)
    m=Message(To, From, Type, Data)
    m.SendData(Socket)

def Receive(Socket):
    m=Message()
    m.ReceiveData(Socket)
    disconnect(Socket)
    return m



ClientID=0
lastMessages=list()

def GetData(ID):
    while True:
        s2=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        SendMessage(s2, 0, ID, Messages.M_GETDATA)
        m=Receive(s2)
        if (m.m_Header.m_Type==Messages.M_TEXT):
            print('Сообщение от клиента '+str(m.m_Header.m_From)+': '+m.m_Data.decode('utf-8'))
            lastMessages.append('Сообщение от клиента '+str(m.m_Header.m_From)+': '+m.m_Data.decode('utf-8')+'::'+datetime.datetime.now().strftime("%m/%d/%Y, %H:%M:%S"))
        
        if (len(lastMessages)>20):
            lastMessages.pop(0)
       
        sleep(2)

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    print("Выберите: 1 - подключиться к серверу. \n 0 - выйти.\n")
    choice=GetValue(0,1)
    if (choice==0):
        sys.exit()

    try:
        f = open("logs.l", 'r')
        ClientID=int(f.readline())
        lastMess=f.readlines()
        f.close()
        print('Последние сообщения: \n')
        for x in lastMess: print(x)

    except IOError:
        ClientID=0
    
    SendMessage(s, 0 , ClientID, Messages.M_INIT)
    m=Receive(s)
    
    if (m.m_Header.m_Type==Messages.M_CONFIRM):
        if (ClientID!=m.m_Header.m_To):
            ClientID=m.m_Header.m_To
        print('Ваш ID: '+str(ClientID)+'\n')
        t=Thread(target=GetData, args=(ClientID,))
        t.daemon=True
        t.start()
    else:
        print('Ошибка')
        sys.exit(0)

while True:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        print("Выберите: 0 - отправить сообщение. \n 1 - выйти.\n")
        choice=GetValue(0,1)
        if (choice==0):
            ID=Addresses.A_ALL
            print("Выберите: 0 - одному клиенту.  \n1 - всем клиентам.\n")
            choice=GetValue(0,1)
            if (choice==0):
                ID=int(input("Введите ID клиента\n"))
            str=input("Введите сообщени\n")
            SendMessage(s, ID, ClientID, Messages.M_TEXT, str)
            if Receive(s).m_Header.m_Type==Messages.M_CONFIRM:
                print("Сообщение отправлено.\n")
            else: print("Ошибка. Сообщение не отправлено.\n")
        elif choice==1:
            f = open("logs.l", 'w')
            f.write(f'{ClientID}'+'\n')
            for x in lastMessages: f.write(x+'\n')
            f.close()

            SendMessage(s,0, ClientID, Messages.M_EXIT0)
            if Receive(s).m_Header.m_Type==Messages.M_CONFIRM:
                print("Успешно.\n")
            else: print("Ошибка\n")
            sys.exit(0)