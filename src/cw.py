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

Code = """
A .-    F ..-.  K -.-   P .--.  U ..-   Z --..    = -...-   / -..-.   + .-.-.
B -...  G --.   L .-..  Q --.-  V ...-  . .-.-.-  : ---...  " .-..-.  @ .--.-.
C -.-.  H ....  M --    R .-.   W .--   , --..--  ; -.-.-.  $ ...-.-  
D -..   I ..    N -.    S ...   X -..-  ? ..--..  ( -.--.   ' .----.  
E .     J .---  O ---   T -     Y -.--  - -....-  ) -.--.-  _ ..--.-

0 ----- 5 .....
1 .---- 6 -....
2 ..--- 7 --...
3 ...-- 8 ---..
4 ....- 9 ----.
""".split()

ASCII = """ !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ"""

# A little function to turn a string of dots/dashes to a bit pattern

def code2bits(c):	# Code string (e.g., ".-" for a Morse A
  c = list(c)		# E.g., A becomes [".", "-"]
  c.reverse()		#       and then, ["_", "."]
  morse = 4		# length-2 silence plus end-character mark
  for d in c:		# Loop through dots/dashes:
    if d == ".":	#   If dot,	
      morse <<= 2	#     make room for dot and length-1 silence,
      morse += 1	#     and add in length-1 dot
    elif d == "-":	#   If dash,
      morse <<= 4	#      make room for dash and length-1 silence,
      morse += 7	#      and add in length-3 dash
    else: return 0	#   Error!  Must be "." or "-"
  return morse		# Normal return (E.g., 1 000 111 0 1b for A)
  
# A little function to turn our code table into a bit-pattern dictionary

def l2d(l):		# [key1,value1, key2,value2, ...] to dictionary
  d = {}		# Dictionary initially empty
  while True:		# Loop to fill it:
    try: val = l.pop()	#   Grab ValueN from tail of list
    except: break;	#   List is empty?  We're all done!
    try: key = l.pop()	#   Grab KeyN from tail of list
    except: return None	#   List is empty? Something went wrong!
    val = code2bits(val)#   Convert dotdash string to keyer bit pattern
    d[key] = val	#   Got key/value? Add entry to dictionary
  return d		# Return dictionary we've been building


Code = l2d(Code)	# Dictionary of code (lacks word space!)
Code[" "] = 0x10	# Add length-4 word space (length-7 total)


print "static unsigned Code[] = { // ***  Clean me up please!  ***\n  ",

comment = []		# List of ASCII characters (to use as comment)
for c in ASCII:		# Loop to print 4 characters/line
  comment += c		#   Add ASCII to comment 
  try: c = Code[c]	#   See if there is a Morse equivalent
  except: c = 0		#   If not, we'll flag with special value
  print "0x%0.7X," % c,	#   Print code element bit pattern in hex
  if len(comment) == 4: #   If we've printed four items,
    print "//",	comment #     Print C++ comment
    comment = []	#     Empty ASCII list for next line
    print "  ",		#     Indent next line two spaces (See BUG!)
if len(comment): 	# If incomplete last line,
  o = 4-len(comment)	#   (Number entries) omitted from last line
  for i in xrange(o):	#   Loop to pad for omitted entries
    print "          ",	#     Print spaces to pad "0xnnnnnnn,"
  print "//", comment	# BUG: If len(ASCII) is a multiple of four, 
print "};"		#   indentation of the closing "};" is FUBAR
