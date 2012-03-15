'''
Created on 25 Feb 2012

@author: jussy

Main entry point for kaffeine client.
'''

import socket, sys, argparse
from lib import *

## Check args
#if len(sys.argv) < 2 or len(sys.argv) > 4:
#    sys.exit('usage: kaffeine.py request-uri [arguments]')
#else:
#    request = sys.argv[1]
#if len(sys.argv) == 3:
#    arg1 = sys.argv[2]
#    if not valid_arg(arg1):
#        sys.exit('error: unknown argument ' + arg1)
#    elif arg1 == '-v':
#        debug_enable()

parser = argparse.ArgumentParser(description='Kaffeine, a HTCPCP-compliant coffee pot server')
parser.add_argument('request-uri', nargs=1, help='Coffee URI')
parser.add_argument('-v', '--verbose', action='store_true', help='Print verbose messages', required=False)
parser.add_argument('-f', '--freeform', action='store_true', help='Run in freeform mode', required=False)
args = vars(parser.parse_args())
    
# Check request-uri is valid
request = args['request-uri']
if not valid_url(request):
    sys.exit('error: invalid request-uri structure')
else:
    parts = request[0].partition('://')
    path = parts[2].split('/')
    host = path[0]
    pot = path[1]
    adds = False
    if len(path) > 2:
        adds = path[2]

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

if args['freeform']:
    freeform_enable(s)

# Determine which method to use. If no additions were
# supplied, method will be BREW, otherwise it will be GET
if not adds:
    # Initial PROPFIND request
    response = propfind(s, pot)
    debug(response)
    c = status_code(response)
    
    if c['error']:
        close_sock(s, 'The server could not complete '
                 + 'the request. Program will exit.')

    order = get_order()
    response = brew(s, pot, order)
    
else:
    response = get(s, pot, adds)

debug(response)
c = status_code(response)
if c['error']:
        close_sock(s, 'The server could not complete '
                 + 'the request. Program will exit.')

data = raw_input('Type "get" to collect your coffee: ')
 
while not data == 'get':
    if data == 'quit':
        close_sock(s, 'Connection closed. Program will exit.')
    data = raw_input('Type "get" to collect your coffee: ')  

while True:
    response = get(s, pot, None)   
    debug(response)
    c = status_code(response)
    
    if c['code'] == '200':
        close_sock(s, 'Thank you for using kaffeine. We look forward'
                 + ' to quenching your digital thirst again.')
    if not c['code'] == '421':
        close_sock(s, 'The server could not complete '
                 + 'the request. Program will exit.')
    data = raw_input('Type "get" to collect your coffee: ')  
    
    





