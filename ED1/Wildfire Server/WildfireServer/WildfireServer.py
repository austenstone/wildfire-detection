import random
from datetime import datetime, time
import random
import json
import time
import serial
import logging
from websocket_server import WebsocketServer
import MySQLdb
import _thread


s = serial.Serial('COM3', 9600)
ip = "149.56.101.13" #73.139.183.89 or 192.168.1.4
username = "miltonlaxer"
password = "Ams188623"
database = "wildfire"

def write(serialdata):
	db = MySQLdb.connect(ip, username, password, database)
	cursor = db.cursor()
	sql = "INSERT INTO `wildfire`.`wildfire1` (`time`, `temp`, `humidity`, `ppm`, `lpg`, `conc`, `smoke`, `o3`) VALUES (NOW(), '{tmp}', '{hum}', '{ppm}', '{lpg}', '{con}', '{smo}', '{o3}');".format(
		tmp=float(serialdata[0]), hum=float(serialdata[1]), ppm=float(serialdata[2]), 
		lpg=float(serialdata[3]), con=float(serialdata[4]), smo=float(serialdata[5]), o3=float(serialdata[6]))
	try:
		print("Success: ", sql)
		cursor.execute(sql)
		db.commit()
	except:
		print("Failed: ", sql)
		try:
			print("Creating...");
			sql = "ALTER `wildfire1` ADD (`time`, `temp`, `humidity`, `ppm`, `lpg`, `conc`, `smoke`, `o3`)";
		except:
			print("Failed creating...");

		db.rollback()

def new_client(client, server):
	print("New Client: ", client["id"], " @", client["address"], "\n")
	server.send_message(client, "Welcome to The Wildfire Websocket Server - " + str(client["address"][0]) + " Client ID: " + str(client["address"][1]))

def client_left(client, server):
	print("Disconnect: ", client["id"], " @", client["address"], "\n")
	server.send_message_to_all("Disconnected from the wildfire server websocket.")

def receive(client, server, message):
	print("Received:   ", message)

def MySQLserverStart():
	while True:
		print(serialread())

def getSerialArray():
	while True:
		sdata = serialread()
		sdata = sdata.split(",")
		if(len(sdata) == 7):
			return sdata
		else:
			continue

def serialread():
	try:
		while True:
			data = s.readline().strip().decode("utf-8")
			if not data:
				continue
			else:
				return data
	except:
		return None

def feedClient(server):
	while True:
		serialdata = getSerialArray()
		write(serialdata)
		server.send_message_to_all(json.dumps({"time": str(datetime.now()),
											    "temp": float(serialdata[0]),
												"humidity": float(serialdata[1]),
												"ppm": float(serialdata[2]),
												"lpg": float(serialdata[3]),
												"conc": float(serialdata[4]),
												"smoke": float(serialdata[5]),
												"o3": float(serialdata[6])}))

def main():
	print("""__          ___ _     _  __ _             _____                          
\ \        / (_) |   | |/ _(_)           / ____|                         
 \ \  /\  / / _| | __| | |_ _ _ __ ___  | (___   ___ _ ____   _____ _ __ 
  \ \/  \/ / | | |/ _` |  _| | '__/ _ \  \___ \ / _ \ '__\ \ / / _ \ '__|
   \  /\  /  | | | (_| | | | | | |  __/  ____) |  __/ |   \ V /  __/ |   
	\/  \/   |_|_|\__,_|_| |_|_|  \___| |_____/ \___|_|    \_/ \___|_| 
	 AUTHORS: Austen Stone, Timothy Sketchley, Noah Gauthier
	""")

	server = WebsocketServer(9998, "0.0.0.0")
	print("\nWebsocket Started: ", server.server_address[0])
	server.set_fn_new_client(new_client)
	server.set_fn_client_left(client_left) 
	server.set_fn_message_received(receive)
	thread = _thread.start_new_thread(feedClient, (server,))
	server.run_forever()

if __name__ == "__main__":
	main()