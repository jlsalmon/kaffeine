'''
Created on 29 Feb 2012

@author: jussy
'''

USR_PORT = 60000
MSG_BUF_SIZE = 1024

HTCPCP_VERSION = ' HTCPCP/1.0\r\n'
METHOD_BREW = 'BREW '
METHOD_GET = 'GET '
METHOD_PROPFIND = 'PROPFIND '
METHOD_WHEN = 'WHEN '

C_SUCCESS = ['200']
C_ERROR = ['406', '407', '408', '418', '504', '505']

ACCEPT_ADDS = 'Accept-Additions: '
CONTENT_TYPE = 'Content-Type: message/coffeepot\r\n\r\n'
MSG_BODY = 'Start'

WELCOME_MSG = '''
Welcome to kaffeine, the virtual coffee shop of the future!
Kaffeine uses the HTCPCP protocol to control an array of 
virtual coffee pots for your enjoyment.
'''
