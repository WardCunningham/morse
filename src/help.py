#! /usr/bin/python

#   Copyright 1998-2004 Ward Cunningham and Jim Wilson
#   Distributed under the GNU GPL V2 license.
#   See http://c2.com/morse
#
#   This file is part of Morse. 
#
#   Morse is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   Morse is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License along
#   with Morse; if not, write to the Free Software Foundation, Inc.,
#   59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

""" 
A little script to turn Ward's help file into a single massive C string
so that it can be compiled into the runtime in place of the bloated code
required to (probably unreliably) find the file, open and read it.  This
script is a filter.  It reads stdin and writes to stdout.
"""
# All we need to do is print the declaration, followed by the lines from
# the help file enclosed between "s.  Don't forget to \-escape the quotes.
 
from sys import stdin

print( "/* DO NOT EDIT!  The following declaration and its initializer are")
print( " * generated from the original HTML file by the script, help.py.")
print( " */\n")

print( "static const char* HelpString =" )
for helpline in stdin:
  print( '  "'+helpline.replace('"','\\"').rstrip()+'\\n"')
print( ';' )


