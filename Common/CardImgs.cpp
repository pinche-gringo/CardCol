//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Common
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 29.03.2002
//COPYRIGHT   : Copyright (C) 2002 - 2006

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#include <cerrno>
#include <cstdlib>

#include <iomanip>
#include <sstream>

#include <cardgames-cfg.h>

#include <YGP/File.h>
#include <YGP/Check.h>
#include <YGP/Trace.h>

#include <gtkmm/widget.h>

#include "CardImgs.h"


#ifdef KDECARDS_DIR
//-----------------------------------------------------------------------------
/// Converts an image-number to a file-name (KDE-style)
/// \param nrImage: Number of image to convert (in the range 0 - 51)
/// \returns std::string: Filename (KDE-style)
//-----------------------------------------------------------------------------
static std::string convert2KDEFile (unsigned int nrImage) {
   Check3 (nrImage < 52);
   std::ostringstream out;
   out << (nrImage + 1) << ".png";
   return out.str ();
}
#endif

#ifdef CARDPICS_DIR
//-----------------------------------------------------------------------------
/// Converts an image-number to a file-name (Cardpics-style)
/// \param nrImage: Number of image to convert (in the range 0 - 51)
/// \returns std::string: Filename (Cardpics-style)
//-----------------------------------------------------------------------------
static std::string convert2CardpicsFile (unsigned int nrImage) {
   Check3 (nrImage < 52);

   // Special handling of aces
   if (nrImage < 5)
      nrImage *= 14;
   else
      nrImage = ((55 - nrImage) >> 2) + ((nrImage & 3) * 14);

   std::ostringstream out;
   out << std::setw (2)
       << std::setfill ('0') << nrImage << ".png";
   return out.str ();
}
#endif


//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
CardImages::~CardImages () {
   TRACE9 ("CardImages::~CardImages ()");
}


//-----------------------------------------------------------------------------
/// Retrieves the specified cardnumber
/// \param nr: Number of card to retrieve
//-----------------------------------------------------------------------------
const Glib::RefPtr<Gdk::Pixbuf> CardImages::getCardImage (unsigned int nr) const {
   TRACE8 ("CardImages::getCardImage (unsigned int) - Request for card " << nr);
   Check1 (nr < size ());
   Check3 (cards_[nr]);
   return cards_[nr];
}

//-----------------------------------------------------------------------------
/// Loads the cards (faces)
/// \param path: Path to files
/// \param thread: Flag if loading in thread
//-----------------------------------------------------------------------------
void CardImages::loadDecks (const std::string& path, bool thread) throw (std::string) {
   TRACE1 ("CardImages::loadDecks (const Gdk::Window&, const char*) - " << path
           << "; Threaded: " << (thread ? "Yes" : "No"));

   std::string file (path);
   if (file[file.size () - 1] != YGP::File::DIRSEPARATOR)
      file += YGP::File::DIRSEPARATOR;

   std::string err;

   typedef std::string (*PCONVERT)(unsigned int);

#ifdef KDECARDS_DIR
#  ifdef CARDPICS_DIR
   PCONVERT fnConvert (&convert2KDEFile);
   TRACE1 ("CardImages::loadDecks (const Gdk::Window&, const char*) - " << path.substr (0, strlen (KDECARDS_DIR))
	   << "<->" << KDECARDS_DIR);
   if (path.substr (0, strlen (KDECARDS_DIR)) != KDECARDS_DIR)
      fnConvert = convert2CardpicsFile;
#  else
   PCONVERT fnConvert (&convert2CardpicsFile);
#  endif
#else
#  ifdef CARDPICS_DIR
   PCONVERT fnConvert (&convert2KDEFile);
#  else
#     error No supported cards installed!
#  endif
#endif

   std::string actFile;
   for (int i (0); i < 52; ++i) {
      actFile = file + fnConvert (i);
      TRACE9 ("CardImages::loadDecks (const Gdk::Window&, const char*) - File " << actFile);

      if (thread)
         gdk_threads_enter ();

      try {
         cards_[i] = Gdk::Pixbuf::create_from_file (actFile);
	 if ((cards_[i]->get_height () != 72) || (cards_[i]->get_width () != 96))
	    cards_[i] = cards_[i]->scale_simple (72, 96, Gdk::INTERP_BILINEAR);
         Check3 (cards_[i]);
      }
      catch (Gdk::PixbufError& e) {
         err = e.what ();
      }
      catch (Glib::FileError& e) {
         err = e.what ();
      }
      catch (...) {
         err = _("Unknown error");
      }
      if (thread)
         gdk_threads_leave ();

      if (err.size ())
         throw (err);
   }
}

//-----------------------------------------------------------------------------
/// Loads the background card
/// \param back: File containing background picture
/// \param thread: Flag if loading in thread
//-----------------------------------------------------------------------------
void CardImages::loadBack (const std::string& back,
                           bool thread) throw (std::string) {
   TRACE1 ("CardImages::loadBack (const Gdk::Window&, const char*) - " << back);
   std::string err;

   if (thread)
      gdk_threads_enter ();
   try {
      back_ = Gdk::Pixbuf::create_from_file (back);
      if ((back_->get_height () != 72) || (back_->get_width () != 96))
	 back_ = back_->scale_simple (72, 96, Gdk::INTERP_BILINEAR);
   }
   catch (Gdk::PixbufError& e) {
      err = e.what ();
   }
   catch (Glib::FileError& e) {
      err = e.what ();
   }
   catch (...) {
      err = _("Unknown error");
   }
   if (thread)
      gdk_threads_leave ();

   if (err.size ())
      throw (err);
}
