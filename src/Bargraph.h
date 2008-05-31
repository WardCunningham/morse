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
#ifndef Bargraph_h
#define Bargraph_h

#include <FL/Fl_Group.H>	// Our parent
#include <FL/Fl_Slider.H>	// Our children

/***	Bargraph - a class to display the Morse student's progress
 *
 *  To offer the student positive reinforcement, we display a bar graph
 *  of error rates for each letter we're teaching.  Here, we lay out
 *  Fl_Slider widgets which work nicely for this purpose.
 */
class Bargraph: public Fl_Group {
public:
  Bargraph(int,int,int,int,const char*);// Mimic Fl_Group
  ~Bargraph();				// deallocate "slider_labels", too
  void tesselate(int gap = 0);		// Retile Sliders
  void disable(const char*);		// Remove characters from lesson plan
  void enable(const char*);		// Add characters to lesson plan
  void resize(int,int,int,int);		// Virtual function override
  int select();				// Randomly select a character
  void grade(int, bool);		// Update error rates
  void graduate();			// Add letter if student's passing
  void activate(const char*);		// Introduce new character
protected:
  int handle(int);			// Virtual method override
private:
  void click_letter(int);		// Manually add/subtract a letter
  void deactivate(const char*);		// Estrange old character
  double overall;			// Overall error "rate"
  int resize_gap;			// Gap for Bargraph::resize(x,y,w,h)
  char* slider_labels;			// Lesson plan shuffled with '\0's
  Fl_Slider* find(int);			// Find Slider for given character
  void apply(void (Fl_Slider::*)(),	// Apply certain method to Sliders 
             const char*);		//   whose name is in list.
};
#endif//Bargraph_h
