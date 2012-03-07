'''
Created on 25 Feb 2012

@author: jussy

Main entry point for kaffeine client.
'''

import socket
import sys
import lib
from consts import *

# Check args
if len(sys.argv) < 2:
    sys.exit('usage: kaffeine.py request-uri')
else:
    request = sys.argv[1]

# Check request-uri is valid
if not lib.valid_url(request):
    sys.exit('Invalid request-uri structure')
else:
    parts = request.partition('://')
    path = parts[2].split('/')
    host = path[0]

# Open socket, connect
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((host, USR_PORT))

print WELCOME_MSG

# Determine which method to use. If no additions were
# supplied, method will be BREW, otherwise it will be GET
if len(path) < 3:
    # Initial PROPFIND request
    response = lib.propfind(s, request)
    print 'Server replies:\n' + response

    if not lib.status_code(response):
        s.close()
        sys.exit('The server could not complete '
                 + 'the request. Program will exit.')

    order = lib.get_order()
    print order
    response = lib.brew(s, request, order)
    
else:
    response = lib.get(s, request)

print 'Server replies:\n' + response

while True:
    data = raw_input('Type "get" to collect your coffee: ')
    response = lib.get(s, request)
    print 'Server replies:\n' + response
    
    if data == 'quit':
        s.close()
        sys.exit('Connection closed. Program will exit.')





