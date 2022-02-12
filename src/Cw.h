/*  Copyright 1998-2004 Ward Cunningham and Jim Wilson
    Distributed under the GNU GPL V2 license.
    See http://c2.com/morse

    This file is part of Morse.

    Morse is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Morse is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with Morse; if not, write to the Free Software Foundation, Inc.,
    59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef Cw_h
#define Cw_h
extern "C" {	// This lets Pyrex find our functions

/***	Cw.h - promises made to cw.cxx clients
 *
 *  Cw.cxx exposes three functions.
 *
 *  The first function, set_cw() is used to initialize the sound system,
 *  the character codespeed, sinewave frequency and loudness.
 *
 *  The second function accepts an ASCII or Unicode character (negated
 *  for nonblocking transmission), and transmits it as Morse.  It returns
 *  true if the character was sent, or false if nonblocking mode was used
 *  and there was no room to send the character.
 *
 *  The third returns the number of seconds since the sound system
 *  swallowed up the last audible peep.  If this value is zero, code
 *  is being sent.
 */
bool set_cw(double WPM = 22, 		// Words/minute (default 20)
	    double freq = 800, 	// Note (default 1Khz)
	    double loudness = 1.0);	// Loudness (default Maximum)

bool send_cw(int ASCII);		// Send character in Morse Code
double idle_cw();			// Seconds we've not been sending

} // end "extern 'C'".  We don't need no steenkin' function signatures!
#endif // Cw_h
