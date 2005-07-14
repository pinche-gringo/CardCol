#ifndef CARDCOLAPPL_H
#define CARDCOLAPPL_H

//$Id$

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


#include <cardgames-cfg.h>

#include "Options.h"

#include <YGP/IVIOAppl.h>


#ifdef HAVE_LIBPTHREAD
#  define DEFPORT                31338
#  define STRING(nr)             #nr
#endif


/**Console part of the Cardgames; cares about reading the INI-file and
 * processing the options
 */
class CardgameAppl : public YGP::IVIOApplication {
 public:
   CardgameAppl (const int argc, const char* argv[])
      : IVIOApplication (argc, argv, lo) { }
   ~CardgameAppl () { }

   static int convertToGameType (const char* pText);

#ifdef HAVE_LIBPTHREAD
   static const unsigned int PORT;
#endif

 protected:
   virtual void readINIFile (const char* pFile);
   virtual bool handleOption (const char option);

   // Program-handling
   virtual bool        shallShowInfo () const { return false; }
   virtual int         perform (int argc, const char* argv[]);
   virtual const char* name () const { return PACKAGE_NAME; }
   virtual const char* description () const;

   // Help-handling
   virtual void showHelp () const;

 private:
   // Prohobited manager functions
   CardgameAppl ();
   CardgameAppl (const CardgameAppl&);
   const CardgameAppl& operator= (const CardgameAppl&);

   Options options;

   static const longOptions lo[];
};

#endif
