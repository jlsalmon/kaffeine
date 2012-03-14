'''
Created on 25 Feb 2012

@author: jussy

Main entry point for kaffeine client.
'''

import socket
import sys
from lib import *

# Check args
if len(sys.argv) < 2 or len(sys.argv) > 3:
    sys.exit('usage: kaffeine.py request-uri [arguments]')
else:
    request = sys.argv[1]
if len(sys.argv) == 3:
    arg = sys.argv[2]
    if not valid_arg(arg):
        sys.exit('unknown argument ' + arg)
    
# Check request-uri is valid
if not valid_url(request):
    sys.exit('Invalid request-uri structure')
else:
    parts = request.partition('://')
    path = parts[2].split('/')
    host = path[0]

# Open socket, connect
try:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
except socket.error, e:
    sys.exit("Strange error creating socket: %s" % e)

try:
    s.connect((host, USR_PORT))
except socket.gaierror, e:
    sys.exit("Address-related error connecting to server: %s" % e)
except socket.error, e:
    sys.exit("Connection error: %s" % e)

print WELCOME_MSG

# Determine which method to use. If no additions were
# supplied, method will be BREW, otherwise it will be GET
if len(path) < 3:
    # Initial PROPFIND request
    response = propfind(s, request)
    debug(response)
    c = status_code(response)
    
    if c['error']:
        s.send('quit')
        s.close()
        sys.exit('The server could not complete '
                 + 'the request. Program will exit.')

    order = get_order()
    print order
    response = brew(s, request, order)
    
else:
    response = get(s, request)

debug(response)
c = status_code(response)
if c['error']:
        s.send('quit')
        s.close()
        sys.exit('The server could not complete '
                 + 'the request. Program will exit.')

data = raw_input('Type "get" to collect your coffee: ')
 
while not data == 'get':
    if data == 'quit':
        s.send('quit')
        s.close()
        sys.exit('Connection closed. Program will exit.')
    data = raw_input('Type "get" to collect your coffee: ')  

while True:
    response = get(s, request)   
    debug(response)
    c = status_code(response)
    
    if c['code'] == '200':
        s.send('quit')
        s.close()
        sys.exit('Thank you for using kaffeine. We look forward'
                 + ' to quenching your digital thirst again.')
    if not c['code'] == '421':
        s.send('quit')
        s.close()
        sys.exit('The server could not complete '
                 + 'the request. Program will exit.')
    data = raw_input('Type "get" to collect your coffee: ')  
    
    





