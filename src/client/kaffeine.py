'''
Created on 25 Feb 2012

@author: jussy
'''
# filename: client.py

import socket
import sys
import lib
import c

if len(sys.argv) < 2:
<<<<<<< HEAD
    sys.exit('usage: client.py request-uri')
    #request = 'coffee://localhost/pot-1/?milk=dash'
=======
    sys.exit('usage: kaffeine.py request-uri')
>>>>>>> 4663b306953db42e75770fdd588e6f92af5b5db9
else:
    request = sys.argv[1]

if not lib.valid_url(request):
    sys.exit('Invalid request-uri structure')
else:
    parts = request.partition('://')
    path = parts[2].split('/')
    host = path[0]

# Open socket, connect
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((host, c.USR_PORT))

print c.WELCOME_MSG

# Initial PROPFIND request
#msg = c.METHOD_PROPFIND + c.PROTOCOL
#s.send(msg)
#print ('Sent: ' + msg)
#data = s.recv(c.MSG_BUF_SIZE)
#print 'Server replies: ' + data
    
if len(path) < 3:
    method = c.METHOD_BREW
    order = lib.get_order()
    
else:
    method = c.METHOD_GET

# Send the message!
msg = method + request + c.PROTOCOL
s.send(msg)
print ('Sent: ' + msg)

data = s.recv(c.MSG_BUF_SIZE)
print ('Server replies: ' + data)

s.send(raw_input('Enter input: '))

data = s.recv(c.MSG_BUF_SIZE)
print ('Server replies: ' + data)

s.close()
sys.exit('Connection closed. Program will exit.')



