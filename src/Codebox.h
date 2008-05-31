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
#ifndef Codebox_h
#define Codebox_h
#include <FL/Fl_Input.H>	// Base class

/***	Codebox - await specific character and append to marching text
 *
 *  I predict this class will find little use outside a Morse teaching
 *  application!  A subclass of the low-level Fl_Input_, it displays a
 *  single line of text.  It intercepts all input events, but discards
 *  most of them.  Exceptions are:
 *
 *  	1. FL_FOCUS - FLTK informs us that the user is ready to copy
 *      2. FL_UNFOCUS - FLTK informs us the user needs a break
 *      3. FL_KEYDOWN - FLTK informs us the user has pressed a keyboard key
 *	       (Most characters are discarded!)
 */		
class Codebox: public Fl_Input_ {
public:
  Codebox(int,int,int,int);	// Unlabeled, restricted text input
  bool teach(int);		// Teach character
protected:
  int handle(int);		// User event handler
  void draw();			// GUI redraw requests
private:
  bool resting;			// True if student needs a break;
  void append(int);		// Append character to right end of line
  volatile int answer;		// The answer we're waiting for
  double give;			// How many seconds we'll wait to hear it
};
#endif // Codebox_h
