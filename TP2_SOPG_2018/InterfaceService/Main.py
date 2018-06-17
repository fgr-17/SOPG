import socket
import sys
import time
import thread
import datetime


def writeLog(id):

	now = datetime.datetime.now()

	date=now.strftime("%Y/%m/%d")
	time=now.strftime("%H:%M:%S")

        fp = open("/tmp/log.txt","a")
        fp.write(str(id)+","+date+","+time+"\n")
        fp.close()


def rcvThread(sock):
	global socketOk
	print("INICIO thread recepcion")
	while True:
		data = sock.recv(128)
		print("LLEGO por socket:"+data)
		if len(data)==0:
			break
		data = data.split(">ID:")
		for d in data:
			if len(d)>=2:
				id = d[0:4]
				if id.isdigit():
					print("Llego acceso con id: "+id)
					writeLog(id)
					print("Se habilita acceso")
					time.sleep(1)
					sock.send(">OUT:1\r\n")

	print("FIN thread recepcion")
	socketOk=False

socketOk=True

while True:
	try:
		# Creo TCP/IP socket
		sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		server_address = ('127.0.0.1', 10000)
		print >>sys.stderr, 'connecting to %s port %s' % server_address
		sock.connect(server_address)
		socketOk=True
		# Creo thread para escuchar paquetes
		thread.start_new_thread( rcvThread, (sock, ) )

		while socketOk==True:
			time.sleep(1)

		sock.close()

	except:
		time.sleep(1)
		print("Socket invalido, reintento...")
