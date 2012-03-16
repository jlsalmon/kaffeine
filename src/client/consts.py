'''
Created on 29 Feb 2012

@author: jussy
'''

USR_PORT = 60000
MSG_BUF_SIZE = 1024
VALID_ARGS = ['-v']
DEBUG = False

HTCPCP_VERSION = ' HTCPCP/1.0\r\n'

METHOD_PROPFIND = 'PROPFIND '
METHOD_BREW = 'BREW '
METHOD_POST = 'POST '
METHOD_GET = 'GET '
METHOD_WHEN = 'WHEN '
METHOOD_PUT = 'PUT '

C_SUCCESS = ['200']
C_ERROR = ['404', '406', '418', '419', '420',
           '421', '422', '423', '424', '425',
           '426', '427', '503', '504', '505']

ACCEPT_ADDS = 'Accept-Additions: '
CONTENT_TYPE = 'Content-Type: message/coffeepot\r\n\r\n'
MSG_BODY = 'Start'

WELCOME_MSG = '''
Welcome to kaffeine, the virtual coffee shop of the future!
Kaffeine uses the HTCPCP protocol to control an array of 
virtual coffee pots for your enjoyment.'''
