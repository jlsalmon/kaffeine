'''
Created on 29 Feb 2012

@author: jussy
'''
import re
import socket
import sys
from consts import *

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
        msg = METHOD_GET + '/' + pot + '/' + adds + HTCPCP_VERSION
    send_msg(sock, msg)
    return recv_msg(sock)
    
def when():
    print 'Not yet implemented'
    
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
    
def close_sock(s, msg):
    s.send('quit')
    print s.recv(MSG_BUF_SIZE)
    s.close()
    sys.exit(msg)
    
def extract_body(msg):
    parts = msg.split('\r\n')
    if len(parts) >=3:
        return parts[3]
    else: return ""
    
def status_code(msg):
    parts = msg.split(' ')
    for c in C_ERROR:
        if re.match(parts[1], c):
            return { 'error' : True, 'code' : c}
    for c in C_SUCCESS:
        if re.match(parts[1], c):
            return { 'error' : False, 'code' : c}

def valid_url(url):
    parts = url[0].partition('://')
    if not re.match('coffee', parts[0]):
        return False 
    path = parts[2].split('/')
    if len(path) < 2:
        return False
    if not re.match('[a-zA-Z0-9.]', path[0]):
        return False
    if not re.match('pot-[0-9]', path[1]):
        return False
    if len(path) > 2:
        if not re.match('\?([a-z-]+=[a-z0-9]+(&[a-z-]+=[a-z0-9]+)*)+',
                         path[2]):
            return False
    return True

def valid_arg(arg):
    for a in VALID_ARGS:
        if re.match(arg, a):
            return True
    return False
    
def get_order():
    if re.match(get_yn_input("Would you like any additions?"), 'y'):
        order = raw_input('Enter addition string: ')
    else: 
        order = ''
    return order + "\r\n"

def get_yn_input(prompt):
    input = raw_input(prompt + " (y/n) ")
    while not re.match("[yn]", input):
        input = raw_input('Please answer y or n: ')
    return input

def debug_enable():
    global DEBUG
    DEBUG = False
    
def debug(msg, prefix = 'Server replies:\n'):
    if DEBUG == True:
        print prefix + msg
    else: print extract_body(msg)
    
def freeform_enable(s):
    print ('\nRunning in freeform mode.')
    while True:
        data = raw_input('Enter input: ')
        if data == 'quit':
            close_sock(s, 'Connection closed. Program will exit.')
        if not re.match('([a-zA-Z]*)\s(/pot-[0-9])(/\?([a-z-]+=[a-z0-9]+(&[a-z-]+=[a-z0-9]+)*)+)*\s(HTCPCP/1\.0)', data):
            print 'Invalid HTCPCP request.'
        else:
            send_msg(s, data)
            print recv_msg(s)