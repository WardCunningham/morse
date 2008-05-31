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
#ifndef Knob_h
#define Knob_h
#include <FL/Fl_Roller.H>		// Base class details

/***	Knob.h - Speed, tone knobs (a subclass of Fl_Roller)
 *
 *  Knobs are like Rollers except that they start sending dots when
 *  the mouse is pressed to give feedback as the user adjusts the
 *  speed and frequency.
 */
class Knob: public Fl_Roller {
public:
  Knob(int,int,int,int,const char*);	// Constructor
protected:
  int handle(int);			// Override event handler
};
#endif // Knob_h
