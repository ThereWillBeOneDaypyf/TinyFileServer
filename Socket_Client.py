import socket
 
 
#实例化，连接，发送，接收回应，关闭
client = socket.socket()
 
client.connect(('127.0.0.1',1234))
end_msg = 'END'
client.send('GET test.txt'.encode())
print('发送数据...')
while True: 
	data = client.recv(2048)
	print('客户端接收到数据:',data.decode())
	msg = data.decode()
	if end_msg in msg:
		break;
 
client.close()