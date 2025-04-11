import socket
import select
from tkinter import Label

from timer import Timer

inputs = list()

def init_socket_connection() -> socket.socket: 
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind(('localhost', 5554))
    server.listen(1)

    print("Server started on port 5554...")
    return server

def start_listening(conn: socket.socket, t: Timer, label: Label):
    conn.setblocking(0)
   
    while True:
        readable, _, _ = select.select([conn], [], [], 0.03)
        if readable:
            data = conn.recv(8) #adjust the amount of bytes if you have to transfer a bigger signal size

            if not data:
                break
            
            match data.decode('utf-8'):

                case 'PAUSE':
                    print('pause')
                    t.pause_timer()

                case 'SPLIT':
                    print('split')
                    t.split()
                    
                case 'START':
                    print('start')
                    t.start_timer();
                
                case 'NONE':
                    print('Socket received a NONE signal')

    conn.close()

def close_connection():
    print('Closing socket connection')

    for s in inputs:
        s.close()
    inputs.clear()

def socket_server_wrap(t: Timer, label: Label):
    server = init_socket_connection()

    conn, addr = server.accept()
    print(f"Connection from {addr}")
    conn.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
    inputs.append(conn)
    start_listening(conn,t,label)
        