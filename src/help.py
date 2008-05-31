#! /usr/bin/python

""" 
A little script to turn Ward's help file into a single massive C string
so that it can be compiled into the runtime in place of the bloated code
required to (probably unreliably) find the file, open and read it.  This
script is a filter.  It reads stdin and writes to stdout.
"""
# All we need to do is print the declaration, followed by the lines from
# the help file enclosed between "s.  Don't forget to \-escape the quotes.
 
from sys import stdin

print "/* DO NOT EDIT!  The following declaration and its initializer are"
print " * generated from the original HTML file by the script, help.py."
print " */\n"

print "static char* HelpString ="
for helpline in stdin:
  print '  "'+helpline.replace('"','\\"').rstrip()+'\\n"'
print ';'


