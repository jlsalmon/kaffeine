"""
File:       consts.py
Author:     Justin Lewis Salmon
Student ID: 10000937
Created on: 29 Feb 2012

Application constants for kaffeine client.

"""

USR_PORT        = 60000
MSG_BUF_SIZE    = 1024
DEBUG           = False

HTCPCP_VERSION  = ' HTCPCP/1.0\r\n'

METHOD_PROPFIND = 'PROPFIND '
METHOD_BREW     = 'BREW '
METHOD_POST     = 'POST '
METHOD_GET      = 'GET '
METHOD_POUR     = 'POUR '
METHOD_WHEN     = 'WHEN '

C_SUCCESS       = ['200']
C_ERROR         = ['404', '406', '418', '419', 
                   '420', '421', '422', '423', 
                   '424', '425', '426', '427', 
                   '428', '503', '504', '505']

ACCEPT_ADDS     = 'Accept-Additions: '
CONTENT_TYPE    = 'Content-Type: message/coffeepot\r\n\r\n'
MSG_BODY        = 'Start'

PROMPT_GET      = 'Type "get" to collect your coffee: '
PROMPT_POUR     = 'Type "pour" to begin pouring your additions: '
PROMPT_WHEN     = 'Type "when" to finish pouring: '

MSG_ERROR       = 'The server could not complete the request. Program will exit.'
MSG_QUIT        = 'Connection closed. Program will exit.'
MSG_COMPLETE    = 'Thank you for using kaffeine. We look forward'\
                     + ' to quenching your\ndigital thirst again.'
MSG_WELCOME     = '''
*	Welcome to kaffeine, the virtual coffee shop of the future!
*	Kaffeine uses the HTCPCP protocol to control an array of
*	virtual coffee pots for your enjoyment.
'''                     
