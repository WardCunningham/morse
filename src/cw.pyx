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


# The next two functions are written in C++ and return  bool.
# Here, we assert they return 16-bit values.  It seems to work.
""" 	
Module cw - send strings in Morse code

Two functions are wrapped.  One must be called first to initialize
the sound subsystem.  The other may then be called repeatedly to
send message strings in Morse code.
"""
cdef extern short set_cw(double wpm, double freq, double volume)
cdef extern short send_cw(int c)

# Normally, a simple call to cw.set() will do, but the function
# gives you control over the frequency (of the tone) and the 
# volume.  You must call this function to initialize the SDL
# before sending anything.

def set(wpm = 20, freq = 1000, volume = 1):
  """
  Set(wpm = 20, freq = 1000, volume = 1.0) sets the keying rate,
      frequency (default: 1Khz), and loudness (default: maximum)
  """
  if not set_cw(wpm, freq, volume): raise IOError
  
# The C(++) version sends a single character.  It is more Pythonic
# to send a string.
  
def send(s):
  """
  Send("message") - sends your "message" in Morse code
  """
  for c in s: send_cw(ord(c))
