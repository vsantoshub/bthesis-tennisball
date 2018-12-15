#!/usr/bin/env python

import socket
from pcduino_gpio import *
import os
import subprocess

"""
A simple tcp server to kill C app correctly
"""
#receive commands to move robot
def errorHandler():
    print "Received command not valid"


#method to kill opencv process
def kill_pid_app():
    TCP_IP = '127.0.0.1'
    TCP_PORT = 9000
    BUFFER_SIZE = 1024
    MESSAGE = "$T1STOP"
    cmd="echo quit > /tmp/app.fifo"

    #r = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    #r.connect((TCP_IP, TCP_PORT))
    #r.send(MESSAGE)
    #data = r.recv(BUFFER_SIZE)
    #r.close()
    print "Application stopped, can be killed now..."
    p = subprocess.Popen(cmd, shell=True, stderr=subprocess.PIPE)
    print "OpenCV process dead.."
    print "Tell Arduino that the user can turn the system off..."
    gpio_out('gpio1', '1')
    print "Desligar o sistema"
    shutdown="init 0"
    p = subprocess.Popen(shutdown, shell=True, stderr=subprocess.PIPE)

host = ''
port = 10600
backlog = 5
size = 1024
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((host,port))
s.listen(backlog)

print "Server running at port 10600"
while True:
    client, address = s.accept()
    data = client.recv(size)
    data = str(data.decode('ascii'))
    if data:
        print("received data: "+data)
        #client.send(data)
        data=data.strip()
        cmd_tst="$KILL_ME"
        if data == cmd_tst:
            print("right signal received")
            kill_pid_app()
            #client.send("Assim que o LED vermelho acender, pode desligar o robo")
        else:
            print("unexpected command")
client.close()

