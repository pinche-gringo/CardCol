//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Console-Application
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 7.7.2005
//COPYRIGHT   : Copyright (C) 2005 - 2009

// This file is part of CardCol.
//
// CardCol is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CardCol is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CardCol.  If not, see <http://www.gnu.org/licenses/>.


#include <cardgames-cfg.h>

#include <cstdio>

#include <YGP/File.h>
#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/INIFile.h>

#include <XGP/XAttribute.h>

#include <card/Images.h>
#include <card/ScoreDlg.h>
#include <card/ComputerPlayer.h>

#ifdef WITH_BURACO
#  include "Buraco.h"
#  include "BuracoCards.h"
#endif
#ifdef WITH_HEARTS
#  include "Hearts.h"
#endif
#ifdef WITH_ROVHULT
#  include "Rovhult.h"
#  include "CardValue.h"
#endif
#ifdef WITH_SGTMAYOR
#  include "SgtMayor.h"
#endif
#include "GameTypes.h"

#include "CardColAppl.h"


const YGP::IVIOApplication::longOptions CardgameAppl::lo[] = {
   { IVIOAPPL_HELP_OPTION },
   { "game", 'g' },
   { "browser", 'b' },
   { "dir-help", 'd' },
   { "file", 'f' },
   { "list-games", 'G' },
#ifdef WITH_NETWORK
   { "listen-at", 'l' },
   { "connect-to", 'c' },
#endif
   { "version", 'V' },
#ifdef SAVE_GAME
   { "save-game", 'S' },
   { "load-game", 'L' },
#endif
   { NULL, '\0' } };


#ifdef WITH_NETWORK
const unsigned int CardgameAppl::PORT (DEFPORT);
#endif



//-----------------------------------------------------------------------------
/// Displays the help
//-----------------------------------------------------------------------------
void CardgameAppl::showHelp () const {
   std::cout << _("Collection of cardgames\n\nUsage: ") << PACKAGE
             << _(" [OPTIONS]\n\n")
      /* For translations: Write the Rovhult as o-slash */
             << "  -g, --game ......... " << _("GAME Select game to start (default: Rovhult)\n")
             << "  -G, --list-games ... " << _("List available games\n")
             << "  -f, --file ......... " << _("FILE Use file as INI file\n")
             << "  -b, --browser ...... " << _("NAME Browser to use to display the help\n")
             << "  -d, --dir-help ..... " << _("DIR Directory to search for help\n")
#ifdef WITH_NETWORK
             << "  -l, --listen-at .... " << _("PORT Awaits connections on port PORT\n")
             << "  -c, --connect-to ... " << _("SERVER:PORT Connects to SERVER:PORT\n")
#endif
#ifdef SAVE_GAME
             << "  -S, --save-game .... " << _("FILE Saves game into FILE\n")
             << "  -L, --load-game .... " << _("FILE Load game from FILE\n")
#endif
             << "  -V, --version ...... " << _("Output version information and exit\n")
             << "  -h, -?, --help ..... " << _("Displays this help and exit\n\n")

      /* For translations: Write one of the Rovhults as an O with slash */
             << _("Valid values for GAME are - unless disabled while configuring - Buraco, Hearts,\n"
		  "Machiavelli, Rovhult, SgtMayor, Twopart and the numbers 0 - 5 (corresponding\n"
		  "to the games in the above order) or the translation of the name (as displayed\n"
		  "below).\n\n");
   showGames ();

   std::cout << _("\nThe INI file can have the following entries:\n\n")
             <<  "  [Game]\n"
                 "  Type=Twopart\n"
                 "  Helpbrowser=firefox\n"
                 "  Helpdir=/usr/share/doc/Cardgames/\n"
                 "  WindowPosX=0\n"
                 "  WindowPosY=0\n"
                 "  WindowWidth=760\n"
                 "  WindowHeight=754\n"
                 "  ScoreDlgPosX=772\n"
                 "  ScoreDlgPosY=1\n\n"
                 "  [Cards]\n"
                 "  Width=72\n"
                 "  Height=96\n"
                 "  Front=/usr/share/carddecks/cards-default/\n"
                 "  Back=/usr/share/carddecks/decks/deck1.png\n\n"
                 "  [Player]\n"
                 "  0=Human\n"
                 "  1=Computer 1\n"
                 "  2=Computer 2\n"
                 "  3=Computer 3\n"
#ifdef WITH_BURACO
                 "\n  [Buraco]\n"
                 "  Cards=11\n"
                 "  EndPoints=3000\n"
#endif
#ifdef WITH_HEARTS
                 "\n  [Hearts]\n"
                 "  EndPoints=100\n"
#endif
#ifdef WITH_ROVHULT
                 "\n  [Rovhult]\n"
                 "  CardNuke=10\n"
                 "  CardReverse=7\n"
                 "  CardSkip=8\n"
#endif
#ifdef WITH_SGTMAYOR
                 "\n  [SgtMayor]\n"
                 "  Tricks=10\n"
#endif
      ;
}

//-----------------------------------------------------------------------------
/// Checks the validity of the passed option
/// \param option Actual option
/// \returns bool Status; false: Invalid option/option-value Require :
///     option not '\0´'
//-----------------------------------------------------------------------------
bool CardgameAppl::handleOption (const char option) {
   Check3 (option != '\0');

   switch (option) {
   case 'g': {
      const char* game (getOptionValue ());
      if (game) {
         int type (convertToGameType (game));
         if (type != GameTypes::NONE)
            options.type = type;
         else {
            Glib::ustring err (_("-warning: Invalid game type `%1'"));
            err.replace (err.find ("%1"), 2, game);
            std::cerr << PACKAGE << err << '\n';
         }
      }
      else
         std::cerr << PACKAGE << _("-warning: No game specified! Ignoring option `g'\n");
      break; }

   case 'd': {
      const char* pDir (getOptionValue ());
      if (pDir)
         options.helpPath = pDir;
      else
         std::cerr << PACKAGE << _("-warning: No directory specified! Ignoring option `d'\n");
      break; }

   case 'b': {
      const char* pBrowser (getOptionValue ());
      if (pBrowser)
         options.browser = pBrowser;
      else
         std::cerr << PACKAGE << _("-warning: No browser specified! Ignoring option `b'\n");
      break; }

   case 'f': {
      const char* pFile (getOptionValue ());
      if (pFile)
         readINIFile (pFile);
      else
         std::cerr << PACKAGE << _("-warning: No file specified! Ignoring option `f'\n");
      break; }

   case 'G':
      showGames ();
      exit (0);
      break;

#ifdef WITH_NETWORK
   case 'l': {
      const char* port (getOptionValue ());
      if (port)
         options.port = port;
      else {
	 std::string e (_("-warning: No port specified - using %1!\n"));
	 e.replace (e.find ("%1"), 2, options.port = STRING (DEFPORT));
	 std::cerr << PACKAGE << e;
      }
      break; }

   case 'c': {
      const char* target (getOptionValue ());
      if (target) {
         char* port (strchr (target, ':'));
         if (port) {
            options.target.assign (target, port - target);
            options.port = port + 1;
         }
         else {
             options.target = target;
             options.port = STRING (DEFPORT);
         }
      }
      else
         std::cerr << PACKAGE << _("-warning: No target specified! Ignoring option `c'\n");
      break; }
#endif

   case 'V':
      std::cout << description () << '\n';
      exit (0);
      break;

#ifdef SAVE_GAME
   case 'L':
   case 'S':
      const char* file (getOptionValue ());
      if (file) {
         if (options.gameFile.size ()) {
            std::string info (_("-warning: Option `%1' has already been specified!"
                                "\nOverwriting old setting\n"));
            info.replace (info.find ("%1"), 2, 1, (options.load ? 'L' : 'S'));
            std::cerr << PACKAGE  << info;
         }

         options.gameFile = file;
         options.load = (option == 'L');
      }
      else {
         std::string info (_("-warning: No file specified! Ignoring option `%1'\n"));
         info.replace (info.find ("%1"), 2, 1, option);
         std::cerr << PACKAGE << info;
      }
      break;
#endif

   default:
      return false;
   }

   return true;
}

//-----------------------------------------------------------------------------
/// Converts a text to a game type
/// \param pText Text to convert
/// \returns int Type of game as understood by the CardgameCollection
//-----------------------------------------------------------------------------
int CardgameAppl::convertToGameType (const char* pText) {
   TRACE9 ("CardgameAppl::convertToGameType (const char*) - " << pText);

   if (!strcmp (pText, "Rovhult"))
      return GameTypes::ROVHULT;

    try {
       return GameTypes::get ()[_(pText)];
    }
    catch (std::out_of_range&) {
       try {
	  YGP::ANumeric value (pText);
	  int iVal (value);
	  if (GameTypes::get ().exists (iVal))
	     return iVal;
       }
       catch (std::invalid_argument&) { }
    }
    return GameTypes::NONE;
}

//-----------------------------------------------------------------------------
/// Reads the options of the INI-file
/// \param pFile Pointer to filename
/// \pre pFile not NULL
//-----------------------------------------------------------------------------
void CardgameAppl::readINIFile (const char* pFile) {
   TRACE5 ("CardgameAppl::readINIFile (const char*) - " << pFile);
   Check3 (pFile);

   if (options.names.empty ()) {
      options.names.push_back (_("Human"));
      options.names.push_back (_("Player 1"));
      options.names.push_back (_("Player 2"));
      options.names.push_back (_("Player 3"));
      options.pNameINIFile = pFile;
   }

   try {
      INIFILE (pFile);
      INISECTION (Game);
      _inifile_.addEntity (options, Game);
      INIATTR2 (Game, unsigned int, CardgameCollection::WIDTH, WindowWidth);
      INIATTR2 (Game, unsigned int, CardgameCollection::HEIGHT, WindowHeight);
      INIATTR2 (Game, int, CardgameCollection::POSX, WindowPosX);
      INIATTR2 (Game, int, CardgameCollection::POSY, WindowPosY);
      INIATTR2 (Game, int, Card::ScoreDlg::LASTX, ScoreDlgPosX);
      INIATTR2 (Game, int, Card::ScoreDlg::LASTY, ScoreDlgPosY);
      INIATTR2 (Game, std::string, options.co.decks, CardFront);
      INIATTR2 (Game, std::string, options.co.back, CardBack);
      INIATTR2 (Game, unsigned int, Card::ComputerPlayer::TIMEOUT, Delay);

      INILIST2 (Player, Glib::ustring, options.names);

      INISECTION (Cards);
      _inifile_.addEntity (options.co, Cards);
      INIATTR2 (Cards, unsigned int, Card::Images::HEIGHT, Height);
      INIATTR2 (Cards, unsigned int, Card::Images::WIDTH, Width);

#ifdef WITH_BURACO
      INISECTION (Buraco);
      INIATTR2 (Buraco, unsigned int, Buraco::ENDPOINTS, EndPoints);
      INIATTR4 (Buraco, BuracoCards::get (), Buraco::CARDS2DEAL, Cards);
#endif

#ifdef WITH_HEARTS
      INISECTION (Hearts);
      INIATTR2 (Hearts, unsigned int, Hearts::ENDPOINTS, EndPoints);
#endif

#ifdef WITH_ROVHULT
      TRACE9 ("Nuke: " << Rovhult::cardNuke << "; Reverse: " << Rovhult::cardReverse
	      << "; Skip: " << Rovhult::cardSkip);
      INISECTION (Rovhult);
      INIATTR4 (Rovhult, CardValue::get (), (unsigned int&)Rovhult::cardNuke, CardNuke);
      INIATTR4 (Rovhult, CardValue::get (), (unsigned int&)Rovhult::cardReverse, CardReverse);
      INIATTR4 (Rovhult, CardValue::get (), (unsigned int&)Rovhult::cardSkip, CardSkip);
#endif

#ifdef WITH_SGTMAYOR
      INISECTION (SgtMayor);
      INIATTR2 (SgtMayor, unsigned int, SgtMayor::ENDTRICKS, Tricks);
#endif
      INIFILE_READ ();
      TRACE9 ("Nuke: " << Rovhult::cardNuke << "; Reverse: " << Rovhult::cardReverse
	      << "; Skip: " << Rovhult::cardSkip);
   }
   catch (YGP::FileError&) { }
   catch (std::exception& error) {
      Glib::ustring err (_("-warning: Error reading INI-file `%1'! %2\n"));
      err.replace (err.find ("%1"), 2, pFile);
      err.replace (err.find ("%2"), 2, error.what ());
      std::cerr << name () << err;
   }

   int type (convertToGameType (options.strType.c_str ()));
   if (type != GameTypes::NONE)
      options.type = type;
   else {
      Glib::ustring err ("-warning: INI-file `%1' contains invalid game type `%2'");
      err.replace (err.find ("%1"), 2, pFile);
      err.replace (err.find ("%2"), 2, options.strType);
      std::cerr << PACKAGE << err << '\n';
   }

   // Correct the timeout of the computer player
   if (Card::ComputerPlayer::TIMEOUT < 100)
      Card::ComputerPlayer::TIMEOUT = 100;

#ifdef WITH_ROVHULT
   CardgameCollection::checkRovhultSpecialCards ();
#endif
}

//-----------------------------------------------------------------------------
/// Performs the job of the applications
/// \param int Number of parameters (without options)
/// \param const char* Array with pointer to arguments
/// \returns int Status
//-----------------------------------------------------------------------------
int CardgameAppl::perform (int, const char**) {
   TRACE5 ("CardgameAppl::perform (int, const char**) - Params: " << args);
   srand (time (NULL));              // Initialize the random number generator

   CardgameCollection win (options);
   Gtk::Main::run (win);
   return 0;
}

//-----------------------------------------------------------------------------
/// Shows a description of the program
//-----------------------------------------------------------------------------
const char* CardgameAppl::description () const {
   static std::string version
      (PACKAGE " V" VERSION " - "
       + std::string (_("Compiled on"))
       + std::string (" " __DATE__ " - " __TIME__ "\n\n")
       + std::string (_("Copyright (C) 2002 - 2009 Markus Schwab; e-mail: g17m0@users.sourceforge.net"
			"\nDistributed under the terms of the GNU General "
			"Public License")));
   return version.c_str ();
 }

//-----------------------------------------------------------------------------
/// Shows the available games
//-----------------------------------------------------------------------------
void CardgameAppl::showGames () const {
   const GameTypes& types (GameTypes::get ());
   std::cout << _("Available games:\n\n");
   for (GameTypes::const_iterator i (types.begin ()); i != types.end (); ++i)
      std::cout << "  " << i->first << ": " << i->second << '\n';
}


//-----------------------------------------------------------------------------
/// Entrypoint of application
/// \param argc Number of parameters
/// \param argv Array with pointer to parameter
/// \returns int Status
//-----------------------------------------------------------------------------
int main (int argc, const char* argv[]) {
   YGP::IVIOApplication::initI18n (PACKAGE, LOCALEDIR);
#ifdef WITH_NETWORK
   Glib::thread_init ();
#endif

   Gtk::Main gtk (&argc, const_cast<char***> (&argv));
   CardgameAppl appl (argc, argv);
   return appl.run ();
}
