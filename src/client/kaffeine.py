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
    sys.exit('usage: kaffeine.py request-uri')
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
  
if len(path) < 3:
    method = c.METHOD_BREW
    # Initial PROPFIND request
    msg = c.METHOD_PROPFIND + request + c.HTCPCP_VERSION
    s.send(msg)
    print 'Sent: ' + msg
    data = s.recv(c.MSG_BUF_SIZE)
    print 'Server replies: ' + data
    order = lib.get_order()
    
else:
    method = c.METHOD_GET

# Send the message!
msg = method + request + c.HTCPCP_VERSION
s.send(msg)
print 'Sent: ' + msg

data = s.recv(c.MSG_BUF_SIZE)
print 'Server replies:\n' + data

while True:
    data = raw_input('Enter input: ')
    s.send(data)
    print 'Server replies:\n' + s.recv(c.MSG_BUF_SIZE)
    
    if data == 'quit':
        s.close()
        sys.exit('Connection closed. Program will exit.')





