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
#include "Knob.h"	// Our contract with the client.
#include "Cw.h"		// Send dot string or silence

/* Must have constructor that calls Fl_Roller's non-default one.
 */
Knob::Knob(int x, int y, int w, int h, const char* l):
  Fl_Roller(x,y,w,h,l) {}  // Doesn't do much else though!

/* Snarf up presses and start sending dots for adjustment purposes.
 * Continue sending them until release.  Pass all events along to
 * the Roller so it can use them for its tapdance.
 */
int Knob::handle(int e) {
  if      (e == FL_PUSH)    send_cw( 0 );	// Dididididididi....
  else if (e == FL_RELEASE) send_cw(' ');	// Blessed silence
  return Fl_Roller::handle(e);			// Do heavy lifting
}
