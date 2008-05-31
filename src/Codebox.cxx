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
#include "Codebox.h"		// Deliverables specification
#include "Cw.h"			// Sound subcontractor agreement
#include <FL/Fl.H>		// Event stuff, box details, etc.
#include <FL/fl_draw.H>		// Box contents (text) rendering
#include <csignal>		// SIGxxx, and signal()
#include <cstdlib>		// srand() and
#include <ctime>		//   time() keep things unpredictable
#include <cstdio>		// Fputs() for our failure

#define MAXCHARS 200		// Big enough to overflow Codebox line.

/***	signal processing
 *
 *  Here lies a mishmash of system-dependent signal processing routines
 *  behind a curtain of obfuscating #ifdef/#endif.  This code was basically
 *  lifted from the SDL test program, "loopwave.c", and is intended to make
 *  sure we don't somehow leak the entire sound resource when we die.
 */
static void signal_catcher(int sig) { exit(0); }
static void signal_set() {
#ifdef SIGHUP
  signal(SIGHUP, signal_catcher);	// Parent process died
#endif
  signal(SIGINT, signal_catcher);	// Interrupt from keyboard
#ifdef SIGQUIT
  signal(SIGQUIT, signal_catcher);	// Quit from keyboard
#endif
  signal(SIGTERM, signal_catcher);	// Termination signal
}

/***	The Codebox constructor
 *
 *  Codebox() calls the base class constructor and sets the maximum character
 *  buffer size to a large enough value that we're pretty sure it won't all
 *  fit on the screen.  If we're wrong, no harm done!
 */
Codebox::Codebox(int x, int y, int w, int h): 
  Fl_Input_(x,y,w,h) {		// Base class mojo
  maximum_size(MAXCHARS);	// A sensible growth policy
  answer = 0;			// Accept no characters for now.
  give = 3.5;			// Initial timeout for lesson failure
  resting = true;		// Let student get set.
  signal_set();			// Signal-catching mojo
  srand(unsigned(time(0)));	// Stir up random number generator
  if (set_cw()) return;		// Initialize code-practice oscillator
  
  /* If set_cw() fails, something's wrong with the CPO.  Print cryptic
   * message (that probably no one will see) and exit.
   */
  fputs("no sound\n", stdout);
  exit(EXIT_FAILURE);
}

/***	The Codebox user and GUI event handlers
 *
 *  Fl_Input_ has a pure-virtual event handler, so we must provide an
 *  override.  Fl_Input has too much extra baggage, and we'd have to do
 *  most of this stuff anyway.  Fortunately, Fl_Input_ does provide a
 *  method we can call to handle uninteresting events and do the heavy
 *  lifting during a draw request.
 */
void Codebox::draw() {			// Called by FLTK to do drawing
  Fl_Boxtype b = box();			// Get box type (downbox?)
  if (damage() & FL_DAMAGE_ALL)		// (Code stolen from Fl_Input.cxx
    draw_box(b, color());		//  after my version dirtied up the
  drawtext(x()+Fl::box_dx(b),		//  box border.  I'm not sure what
  	   y()+Fl::box_dy(b),		//  this does, but it seems to work.
           w()-Fl::box_dw(b),
	   h()-Fl::box_dh(b));
}
int Codebox::handle(int event) {	// Called by FLTK when user twitches
  int c = Fl::event_text()[0];		// "Cooked" key value
  if ('a'<=c && c<='z') c += 'A'-'a';	//  Tableless "toupper()"
  switch (event) {
    case FL_KEYDOWN: case FL_KEYUP:	// Student has jiggled a key
      switch (Fl::event_key()) {	//   Check raw key value
        case FL_Escape: return 0;	//     Hand back FLTK close requests
        case FL_Tab:    return 0;	//     Hand back Tab focus navigation
	case FL_Enter:	return 0;	//     Seems to affect navigation too.
      }
      if (event == FL_KEYUP) return 1;	//   Swallow, ignore key releases
      if (!idle_cw()) 	     return 1;	//   Ignore answers while sending
      if (!c || c != answer) return 1;	//   Patiently ignore wrong answers
      answer = 0; 	     return 1;	//     but reward correct ones

    case FL_FOCUS:			// Resume (or begin) practice
      resting = false; break;		//   Let teach() resume its work.
      
    case FL_UNFOCUS:			// Take a rest
      cut(0, maximum_size());		//   Clear displayed text
      resting = true; break;		//   Go have a cup of coffee.
  }
  return handletext(event, x(),y(),w(),h());	// Default event handler
}

/***	append - Append character to right side of Codebox line
 *
 *  A convenience function.  Regardess of where the "point" is, it is
 *  moved to the end of the line, and the character is inserted there.
 */
void Codebox::append(int c) {		// Append character to displayed line
  char s[] = { c, '\0' };		//   A 1-char string version of c.
  if (size() >= maximum_size())		//   If no room for character,
     cut(0,1);				//     discard one off left side
  position(maximum_size());		//   Return to right end of buffer
  insert(s);				//   Add character to right side
}

/***	teach - Teach a character and return a pass/fail grade
 *
 *  Teach() is passed a character.  It patiently sends the character over
 *  and over until the student types it into the Codebox.  If the student
 *  must be prompted with the answer, she fails, and teach() returns false.
 *  If she types the character before needing prompting, teach() returns
 *  true.
 */
bool Codebox::teach(int c) {		// Ask Codebox to teach a character
  answer = c;				//   Set up response we'll insist on
  bool grade = true;			//   Adopt optimistic attitude
  while (resting) Fl::wait();		//   Wait until she's ready
  send_cw(c);				//   Send new character
  while (answer) {			//   Until we get it:
    if (idle_cw() > give) {		//     If student takes too long,
      append(c); send_cw(c);		//       Give hint, resound letter,
      grade = false;			//         and hand out an 'F'
    } else if (resting) {		//     If student asks for a break
      while (resting) Fl::wait();	//       Wait for her to come back
      send_cw(c);			//       and resume where we left off
    } else Fl::check();			//     Otherwise, just check events
  }
  append(c);				//   Echo correct answer
  if (grade) {				//   If student passed the test,
    give = 0.875*give + 0.25*idle_cw();	//     Update slack. (Code stolen
    if (give > 6) give = 6;		//       from Ward's version.)
  }
  return grade;				//   Return pass/fail score. 
}
