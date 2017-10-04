from datetime import datetime, time
import random
import json
import time
import serial
import logging
from websocket_server import WebsocketServer
import MySQLdb

ip = "73.139.183.89" #73.139.183.89 or 192.168.1.4
username = "wildfir"
password = "1102"
database = "wildfire"

def readAll():
    db = MySQLdb.connect(ip, username, password, database)
    cursor = db.cursor()
    cursor.execute("SELECT * FROM demo")
    if(cursor.rowcount > 0):
        data = cursor.fetchall()
        print("TABLE:")
        for row in data:
            print(row[0], " ", str(row[1]).strip())
    else:
        print("The table is empty!")
    db.close()

def write(time, temp):
    db = MySQLdb.connect(ip, username, password, database)
    cursor = db.cursor()
    print("Trying to insert " + time + ", " + temp + "...")
    try:
        sql = "INSERT INTO demo(time, temp) VALUES (NOW(), '{tmp}')".format(tmp=temp)
        print(sql)
        cursor.execute(sql)
        db.commit()
    except:
        print("Failed to insert")
        db.rollback()

def empty(list):
    db = MySQLdb.connect(ip, username, password, database)
    cursor = db.cursor()
    print("Trying to empty list...")
    try:
        sql = "TRUNCATE {tablename}".format(tablename=list)
        cursor.execute(sql)
        db.commit()
    except:
        print("Failed to empty table ", list)
        db.rollback()

def readSerial():
    try:
        s = serial.Serial('COM3', 9600)
        return s.readline().strip().decode("utf-8")
    except:
        print("No serial connection...")
        time.sleep(5)

def serverStart():
    server = WebsocketServer(9998)
    print("\nWebsocket Started: ", server.server_address[0])
    server.set_fn_new_client(new_client)
    server.set_fn_client_left(client_left) 
    server.set_fn_message_received(receive)
    server.run_forever()

def new_client(client, server):
    print("New Client: ", client["id"], " @", client["address"], "\n")
    server.send_message(client, "Welcome to The Wildfire Websocket Server - " + str(client["address"][0]) + " Client ID: " + str(client["address"][1]))
    while(True):
        try:
            #server.send_message_to_all(readSerial())
            server.send_message_to_all(json.dumps({"time": str(datetime.now()),
                                                   "temp": random.randint(0, 99),
                                                   "gas": random.randint(10, 99),
                                                   "ppm": random.randint(10, 99),
                                                   "co": random.randint(10, 99)}))
            time.sleep(1)
        except:
            time.sleep(1)

def client_left(client, server):
        print("Disconnect: ", client["id"], " @", client["address"], "\n")
        try:
            server.send_message_to_all("Disconnected from the wildfire server websocket.")
        except:
            return

def receive(client, server, message):
        print("Received:   ", message)

def main():
    print("""__          ___ _     _  __ _             _____                          
\ \        / (_) |   | |/ _(_)           / ____|                         
 \ \  /\  / / _| | __| | |_ _ _ __ ___  | (___   ___ _ ____   _____ _ __ 
  \ \/  \/ / | | |/ _` |  _| | '__/ _ \  \___ \ / _ \ '__\ \ / / _ \ '__|
   \  /\  /  | | | (_| | | | | | |  __/  ____) |  __/ |   \ V /  __/ |   
    \/  \/   |_|_|\__,_|_| |_|_|  \___| |_____/ \___|_|    \_/ \___|_| 
     AUTHORS: Austen Stone, Timothy Sketchley, Noah Gauthier
    """)

    print("COMMAND WINDOW")
    print("1. Run")
    print("2. Read")
    print("3. Write")
    print("4. Empty")
    print("5. Status")
    while(True):
        command = input("\nCommand: ")
        if(command == "run" or command == "1"):
            serverStart()
            #_thread.start_new_thread (serverStart, ())
        if(command == "read" or command == "2"):
            readAll()
        if(command == "write" or command == "3"):
            new = input("Enter value: ")
            write(str(new))
        if(command == "empty" or command == "4"):
            empty("demo")
        if(command == "Status" or command == "5"):
            getStatus()

if __name__ == "__main__":
    main()