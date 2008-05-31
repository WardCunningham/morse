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

/***	Cw.cxx - transmit CW on multimedia audio
 *
 *  Cw.cxx contains routines that send practice code using the "Simple
 *  DirectMedia Layer" (SDL, see http://www.libsdl.org).  This library
 *  purports to support similar operations on Linux, Windows, Mac, and
 *  several other platforms.  We shall see.
 *
 *  Basically, the modus operandi:
 *
 *    0. Catch signals which might cause us to exit ungracefully.
 *	  Make sure SDL_CloseAudio() and SDL_Quit() are called, in
 *	  that order, when we do exit gracefully.  Signal catching
 *	  should be done by the caller.  We'll register the two
 *	  SDL functions using "atexit()" here.
 *
 *    1. Call SDL_Init() to initialize the multimedia system.  The
 *        particular subset of features are selected by a bit mask.
 *	  we'll only need audio.  We don't need a Joystick; we won't
 *	  use accelerated graphics; we'll eschew CD audio and SDL's
 *	  media file parsing; etc, etc.
 *
 *    2. Call SDL_OpenAudio() to specify a) the format of waveform data
 *	  we'll be handing to SDL (16-bit native signed mono), and
 *	  b) a "callback" function capable of generating this data
 *	  and placing it into an arbitrary buffer, on demand from the
 *	  audio subsystem.
 *    
 *    3. Call SDL_PauseAudio(0) to get the ball rolling.
 *
 *  Here, we provide exactly three functions.  One function initializes SDL
 *  (if necessary) and sets the frequency, code speed and loudness for the
 *  subsequent "transmission".  Another accepts a single ASCII (or Uni-
 *  code since both are the same for all Morse characters) and emits the
 *  appropriate sounds on to the sound card.  A third simply returns the
 *  number of seconds since the last pending code element (dot, dash or
 *  space) was swallowed by the sound system.  This is useful for Ward's
 *  Morse machine to time the students response *after* she's heard the
 *  letter (instead of when it was sent). 
 *
 *  Somewhere in the ARRL Handbook is a formula that gives the WPM rate
 *  in terms of the dots/second or dots/minute, or somesuch.  However
 *  the Handbook has grown so large and cluttered, and the indexing so
 *  inadequate, that I cannot lay my eyes on it.  We'll have to rely on
 *  Google instead...
 *
 *  In, "The Art & Skill of Radio-Telegraphy", 2nd Ed, William Pierpont,
 *  (http://www.geocities.com/gm0rse/n0hff/c23.htm), N0HFF suggests there
 *  are "50 elements used as today's standard word".
 *
 *  Elsewhere, "PARIS" is frequently mentioned as the "standard word":
 *
 *    P = .--. = 1+1 + 3+1 + 3+1 + 1+3 = 14 elements
 *    A = .-   = 1+1 + 3+3	       =  8 elements
 *    R = .-.  = 1+1 + 3+1 + 1+3       = 10 elements  Total: 50 elements
 *    I = ..   = 1+1 + 1+3	       =  6 elements
 *    S = ...  = 1+1 + 1+1 + 1+7       = 12 elements
 *
 *  Thus, if unanimity is to be believed, the standard word has 50 elements,
 *  and the formula that I can't find in the Handbook is:
 *
 *	      WPM = (DPM*2)/50 = dpm/25, where DPM is dots per minute
 *
 *  (I vaguely remember "25" in the ARRL formula, so maybe this is right.)
 *
 *  For a code speed, WPM, we'll need an element time, E:
 *
 *		E = 60/(50*WPM) = N/f
 *
 *  Where f is the tone frequency, and N is the number of cycles of that tone.
 *  Thus:
 *		N = 1.2*f / WPM
 *
 *  BUG:  We wait in a thread by simply "while (...);".  There must be a
 *  way to relinquish the CPU than waiting to be hit on the head by a clock,
 *  but I can't figure it out.
 */ 
#include <cstdlib>	// atexit(), realloc()
#include <cmath>	// Round(), sin(), and M_PI dammit!
#include <SDL.h>	// Buncha stuff.
#include <SDL_audio.h>	//   and lots more, too
#include "Cw.h"		// Our contract with the outside world

#ifndef M_PI		// For brain-dead environments
#  define M_PI            3.14159265358979323846
#endif

/***	Morse - Store your character here
 *
 *  When Morse is zero, send_cw() can stuff in a character, and it will soon
 *  be sent.  There may be a slight delay before the first voiced element
 *  is heard because the SDL audio buffer may just have been filled, and
 *  it must drain before tone generation can begin.  To minimize this delay,
 *  strive to make SDL_AudioSpec::samples, below, as small as possible.
 */
static volatile unsigned Morse;	// Morse character that will be sent
static volatile unsigned shift;	// Morse character currently being sent
static volatile unsigned idle=1;// Number of sinewave cycles after going idle.
static unsigned cycles, dot;	// Current cycle count, count for each dot
static const Uint8* data;	// Next sample byte to fetch from sine buffer
static const Uint8* dataend;	// End (just past) sinewave table
static int need;		// Nonzero to change freq/wpm/loudness
static double amplitude;	//   New sinewave amplitude
static double speed;		//   New speed (wpm)
static double freq;		//   New (actual!) tone frequency (Hz)

static union {	// Sinewave buffer (enough for one cycle)
  Uint8*  c;	//   As char*, (required by waveget)
  Sint16* s;	//   As sample* (required by set_cw())
  void*   v;	//   As void* (required by realloc())
} buffer;

static void waveget(void*, Uint8*, int);// Audio "callback".  See below.

static SDL_AudioSpec spec = {
  11025,	// Sample rate
  AUDIO_S16SYS, // Native, signed 16-bit data
  1,		// Monaural ought to be sufficient
  0,		// Silence value (Set by SDL)
  512,		// Smaller is better (subject to starvation)
  0,		// Undocumented "padding", see SDL_audio.h.
  0,		// Buffer size (calculated by SDL_OpenAudio)
  waveget,	// Code practice oscillator
  0,		// We don't need no steenkin "user data"!
};

/***	fill_cb - (re)alloc(ate) and fill circular buffer
 *
 *  Fill_cb() reallocates the sine wave buffer and fills it with signed,
 *  16-bit samples comprising a single cycle of a sinusoidal waveform.
 *  The static local pointers, data and dataend, are set to point to the 
 *  beginning and end of the buffer, respectively.
 *
 *  If fill_cb() succeeds, it returns a pointer to the reallocated buffer.
 *  If it fails, the buffer contents are left unchanged, and the original
 *  value is returned so the system can limp along at the old frequency. 
 *
 *  BUG: We must contend with the dangers of preemptimve multitasking.
 *  The cost of failure here will most likely be only a thunk or click,
 *  and even that is extremely unlikely since the buffer is rearranged
 *  when the speakers are ostensibly silent.  Still, if garbage collection
 *  were immediate, the buffer happened to be reallocate into a new
 *  MMU segment causing the old segment to be completely abandoned, we
 *  *could* get a segfault.  There are several ways to solve this problem,
 *  (including mutexes or coallocated buffers), but neither seems worthwhile
 *  for the obfuscation that would ensue.  This aint't life support.
 */
static bool fill_cb() {			
  if (void* newb =				// Attempt to resize buffer
      realloc(buffer.v, sizeof(Sint16)*need)) {	// If reallocateion succeeded,
    data = (Uint8*)(buffer.v = newb);		//   Update buffer, beginning
    dataend = data + sizeof(Sint16)*need;	//     and ending pointers
  } else if (!buffer.v) return false;		// FUBAR on initial allocation
  else need = (dataend-buffer.c)/sizeof(Sint16);// Subsequent failure? use old.
  
  /* Now, let's quickly fill the new buffer before anyone notices.
   */
  double a = 0, da = 2*M_PI/need;		// Current, sampling angles
  for (int i = 0; i < need; a += da, i++)	// Fill new buffer with samples
    buffer.s[i] = Sint16(
      round(amplitude*sin(a)));
  freq = double(spec.freq)/need;		// Actual keyed tone
  need = 0;					// Flags "change complete"
   
  /* Now that we have a little breathing room, let's calculate the dot time
   * in cycles of the new frequency.
   */
  dot = int(round(1.2*freq/speed));	// Number of cycles in one code element
  return true;
}  

/***	set_cw - set code speed, note, and loudness
 *
 *  Set_cw() is passed three double values for codespeed (in WPM), tone
 *  frequency (in Hz), and loudness (from 0.0 to 1.0).  It generates one
 *  cycle of the required note into a realloc()'d buffer and calculates
 *  the (rounded, integer) number of full cycles in each dot time for the
 *  desired code speed.  If necessary, it initializes the SDL subsystem.
 *  It returns a boolean: true if it succeeds or false if it fails.
 *
 *  BUG:  Speed, frequency, loudness changes immediately affect queued
 *    code.  Send a blank or two to assure that the changes happen during 
 *    a quiet period between characters.
 *
 *  FEATURE:  Speed, frequency, loudness changes immediately affect queued 
 *    code.  This allows the parameters to be changed while sending a con-
 *    tinuous stream of dits, thus facilitating interactive adjustment.
 */
bool set_cw(double wpm, double freq, double loudness) {

  /* Calculate and publish (in the local newspaper), the sample buffer
   * length (in samples), the sine wave amplitude, and the required
   * speed (wpm).  If SDL has not been initialized, generate the sine
   * wave samples and dot length.  If the SDL has been initialized,
   * we'll let wavget() do that for us when the oscillator is silent.
   */
  amplitude = loudness*double(0x7FFF); speed = wpm; 	// For fill_cb()
  need = size_t(round(double(spec.freq)/freq));		// Samples in cycle

  /* See if audio subsystem has been initialized.  If it hasn't,
   * jump through the appropriate hoops.  Spec.size makes a convenient
   * flag to see if we've already done this.
   */
  if (!spec.size) {				// If SDL not initialized,
    if (SDL_Init(SDL_INIT_AUDIO)) return false;	//   Step 1 (or report failure)
    atexit(SDL_Quit);				//   For graceful exit
    if (SDL_OpenAudio(&spec, 0)) return false;	//   Step 2 (or report failure)
    atexit(SDL_CloseAudio);			//   For graceful exit
    if (!fill_cb()) return false;		//   Fill buffer (or not)
    SDL_PauseAudio(false);			//   Awaken beast
  }
  return true;
}

/***	waveget - signal generator
 *
 *  Waveget() is passed two pointers and an integer.  The first pointer is 
 *  not used.  The second points to a (raw) byte buffer whose length is
 *  specified by the integer.  Waveget() schlepps the data from the sine wave
 *  table (or zero) into the buffer upon request from the SDL.
 *
 *  After we've fed the beast, we see if it is currently silent and
 *  make any frequency, speed or loudness changes while it sleeps.
 */
void waveget(void*, Uint8* destination, int n) {
  while (n--) {				// Loop to copy waveform data:
    *destination++ = shift&1? *data: 0;	//   Schlepp tone (or silence)
    if (++data < dataend) continue;	//   Structured taboo! (much clearer!)
    data = buffer.c; cycles++;		//   Buffer wrap? Reset, count cycle
    Morse|shift? (idle = 0): idle++;	//   Pending stuff => not idle
    if (cycles < dot)     continue;	//   See what I mean? We're whittling!
    cycles = 0; shift >>= 1;		//   If element done, get next one.
    if (shift > 1)        continue;	//   More character? whittle rest.
    shift = Morse;			//   Exhausted this? Get next
    if (Morse != 5) Morse = 0;		//   Not didididi... make way for more
  }
  if (need && !(shift&1)) fill_cb();	// If silent, make change now.
}

/***  Character table - one bit for each "code element"
 *
 *  A dot and and element space each require one bit, A dash and a
 *  letter space each require three bits.  A word space requires seven
 *  bits.  A final bit, set to mark the end of the character, is not sent.
 *  Thus, we can embed letter spaces after the characters as well as before.
 *  Also, we can encode periods of silence.
 *
 *  Thus, a dash, requires four bits, three with the key closed, and one
 *  to to separate it from its neighbor.  So characters of up to eight
 *  dashes may be accomodated.  I don't think even the new '@' requires
 *  this duration.  The market has forced us to have 32 bits (or more),
 *  we might as well put at least 31 of them to good use.
 *
 *  The code elements are arranged little endian, even though big endian
 *  might seem more natural.  Little endian frees us from the tyrany of
 *  growing registers.  16 bits are passe.  32 bits are old hat.  64 bits
 *  are here now.  I'm sure the marketing department at AMD and Intel are
 *  devising of ways to convince us that 128 and 256 bits are needed to 
 *  assure a better sex life.  Microsoft and the Gamer Doodz are trying
 *  to think up ways of wasting all those bits, too.
 *
 *  For help generating this table, see the Python script, cw.py
 */
static int Code[] = { 
  0x0000010, 0x0000000, 0x0045D5D, 0x0000000, // ' ', '!', '"', '#'
  0x00475D5, 0x0000000, 0x0000000, 0x045DDDD, // '$', '%', '&', "'"
  0x0045DD7, 0x0475DD7, 0x0000000, 0x001175D, // '(', ')', '*', '+'
  0x0477577, 0x0047557, 0x011D75D, 0x0011757, // ',', '-', '.', '/'
  0x0477777, 0x011DDDD, 0x0047775, 0x0011DD5, // '0', '1', '2', '3'
  0x0004755, 0x0001155, 0x0004557, 0x0011577, // '4', '5', '6', '7'
  0x0045777, 0x0117777, 0x0115777, 0x01175D7, // '8', '9', ':', ';'
  0x0000000, 0x0011D57, 0x0000000, 0x0045775, // '<', '=', '>', '?'
  0x01175DD, 0x000011D, 0x0001157, 0x00045D7, // '@', 'A', 'B', 'C'
  0x0000457, 0x0000011, 0x0001175, 0x0001177, // 'D', 'E', 'F', 'G'
  0x0000455, 0x0000045, 0x0011DDD, 0x00011D7, // 'H', 'I', 'J', 'K'
  0x000115D, 0x0000477, 0x0000117, 0x0004777, // 'L', 'M', 'N', 'O'
  0x00045DD, 0x0011D77, 0x000045D, 0x0000115, // 'P', 'Q', 'R', 'S'
  0x0000047, 0x0000475, 0x00011D5, 0x00011DD, // 'T', 'U', 'V', 'W'
  0x0004757, 0x0011DD7, 0x0004577,	      // 'X', 'Y', 'Z'
};

/***	Send_cw - Convert character to Morse and send it
 *
 *  Send_cw() is a single routine that supports blocking and nonblocking
 *  transmission of its ASCII-coded parameter in Morse on the speakers.
 *  Since Unicode is the same as ASCII for all the Morse characters, it
 *  may be used as well.  Morse characters are in [.?/-@ 0-9A-Za-z].
 *
 *  An illegal character generates a zero-length silence.  It may be used
 *  to wait for the queue to clear or to test the state of the queue if
 *  a non-blocking call is used.
 *
 *  Send_cw() returns true after placing the character in the queue, or
 *  false if there was presently no room.  To get the non-blocking version,
 *  simply negate the character you pass to Send_cw().
 *
 *  The special value zero conspires to send a continuous stream of dots
 *  until another legitimate character (typically a blank) stops it.
 */
bool send_cw(int letter) {
  if (!letter) { Morse = 5; }		// Continuous dits? Send special value
  else if (Morse == 5) Morse = 0;	// Sending dits? Legit char to cancel
    
  if (letter <= 0) { 			// If non-blocking read
    if (Morse) return false;		//   If no room, return "failed"
    letter = -letter;			//   If room, convert to "normal" ASCII
  }
  if ('a' <= letter && letter <= 'z')	// Tableless "toupper()"
    letter -= 'a'-'A';
  if (' ' <= letter && letter <= 'Z')	// If in Code[] table,
    letter = Code[letter-' '];		//   convert it to Morse.
  else letter = 0;			// If not, it's not Morse
  while (Morse);			// Wait for queue to empty
  Morse = letter; idle = 0;		// Stuff in latest char
  return true;				//  and report "success!"
}

/***	idle_cw - return number of cycles since queue went empty
 *
 *  Ward's Morse machine needs to know how long it's been since we sent
 *  the training character.  The first attempt at this used the extremely
 *  arcane Fl_add_timeout(), etal.  It worked after a fashion, but it
 *  was only able to measure the time from when the character was placed
 *  in the buffer.  Thus, a '0' gave the student much less time to answer
 *  than did an 'E'.  The "idle_cw()" scheme greatly simplifies the "wait-
 *  for-student-response-and-tell-me-how-long-it-took" problem, as well.
 *
 *  BUG1:  Preemptive multitasking!  I believe I have forseen all possible
 *  race or lockup conditions, but one is never sure.
 *
 *  BUG2:  The time is measured from when the sound system swallows the
 *  character, not from when it finishes emerging from the speakers.  There
 *  is a slight delay since the buffer emptying lags filling.
*/
double idle_cw() { return idle/freq; }	// Convert cycles to seconds
