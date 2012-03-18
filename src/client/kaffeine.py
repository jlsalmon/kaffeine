'''
Created on 25 Feb 2012

@author: jussy

Main entry point for kaffeine client.
'''

import socket, sys, argparse, re
from consts import *
    
def main():
    args = setup_args()
    # Check request-uri is valid
    request = args['request-uri']
    if not verify_url(request):
        sys.exit('error: invalid request-uri structure')
    else:
        host, pot, adds = split_request(request)
    
    # Open socket, connect
    s = open_sock(host)
    print MSG_WELCOME
    
    if args['verbose']:
    	debug_enable()
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
            close_sock(s, MSG_ERROR)
    
        adds = get_order()
        response = brew(s, pot, adds)
        
    else:
        response = get(s, pot, adds)
    
    debug(response)
    c = status_code(response)
    if c['error']:
        close_sock(s, MSG_ERROR)
    
    if not adds or not re.match(".*unspecified.*", adds):
        prompt_get(s, pot)
    
    else:
        prompt_pour(s, pot)
        prompt_when(s, pot)
        prompt_get(s, pot)
    
def setup_args():
    parser = argparse.ArgumentParser(description='kaffeine, a HTCPCP-compliant coffee pot client')
    parser.add_argument('request-uri', nargs=1, help='Coffee URI')
    parser.add_argument('-v', '--verbose', action='store_true', help='print verbose messages', required=False)
    parser.add_argument('-f', '--freeform', action='store_true', help='run in freeform mode', required=False)
    return vars(parser.parse_args())

def split_request(req):
    parts = req[0].partition('://')
    path = parts[2].split('/')
    host = path[0]
    path = path[1].split('?')
    pot = path[0]
    adds = False
    if len(path) > 1:
        adds = path[1]
    return host, pot, adds
    
def propfind(sock, request):
    msg = METHOD_PROPFIND + '/' + request + HTCPCP_VERSION
    send_msg(sock, msg)
    return recv_msg(sock)
    
def brew(sock, pot, order):
    if not order == '':
        msg = METHOD_BREW + '/' + pot + HTCPCP_VERSION \
            + ACCEPT_ADDS + order + CONTENT_TYPE + MSG_BODY
    else:
        msg = METHOD_BREW + '/' + pot + HTCPCP_VERSION \
            + CONTENT_TYPE + MSG_BODY
    send_msg(sock, msg) 
    return recv_msg(sock)

def get(sock, pot, adds):
    
    if not adds:
        msg = METHOD_GET + '/' + pot + HTCPCP_VERSION
    else:
        msg = METHOD_GET + '/' + pot + '?' + adds + HTCPCP_VERSION
    send_msg(sock, msg)
    return recv_msg(sock)
  
def pour(sock, pot):
    send_msg(sock, METHOD_POUR + '/' + pot + HTCPCP_VERSION)
    return recv_msg(sock)
      
def when(sock, pot):
    send_msg(sock, METHOD_WHEN + '/' + pot + HTCPCP_VERSION)
    return recv_msg(sock)
    
def send_msg(s, msg):
    try:
        s.send(msg)
        debug(msg, 'Sent:\n')
    except socket.error, e:
        sys.exit("Error sending data: %s" % e)

def recv_msg(s):
    try:
        buf = s.recv(MSG_BUF_SIZE)
    except socket.error, e:
        sys.exit("Error receiving data: %s" % e)
    return buf
    
def open_sock(host):
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
    return s 

def close_sock(s, msg):
    s.send('quit')
    print s.recv(MSG_BUF_SIZE)
    s.close()
    sys.exit(msg)
    
def extract_body(msg):
    parts = msg.split('\r\n\r\n')
    if len(parts) >= 2:
        return parts[1]
    else: return ""
    
def status_code(msg):
    parts = msg.split(' ')
    for c in C_ERROR:
        if re.match(parts[1], c):
            return { 'error' : True, 'code' : c}
    for c in C_SUCCESS:
        if re.match(parts[1], c):
            return { 'error' : False, 'code' : c}

def verify_url(url):
    parts = url[0].partition('://')
    if not re.match('coffee', parts[0]):
        return False 
    path = parts[2].split('/')
    if len(path) < 2:
        return False
    if not re.match('[a-zA-Z0-9\.]+', path[0]):
        return False
    if not re.match('pot-[0-9]', path[1]):
        return False
    if re.match('\?.*', path[1]):
        path = path[1].split('?')
        if not re.match('\?([a-z-]+=[a-z0-9]+(&[a-z-]+=[a-z0-9]+)*)+',
                         path[1]):
            return False
    return True
    
def prompt_get(s, pot):
    get_input(PROMPT_GET, 'get', s)
    
    while True:
        response = get(s, pot, None)   
        debug(response)
        c = status_code(response)
        
        if c['code'] == '200':
            close_sock(s, MSG_COMPLETE)
        if not c['code'] in ['421', '428']:
            close_sock(s, MSG_ERROR)
        get_input(PROMPT_GET, 'get', s)
            
def prompt_pour(s, pot):
    get_input(PROMPT_POUR, 'pour', s)
     
    while True:
        response = pour(s, pot)   
        debug(response)
        c = status_code(response)
        
        if c['code'] == '200':
            return
        if not c['code'] in ['421', '428']:
            close_sock(s, MSG_ERROR)
        get_input(PROMPT_POUR, 'pour', s)
        
def prompt_when(s, pot):
    get_input(PROMPT_WHEN, 'when', s)
        
    while True:
        response = when(s, pot)   
        debug(response)
        c = status_code(response)
        
        if c['code'] == '200':
            return
        if not c['code'] in ['421', '428']:
            close_sock(s, MSG_ERROR)
        get_input(PROMPT_WHEN, 'when', s)
        
def get_order():
    if re.match(get_yn_input("Would you like any additions?"), 'y'):
        finished = False
        order = ''
        while not finished:
            add = raw_input('Enter addition type: ')
            quant = raw_input('Enter quantity (or hit return to live dangerously): ')
            if re.match(quant, '\n'):
                quant = 'unspecified'
            order += add + '=' + quant
            if re.match(get_yn_input("Any more additions?"), 'n'):
                finished = True
            else:
                order += '&'
        return order + "\r\n"
    else: 
        return ''

def get_input(prompt, target, s):
    data = raw_input(prompt)
    while not data == target:
        if data == 'quit':
            close_sock(s, MSG_QUIT)
        data = raw_input(prompt)
    
def get_yn_input(prompt):
    input = raw_input(prompt + " (y/n) ")
    while not re.match("[yn]", input):
        input = raw_input('Please answer y or n: ')
    return input

def debug_enable():
    global DEBUG
    DEBUG = True
    
def debug(msg, prefix='Server replies:\n'):
    if DEBUG == True:
        print prefix + msg
    elif not re.match('Sent', prefix): 
        print extract_body(msg)
    
def freeform_enable(s):
    print ('\nRunning in freeform mode.')
    while True:
        data = raw_input('Enter input: ')
        if data == 'quit':
            close_sock(s, 'Connection closed. Program will exit.')
        if not re.match('([a-zA-Z]*)\s(/pot-[0-9])(\?([a-z-]+=[a-z0-9]+(&[a-z-]+=[a-z0-9]+)*)+)*\s(HTCPCP/1\.0)', data):
            print 'Invalid HTCPCP request.'
        else:
            send_msg(s, data)
            debug(recv_msg(s))

if __name__ == "__main__":
    main()


