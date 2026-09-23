// PROJECT     : Cardgames
// SUBSYSTEM   : Application
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 9.9.2002
// COPYRIGHT   : Copyright (C) 2002 - 2018, 2026

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

#if !(defined(WITH_HEARTS) || defined(WITH_BURACO) || defined(WITH_TWOPART) || defined(WITH_ROVHULT) ||                          \
      defined(WITH_SGTMAYOR) || defined(WITH_JABBERWOCKY) || defined(WITH_MACHIAVELLI))
#    error All games are disabled!
#endif

#include <cerrno>
#include <cstdlib>

#include <fstream>
#include <locale>
#include <string>

#include <gtkmm/messagedialog.h>
#include <gtkmm/popovermenubar.h>

#include <giomm/menu.h>
#include <giomm/simpleaction.h>
#include <giomm/simpleactiongroup.h>

#include <glibmm/variant.h>

#include <XGP/XDialog.h>

#include <YGP/ANumeric.h>
#include <YGP/Check.h>
#include <YGP/File.h>
#include <YGP/INIFile.h>
#include <YGP/Trace.h>

#include <XGP/XAbout.h>

#include <card/ComputerPlayer.h>
#include <card/DeckSelect.h>
#include <card/Human.h>
#include <card/PlayerDlg.h>
#include <card/ScoreDlg.h>

#include "GameTypes.h"

#ifdef WITH_BURACO
#    include "Buraco.h"
#    include "BuracoCards.h"
#endif
#ifdef WITH_HEARTS
#    include "Hearts.h"
#endif
#ifdef WITH_JABBERWOCKY
#    include "Jabberwocky.h"
#endif
#ifdef WITH_MACHIAVELLI
#    include "Machiavelli.h"
#endif
#ifdef WITH_ROVHULT
#    include "CardValue.h"
#    include "Rovhult.h"
#endif
#ifdef WITH_SGTMAYOR
#    include "SgtMayor.h"
#endif
#ifdef WITH_TWOPART
#    include "Twopart.h"
#endif

#include "Settings.h"

#if defined CARDPICS_DIR
#    define CARDDECKS_DIR CARDPICS_DIR
#    define CARDDECKS_FRONT
#    define CARDDECKS_BACK "78.png"
#elif defined GNOMECARDS_DIR
#    define CARDDECKS_DIR GNOMECARDS_DIR
#    define CARDDECKS_FRONT "gnomangelo_bitmap.svg"
#    define CARDDECKS_BACK "gnomangelo_bitmap.svg"
#elif defined KDECARDS_DIR
#    define CARDDECKS_DIR KDECARDS_DIR
#    define CARDDECKS_FRONT "cards-default/"
#    define CARDDECKS_BACK "decks/deck1.png"
#else
#    define CARDDECKS_DIR ""
#    define CARDDECKS_FRONT ""
#    define CARDDECKS_BACK ""
#endif

#include "CardOptions.h"
#include "CardOptions.meta"
#include "Options.h"
#include "Options.meta"

#include "CardColAppl.h"

#include "CardCol.h"

int CardgameCollection::POSX(-1);
int CardgameCollection::POSY(-1);
unsigned int CardgameCollection::WIDTH(760);
unsigned int CardgameCollection::HEIGHT(700);

namespace YGP {

/// Writes the contents of the passed values to the passed stream
/// (in its own section named \c section).
/// \param stream Stream to write to
/// \param section Name of section to write
/// \param values Values to write
template <>
void INIList<Glib::ustring>::write(std::ostream& stream, const char* section, const std::vector<Glib::ustring>& values) {
    writeHeader(stream, section);
    for (unsigned int i(0); i < values.size(); ++i)
        stream << i << '=' << values[i].c_str() << '\n';
    stream << '\n';
}

} // namespace YGP

#include "IconAuthor.h"
#include "IconProgram.h"

/* XPM for the Joker; borrowed (and simplified) from the Warwick cardset of the
   KDE cardgames */
static const char* xpmJoker[] = {"72 96 65 1",
                                 " 	c None",
                                 ".	c #030303",
                                 "+	c #0E322E",
                                 "@	c #1D6266",
                                 "#	c #40818D",
                                 "$	c #51AEC8",
                                 "%	c #CA0303",
                                 "&	c #A39F9A",
                                 "*	c #B5B5B1",
                                 "=	c #84CCD0",
                                 "-	c #A30404",
                                 ";	c #F6CE02",
                                 ">	c #26829E",
                                 ",	c #D03333",
                                 "'	c #B2DEDE",
                                 ")	c #710607",
                                 "!	c #863A3D",
                                 "~	c #32C1A1",
                                 "{	c #DF7878",
                                 "]	c #E3DBD4",
                                 "^	c #5E2E32",
                                 "/	c #C6E6E6",
                                 "(	c #470707",
                                 "_	c #F2E2D6",
                                 ":	c #96D6D6",
                                 "<	c #967E02",
                                 "[	c #14221F",
                                 "}	c #D2EAEA",
                                 "|	c #AAD6E2",
                                 "1	c #E69A9A",
                                 "2	c #E5EEEE",
                                 "3	c #228672",
                                 "4	c #EAC60A",
                                 "5	c #24534C",
                                 "6	c #228E76",
                                 "7	c #29271E",
                                 "8	c #F6E6E6",
                                 "9	c #121A1A",
                                 "0	c #D1CECB",
                                 "a	c #5FCDB5",
                                 "b	c #26927A",
                                 "c	c #706A68",
                                 "d	c #F2F2F2",
                                 "e	c #978984",
                                 "f	c #768188",
                                 "g	c #2A8EAA",
                                 "h	c #220202",
                                 "i	c #F2F6F6",
                                 "j	c #16424E",
                                 "k	c #269E86",
                                 "l	c #227A7A",
                                 "m	c #ECD1C2",
                                 "n	c #353532",
                                 "o	c #745A42",
                                 "p	c #71BCD4",
                                 "q	c #0E1010",
                                 "r	c #AA222A",
                                 "s	c #FAFAFA",
                                 "t	c #625202",
                                 "u	c #2E96B6",
                                 "v	c #32A3BF",
                                 "w	c #C55151",
                                 "x	c #C69292",
                                 "y	c #5A554F",
                                 "z	c #C9BBAC",
                                 ".q.7n**************************************************************n....",
                                 "..y00sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssszcq..",
                                 ".c0iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiisss&q.",
                                 ".&sssssssssssssssssssssssssssssssssi$$$psssssssssssssssssssssssssisisiq.",
                                 "&ssisisiz...ydsisisisisisisis/}$vv>>>n7>vvpisisisisisisisisisisisssissi.",
                                 "&ssssssi]n.[essssssssssssis'|vvuc@...;4qv$|'sssssssssssssssssssssssisisq",
                                 "&is2sdsis*.yssdsdsdsdsdsds:$vvuo(h.@@tt@v$||s2sdsdsdsdsdsdsdsdsdsssssii.",
                                 "&sdssssss*.ysssssdsssdssss]$vv!%h.lvvl>v//ssdsssdsssdsssdsssdssssisisssq",
                                 "&ssisdsis*.y2sdsssdsssds=vvv#r%h..uvvv/sdisssdsssdsssdsssdsssdsissssdsi.",
                                 "&ssdssisd*.yssssdsss2s/||$vur%%h..5vvv$||'sdsssdsssdsssdsssdssssdsdsssiq",
                                 "&issdsssi*.y2sdsssds'=vvvvuo%%%h...uvvgvv$p=p=p=p|sssdsssdsssdsssssdssi.",
                                 "&ssssdssd*.ysssdisss8pvuvvvr%%(....9vvvvuvv>9[9jvv$$$$$$$'dsssisisssdssq",
                                 "&sdsssdss*.ydsssd=vvvuvvv>^%%%h....n.>>l7n7ysss*nnn[.9n5gv$|issssdsssds.",
                                 "&issdsssd*.ysdss2pgvl@@5@9ywr%h..yydy.cciss_1111111c[o;o@vpdsdsdssisssdq",
                                 "&sdycsssi*.ydsd/$>jjc&e&7.*d11&e&issdqss8{ww%%%%roof>yty>vp2ssssdssdsss.",
                                 "&i].qyisz7.ys'$$@o00]siiy.*ssddsidss2.,,%%%%%r#f#uvvvg3gv=dssdsssdssisdq",
                                 "&ss]7....qcd/$u@cmw%%%%%(90d]00z00dssdh%%%%!vvv$$:siissississsdsssdsssi.",
                                 "&sss2ycycess}p@c{%r!!r%-y0&&yyyyyyeeido)))ov$p|ssssissssiissdsissssdssiq",
                                 "&ssss]cfids/pv@^,!fvu#r(&&fe08]88_eecidee9vv$p'issssdssssisssssdsdssssi.",
                                 "&iss0y0&css8=v@)!uvvvv#[&y&yn708n77]znss]quvvvv$=idsssdssdsdsdssssdsdssq",
                                 "&sss0[i07ssd}=@)!vvvvuvv+&]zz*mz8zz*_]qs]5>vuv$pssssdssdss=$vvu|dssssds.",
                                 "&iis]7i0nsd/pg5oo>uvvvvv+&&[..z[0q..emc&d0[vvu$|||||/sss/||$l@gv=|||dsdq",
                                 "&sis07i*nsi}$@o;o5vvuvvv+&xeee070eeee]09s]fjjjj@vuvp/s2s/|$v^hlvujju2ss.",
                                 "*iss2*ny]ssd|u575uvvvvuvgj*_m]y&m_]_m_07ssz7z00e[[jv$$'d=$$!)@gvv[.g$ddq",
                                 "&sisd0i20isdd'p$$uvvvvvvu+&8]_y&znmz_m0qssssdsdd&...+gvvvg>^(5[..>vv$$i.",
                                 "&sss07s0ndssssss'$uvvuvv$@cm_e0cymey__ffsiidsss0c....+>uvj.ny9...>v:issq",
                                 "&sss070f*sdiissds}pvvvvuvv[&_&ooyy.]_m9dsssisdm!.....(^l@qycz&[.5>vvpdi.",
                                 "&sis].yzsssssssssd2puvuvvj&yz]z9..*m_n*7yssdm,,%)...(-wc9cmz08z7q[>vv$dq",
                                 "&sis07]y0ssisssis|$vvvv>5ziiy&m]zz__.......(%%%%%-[)%%)fyeycmy&n^r(.g'd.",
                                 "&sss][d0nssisisssd=vvvl50ssssefmcyme......h)%%%%%,1m,%wcnf&oz&[7>voju=iq",
                                 "&sis2&s]*sdsssd/==av$v5&iiisis&cyyy.........)%%%%wssmw-^ecyyo9@vv$~=|8s.",
                                 "&sss}7n7cssdsisi}=~~~@)^*sssiw,-----.........(-)w]dss8c.9ey@k3~~'}}sisdq",
                                 "&sds]7siidssd}}=~~~kj)%%)e0m,%%%%%%%h........n9.^]ds]c.9eoeng~~~aa}ssss.",
                                 "&sss09e*sssi}=a~~~~5)%%%%(.-%%%%%%%%%....qycfi*.qnzzy.7*&r%)5v~=dssisidq",
                                 "&iss]9e&ssss}:~~~~67-%%%)..-%%%%%%%%%..e&&isds]c957n.n0sd{%%(3~~/sssssi.",
                                 "&dss07ssiiis}/~~~~@)%%%)...h-%%%%%%%h00sssssss2&jl.qy]sdi8,%(3~~a2siisiq",
                                 "&sis}nnncii}a~~~~@.(-,^7n7q..)----...iidsississ&..q9.c]si]y((@~~|}dsssi.",
                                 "&sss2&e&siii}aa~~@..(xdsi*7.........qsisiss]&e&c.[l6[.y0zn....b~~adsdssq",
                                 "&sds09&ffsiis}a~69..yzsse[59.........ssssi]cey..[6~~k+h!^.....6~~a:2ssi.",
                                 "&iss07szys/aaa~~@...esifq@~k9........ndss07q7[.nf@~39^%%-(....6~~~'issiq",
                                 "&sss0qny0ssi/a~~@nnem_]n3~~k[........qsd*[_28]8]_&9.(%%%{fn7q9b~~'2iisi.",
                                 "&sis][i0nii}a~~@cdd{%1y+k3l9.........q2eyc__]_]_]]e.(-%,msiiy3~~~~a2sssq",
                                 "&dss]7s*5s}:a~~@fd1,%!7l~+t9oe&e.....q][__m______]e..hw0dsi&@v~~~~2isdi.",
                                 "&sss20s20}a~~~~@cs{%%(5~~+[qxy7n*q...qs*n_8_____]]e...),1dfl~~~~~'issssq",
                                 "&isdsisssi2=~~3(%%{sic3~~~+&;7<9]nn7n.*..hhh7zzzzc...)%%%(3~~~~:ssisdsi.",
                                 "&ssssiii}/:~~~l)%%{sic@~~~+-xccc4e&ed..q)%%-((9j+7ycy!%-n6~~~~~a2iisssiq",
                                 "&sdsssssi':a~~@^11eyc7l~~~+-%w{w_9<<c..)%%%%%-+~lcdis{%)@~~~~~~aissdssi.",
                                 "&ssissis}aaa~~3ciif...l~~5)%%%%%_q47..-%%%%%%)+~3qnnnezc@~~~~:}}isssissq",
                                 "&sssdsii}2=~~~~@cmcn7nq3~+-%%%%%%-..74qm%%%%%)5~~l...ciscl~~aaa}iidssds.",
                                 "&sdssss2a~~~~~~@)%{ii2cl~+)%%%%%)..o<<[mw{w%-+~~~l7yoe1xy@~~=''issssssdq",
                                 "&ssdsis}a~~~~~b7-%!ccy7+5q((-%%)...2&e&4oyc{)+~~~lcis{%%(l~~~==iisdsdss.",
                                 "&dsissss'~~~~37%%%)...c***z7hhh..*.n7nnm7<7;&+~~~lcss{%%(@~~:isissssssdq",
                                 "&isisi'~~~~k3cd1,)...e]_]m___m_nziq..qq*7ntxq[+~k@(%%{ic@~~~~a}]}i02dsi.",
                                 "&sisi2~~~~~@&sis0wh..e__]___]8_898q....q&&eo7t+~l[!%,1sc3~~aa}snzs[]issq",
                                 "&sss2a~~~~3yissm,%-(.em__]]_8]cyed..........9l3b+c{%{ssc@~~aisin0s7]ssi.",
                                 "&isis}'~~b9qnnc1%%%(.9x_______7*dsq........[k~~3y]8menn@~~:}2si]yn.0ssiq",
                                 "&sssi'~~~6....(-%-(93~@fn.77h7miisn........[k~@.csie...@~~a~a}inzs7]2si.",
                                 "&issi:a~~3.....^!hjk~~b[..cec]sisis.........[5[&si0y..[k~a}sissff&9]sssq",
                                 "&ssii}a~~3....nzmy.[6l[.ce&e/ssissi..........7*iiixh..@~~aa2siss*ee2idi.",
                                 "&issss}'~~@()y]sd0c.9q..&iiiiiissss...----)..q7nn^,%(.5~~~~~}sic7nn]sssq",
                                 "&sisss}a~~3(%wmsss]y..3j&sssssssd00h%%%%%%%-h...)%%%)@~~~~:iiisssi[]dsi.",
                                 "&sssiii/~~l(%%{is0n.n[j[c]sssi&&eq.%%%%%%%%%-.h)%%%-73~~~~:}isis&&9mssiq",
                                 "&ssssisi=~k5)%r&*n.y0zn..*decyq....%%%%%%%%%-.(%%%%)5~~~~a:}siss&&90sds.",
                                 "&siss2~a~~~3neof9.c]sd2n.[7q.......h%%%%%%%r00e(%%-jk~~~:}}isisssi7]ssdq",
                                 "&ssii}2'~~kb@ye9.c8isi]w)-(.........-----,wdsi2*7)@~~~a2iisissiyn7n]2ss.",
                                 "&ii|:~~~k59ooycf!-,]sd{%%%%).........yyyo&ssssss&+~~~a:a/sssiss*_s&2ssdq",
                                 "*i=g+oug+[&*yee7fw%,z1r%%%%%)h......e]yf0fediss0lluuv=issisisssn0i7]ssd.",
                                 "&2/g.(r)neymfyeyf)%%-7)%%%%%(......._m*z_m&ysdz5>vvvv$|ssissisi0y070ssiq",
                                 "&i$vvg[9nz_0zmcqcw%(...)-,,m8diy7*n]_&..9z_zyejuvvuvp2isisisisis0y.]iss.",
                                 "&sdpvvu5.7&zcy[@>^(.....!zdiisisi9m_]qoyyo&]*+vuvvvvvp}sssisssi*c][]ssdq",
                                 "&ss2=v>...9on.jvvl+....o]ssssiisce]_ye]yc0e80c@vvuvvvv$'issisisn0i7]2ss.",
                                 "&dp$$v>..[@(^>gvvvu+...&sssisssd70_]zmnz&y8_m&+vvvgvvuv$$$'ssss02i02ssdq",
                                 "&i2$>.9vvu@(!$$=d|$$$j[[e00z70di9m_m8__]&ym88*5>vvvvvvg57@g=2is0y7*2sss.",
                                 "&ssduj5uvl.!v$|/sss/pvvv@jjjjf2i9m_eeeem70eee&&+vvuvvv5<;t@p}sin0s[]isdq",
                                 "&sid|||=v>@>$||/isi/|:|||vvvv[0i&y_e..q09z..9&x+vvvvuvlow5>p/sin0i7]iss.",
                                 "&sssdsd8|vvvv=sssssisiid=$vvvl5]i9mm**zmzmzzzm&+vvvvvvvo(@=/2ssnzs7]ssdq",
                                 "&ississisddssssisisisss=$vuvvvq]iin*m77n_z7no&y&+#uvvu#!)@v:dssc&0n]sis.",
                                 "&sississssssdsisssisssdsd|p$vv9feiscee888_m0ece*(r#vvf!,^@vp/issfo}sisdq",
                                 "&sisssssdsdsssisdssdsssds|p$uo)))odi&eoyoyyc&*0^)%r!!r%{c5p}ssfyccy]sss.",
                                 "&ssdssdsssssiisssdssds=p$vvu!%%%%.2is200000]207(%%%%%{mc@u$'sc9....n]siq",
                                 "&ssissdsdssss2=uu>gvvvg#f#r%%%%,,,.2sid2didii*.ydds]0zc@$$'sy.70iiy..]s.",
                                 "&dssdsssssii2pv>5ty>#ccr%%%%ww{_dsqdss2&e&11d*.ne&ecjj>$/sssy.*sisdfyddq",
                                 "&ssssdsdsdss8pv@o;o[c11111118sddcc.odcy..h%rwn[@@@@>vvp/ssssy.*isssssss.",
                                 "&dsdsssssssd/$v#@7qq9nn7*dddc7nnl>>.7....h%%%^gvvvvuvv:sdsd2y.*dsdsissiq",
                                 "&sssdsdsdssiis'$pp$$$$$vj[[9>vvvvvvv[....(%%rvvvvvp}dsssssisy.*sssssdsi.",
                                 "&sdssssssdssssss2sssid|p=ppp=p$vuvvvv...h%%-ovuvu$p'dsdsdsidy.*ssdsssssq",
                                 "&sssdsdsssdssssssdssssssssssdd|||vvuv5..h%%rvv$||/sssssssdssy.*ssisdsii.",
                                 "&sdssssssssdsdsdssdssssisssissdsd}vvvu..h%r#vvv=dss2sdsdssddy.*dsssssssq",
                                 "&ssdsdsdsdssssssdssdsdsisssiss}}v>>vvl.h%!vv$}ssisssssssdsssy.*isdsdsdi.",
                                 "&dssssssssdsdsdsssisssssisis'|$v@tt@@..(ovuu$:isssdsdsdsssidy.*ssssssssq",
                                 "&ssdsdsdsssssssdsssdsdsssdss'|vvq4;...@cvu$|'sssdssssssdsds&9.n]idsdssi.",
                                 "&dsssssssdsdsdsssdssssdsssdsspvv>77l>>vv$/}ssdsssdsdsdssssdy...zsssissiq",
                                 "qfsdsdssssssssdsssdsdssdsssdsdsip$$pdiddssssssdsssssssdsdsssssisisssdsq.",
                                 ".q&sssdsdsdsdssdsisssdsssdsssssssssdsssssdssdsssdsdsdssssssdsisssdssseq.",
                                 ".qqccssssssssdssssssssdsssdsdsisiisisdsdssdssdsssssssdsdsssdssdsssdccqq.",
                                 ".q..qnnnnnnnnnnnnn7nn7nnnnnnnn7nnnnn7nnn7nn7nn7nn7nn7nn7nnn7n7nn7nn9..qq"};

//-----------------------------------------------------------------------------
/// Defaultconstructor; all widget are created
/// \param opts Options for the program
//-----------------------------------------------------------------------------
CardgameCollection::CardgameCollection(Options& opts)
    : XApplication(PACKAGE " V" PRG_RELEASE), status(), filler(), cardFaces(), cards(),
#ifdef WITH_NETWORK
      aCommThreads(), mxGuiCmd(), dlgChat(NULL),
#endif
      mxThreadCmd(), cmgr(), playerPos(0), options(opts), aPlayer(), oldGame(GameTypes::NONE), actGame(opts.type), restart(false),
      game(NULL) {
    TRACE9("CardGameCollection::CardGameCollection(Options&) - Game: " << actGame);

    setIconProgram(picGame, sizeof(picGame));
    // Remark: Under GTK4 a client can no longer set a window's position (see AnimWindow.h)
    set_default_size(WIDTH, HEIGHT);

    helpBrowser = options.browser;

    // Create controls
    Glib::RefPtr<Gio::Menu> menu(Gio::Menu::create());

    Glib::RefPtr<Gio::Menu> menuGame(Gio::Menu::create());
    Glib::RefPtr<Gio::Menu> secGame(Gio::Menu::create());
    apMenus[NEW] = grpAction->add_action("New", mem_fun(*this, &CardgameCollection::newGame));
    secGame->append(_("_New"), "win.New");
    apMenus[END] = grpAction->add_action("End", mem_fun(*this, &CardgameCollection::endGame));
    secGame->append(_("_End"), "win.End");
    menuGame->append_section(secGame);
#ifdef WITH_NETWORK
    Glib::RefPtr<Gio::Menu> secNet(Gio::Menu::create());
    apMenus[CONNECT] = grpAction->add_action("Connect", mem_fun(*this, &CardgameCollection::connect));
    apMenus[CONNECT]->set_enabled(false);
    secNet->append(_("Co_nnect ..."), "win.Connect");
    apMenus[CHAT] = grpAction->add_action("Chat", mem_fun(*this, &CardgameCollection::showChatDlg));
    apMenus[CHAT]->set_enabled(false);
    secNet->append(_("_Chat ..."), "win.Chat");
    menuGame->append_section(secNet);
#endif
    grpAction->add_action("Quit", mem_fun(*this, &CardgameCollection::exit));
    menuGame->append(_("_Quit"), "win.Quit");
    menu->append_submenu(_("_Game"), menuGame);

    Glib::RefPtr<Gio::Menu> menuOptions(Gio::Menu::create());

    Glib::RefPtr<Gio::Menu> menuChgGame(Gio::Menu::create());
    actChgGame = grpAction->add_action_radio_integer("ChgGame", mem_fun(*this, &CardgameCollection::changeGame), (int)actGame);
#ifdef WITH_BURACO
    menuChgGame->append(_("_Buraco"), Glib::ustring::compose("win.ChgGame(%1)", (int)GameTypes::BURACO));
#endif
#ifdef WITH_HEARTS
    menuChgGame->append(_("_Hearts"), Glib::ustring::compose("win.ChgGame(%1)", (int)GameTypes::HEARTS));
#endif
#ifdef WITH_JABBERWOCKY
    menuChgGame->append(_("_Jabberwocky"), Glib::ustring::compose("win.ChgGame(%1)", (int)GameTypes::JABBERWOCKY));
#endif
#ifdef WITH_MACHIAVELLI
    menuChgGame->append(_("_Machiavelli"), Glib::ustring::compose("win.ChgGame(%1)", (int)GameTypes::MACHIAVELLI));
#endif
#ifdef WITH_ROVHULT
    // xgettext: For translations: Write the Rovhult as o-slash
    menuChgGame->append(_("_Rovhult"), Glib::ustring::compose("win.ChgGame(%1)", (int)GameTypes::ROVHULT));
#endif
#ifdef WITH_SGTMAYOR
    menuChgGame->append(_("_Sgt. Mayor"), Glib::ustring::compose("win.ChgGame(%1)", (int)GameTypes::SGTMAYOR));
#endif
#ifdef WITH_TWOPART
    menuChgGame->append(_("_Twopart"), Glib::ustring::compose("win.ChgGame(%1)", (int)GameTypes::TWOPART));
#endif
    menuOptions->append_submenu(_("_Change game"), menuChgGame);

    grpAction->add_action("ChgDecks", mem_fun(*this, &CardgameCollection::showChangeDeckDlg));
    menuOptions->append(_("Change _decks ..."), "win.ChgDecks");
    grpAction->add_action("ChgNames", mem_fun(*this, &CardgameCollection::changeNames));
    menuOptions->append(_("Change _names ..."), "win.ChgNames");
    grpAction->add_action("Prefs", mem_fun(*this, &CardgameCollection::editPreferences));
    menuOptions->append(_("_Preferences ..."), "win.Prefs");

    Glib::RefPtr<Gio::Menu> secSave(Gio::Menu::create());
    grpAction->add_action("SavePrefs", mem_fun(*this, &CardgameCollection::savePreferences));
    secSave->append(_("_Save preferences"), "win.SavePrefs");
    menuOptions->append_section(secSave);

#if TRACELEVEL >= 1
    Glib::RefPtr<Gio::Menu> secDebug(Gio::Menu::create());
    grpAction->add_action("Debug", mem_fun(*this, &CardgameCollection::toggleDebug));
    secDebug->append(_("_Debug"), "win.Debug");
    menuOptions->append_section(secDebug);
#endif
    menu->append_submenu(_("_Options"), menuOptions);

    // Placeholder section filled/emptied by the active game's addMenus()/removeMenus()
    menuGameSection = Gio::Menu::create();
    menu->append_section(menuGameSection);
    actionsGame = Gio::SimpleActionGroup::create();
    insert_action_group("game", actionsGame);

    addHelpMenu(menu);

    Gtk::PopoverMenuBar* menuBar(Gtk::make_managed<Gtk::PopoverMenuBar>(menu));
    getClient().append(*menuBar);
    // Remark: GTK4's PopoverMenuBar has no equivalent of the old right-justified Help menu

    Check3(apMenus[NEW]);
    Check3(apMenus[END]);
    apMenus[NEW]->set_enabled(false);
    apMenus[END]->set_enabled(false);

    // Remark: Until a game is started, there's no widget expanding to fill the
    // client area, so filler pushes the statusbar to the bottom of the window
    filler.set_vexpand();
    filler.show();
    getClient().append(filler);

    status.show();
    getClient().append(status);

    show();
    // Remark: Under GTK4 a client can no longer set a window's position (see AnimWindow.h)
#ifdef WITH_NETWORK
    mxGuiCmd.lock();
#endif

    Glib::signal_idle().connect(bind_return(mem_fun(*this, &CardgameCollection::loadCards), false));
    makePlayer();

#ifdef WITH_NETWORK
    autoConnect(options);
#endif
}

//----------------------------------------------------------------------------
/// Creates the default player in the game; basing on the names read from the
/// INI file
//----------------------------------------------------------------------------
void CardgameCollection::makePlayer() {
    Check3(options.names.size());
    std::vector<Glib::ustring>::iterator i(options.names.begin());
    aPlayer.push_back(new Card::Human(*i));
    while (++i != options.names.end())
        aPlayer.push_back(new Card::ComputerPlayer(*i));
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
CardgameCollection::~CardgameCollection() {
    TRACE9("CardgameCollection::~CardgameCollection()");
    if (game) {
        game->clean();
        delete game;
    }

    for (std::vector<Card::Player*>::iterator i(aPlayer.begin()); i != aPlayer.end(); ++i)
        delete *i;

#ifdef WITH_NETWORK
    removeCommThreads();
#endif
}

//-----------------------------------------------------------------------------
/// Starts a game; if the type has changed also deleting the old one
//-----------------------------------------------------------------------------
void CardgameCollection::startGame() {
    TRACE6("CardgameCollection::startGame() - Old game type " << oldGame << " -> New: " << actGame);

    // Check if the game has been changed; if so destroy the old one
    unsigned int oldDecks(0), oldJoker(0);

    if (game) {
        oldDecks = game->numberOfDecks();
        oldJoker = game->numberOfJokers();

        if (oldGame != actGame) {
            // Remove menubar and update GUI to not interfere with new game
            game->removeMenus(menuGameSection, actionsGame);
            Glib::RefPtr<Glib::MainContext> ctx(Glib::MainContext::get_default());
            while (ctx->iteration(false))
                ;

            getClient().remove(*game);
            delete game;
        }
    }

    if (oldGame != actGame) {
        oldGame = actGame;
        switch (oldGame) {
#ifdef WITH_ROVHULT
        case GameTypes::ROVHULT:
            game = new Card::TGame<Rovhult, CardgameCollection>(*this, &CardgameCollection::gameEvents);
            break;
#endif

#ifdef WITH_TWOPART
        case GameTypes::TWOPART:
            game = new Card::TGame<Twopart, CardgameCollection>(*this, &CardgameCollection::gameEvents);
            break;
#endif

#ifdef WITH_HEARTS
        case GameTypes::HEARTS:
            game = new Card::TGame<Hearts, CardgameCollection>(*this, &CardgameCollection::gameEvents);
            break;
#endif

#ifdef WITH_BURACO
        case GameTypes::BURACO:
            game = new Card::TGame<Buraco, CardgameCollection>(*this, &CardgameCollection::gameEvents);
            break;
#endif

#ifdef WITH_MACHIAVELLI
        case GameTypes::MACHIAVELLI:
            game = new Card::TGame<Machiavelli, CardgameCollection>(*this, &CardgameCollection::gameEvents);
            break;
#endif

#ifdef WITH_SGTMAYOR
        case GameTypes::SGTMAYOR:
            game = new Card::TGame<SgtMayor, CardgameCollection>(*this, &CardgameCollection::gameEvents);
            break;
#endif

#ifdef WITH_JABBERWOCKY
        case GameTypes::JABBERWOCKY:
            game = new Card::TGame<Jabberwocky, CardgameCollection>(*this, &CardgameCollection::gameEvents);
            break;
#endif

        default:
            Check(0);
        }
        game->addMenus(menuGameSection, actionsGame);
        filler.hide();
    }

    // Change number of jokers if necessary
    if (oldJoker != game->numberOfJokers()) {
        TRACE5("CardgameCollection::startGame() - Re-adding jokers " << oldJoker << "->" << game->numberOfJokers());
        for (unsigned int i(0); i < oldJoker; ++i)
            cardFaces.delImage(cardFaces.size() - 1);

        for (unsigned int i(0); i < game->numberOfJokers(); ++i)
            cardFaces.addImage(xpmJoker);
    }

    // Change number of decks if necessary
    if ((oldDecks != game->numberOfDecks()) || (oldJoker != game->numberOfJokers())) {
        TRACE5("CardgameCollection::startGame() - Changing carddecks " << oldDecks << "->" << game->numberOfDecks());

        cards.clear();
        for (unsigned int i(0); i < game->numberOfDecks(); ++i)
            cards.addPacket(cardFaces);
    }

    Check3(game);
    Glib::ustring name(GameTypes::get()[actGame]);
    name += " - " PACKAGE " V" PRG_RELEASE;
    set_title(name);

#ifdef SAVE_GAME
    if (options.gameFile.size() && options.load) {
        std::ifstream input(options.gameFile.c_str());
        char buffer[1024];

        input.getline(buffer, sizeof(buffer));
        game->setCardOrder(buffer);
    }
#endif

    if (cmgr.getMode() != YGP::ConnectionMgr::CLIENT) {
        game->start();

#ifdef SAVE_GAME
        if (options.gameFile.size() && !options.load) {
            std::ofstream output(options.gameFile.c_str());
            output << game->getCardOrder();
        }
#endif

        game->clearCardOrder();
    }
    else
        game->setGameStatus(Card::Game::NONE);
}

//-----------------------------------------------------------------------------
/// Starts a new game; running games are ended
//-----------------------------------------------------------------------------
void CardgameCollection::newGame() {
    TRACE7("CardgameCollection::newGame() - New; Game running: " << (game && game->isRunning() ? "Yes" : "No"));
    if (game && game->isRunning()) {
        Gtk::MessageDialog dlg(_("A game is already running. Do you really want to end it and start another?"), false,
                               Gtk::MessageType::QUESTION, Gtk::ButtonsType::YES_NO);
        dlg.set_title(PACKAGE);
        if (XGP::runModal(dlg) == static_cast<int>(Gtk::ResponseType::YES)) {
            restart = true;
            Glib::signal_idle().connect(bind_return(mem_fun(*this, &CardgameCollection::restartGame), false));
        }
    }
    else {
#ifdef WITH_NETWORK
        if (stopClientWaiting())
#endif
            startGame();
    }
}

//-----------------------------------------------------------------------------
/// Ends a running game
//-----------------------------------------------------------------------------
void CardgameCollection::endGame() {
    Check3(game && game->isRunning());
    Gtk::MessageDialog dlg(_("Do you really want to end the game?"), false, Gtk::MessageType::QUESTION, Gtk::ButtonsType::YES_NO);
    dlg.set_title(PACKAGE);
    if (XGP::runModal(dlg) == static_cast<int>(Gtk::ResponseType::YES)) {
        restart = false;
        restartGame();
    }
}

//-----------------------------------------------------------------------------
/// Changes the type of the next game
/// \param game Type of the next game
//-----------------------------------------------------------------------------
void CardgameCollection::changeGame(int game) {
    TRACE9("CardgameCollection::changeGame(games) - " << game);
    Check3((unsigned int)game < GameTypes::LAST);

    actGame = game;
}

//-----------------------------------------------------------------------------
/// Opens a dialog allowing to change the card decks
//-----------------------------------------------------------------------------
void CardgameCollection::showChangeDeckDlg() {
    Card::DeckSelectDlg& dlg(*Card::DeckSelectDlg::create(options.co.decks, options.co.back));
    dlg.set_transient_for(*this);
    dlg.setDecks.connect(mem_fun(*this, &CardgameCollection::changeDecks));
}

//-----------------------------------------------------------------------------
/// Opens a dialog allowing to change the names of the players
//-----------------------------------------------------------------------------
void CardgameCollection::changeNames() {
    Card::PlayerDlg* dlg(Card::PlayerDlg::create(aPlayer));
    dlg->sigCommit.connect(mem_fun(*this, &CardgameCollection::changePlayernames));
    dlg->set_transient_for(*this);
}

//-----------------------------------------------------------------------------
/// Edits the preferences
//-----------------------------------------------------------------------------
void CardgameCollection::editPreferences() {
    Settings* settings(Settings::create(*this, options));
    settings->sigCardResize.connect(mem_fun(*this, &CardgameCollection::resizeCards));
#ifdef WITH_NETWORK
    settings->sigCommit.connect(mem_fun(*this, &CardgameCollection::sendSettings));
#endif
}

//-----------------------------------------------------------------------------
/// Saves the settings
//-----------------------------------------------------------------------------
void CardgameCollection::savePreferences() {
    TRACE2("CardgameCollection::savePreferences() - Save file " << options.pNameINIFile);
    std::ofstream inifile(options.pNameINIFile);
    inifile.imbue(std::locale::classic()); // Avoid locale-dependent thousands separators
    if (inifile) {
        options.strType = GameTypes::get()[options.type];
        YGP::INIFile::write(inifile, "Game", options);

        // Remark: Under GTK4 a client can no longer query a window's position (see AnimWindow.h)
        int width(get_size(Gtk::Orientation::HORIZONTAL)), height(get_size(Gtk::Orientation::VERTICAL));
        inifile << "Delay=" << Card::ComputerPlayer::TIMEOUT << "\nWindowPosX=" << POSX << "\nWindowPosY=" << POSY
                << "\nWindowWidth=" << width << "\nWindowHeight=" << height << "\nScoreDlgPosX=" << Card::ScoreDlg::LASTX
                << "\nScoreDlgPosY=" << Card::ScoreDlg::LASTY << "\n\n";

        YGP::INIFile::write(inifile, "Cards", options.co);
        inifile << "Width=" << Card::Images::WIDTH << "\nHeight=" << Card::Images::HEIGHT << "\n\n";

        for (unsigned int i(0); i < aPlayer.size(); ++i)
            options.names[i] = aPlayer[i]->getName();
        YGP::INIList<Glib::ustring>::write(inifile, "Player", options.names);

#ifdef WITH_BURACO
        YGP::INIFile::writeSectionHeader(inifile, "Buraco");
        inifile << "Cards=" << BuracoCards::get()[Buraco::CARDS2DEAL] << "\nEndPoints=" << Buraco::ENDPOINTS << '\n';
#endif

#ifdef WITH_HEARTS
        inifile << '\n';
        YGP::INIFile::writeSectionHeader(inifile, "Hearts");
        inifile << "EndPoints=" << Hearts::ENDPOINTS << '\n';
#endif

#ifdef WITH_ROVHULT
        inifile << '\n';
        YGP::INIFile::writeSectionHeader(inifile, "Rovhult");
        inifile << "CardNuke=" << CardValue::get()[Rovhult::cardNuke]
                << "\nCardReverse=" << CardValue::get()[Rovhult::cardReverse]
                << "\nCardSkip=" << CardValue::get()[Rovhult::cardSkip] << '\n';
#endif
#ifdef WITH_SGTMAYOR
        inifile << '\n';
        YGP::INIFile::writeSectionHeader(inifile, "SgtMayor");
        inifile << "Tricks=" << SgtMayor::ENDTRICKS << '\n';
#endif
        inifile << '\n';

        inifile.flush();
        if (inifile)
            return;
    }

    Glib::ustring msg(_("Couldn't save options (to file %1)!\n\nReason: %2."));
    msg.replace(msg.find("%1"), 2, options.pNameINIFile);
    msg.replace(msg.find("%2"), 2, strerror(errno));
    Gtk::MessageDialog dlgErr(msg, false, Gtk::MessageType::ERROR);
    XGP::runModal(dlgErr);
}

//-----------------------------------------------------------------------------
/// Saves the settings
//-----------------------------------------------------------------------------
void CardgameCollection::exit() {
    if (game) {
        if (game->isRunning()) {
            Gtk::MessageDialog dlg(_("A game is running. Do you really want to quit?"), false, Gtk::MessageType::QUESTION,
                                   Gtk::ButtonsType::YES_NO);
            dlg.set_title(PACKAGE);
            if (XGP::runModal(dlg) == static_cast<int>(Gtk::ResponseType::YES)) {
                if (game->canBeStopped())
                    Glib::signal_idle().connect(mem_fun(*this, &CardgameCollection::terminateGameAndExit));
                else {
                    Glib::signal_idle().connect(mem_fun(*this, &CardgameCollection::wait4EndGameAndExit));
                    return;
                }
            }
            else
                return;
        }
    }
    hide();
}

//-----------------------------------------------------------------------------
/// Stops the game and quits application
/// \returns bool Always false
//-----------------------------------------------------------------------------
bool CardgameCollection::terminateGameAndExit() {
    Check2(game);
    game->stop();
    return false;
}

//-----------------------------------------------------------------------------
/// Waits til the game can be ended; stops it and quits application
/// \returns bool Always false
//-----------------------------------------------------------------------------
bool CardgameCollection::wait4EndGameAndExit() {
    Check2(game);
    restart = -1U;
    game->end(false);
    return false;
}

#if TRACELEVEL >= 0
//-----------------------------------------------------------------------------
/// Toggles showing/hiding the cards
//-----------------------------------------------------------------------------
void CardgameCollection::toggleDebug() {
    static bool open = false;
    open = !open;
    if (game)
        game->playOpen(open);
}
#endif

//-----------------------------------------------------------------------------
/// Returns the name of the file to display in the help
/// \returns \c Name of file to display
//-----------------------------------------------------------------------------
const char* CardgameCollection::getHelpfile() {
    std::string file(options.helpPath);
    if (file[file.size() - 1] != YGP::File::DIRSEPARATOR)
        file += YGP::File::DIRSEPARATOR;

    if (game) {
        std::string name(game->name());
        size_t pos;
        while ((pos = name.find(" ")) != std::string::npos)
            name.replace(pos, 1, 0, '\0');

        file += name;
        file += ".html";
    }
    else
        file += "CardCol.html";
    return file.c_str();
}

//-----------------------------------------------------------------------------
/// Shows the about box for the program
//-----------------------------------------------------------------------------
void CardgameCollection::showAboutbox() {
    std::string ver(_("Copyright(C) 2002 - 2018 Markus Schwab\ne-mail: g17m0@users.sourceforge.net\n\nCompiled on %1 at %2"));
    ver.replace(ver.find("%1"), 2, __DATE__);
    ver.replace(ver.find("%2"), 2, __TIME__);

    XGP::XAbout* about(XGP::XAbout::create(ver, PACKAGE " V" VERSION));
    about->setIconProgram(picGame, sizeof(picGame));
    about->setIconAuthor(picAuthor, sizeof(picAuthor));
    about->set_transient_for(*this);
}

//-----------------------------------------------------------------------------
/// Callback to change the names of the playing people
//-----------------------------------------------------------------------------
void CardgameCollection::changePlayernames() {
    TRACE2("CardgameCollection::changePlayernames");
#ifdef WITH_NETWORK
    if (cmgr.getMode() != YGP::ConnectionMgr::NONE)
        broadcastNames();
#endif

    if (game)
        game->changeNames(aPlayer);
}

//-----------------------------------------------------------------------------
/// Callback to change the carddecks
/// \param deck Deck to set
/// \param back Back of cards to set
//-----------------------------------------------------------------------------
void CardgameCollection::changeDecks(const std::string& deck, const std::string& back) {
    TRACE2("CardgameCollection::changeDecks(2x const std::string&)");

    unsigned int option(0);
    if ((deck.size() && (deck != options.co.decks)) || !cardFaces.size()) {
        option = 1;
        options.co.decks = deck;
    }
    if ((back.size() && (back != options.co.back)) || !cardFaces.hasBack()) {
        option |= 2;
        options.co.back = back;
    }

    changeCards((void*)option);
}

//-----------------------------------------------------------------------------
/// Resizes the cards
/// \pre CardImages::HEIGHT/WIDTH must already contain the new sizes
//-----------------------------------------------------------------------------
void CardgameCollection::resizeCards() {
    cardFaces.resizeAll();
    cards.update();

    if (game)
        game->resizeCards();
}

//-----------------------------------------------------------------------------
/// Loads the cards
/// \param opt Actually a bit field! Option indicationg what to load
/// \returns void* Status; Not NULL when loading was OK, NULL otherwise
//-----------------------------------------------------------------------------
void* CardgameCollection::changeCards(void* opt) {
    TRACE2("CardgameCollection::changeCards(void*) - Option: " << opt);
    if (!opt)
        return this;

    // Cards need an realized (!) parent, so ensure that the window is already
    // shown
    Check3(this->is_realized());
    TRACE3("CardgameCollection::changeCards(void*) - Use " << options.co.decks << " and " << options.co.back);

    void* rc(NULL);
    bool enable(false);
    try {
        if ((unsigned long)opt & 1)
            cardFaces.loadDecks(options.co.decks);
        if ((unsigned long)opt & 2)
            cardFaces.loadBack(options.co.back);
        cards.getCards().size() ? cards.update() : cards.addPacket(cardFaces);

        enable = true;
        rc = this;
    }
    catch (std::exception& e) {
        Glib::ustring msg(_("Couldn't load the card images!\n\nReason: %1!"));
        msg.replace(msg.find("%1"), 2, e.what());
        showMessage(msg);
    }

    Check3(apMenus[NEW]);
    apMenus[NEW]->set_enabled(enable);

#ifdef WITH_NETWORK
    Check3(apMenus[CONNECT]);
    apMenus[CONNECT]->set_enabled(enable);
#endif
    return rc;
}

//----------------------------------------------------------------------------
/// Shows an error from the communication thread
/// \param msg Received message to handle
/// \returns bool False
/// \remarks msg wil be deleted at the end
//----------------------------------------------------------------------------
bool CardgameCollection::showMessage(const std::string msg) {
    Gtk::MessageDialog* dlg(new Gtk::MessageDialog(msg, false, Gtk::MessageType::ERROR));
    dlg->set_title(PACKAGE);
    dlg->signal_response().connect(bind(ptr_fun(&CardgameCollection::closeDialog), dlg));
    dlg->show();
    return false;
}

//-----------------------------------------------------------------------------
/// Frees the passed dialog
/// \param int Response of dialog (ignored)
/// \param dlg Dialog to close additionally
//-----------------------------------------------------------------------------
void CardgameCollection::closeDialog(int, const Gtk::Dialog* dlg) {
    Check1(dlg);
    delete dlg;
}

//-----------------------------------------------------------------------------
/// Checks the user-input after asking if he wants to end the game; depending
/// on the answer either stops or continues
/// \returns bool Flag, if the game has already been started
//-----------------------------------------------------------------------------
bool CardgameCollection::restartGame() {
    TRACE8("CardgameCollection::restartGame() - Restart: " << restart);
    Check1(game);

    if (game->isRunning()) {
        status.pop();
        status.push(_("User canceled"));

        if (game->canBeStopped()) {
            TRACE9("CardgameCollection::restartGame() - Game can be stopped");
            game->stop();
            if (restart)
                startGame();
        }
        else {
            TRACE9("CardgameCollection::restartGame() - Delaying stop of game");
            game->end((actGame == oldGame) ? restart : false);
            return false;
        }
    }
    else {
        restart = false;
        startGame();
    }
    return true;
}

//-----------------------------------------------------------------------------
/// Loads the cards (from png-files)
/// \remarks Cards need an realized (!) parent, so make somehow sure, that the
///      window already exists
//-----------------------------------------------------------------------------
void CardgameCollection::loadCards() {
    Check3(is_realized());
    status.push(_("Loading cardimages ..."));

    // This code needs the game-IDs in a sequence starting with 0!
    if (options.type >= GameTypes::LAST)
        options.type = 0;
    Check3(actChgGame);
    actChgGame->change_state(Glib::Variant<int>::create((int)options.type));

    void* rc(changeCards((void*)-1));
    if (rc) {
        Check3(apMenus[NEW]);
        apMenus[NEW]->set_enabled(true);

#ifdef WITH_NETWORK
        Check3(apMenus[CONNECT]);
        apMenus[CONNECT]->set_enabled(true);
#endif

        status.pop();
        if (cmgr.getMode() != YGP::ConnectionMgr::CLIENT) {
            status.push(_("Start a new game with Ctrl+N (or Game -> New)"));
        }
    }

    Check3(cardFaces.size());
}

//-----------------------------------------------------------------------------
/// Handling of game-events
/// \param status New status of game
//-----------------------------------------------------------------------------
void CardgameCollection::gameEvents(unsigned int status) {
    TRACE8("CardgameCollection::gameEvents(unsigned int) const - New status: " << status << "; Restart: " << restart);

    switch (status) {
    case Card::Game::PLAYING:
        Check3(apMenus[END]);
        apMenus[END]->set_enabled(true);
#ifdef WITH_NETWORK
        Check3(apMenus[CONNECT]);
        apMenus[CONNECT]->set_enabled(false);
#endif
        break;

    case Card::Game::STOPPED:
        Check3(apMenus[END]);
        apMenus[END]->set_enabled(false);
#ifdef WITH_NETWORK
        Check3(apMenus[CONNECT]);
        apMenus[CONNECT]->set_enabled(true);
#endif

        if (restart == 1) {
            // (Re)start the (new) game, when the event queue is empty (and
            // therefore the old game has ended).
            Glib::signal_idle().connect(bind_return(mem_fun(*this, &CardgameCollection::doStartGame), false));
        }
        else if (restart == -1U)
            Glib::signal_idle().connect(bind_return(mem_fun(*this, &CardgameCollection::hide), false));

        restart = false;
        break;
    }
}

//----------------------------------------------------------------------------
/// Starts the game and unlocks a (locked) msg-handling mutex
//----------------------------------------------------------------------------
void CardgameCollection::doStartGame() {
    startGame();
#ifdef WITH_NETWORK
    mxThreadCmd.unlock();
#endif
}

#ifdef WITH_ROVHULT
//-----------------------------------------------------------------------------
/// Checks if R�vhult's special cards are valid; reset them if not
//-----------------------------------------------------------------------------
void CardgameCollection::checkRovhultSpecialCards() {
    Card::Widget::NUMBERS* cards[] = {&Rovhult::cardNuke, &Rovhult::cardReverse, &Rovhult::cardSkip};

    for (unsigned int i(0); i < (sizeof(cards) / sizeof(*cards) - 1); ++i)
        for (unsigned int j(i + 1); j < (sizeof(cards) / sizeof(*cards)); ++j)
            if (*cards[i] == *cards[j]) {
                Rovhult::cardNuke = Card::Widget::TEN;
                Rovhult::cardReverse = Card::Widget::SEVEN;
                Rovhult::cardSkip = Card::Widget::EIGHT;

                Gtk::MessageDialog* dlg(new Gtk::MessageDialog(_("Invalid values for Rovhult's special cards!\n"
                                                                 "Resetting them to default values."),
                                                               false, Gtk::MessageType::ERROR));
                dlg->set_title(PACKAGE);
                dlg->signal_response().connect(bind(ptr_fun(&CardgameCollection::closeDialog), dlg));
                dlg->show();
                return;
            }
}
#endif
