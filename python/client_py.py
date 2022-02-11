import socket

s = socket.socket()

myPort, myHost = 7655, socket.gethostname()
targetPort, targetHost = 1234, socket.gethostname()

myAddr = (myHost, myPort)
targetAddr = (targetHost, targetPort)

s.bind(myAddr)
s.connect(targetAddr)

while True:
    msg = input("Enter text: ")
    recv = b""

    s.send(msg.encode("UTF8"))
    s.send(b";")

    if(msg == "q"):
        break

    while True:
        ch = s.recv(2)
        recv += ch
        if(b";" in ch): break

    print("Recieved:", recv[:-1:1].decode())

s.close()
