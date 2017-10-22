'''
reads a text, containing polish special characters, from standard input and outputs a text with ascii characters only
'''

import sys

text = sys.stdin.read()
text = text.lower().replace('ą','a').replace('ć','c').replace('ę','e').replace('ł','l').replace('ó','o').replace('ń','n').replace('ś','s').replace('ż','z').replace('ź','z').replace(' ','').replace(',','\n').replace('\r','')
words = text.split('\n')
words = sorted(words)
text = "\n".join(words)[1:]
print(text)