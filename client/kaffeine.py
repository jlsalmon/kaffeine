'''
Created on 25 Feb 2012

@author: jussy
'''
# filename: client.py

import socket
import sys
import lib
import c

if 1 > len(sys.argv) > 2:
    sys.exit('usage: client.py request-uri')
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
s.send(c.METHOD_PROPFIND + c.PROTOCOL)
data = s.recv(c.MSG_BUF_SIZE)
print 'Server replies: ' + data
    
if len(path) < 3:
    method = c.METHOD_BREW
    order = lib.get_order()
    
else:
    method = c.METHOD_GET

# Send the message!
s.send(method + request + c.PROTOCOL)

data = s.recv(c.MSG_BUF_SIZE)
print 'Server replies: ' + data
s.send(raw_input())
    
s.close()



