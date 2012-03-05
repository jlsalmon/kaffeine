'''
Created on 29 Feb 2012

@author: jussy
'''
import re

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
        if not re.match('\?([a-z]+=[a-z0-9]+(&[a-z]+=[a-z0-9]+)*)+', path[2]):
            return False
    return True

def get_order():
    milk = raw_input("Would you like milk? (y/n) ")
    while not re.match("[yn]", milk):
        milk = raw_input('Please answer y or n: ')
    
    sugar = raw_input("Would you like sugar? (y/n) ")
    while not re.match("[yn]", sugar):
        milk = raw_input('Please answer y or n: ')
    return (milk, sugar)