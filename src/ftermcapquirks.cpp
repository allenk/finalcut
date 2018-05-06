/***********************************************************************
* ftermcapquirks.cpp - Termcap quirks for some well-known terminals    *
*                                                                      *
* This file is part of the Final Cut widget toolkit                    *
*                                                                      *
* Copyright 2018 Markus Gans                                           *
*                                                                      *
* The Final Cut is free software; you can redistribute it and/or       *
* modify it under the terms of the GNU Lesser General Public License   *
* as published by the Free Software Foundation; either version 3 of    *
* the License, or (at your option) any later version.                  *
*                                                                      *
* The Final Cut is distributed in the hope that it will be useful,     *
* but WITHOUT ANY WARRANTY; without even the implied warranty of       *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
* GNU Lesser General Public License for more details.                  *
*                                                                      *
* You should have received a copy of the GNU Lesser General Public     *
* License along with this program.  If not, see                        *
* <http://www.gnu.org/licenses/>.                                      *
***********************************************************************/

#include "final/ftermcapquirks.h"

// static class attributes
char                FTermcapQuirks::termtype[256]  = {};
FTermcap::tcap_map* FTermcapQuirks::tcap           = 0;
FTermDetection*     FTermcapQuirks::term_detection = 0;


//----------------------------------------------------------------------
// class FTermcapQuirks
//----------------------------------------------------------------------

// constructors and destructor
//----------------------------------------------------------------------
FTermcapQuirks::FTermcapQuirks()
{ }

//----------------------------------------------------------------------
FTermcapQuirks::~FTermcapQuirks()  // destructor
{ }


// public methods of FTermcapQuirks
//----------------------------------------------------------------------
void FTermcapQuirks::setTerminalType (char tt[])
{
  std::strncpy (termtype, tt, sizeof(termtype) - 1);
}

//----------------------------------------------------------------------
void FTermcapQuirks::setTermcapMap (FTermcap::tcap_map* tc)
{
  tcap = tc;
}

//----------------------------------------------------------------------
void FTermcapQuirks::setFTermDetection (FTermDetection* td)
{
  term_detection = td;
}



// private methods of FTermcapQuirks
//----------------------------------------------------------------------
void FTermcapQuirks::terminalFixup()
{
  FTermDetection* td = term_detection;

  if ( td->isCygwinTerminal() )
  {
    init_termcap_cygwin_quirks();
  }
  else if ( td->isLinuxTerm() )
  {
    init_termcap_linux_quirks();
  }
  else if ( td->isRxvtTerminal() )
  {
    init_termcap_rxvt_quirks();
  }
  else if ( td->isGnomeTerminal() )
  {
    init_termcap_vte_quirks();
  }
  else if ( td->isTeraTerm() )
  {
    init_termcap_teraterm_quirks();
  }
  else if ( td->isSunTerminal() )
  {
    init_termcap_sun_quirks();
  }
  else if ( td->isPuttyTerminal() )
  {
    init_termcap_putty_quirks();
  }
  else if ( td->isScreenTerm() )
  {
    init_termcap_screen_quirks();
  }
#if defined(__FreeBSD__) || defined(__DragonFly__)
  else if ( td->isFreeBSDTerm() )
  {
    init_termcap_freebsd_quirks();
  }
#endif

  // xterm and compatible terminals
  if ( td->isXTerminal() && ! td->isPuttyTerminal() )
    init_termcap_xterm_quirks();

  // Fixes general quirks
  init_termcap_general_quirks();
}

#if defined(__FreeBSD__) || defined(__DragonFly__)
//----------------------------------------------------------------------
void FTermcapQuirks::init_termcap_freebsd_quirks()
{
  // FreeBSD console fixes

  TCAP(fc::t_acs_chars) = \
      C_STR("-\036.\0370\333"
            "a\260f\370g\361"
            "h\261j\331k\277"
            "l\332m\300n\305"
            "q\304t\303u\264"
            "v\301w\302x\263"
            "y\363z\362~\371");

    TCAP(fc::t_set_attributes) = \
        C_STR(CSI "0"
                  "%?%p1%p6%|%t;1%;"
                  "%?%p2%t;4%;"
                  "%?%p1%p3%|%t;7%;"
                  "%?%p4%t;5%;m"
                  "%?%p9%t\016%e\017%;");

  FTermcap::attr_without_color = 18;
}
#endif

//----------------------------------------------------------------------
void FTermcapQuirks::init_termcap_cygwin_quirks()
{
  // Set invisible cursor for cygwin terminal
  if ( ! TCAP(fc::t_cursor_invisible) )
    TCAP(fc::t_cursor_invisible) = \
        C_STR(CSI "?25l");

  // Set visible cursor for cygwin terminal
  if ( ! TCAP(fc::t_cursor_visible) )
    TCAP(fc::t_cursor_visible) = \
        C_STR(CSI "?25h");

  // Set ansi blink for cygwin terminal
  if ( ! TCAP(fc::t_enter_blink_mode) )
    TCAP(fc::t_enter_blink_mode) = \
        C_STR(CSI "5m");

  // Set enable alternate character set for cygwin terminal
  if ( ! TCAP(fc::t_enable_acs) )
    TCAP(fc::t_enable_acs) = \
        C_STR(ESC "(B" ESC ")0");

  // Set background color erase for cygwin terminal
  FTermcap::background_color_erase = true;

  // Include the Linux console quirks
  init_termcap_linux_quirks();
}

//----------------------------------------------------------------------
void FTermcapQuirks::init_termcap_linux_quirks()
{
  /* Same settings are used by cygwin */

  // Set ansi foreground and background color
  if ( FTermcap::max_color > 8 )
  {
    TCAP(fc::t_set_a_foreground) = \
        C_STR(CSI "3%p1%{8}%m%d%?%p1%{7}%>%t;1%e;22%;m");
    TCAP(fc::t_set_a_background) = \
        C_STR(CSI "4%p1%{8}%m%d%?%p1%{7}%>%t;5%e;25%;m");
    // Avoid underline, blink and dim mode
    FTermcap::attr_without_color = 26;
  }
  else
  {
    TCAP(fc::t_set_a_foreground) = \
        C_STR(CSI "3%p1%dm");
    TCAP(fc::t_set_a_background) = \
        C_STR(CSI "4%p1%dm");
    // Avoid underline and dim mode
    FTermcap::attr_without_color = 18;
  }

  // Set select graphic rendition attributes
  TCAP(fc::t_set_attributes) = \
      C_STR(CSI "0"
                "%?%p6%|%t;1%;"
                "%?%p1%p3%|%t;7%;"
                "%?%p4%t;5%;m"
                "%?%p9%t\016%e\017%;");

  TCAP(fc::t_enter_alt_charset_mode) = C_STR("\016");
  TCAP(fc::t_exit_alt_charset_mode) = C_STR("\017");
  TCAP(fc::t_exit_attribute_mode) = C_STR(CSI "0m\017");
  TCAP(fc::t_exit_bold_mode) = C_STR(CSI "22m");
  TCAP(fc::t_exit_blink_mode) = C_STR(CSI "25m");
  TCAP(fc::t_exit_reverse_mode) = C_STR(CSI "27m");
  TCAP(fc::t_exit_secure_mode) = 0;
  TCAP(fc::t_exit_protected_mode) = 0;
  TCAP(fc::t_exit_crossed_out_mode) = 0;
  TCAP(fc::t_orig_pair) = C_STR(CSI "39;49;25m");

  // Avoid underline and dim mode
  TCAP(fc::t_enter_dim_mode)       = 0;
  TCAP(fc::t_exit_dim_mode)        = 0;
  TCAP(fc::t_enter_underline_mode) = 0;
  TCAP(fc::t_exit_underline_mode)  = 0;
}

//----------------------------------------------------------------------
void FTermcapQuirks::init_termcap_xterm_quirks()
{
  // Fallback if "Ic" is not found
  if ( ! TCAP(fc::t_initialize_color) )
  {
    TCAP(fc::t_initialize_color) = \
        C_STR(OSC "4;%p1%d;rgb:"
                  "%p2%{255}%*%{1000}%/%2.2X/"
                  "%p3%{255}%*%{1000}%/%2.2X/"
                  "%p4%{255}%*%{1000}%/%2.2X" ESC "\\");
  }

  // Fallback if "vi" is not found
  if ( ! TCAP(fc::t_cursor_invisible) )
    TCAP(fc::t_cursor_invisible) = \
        C_STR(CSI "?25l");

  // Fallback if "ve" is not found
  if ( ! TCAP(fc::t_cursor_normal) )
    TCAP(fc::t_cursor_normal) = \
        C_STR(CSI "?12l" CSI "?25h");
}

//----------------------------------------------------------------------
void FTermcapQuirks::init_termcap_rxvt_quirks()
{
  // Set enter/exit alternative charset mode for rxvt terminal
  if ( std::strncmp(termtype, "rxvt-16color", 12) == 0 )
  {
    TCAP(fc::t_enter_alt_charset_mode) = \
        C_STR(ESC "(0");
    TCAP(fc::t_exit_alt_charset_mode)  = \
        C_STR(ESC "(B");
  }

  // Set ansi foreground and background color
  if ( ! term_detection->isUrxvtTerminal() )
  {
    TCAP(fc::t_set_a_foreground) = \
        C_STR(CSI "%?%p1%{8}%<%t%p1%{30}%+%e%p1%'R'%+%;%dm");
    TCAP(fc::t_set_a_background) = \
        C_STR(CSI "%?%p1%{8}%<%t%p1%'('%+%e%p1%{92}%+%;%dm");
  }
}

//----------------------------------------------------------------------
void FTermcapQuirks::init_termcap_vte_quirks()
{
  // gnome-terminal has NC=16 however, it can use the dim attribute
  FTermcap::attr_without_color = 0;

  // set exit underline for gnome terminal
  TCAP(fc::t_exit_underline_mode) = \
      C_STR(CSI "24m");
}

//----------------------------------------------------------------------
void FTermcapQuirks::init_termcap_putty_quirks()
{
  FTermcap::background_color_erase = true;
  FTermcap::osc_support = true;

  // PuTTY has NC=22 however, it can show underline and reverse
  // and since version 0.71 is the dim attribute is also supported
  FTermcap::attr_without_color = 0;

  // Set ansi foreground and background color
  TCAP(fc::t_set_a_foreground) = \
      C_STR(CSI "%?%p1%{8}%<"
                "%t3%p1%d"
                "%e%p1%{16}%<"
                "%t9%p1%{8}%-%d"
                "%e38;5;%p1%d%;m");

  TCAP(fc::t_set_a_background) = \
      C_STR(CSI "%?%p1%{8}%<"
                "%t4%p1%d"
                "%e%p1%{16}%<"
                "%t10%p1%{8}%-%d"
                "%e48;5;%p1%d%;m");

  TCAP(fc::t_set_attributes) = \
      C_STR(CSI "0"
                "%?%p1%p6%|%t;1%;"
                "%?%p5%t;2%;"  // since putty 0.71
                "%?%p2%t;4%;"
                "%?%p1%p3%|%t;7%;"
                "%?%p4%t;5%;m"
                "%?%p9%t\016%e\017%;");
  // PuTTY 0.71 or higher
  TCAP(fc::t_enter_dim_mode) = \
      C_STR(CSI "2m");

  // PuTTY 0.71 or higher
  TCAP(fc::t_exit_dim_mode) = \
      C_STR(CSI "22m");

  if ( ! TCAP(fc::t_clr_bol) )
    TCAP(fc::t_clr_bol) = \
        C_STR(CSI "1K");

  if ( ! TCAP(fc::t_orig_pair) )
    TCAP(fc::t_orig_pair) = \
        C_STR(CSI "39;49m");

  if ( ! TCAP(fc::t_orig_colors) )
    TCAP(fc::t_orig_colors) = \
        C_STR(OSC "R");

  if ( ! TCAP(fc::t_column_address) )
    TCAP(fc::t_column_address) = \
        C_STR(CSI "%i%p1%dG");

  if ( ! TCAP(fc::t_row_address) )
    TCAP(fc::t_row_address) = \
        C_STR(CSI "%i%p1%dd");

  if ( ! TCAP(fc::t_enable_acs) )
    TCAP(fc::t_enable_acs) = \
        C_STR(ESC "(B" ESC ")0");

  if ( ! TCAP(fc::t_enter_am_mode) )
    TCAP(fc::t_enter_am_mode) = \
        C_STR(CSI "?7h");

  if ( ! TCAP(fc::t_exit_am_mode) )
    TCAP(fc::t_exit_am_mode) = \
        C_STR(CSI "?7l");

  if ( ! TCAP(fc::t_enter_pc_charset_mode) )
    TCAP(fc::t_enter_pc_charset_mode) = \
        C_STR(CSI "11m");

  if ( ! TCAP(fc::t_exit_pc_charset_mode) )
    TCAP(fc::t_exit_pc_charset_mode) = \
        C_STR(CSI "10m");

  if ( ! TCAP(fc::t_key_mouse) )
    TCAP(fc::t_key_mouse) = \
        C_STR(CSI "M");
}

//----------------------------------------------------------------------
void FTermcapQuirks::init_termcap_teraterm_quirks()
{
  // Tera Term eat_nl_glitch fix
  FTermcap::eat_nl_glitch = true;

  // Tera Term color settings
  TCAP(fc::t_set_a_foreground) = \
      C_STR(CSI "38;5;%p1%dm");
  TCAP(fc::t_set_a_background) = \
      C_STR(CSI "48;5;%p1%dm");
  TCAP(fc::t_exit_attribute_mode) = \
      C_STR(CSI "0m" SI);
  TCAP(fc::t_orig_pair) = \
      C_STR(CSI "39;49m");
}

//----------------------------------------------------------------------
void FTermcapQuirks::init_termcap_sun_quirks()
{
  // Sun Microsystems workstation console eat_nl_glitch fix
  FTermcap::eat_nl_glitch = true;
}

//----------------------------------------------------------------------
void FTermcapQuirks::init_termcap_screen_quirks()
{
  // Fallback if "Ic" is not found
  if ( ! TCAP(fc::t_initialize_color) )
  {
    if ( term_detection->isTmuxTerm() )
    {
      TCAP(fc::t_initialize_color) = \
          C_STR(ESC "Ptmux;" ESC OSC "4;%p1%d;rgb:"
                "%p2%{255}%*%{1000}%/%2.2X/"
                "%p3%{255}%*%{1000}%/%2.2X/"
                "%p4%{255}%*%{1000}%/%2.2X" BEL ESC "\\");
    }
    else
    {
      TCAP(fc::t_initialize_color) = \
          C_STR(ESC "P" OSC "4;%p1%d;rgb:"
                "%p2%{255}%*%{1000}%/%2.2X/"
                "%p3%{255}%*%{1000}%/%2.2X/"
                "%p4%{255}%*%{1000}%/%2.2X" BEL ESC "\\");
    }
  }
}

//----------------------------------------------------------------------
void FTermcapQuirks::init_termcap_general_quirks()
{
  static const int not_available = -1;

  if ( FTermcap::tabstop == not_available )
    FTermcap::tabstop = 8;

  if ( FTermcap::attr_without_color == not_available )
    FTermcap::attr_without_color = 0;

  // Fallback if "AF" is not found
  if ( ! TCAP(fc::t_set_a_foreground) )
    TCAP(fc::t_set_a_foreground) = \
        C_STR(CSI "3%p1%dm");

  // Fallback if "AB" is not found
  if ( ! TCAP(fc::t_set_a_background) )
    TCAP(fc::t_set_a_background) = \
        C_STR(CSI "4%p1%dm");

  // Fallback if "Ic" is not found
  if ( ! TCAP(fc::t_initialize_color) )
  {
    TCAP(fc::t_initialize_color) = \
        C_STR(OSC "P%p1%x"
                  "%p2%{255}%*%{1000}%/%02x"
                  "%p3%{255}%*%{1000}%/%02x"
                  "%p4%{255}%*%{1000}%/%02x");
  }

  // Fallback if "ti" is not found
  if ( ! TCAP(fc::t_enter_ca_mode) )
    TCAP(fc::t_enter_ca_mode) = \
        C_STR(ESC "7" CSI "?47h");

  // Fallback if "te" is not found
  if ( ! TCAP(fc::t_exit_ca_mode) )
    TCAP(fc::t_exit_ca_mode) = \
        C_STR(CSI "?47l" ESC "8" CSI "m");

  // Set ansi move if "cm" is not found
  if ( ! TCAP(fc::t_cursor_address) )
    TCAP(fc::t_cursor_address) = \
        C_STR(CSI "%i%p1%d;%p2%dH");

  // Test for standard ECMA-48 (ANSI X3.64) terminal
  if ( TCAP(fc::t_exit_underline_mode)
    && std::strncmp(TCAP(fc::t_exit_underline_mode), CSI "24m", 5) == 0 )
  {
    // Seems to be a ECMA-48 (ANSI X3.64) compatible terminal
    TCAP(fc::t_enter_dbl_underline_mode) = \
        C_STR(CSI "21m");  // Exit single underline, too

    TCAP(fc::t_exit_dbl_underline_mode) = \
        C_STR(CSI "24m");

    TCAP(fc::t_exit_bold_mode) = \
        C_STR(CSI "22m");  // Exit dim, too

    TCAP(fc::t_exit_dim_mode) = \
        C_STR(CSI "22m");

    TCAP(fc::t_exit_underline_mode) = \
        C_STR(CSI "24m");

    TCAP(fc::t_exit_blink_mode) = \
        C_STR(CSI "25m");

    TCAP(fc::t_exit_reverse_mode) = \
        C_STR(CSI "27m");

    TCAP(fc::t_exit_secure_mode) = \
        C_STR(CSI "28m");

    TCAP(fc::t_enter_crossed_out_mode) = \
        C_STR(CSI "9m");

    TCAP(fc::t_exit_crossed_out_mode) = \
        C_STR(CSI "29m");
  }
}