'''
Created on 29 Feb 2012

@author: jussy
'''
import re
from consts import *

def propfind(sock, request):
    msg = METHOD_PROPFIND + request + HTCPCP_VERSION
    sock.send(msg)
    print 'Sent:\n' + msg
    return sock.recv(MSG_BUF_SIZE)
    
def brew(sock, request, order):
    msg = METHOD_BREW + request + HTCPCP_VERSION + ACCEPT_ADDS + order + CONTENT_TYPE + MSG_BODY
    sock.send(msg)
    print 'Sent:\n' + msg
    return sock.recv(MSG_BUF_SIZE)
    
def get(sock, request):
    msg = METHOD_GET + request + HTCPCP_VERSION
    sock.send(msg)
    print 'Sent:\n' + msg
    return sock.recv(MSG_BUF_SIZE)
    
def when():
    print 'Not yet implemented'
    
def status_code(msg):
    parts = msg.split(' ')
    for c in C_ERROR:
        if re.match(parts[1], c):
            return False
    return True
    
def valid_url(url):
    parts = url.partition('://')
    if not re.match('coffee', parts[0]):
        return False 
    path = parts[2].split('/')
    if len(path) < 2:
        return False
    if not re.match('[a-zA-Z0-9]', path[0]):
        return False
    if not re.match('pot-[0-9]', path[1]):
        return False
    if len(path) > 2:
        if not re.match('\?([a-z-]+=[a-z0-9]+(&[a-z-]+=[a-z0-9]+)*)+', path[2]):
            return False
    return True

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
