"""
File:       kaffeine.py
Author:     Justin Lewis Salmon
Student ID: 10000937
Created on: 25 Feb 2012

Client script for use with kaffeine server, or any
other HTCPCP-compliant coffee-pot server.

"""

import socket, sys, argparse, re, pickle
from consts import *
    
def main():
    """"kaffeine, a HTCPCP-compliant coffee pot client
    
    usage: kaffeine.py [-h] [-v] [-f] request-uri

    positional arguments:
      request-uri     Coffee URI

    optional arguments:
      -h, --help      show this help message and exit
      -v, --verbose   print verbose messages
      -f, --freeform  run in freeform mode
 
    """
    args = setup_args()
    # Verify and parse request-uri.
    request = args['request-uri']
    if request[0] == 'coffee:':
        host, pot, adds = load_request()
    elif not verify_url(request):
        sys.exit('error: invalid request-uri structure')
    else:
        host, pot, adds = split_request(request)
    
    s = open_sock(host)
    print MSG_WELCOME
    
    if args['verbose']:
    	debug_enable()
    if args['freeform']:
        freeform_enable(s)
    
    # Determine which method to use. If no additions were
    # supplied, method will be BREW, otherwise it will be GET.
    if not adds:
        # Initial PROPFIND request.
        response = propfind(s, pot)
        debug(response)
        c = status_code(response)
        if c['error']:
            close_sock(s, MSG_ERROR)
        # Prompt for additions.
        adds = get_order()
        response = brew(s, pot, adds)      
    else:
        response = get(s, pot, adds)
    
    debug(response)
    c = status_code(response)
    if c['error']:
        close_sock(s, MSG_ERROR)
    
    # Save this request for next time.
    save_request({'host':host, 'pot':pot, 'adds':adds})
    
    # If there are no additions, or all addition quantities have
    # been specified, prompt for a GET request. Otherwise, prompt
    # for the POUR -> WHEN -> GET sequence.
    if not adds or not re.match(".*unspecified.*", adds):
        prompt_get(s, pot)
    else:
        prompt_pour(s, pot)
        prompt_when(s, pot)
        prompt_get(s, pot) 
    
def setup_args():
    """Return our args array which argparse built for us. """
    parser = argparse.ArgumentParser(description='kaffeine, a HTCPCP-compliant coffee pot client')
    parser.add_argument('request-uri', nargs=1, help='Coffee URI')
    parser.add_argument('-v', '--verbose', action='store_true', help='print verbose messages', required=False)
    parser.add_argument('-f', '--freeform', action='store_true', help='run in freeform mode', required=False)
    return vars(parser.parse_args())

def split_request(req):
    """Return a tuple containing host, pot no and additions. """
    parts = req[0].partition('://')
    path = parts[2].split('/')
    host = path[0]
    path = path[1].split('?')
    pot = path[0]
    adds = False
    if len(path) > 1:
        adds = path[1]
    return host, pot, adds
    
def save_request(request):
    """ Pickle a coffee request to a file. """
    with open("orders.coffee", "wb") as f:
        pickle.dump(request, f)
    
def load_request():
    """Unpickle a coffee request from a file. """
    with open("orders.coffee", "rb") as f:
        request = pickle.load(f)
        print ('\nRepeating your previous order of ' + request['adds']
			+ ' at ' + request['host'] + ' on ' + request['pot'])
        return request['host'], request['pot'], request['adds']
    
def propfind(sock, request):
    """Send a PROPFIND request and return the response. """
    msg = METHOD_PROPFIND + '/' + request + HTCPCP_VERSION
    send_msg(sock, msg)
    return recv_msg(sock)
    
def brew(sock, pot, order):
    """Send a BREW request and return the response. """
    if not order == '':
        msg = METHOD_BREW + '/' + pot + HTCPCP_VERSION \
            + ACCEPT_ADDS + order + CONTENT_TYPE + MSG_BODY
    else:
        msg = METHOD_BREW + '/' + pot + HTCPCP_VERSION \
            + CONTENT_TYPE + MSG_BODY
    send_msg(sock, msg) 
    return recv_msg(sock)

def get(sock, pot, adds):
    """Send a GET request and return the response. """
    if not adds:
        msg = METHOD_GET + '/' + pot + HTCPCP_VERSION
    else:
        msg = METHOD_GET + '/' + pot + '?' + adds + HTCPCP_VERSION
    send_msg(sock, msg)
    return recv_msg(sock)
  
def pour(sock, pot):
    """Send a POUR request and return the response. """
    send_msg(sock, METHOD_POUR + '/' + pot + HTCPCP_VERSION)
    return recv_msg(sock)
      
def when(sock, pot):
    """Send a WHEN request and return the response. """
    send_msg(sock, METHOD_WHEN + '/' + pot + HTCPCP_VERSION)
    return recv_msg(sock)
    
def send_msg(s, msg):
    """Send a message down a socket. """
    try:
        s.send(msg)
        debug(msg, 'Sent:\n')
    except socket.error, e:
        sys.exit("Error sending data: %s" % e)

def recv_msg(s):
    """ Receive a message through a socket. """
    try:
        buf = s.recv(MSG_BUF_SIZE)
    except socket.error, e:
        sys.exit("Error receiving data: %s" % e)
    return buf
    
def open_sock(host):
    """Return a socket, once opened successfully. """
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
    """Close a socket and exit with the given message. """
    s.send('quit')
    print s.recv(MSG_BUF_SIZE)
    s.close()
    sys.exit(msg)
    
def extract_body(msg):
    """Return the message body of a HTCPCP message. """
    parts = msg.split('\r\n\r\n')
    if len(parts) >= 2:
        return parts[1]
    else: return ""
    
def status_code(msg):
    """Return the status code from an HTCPCP response. """
    parts = msg.split(' ')
    for c in C_ERROR:
        if re.match(parts[1], c):
            return { 'error' : True, 'code' : c}
    for c in C_SUCCESS:
        if re.match(parts[1], c):
            return { 'error' : False, 'code' : c}

def verify_url(url):
    """Determine the validity of a request-uri. """
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
    """Get the word "get" from the user, and send a GET request."""
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
    """Get the word "pour" from the user, and send a POUR request."""
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
    """Get the word "when" from the user, and send a WHEN request."""
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
    """Return a string of HTCPCP additions. """
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
    """Get a specific target string from the user. """
    data = raw_input(prompt)
    while not data == target:
        if data == 'quit':
            close_sock(s, MSG_QUIT)
        data = raw_input(prompt)
    
def get_yn_input(prompt):
    """ Get a "y" or an "n" from the user. """
    input = raw_input(prompt + " (y/n) ")
    while not re.match("[yn]", input):
        input = raw_input('Please answer y or n: ')
    return input

def debug_enable():
    global DEBUG
    DEBUG = True
    
def debug(msg, prefix='Server replies:\n'):
    """Print a full HTCPCP message if in debug mode. 
    Otherwise, print only the message body.
    
    """
    if DEBUG == True:
        print prefix + msg
    elif not re.match('Sent', prefix): 
        print extract_body(msg)
    
def freeform_enable(s):
    """Allow the user to enter arbitrary HTCPCP requests. """
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

# Invoke main method if run as script
if __name__ == "__main__":
    main()


