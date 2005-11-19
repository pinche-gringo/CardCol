//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Application
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 9.9.2002
//COPYRIGHT   : Copyright (C) 2002 - 2005

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

#include <cerrno>
#include <cstdlib>

#include <string>
#include <fstream>

#include <gtkmm/stock.h>
#include <gtkmm/radioaction.h>
#include <gtkmm/messagedialog.h>

#include <YGP/File.h>
#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/INIFile.h>
#include <YGP/ANumeric.h>
#include <YGP/Tokenize.h>
#include <YGP/PathSrch.h>

#include <XGP/XAbout.h>

#include <Human.h>
#include <ComputerPlayer.h>

#include <PlayerDlg.h>
#include <DeckSelect.h>


#include "Hearts.h"
#include "Buraco.h"
#include "Rovhult.h"
#include "Twopart.h"
#include "Settings.h"
#include "SgtMayor.h"
#include "Machiavelli.h"

#include "Options.h"
#include "Options.meta"

#include "CardColAppl.h"

#include "CardCol.h"


const unsigned int CardgameCollection::WIDTH (760);
const unsigned int CardgameCollection::HEIGHT (750);


/// Writes the contents of the passed values to the passed stream
/// (in its own section named \c section).
/// \param stream: Stream to write to
/// \param section: Name of section to write
/// \param values: Values to write
template <>
void YGP::INIList<Glib::ustring>::write (std::ostream& stream, const char* section, const std::vector<Glib::ustring>& values) {
   writeHeader (stream, section);
   for (unsigned int i (0); i < values.size (); ++i)
     stream << i << '=' << values[i].c_str () << '\n';
   stream << '\n';
 }


// Pixmap for program
const char* CardgameCollection::xpmGame[] = {
   /* width height ncolors chars_per_pixel */
   "48 48 239 2",
   /* colors */
   "   c #000000",
   " . c #F6F6F8",
   " X c #CACADA",
   " o c #F2F2F4",
   " O c #DAD9E3",
   " + c #D9D7E2",
   " @ c #D8D7E1",
   " # c #D7D5E0",
   " $ c #D4D3DD",
   " % c #D0CFD9",
   " & c #F8F7F3",
   " * c #F5F5F0",
   " = c #EFEFEA",
   " - c #EDEDE8",
   " ; c #DCDBD4",
   " : c #E8E8ED",
   " > c #EBEBE6",
   " , c #FCFBFA",
   " < c #FBFBF9",
   " 1 c #DAD9D2",
   " 2 c #E7E6EC",
   " 3 c #D2D1DE",
   " 4 c #FAF9F8",
   " 5 c #F9F9F7",
   " 6 c #E5E4EA",
   " 7 c #D1CFDD",
   " 8 c #D0CFDC",
   " 9 c #F8F7F6",
   " 0 c #F7F7F5",
   " q c #C6C6C8",
   " w c #7F7F7F",
   " e c #F9F9FA",
   " r c #F7F7F8",
   " t c #CCCBDB",
   " y c #CBCBDA",
   " u c #F3F3F4",
   " i c #DDDCE5",
   " p c #DBDAE3",
   " a c #DAD8E2",
   " s c #F1F0E8",
   " d c #D9D8E1",
   " f c #EFEEE6",
   " g c #D7D6DF",
   " h c #F7F6F1",
   " j c #F5F4EF",
   " k c #F3F2ED",
   " l c #F1F0EB",
   " z c #ECEBF0",
   " x c #EBEBEF",
   " c c #EAE9EE",
   " v c #D5D4E0",
   " b c #ECECE6",
   " n c #DBDAD2",
   " m c #D4D2DF",
   " M c #D3D2DE",
   " N c #FBFAF8",
   " B c #FAFAF7",
   " V c #D9D8D0",
   " C c #D2D0DD",
   " Z c #E9E8E3",
   " A c #F9F8F6",
   " S c #F8F8F5",
   " D c #E7E6E1",
   " F c #F7F6F4",
   " G c #F6F6F3",
   " H c #D5D4CC",
   " J c #E5E4DF",
   " K c #F5F4F2",
   " L c #D3D2CA",
   " P c #F3F2F0",
   " I c #F2F2EF",
   " U c #D1D0C8",
   " Y c #CFCEC6",
   " T c #C7C6D2",
   " R c #FEFEFE",
   " E c #FCFCFC",
   " W c #E2E1E9",
   " Q c #CDCCDB",
   " ! c #E0DFE7",
   " ~ c #DEDDE5",
   " ^ c #DDDBE4",
   " / c #3F3F3F",
   " ( c #DCDBE3",
   " ) c #DAD9E1",
   " _ c #EDECED",
   " ` c #F1F0F4",
   " ' c #F0F0F3",
   " ] c #EFEEF2",
   " [ c #F2F1EB",
   " { c #D8D7E2",
   " } c #D6D5E0",
   " | c #FE7F87",
   ".  c #D5D3DF",
   ".. c #D4D3DE",
   ".X c #D2D1DC",
   ".o c #FAF9F6",
   ".O c #F8F7F4",
   ".+ c #F7F7F3",
   ".@ c #CECDD8",
   ".# c #F6F5F2",
   ".$ c #F5F5F1",
   ".% c #CCCBD6",
   ".& c #F4F3F0",
   ".* c #F3F3EF",
   ".= c #D2D1C8",
   ".- c #F1F1ED",
   ".; c #E0DFD9",
   ".: c #F0EFEC",
   ".> c #DDDDD6",
   "., c #FEFDFD",
   ".< c #FDFDFC",
   ".1 c #FBFBFA",
   ".2 c #F9F9F8",
   ".3 c #E5E4EB",
   ".4 c #E3E2E9",
   ".5 c #CECDDB",
   ".6 c #E1E0E7",
   ".7 c #FE0010",
   ".8 c #F4F3F6",
   ".9 c #C9C9D9",
   ".0 c #FE3F4B",
   ".q c #DBDAE4",
   ".w c #D9D8E2",
   ".e c #D8D6E1",
   ".r c #D7D6E0",
   ".t c #D6D4DF",
   ".y c #D5D4DE",
   ".u c #D2D0DB",
   ".i c #CFCED8",
   ".p c #F7F6F2",
   ".a c #F6F6F1",
   ".s c #000010",
   ".d c #EDECE8",
   ".f c #FDFCFB",
   ".g c #FCFCFA",
   ".h c #E8E7ED",
   ".j c #EBEAE6",
   ".k c #D3D2DF",
   ".l c #FBFAF9",
   ".z c #FAFAF8",
   ".x c #D9D8D1",
   ".c c #E6E5EB",
   ".v c #D1D0DD",
   ".b c #F9F8F7",
   ".n c #F8F8F6",
   ".m c #D7D6CF",
   ".M c #E4E3E9",
   ".N c #D6D6CE",
   ".B c #F6F6F4",
   ".V c #C4C4D0",
   ".C c #FCFCFD",
   ".Z c #FAFAFB",
   ".A c #F8F8F9",
   ".S c #CCCCDB",
   ".D c #CBCADA",
   ".F c #CACAD9",
   ".G c #DEDDE6",
   ".H c #F5F3EC",
   ".J c #DCDBE4",
   ".K c #DBD9E3",
   ".L c #DAD9E2",
   ".P c #D8D7E0",
   ".I c #D6D5DE",
   ".U c #F6F5F0",
   ".Y c #F5F5EF",
   ".T c #F3F3ED",
   ".R c #EEEEF2",
   ".E c #D4D3DF",
   ".W c #E7E6EB",
   ".Q c #D3D1DE",
   ".! c #EAE9E4",
   ".~ c #D2D1DD",
   ".^ c #FAF9F7",
   "./ c #F9F9F6",
   ".( c #D8D7CF",
   ".) c #D1CFDC",
   "._ c #E8E7E2",
   ".` c #F8F7F5",
   ".' c #F7F7F4",
   ".] c #E6E5E0",
   ".[ c #E5E5DF",
   ".{ c #F5F5F2",
   ".} c #D4D3CB",
   ".| c #545454",
   "X  c #E4E3DE",
   "X. c #E3E3DD",
   "XX c #F4F3F1",
   "Xo c #D2D1C9",
   "XO c #CAC9D5",
   "X+ c #E1E1DB",
   "X@ c #C9C9D4",
   "X# c #F1F1EE",
   "X$ c #D0CFC7",
   "X% c #C8C7D3",
   "X& c #CECDC5",
   "X* c #FDFDFD",
   "X= c #CECDDC",
   "X- c #CDCDDB",
   "X; c #E1E0E8",
   "X: c #CCCBDA",
   "X> c #DFDEE6",
   "X, c #DEDCE5",
   "X< c #DDDCE4",
   "X1 c #DBDAE2",
   "X2 c #DAD8E1",
   "X3 c #EFEEE5",
   "X4 c #F1F1F4",
   "X5 c #EFEFF2",
   "X6 c #F2F2EB",
   "X7 c #D7D6E1",
   "X8 c #D5D4DF",
   "X9 c #D4D2DE",
   "X0 c #F9F8F5",
   "Xq c #F8F8F4",
   "Xw c #F7F6F3",
   "Xe c #F6F6F2",
   "Xr c #CDCCD7",
   "Xt c #F4F4F0",
   "Xy c #D3D2C9",
   "Xu c #E3E2DC",
   "Xi c #F2F2EE",
   "Xp c #D1D0C7",
   "Xa c #DFDED8",
   "Xs c #DEDED7",
   "Xd c #EEEEEA",
   "Xf c #FEFEFD",
   "Xg c #FCFCFB",
   "Xh c #FBFAFA",
   "Xj c #FAFAF9",
   "Xk c #E6E5EC",
   "Xl c #D1D0DE",
   "Xz c #E4E3EA",
   "Xx c #E3E3E9",
   "Xc c #CFCEDC",
   "Xv c #E2E1E8",
   "Xb c #CDCCDA",
   "Xn c #E1DFE7",
   "Xm c #DEDDE4",
   "XM c None",
   /* pixels */
   ".|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.| q R R R R R R R q q.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.|",
   ".|                                     E E R _Xn !X> ~ i _ _ R R q.|                           R",
   ".|   R R R R R R R R R R R R R R R R R R.4X; ! !X>X>X, i.J p.K R R R R R R R R R R R R R R R R R",
   ".|   R R R R R R R R R R R R R R R R RXz.4X; ! !X>.G i.J.J p O a.w @ R R R R R R R R R R R R R R",
   ".|   R R R R R R R R R R R R R R R 2.3Xz WX; ! !X> ~ |.J p O a.w @.eX7 # } R R R R R R R R R R R",
   ".|   R R R R R R R R R R R R R R.h 2.3Xz WX; ! !X>X, ^.7 | O.w + @X7 # } }X8X8 R R R R R R R R R",
   ".|   R R R R R R R R R R R R x c : 2.3XzXv.6X>X>.G |.7.7.7 | | |X7 # } }X8. .E.EX9 R R R R R R R",
   ".|   R R R R R R R R R R R.R z c.h 2.3.4XvXnX>X>.0.7.7.0 O.w @X7 # } vX8.E.EX9 M M.Q.Q m R R R R",
   ".|   R R R R R R R R R ` `.R z c.h 2.3.4Xv ! |.7.7.7.q O.w {.eX7 } v.0 | m M M.Q 3 3.~.~.~ M R R",
   ".|   R R R R R R R R . o `.R z c.h 2 6.4X;.0.7.7.0 |.7.7.0 | |.7.7.7.7.7.7 | 3 3 C.v.v.v.v 3 3 R",
   ".|   R R R R R R R.A . oX4.R z c 2 | | |.7.7.7X, ^.q.7.7 |X7 } }X8.E.k M 3 3 |.v.v.v.v 7 7 7 C R",
   ".|   R R R R RX* E.A . o `.R x c 2.cXz |.7.0X, i.J p |.7.7 # }X8.E.k M 3 3.v.v 8 8 8 8 8 8 8 8 R",
   ".|.s R R R R R.C.Z.A . o '.R x.h.c.3.4X; ! | | ^.q O.w.7.7 } v.E m M 3Xl 8XcXcXcX=.5.5XcXcXcXc R",
   ".|   R R r r.C E e ..8 ` ] z c 2XkXz W | | | |.J.K a +.7.7 |. .E M 3Xl 8XcXcX=X=.5.5.5.5XcXcXc R",
   ".|   R e.Z.Z.Z e r.8 o ' |.7.7.7.7.7.7.7.7.7.7 | O | |.7.7.7.E M 3Xl 8XcXcX=X- Q Q Q Q Q QX=Xc R",
   ".|  .8 . r r r ..8 o '.R z.7.7.7.7.7.7.7.7.7.7.K a @X7 }.0.7X9.Q 3.vXcX=X= QX: y y y y.S Q Q.5 R",
   ".|   o u.8.8.8 oX4X5.R z c |.7.7.7.7.7.7.7.7.7 O.w @.r }X8.E | 3.vXcXcX= QX: y.D.D.D.D.D.S.S.5 R",
   ".|   ] ' ' ' 'X5.R.R x c 2 |.7.7.7.7.7.7.7.7.7 | @X7 }X8.EX9.Q C.vXc.5X-X: y.D.9.9.9 X.D tX: Q R",
   ".|   z z z z z z x c : 2 6 |.7.7.7.7.7.7.7.7.7.7.e # }. .. M.~.v 8.5.5 Q y.D.9.9.9.9.9 X XX: Q R",
   ".|   c c c c c c.h 2.cXz.4 |.7.7.7.7 | | | | | |.r }X8.EX9.Q C.v 8XcX- Q.D X.9.9.9.9.9 X X y Q R",
   ".|  .W 2 2 2.W.c 6.M.4Xv !X>X> ~X< ( p.L d.w @.r #.t. X9 M.~.v.) 8Xc Q.S.D X.9.9.9.9.9 X.F yXb R",
   ".|  .M.M.MXx.4.4Xv.6 !X> ~X<X<.J p.L p.L d.w @.r.t.t. X9 M.~.v.) 8Xc Q.S.D X.9.9.9.9.9 X.F yXb R",
   ".|  .6.6.6 ! ! !X> ~X<X< ( p p R R R R R R R R R R R R R R R R R R R R R R R R R R R R R R R R R",
   ".|   ~ ~ ~XmX<X< ( (X1.L ) R R R RXwXw.$Xe.#.$XtXt.*.* k l.: = - b.! Z.]X.X+.;.> ; n 1 V V.x 1 R",
   ".|   ( ( pX1.L.LX2 d.P.P R R.+.+.pXwXwXe.#.#.#.$.$Xt.&Xi.- l = - b.!._.]X.X+Xs ; 1.(.m.N.N.m.( R",
   ".|   ) d d.P.P g g # } R RXw.+.+ &.O.'.+Xw G.#Xe.$.$.&.*Xi.-.:Xd.d.!._ JXu.;.> 1.(.N H.}.} H.N R",
   ".|   g g }.I.y.y $ $ R R.+.pXw.+.'.'.'.'Xw.'Xw G G.$.$.*Xi.- = / w.!._.[X+Xa ;.x.N.}Xy.=Xo L H R",
   ".|  .y.y $.X.X.u.u.u R.p.+ &.OXq S S S S.`.' F.' G G.$Xt.*X#     w.j._.[X+Xs n.m.}.=X$X$Xp.= L R",
   ".|  .X.X.u.u % %.i R R.+.OXqX0.O S S S S A S.`.'.' G G.{ / w     w > Z JX+.> 1.N LX$ YX& YXpXo R",
   ".|  .u %.i.i.@Xr R R.+.OX0.O S A A./././ A.n A S.'.' G w l.d     w b Z.]XuXs 1.N LXp Y YX&X$Xo R",
   ".|  .i.@Xr.%.%XO R.+.O.O SX0./ 5.^ B B B.^ 5./.n.n w w.H.+.& /   w b.! DX.Xa n.(.}XpX$ YX$X$ L R",
   ".|  .%.%XOX@ T R R & & SX0././ 5.^.z.z.z 4 5.z 5 / / Z fXq S w   w - >._ JX+.>.x HXo UXpXpXo.} R",
   ".|  XOX@X% T R R.+.+XqX0.o./.^.z N.l < <.lXj w / 4 w w w w w /   /Xd b.!.]XuXa ; V.N.} L.} H.N R",
   ".|  X@X% T.V R.p &.OX0././ B.z <Xj.l < <.l / <Xj 4.z.b.n.' G w    .:Xd.j Z JX+Xa ;.x.m.N.m.(.x R",
   ".|  X% T.V R R.p.+Xq././ B N.z <.1.1 w w   /.1.1.l.z.z.b.n 0 /    .- = -.j._ JXu.;Xs ; ; ; ;.> R",
   ".|   T T.V R.a.a &X0X0.^.z.z < ,XgXg.f.< EXgXg.1.l w.z 4 w w w w w w.- =.d.j Z.]X X+.;.;Xa.;.; R",
   ".|   T.V R.a.a.p.O.O./ B.z < ,Xg.<Xg E E EXg w         w.2 5 0.B.{.& P.-Xd.d > Z._.] JX X.X X  R",
   ".|   T R R.Y h.pXq S./.z.z.1XgXg E.<.< w                 / 5.n.` G K P P.- = -.d.j.! Z._._._._ R",
   ".|  X% R.T j.U.pXq S.^.^ <.g.fXg.< /                       /.b.n 0.B K P I.-.:Xd -.d b.j >.j.j R",
   ".|   R R.T j *.p.+ S.^.z <XgXg w                             w.n 9 0.B KXt P IX#.:.:XdXdXd -Xd R",
   ".|   R [.T j *.p.+ S.^.z <Xg w                               w 5 5.` 0 G.{.{XX P I IX#X#.-.:.: R",
   ".|   s [ k.T j.p.+ S.^.z <Xg                                Xj.z 5 5.` 0.B.B.{ K KXt PXX P I P R",
   ".|   s sX6.T j.a.pXq.^ B <Xg             /   w            .1.1.z.z 5 5.n 0.` 0.B.B.B.B.{.{.{ K R",
   ".|   f s [ k j.UXe.O./.^ <.gXg w w w R R /  XfXf w w wXgXg.1.1Xj 4.z 5.n 5.n.n.n 9 0 0 9 0.B F R",
   ".|  X3 f [ [.T *.$.+X0./.z.1.1Xg.<X*XfXf       R R RX*.<.<Xg.1.1Xj 4.z.2 5 5 5 5 5 5 5.b 5.n 0 R",
   ".|  X3 f s l k j *.p S A B <.g.gXg.<X*X*X* wXf R R R.,.<.<Xg.1.1 <Xj.z.2.2.2 4XjXjXjXjXjXj 4.b R",
   ".|   bX3 f sX6.* j.a.O S./.z <.1XgXg E.<.<X*., R R RX*.<.<Xg.1 <.1 <XjXjXj.l.1.1.1.1.1.1.1Xh 4 R",
   " R R R R R R R R R R R R R R R R R R R R R R R R R R R R R R R R R R R R R R R R R R R R R R R R" };

// Pixmap for author
const char* CardgameCollection::xpmAuthor[] = {
   "56 43 5 1",
   " 	c None",
   "!   c #0000FF",
   "@	c #000000",
   "-	c #AEAAAE",
   ".	c #FFFFFF",
   ".......................................................@",
   "..-----------------------------------------------------@",
   "..-----------------------------------------------------@",
   "..----------------------!!!!!!-------------------------@",
   "..--------------------!!!!@@!!!!-----------------------@",
   "..------------------!!!!@@---@@!!!---------------------@",
   "..-----------------!!!@@--------!!!--------------------@",
   "..-----------------!!!@---!!----!!!--------------------@",
   "..----------------!!!@---!!!!@---!!!-------------------@",
   "..----------------!!@---!!!!!!@---!!@------------------@",
   "..----------------!!@----!!!!@----!!@------------------@",
   "..----------------!!!-----!!@----!!!@------------------@",
   "..-----------------!!!-----@-----!!!@------------------@",
   "..-----------------!!!!---------!!!@-------------------@",
   "..------------------!!!!-------!!!@--------------------@",
   "..--------------------!!!!--!!!!@@---------------------@",
   "..----------------------!!!!!!@@-----------------------@",
   "..-----------------------!!!!@-------------------------@",
   "..------------------------!!@--------------------------@",
   "..------------------------!!@--------------------------@",
   "..------------------!!!!!!!!!!!!!!---------------------@",
   "..-----------------!!!!!!!!!!!!!!!!--------------------@",
   "..----------------!!!@@@@@!!@@@@@!!!-------------------@",
   "..----------------!!@-----!!@-----!!@------------------@",
   "..----------------!!@-----!!@-----!!@------------------@",
   "..----------------!!@-----!!@-----!!@------------------@",
   "..----------------!!@-----!!@-----!!@------------------@",
   "..-----------------@@-----!!@------@@------------------@",
   "..------------------------!!@--------------------------@",
   "..--------------------!!!!!!!!!!-----------------------@",
   "..------------------!!!!!!!!!!!!!!---------------------@",
   "..-----------------!!!@@@@@@@@@!!!!--------------------@",
   "..-----------------!!@----------!!!!-------------------@",
   "..----------------!!!@-----------!!!@------------------@",
   "..----------------!!@------------!!!@------------------@",
   "..----------------!!@-------------!!@------------------@",
   "..----------------!!@-------------!!@------------------@",
   "..----------------!@--------------!!@------------------@",
   "..----------------!!@--------------@@------------------@",
   "..-----------------!@----------------------------------@",
   "..------------------@----------------------------------@",
   "..-----------------------------------------------------@",
   "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" };


/* XPM for the Joker; borrowed (and simplified) from the Warwick cardset of the
   KDE cardgames */
static char * xpmJoker[] = {
"72 96 65 1",
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
/// \param opts: Options for the program
//-----------------------------------------------------------------------------
CardgameCollection::CardgameCollection (Options& opts)
   : XApplication (PACKAGE " V" PRG_RELEASE)
#ifdef HAVE_LIBPTHREAD
     , playerPos (0) , dlgChat (NULL)
#endif
     , options (opts)
     , oldGame (GameTypes::NONE), actGame (opts.type)
     , restart (false), game (NULL) {
   TRACE9 ("CardGameCollection::CardGameCollection (Options&)");

   setIconProgram (xpmGame);
   set_default_size (WIDTH, HEIGHT);

   helpBrowser = options.browser;

   // Create controls
   Glib::ustring ui ("<ui><menubar name='Menu'>"
		     "  <menu action='Game'>"
		     "    <menuitem action='New'/>"
		     "    <menuitem action='End'/>"
#ifdef HAVE_LIBPTHREAD
		     "    <separator/>"
		     "    <menuitem action='Connect'/>"
		     "    <menuitem action='Chat'/>"
#endif
		     "    <separator/>"
		     "    <menuitem action='Quit'/>"
		     "  </menu>"
		     "  <menu action='Options'>"
		     "    <menu action='ChgGame'>"
		     "      <menuitem action='Rovhult'/>"
		     "      <menuitem action='Twopart'/>"
		     "      <menuitem action='Hearts'/>"
		     "      <menuitem action='Buraco'/>"
		     "      <menuitem action='SgtMayor'/>"
		     "      <menuitem action='Machiavelli'/>"
		     "    </menu>"
		     "    <menuitem action='ChgDecks'/>"
		     "    <menuitem action='ChgNames'/>"
		     "    <menuitem action='Prefs'/>"
		     "    <separator/>"
		     "    <menuitem action='SavePrefs'/>"
#if TRACELEVEL >= 1
		     "    <separator/>"
		     "    <menuitem action='Debug'/>"
#endif
		     "  </menu>"
		     "  <placeholder name='GameMenu'/>");

   grpAction->add (Gtk::Action::create ("Game", _("_Game")));
   grpAction->add (apMenus[NEW] = Gtk::Action::create ("New", Gtk::Stock::NEW),
		   mem_fun (*this, &CardgameCollection::newGame));
   grpAction->add (apMenus[END] = Gtk::Action::create ("End", Gtk::Stock::CLOSE, _("_End")),
		   mem_fun (*this, &CardgameCollection::endGame));
#ifdef HAVE_LIBPTHREAD
   grpAction->add (apMenus[CONNECT] = Gtk::Action::create ("Connect", _("_Connect ...")),
		   Gtk::AccelKey (_("<shft><ctl>C")),
		   mem_fun (*this, &CardgameCollection::connect));
   apMenus[CONNECT]->set_sensitive (false);
   grpAction->add (apMenus[CHAT] = Gtk::Action::create ("Chat", _("_Chat ...")),
		   Gtk::AccelKey (_("<alt><ctl>C")),
		   mem_fun (*this, &CardgameCollection::showChatDlg));
   apMenus[CHAT]->set_sensitive (false);
#endif
   grpAction->add (Gtk::Action::create ("Quit", Gtk::Stock::QUIT),
		   mem_fun (*this, &CardgameCollection::exit));

   grpAction->add (Gtk::Action::create ("Options", _("_Options")));
   grpAction->add (Gtk::Action::create ("ChgGame", _("_Change game")));

   Gtk::RadioButtonGroup grpGames;
   // xgettext: For translations: Write the Rovhult as o-slash
   grpAction->add (apMenus[ROVHULT] = Gtk::RadioAction::create (grpGames, "Rovhult", _("_Rovhult")),
		   Gtk::AccelKey (_("<ctl>R")),
		   bind (mem_fun (*this, &CardgameCollection::changeGame), (int)GameTypes::ROVHULT));
   grpAction->add (apMenus[TWOPART] = Gtk::RadioAction::create (grpGames, "Twopart", _("_Twopart")),
		   Gtk::AccelKey (_("<ctl>T")),
		   bind (mem_fun (*this, &CardgameCollection::changeGame), (int)GameTypes::TWOPART));
   grpAction->add (apMenus[HEARTS] = Gtk::RadioAction::create (grpGames, "Hearts", _("_Hearts")),
		   Gtk::AccelKey (_("<ctl>H")),
		   bind (mem_fun (*this, &CardgameCollection::changeGame), (int)GameTypes::HEARTS));
   grpAction->add (apMenus[BURACO] = Gtk::RadioAction::create (grpGames, "Buraco", _("_Buraco")),
		   Gtk::AccelKey (_("<ctl>B")),
		   bind (mem_fun (*this, &CardgameCollection::changeGame), (int)GameTypes::BURACO));
   grpAction->add (apMenus[SGTMAYOR] = Gtk::RadioAction::create (grpGames, "SgtMayor", _("_Sgt. Mayor")),
		   Gtk::AccelKey (_("<ctl>Y")),
		   bind (mem_fun (*this, &CardgameCollection::changeGame), (int)GameTypes::SGTMAYOR));
   grpAction->add (apMenus[MACHIAVELLI] = Gtk::RadioAction::create (grpGames, "Machiavelli", _("_Machiavelli")),
		   Gtk::AccelKey (_("<ctl>M")),
		   bind (mem_fun (*this, &CardgameCollection::changeGame), (int)GameTypes::MACHIAVELLI));

   grpAction->add (Gtk::Action::create ("ChgDecks", _("Change _decks ...")),
		   Gtk::AccelKey (_("<ctl>D")),
		   mem_fun (*this, &CardgameCollection::showChangeDeckDlg));
   grpAction->add (Gtk::Action::create ("ChgNames", _("Change _names ...")),
		   Gtk::AccelKey (_("<ctl>C")),
		   mem_fun (*this, &CardgameCollection::changeNames));
   grpAction->add (Gtk::Action::create ("Prefs", Gtk::Stock::PREFERENCES),
		   Gtk::AccelKey (_("F9")),
		   mem_fun (*this, &CardgameCollection::editPreferences));
   grpAction->add (Gtk::Action::create ("SavePrefs", Gtk::Stock::SAVE,
					_("_Save preferences")),
		   mem_fun (*this, &CardgameCollection::savePreferences));
#if TRACELEVEL >= 1
   grpAction->add (Gtk::Action::create ("Debug", "_Debug"),
		   Gtk::AccelKey ("<ctl>G"),
		   mem_fun (*this, &CardgameCollection::toggleDebug));
#endif

   addHelpMenu (ui);
   ui += "</menubar></ui>";
   mgrUI->insert_action_group (grpAction);
   add_accel_group (mgrUI->get_accel_group ());
   mgrUI->add_ui_from_string (ui);

   getClient ().pack_start (*mgrUI->get_widget("/Menu"), Gtk::PACK_SHRINK);

   ((Gtk::MenuItem*)(mgrUI->get_widget("/Menu/Help")))->set_right_justified ();

   Check3 (apMenus[NEW]); Check3 (apMenus[END]);
   apMenus[NEW]->set_sensitive (false);
   apMenus[END]->set_sensitive (false);

   status.show ();
   getClient ().pack_end (status, Gtk::PACK_SHRINK);

   show ();
   mxGuiCmd.lock ();

   Glib::signal_idle ().connect
       (bind_return (mem_fun (*this, &CardgameCollection::loadCards), false));
   makePlayer ();

#ifdef HAVE_LIBPTHREAD
   autoConnect (options);
#endif
}

//----------------------------------------------------------------------------
/// Creates the default player in the game; basing on the names read from the
/// INI file
//----------------------------------------------------------------------------
void CardgameCollection::makePlayer () {
   Check3 (options.names.size ());
   std::vector<Glib::ustring>::iterator i (options.names.begin ());
   aPlayer.push_back (new Human (*i));
   while (++i != options.names.end ())
       aPlayer.push_back (new ComputerPlayer (*i));
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
CardgameCollection::~CardgameCollection () {
   TRACE9 ("CardgameCollection::~CardgameCollection ()");
   if (game) {
      game->clean ();
      delete game;
   }

   for (std::vector<Player*>::iterator i (aPlayer.begin ());
        i != aPlayer.end (); ++i)
      delete *i;

#ifdef HAVE_LIBPTHREAD
   removeCommThreads ();
#endif
}

//-----------------------------------------------------------------------------
/// Starts a game; if the type has changed also deleting the old one
//-----------------------------------------------------------------------------
void CardgameCollection::startGame () {
   TRACE6 ("CardgameCollection::startGame () - Old game type " << oldGame
           << " -> New: " << actGame);

   // Check if the game has been changed; if so destroy the old one
   unsigned int oldDecks (0), oldJoker (0);

   if (game) {
      oldDecks = game->numberOfDecks ();
      oldJoker = game->numberOfJokers ();

      if (oldGame != actGame) {
	 // Remove menubar and update GUI to not interfere with new game
	 game->removeMenus (mgrUI);
	 Glib::RefPtr<Glib::MainContext> ctx (Glib::MainContext::get_default ());
	 while (ctx->iteration (false)) ;

         getClient ().remove (*game);
         delete game;
      }
   }

   if (oldGame != actGame) {
      oldGame = actGame;
      switch (oldGame) {
      case GameTypes::ROVHULT:
         game = new TGame<Rovhult, CardgameCollection>
            (*this, &CardgameCollection::gameEvents);
         break;

      case GameTypes::TWOPART:
         game = new TGame<Twopart, CardgameCollection>
            (*this, &CardgameCollection::gameEvents);
         break;

      case GameTypes::HEARTS:
         game = new TGame<Hearts, CardgameCollection>
            (*this, &CardgameCollection::gameEvents);
         break;

      case GameTypes::BURACO:
         game = new TGame<Buraco, CardgameCollection>
            (*this, &CardgameCollection::gameEvents);
         break;

      case GameTypes::MACHIAVELLI:
         game = new TGame<Machiavelli, CardgameCollection>
            (*this, &CardgameCollection::gameEvents);
         break;

      case GameTypes::SGTMAYOR:
         game = new TGame<SgtMayor, CardgameCollection>
            (*this, &CardgameCollection::gameEvents);
         break;

      default:
         Check (0);
      }
      game->addMenus (mgrUI);
   }

   // Change number of jokers if necessary
   if (oldJoker != game->numberOfJokers ()) {
      TRACE5 ("CardgameCollection::startGame () - Re-adding jokers "
              << oldJoker << "->" << game->numberOfJokers ());
      for (unsigned int i (0); i < oldJoker; ++i)
         cardFaces.delImage (cardFaces.size () - 1);

      for (unsigned int i (0); i < game->numberOfJokers (); ++i)
         cardFaces.addImage (xpmJoker);
   }

   // Change number of decks if necessary
   if ((oldDecks != game->numberOfDecks ())
       || (oldJoker != game->numberOfJokers ())) {
      TRACE5 ("CardgameCollection::startGame () - Changing carddecks "
              << oldDecks << "->" << game->numberOfDecks ());

      cards.clear ();
      for (unsigned int i (0); i < game->numberOfDecks (); ++i)
         cards.addPacket (cardFaces);
   }

   Check3 (game);
   GameTypes types;
   Glib::ustring name (types[actGame]);
   name += " - " PACKAGE " V" PRG_RELEASE;
   set_title (name);

#ifdef SAVE_GAME
   if (options.gameFile.size () && options.load) {
      std::ifstream input (options.gameFile.c_str ());
      char buffer[1024];

      input.getline (buffer, sizeof (buffer));
      game->setCardOrder (buffer);
   }
#endif

   if (cmgr.getMode () != YGP::ConnectionMgr::CLIENT) {
      game->start ();

#ifdef SAVE_GAME
      if (options.gameFile.size () && !options.load) {
         std::ofstream output (options.gameFile.c_str ());
         output << game->getCardOrder ();
      }
#endif

      game->clearCardOrder ();
   }
   else
      game->setGameStatus (Game::NONE);
}

//-----------------------------------------------------------------------------
/// Starts a new game; running games are ended
//-----------------------------------------------------------------------------
void CardgameCollection::newGame () {
   TRACE7 ("CardgameCollection::newGame () - New; Game running: "
	   << (game && game->isRunning () ? "Yes" : "No"));
   if (game && game->isRunning ()) {
      Gtk::MessageDialog dlg (_("A game is already running. Do you really"
				" want to end it and start another?"), false,
			      Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO);
      dlg.set_title (PACKAGE);
      if (dlg.run () == Gtk::RESPONSE_YES) {
	 restart = true;
	 restartGame ();
      }
   }
   else {
      if (stopClientWaiting ())
	 startGame ();
   }
}

//-----------------------------------------------------------------------------
/// Ends a running game
//-----------------------------------------------------------------------------
void CardgameCollection::endGame () {
   Check3 (game && game->isRunning ());
   Gtk::MessageDialog dlg (_("Do you really want to end the game?"),
			   false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO);
   dlg.set_title (PACKAGE);
   if (dlg.run () == Gtk::RESPONSE_YES) {
      restart = false;
      restartGame ();
   }
}

//-----------------------------------------------------------------------------
/// Changes the type of the next game
/// \param game: Type of the next game
//-----------------------------------------------------------------------------
void CardgameCollection::changeGame (int game) {
   TRACE9 ("CardgameCollection::changeGame (games) - " << game);
   Check3 ((unsigned int)game < GameTypes::LAST);

   actGame = game;
}

//-----------------------------------------------------------------------------
/// Opens a dialog allowing to change the card decks
//-----------------------------------------------------------------------------
void CardgameCollection::showChangeDeckDlg () {
   CarddeckSelectDlg<CardgameCollection>::create
      (*this, &CardgameCollection::changeDecks, CARDDECKS_DIR, options.decks, options.back);
}

//-----------------------------------------------------------------------------
/// Opens a dialog allowing to change the names of the players
//-----------------------------------------------------------------------------
void CardgameCollection::changeNames () {
   PlayerDlg<CardgameCollection>::create (*this, &CardgameCollection::changePlayernames, aPlayer);
}

//-----------------------------------------------------------------------------
/// Edits the preferences
//-----------------------------------------------------------------------------
void CardgameCollection::editPreferences () {
   Settings::create (get_window (), options);
}

//-----------------------------------------------------------------------------
/// Saves the settings
//-----------------------------------------------------------------------------
void CardgameCollection::savePreferences () {
   TRACE2 ("CardgameCollection::savePreferences () - Save file " << options.pNameINIFile);
   std::ofstream inifile (options.pNameINIFile);
   if (inifile) {
      options.strType = options.type + '0';
      YGP::INIFile::write (inifile, "Game", options);
      for (unsigned int i (0); i < aPlayer.size (); ++i)
	 options.names[i] = aPlayer[i]->getName ();
      YGP::INIList<Glib::ustring>::write (inifile, "Player", options.names);

      YGP::INIFile::writeSectionHeader (inifile, "Buraco");
      inifile << "EndPoints=" << Buraco::ENDPOINTS << '\n';
   }
   else {
      Glib::ustring msg (_("Couldn't save options (to file %1)!\n\nReason: %2."));
      msg.replace (msg.find ("%1"), 2, options.pNameINIFile);
      msg.replace (msg.find ("%2"), 2, strerror (errno));
      Gtk::MessageDialog (msg, false, Gtk::MESSAGE_ERROR).run ();
   }
}

//-----------------------------------------------------------------------------
/// Saves the settings
//-----------------------------------------------------------------------------
void CardgameCollection::exit () {
   if (game) {
      if (game->isRunning ()) {
	 Gtk::MessageDialog dlg (_("A game is running. Do you really want to quit?"),
				 false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO);
	 dlg.set_title (PACKAGE);
	 if (dlg.run () == Gtk::RESPONSE_YES) {
	    if (game->canBeStopped ())
	       game->stop ();
	    else {
	       restart = -1U;
	       game->end (false);
	       return;
	    }
	 }
	 else
	    return;
      }
   }
   hide ();
}

#if TRACELEVEL >= 0
//-----------------------------------------------------------------------------
/// Toggles showing/hiding the cards
//-----------------------------------------------------------------------------
void CardgameCollection::toggleDebug () {
   static bool open = false;
   open = !open;
   if (game)
      game->playOpen (open);
}
#endif

//-----------------------------------------------------------------------------
/// Returns the name of the file to display in the help
/// \returns \c Name of file to display
//-----------------------------------------------------------------------------
const char* CardgameCollection::getHelpfile () {
   std::string file (options.helpPath);
   if (file[file.size () - 1] != YGP::File::DIRSEPARATOR)
      file += YGP::File::DIRSEPARATOR;

   if (game) {
      std::string name (game->name ());
      unsigned int pos;
      while ((pos = name.find (" ")) != std::string::npos)
         name.replace (pos, 1, 0, '\0');

      file += name;
      file += ".html";
   }
   else
      file += "CardCol.html";
   return file.c_str ();
}

//-----------------------------------------------------------------------------
/// Shows the about box for the program
//-----------------------------------------------------------------------------
void CardgameCollection::showAboutbox () {
   std::string ver (_("Copyright (C) 2002 - 2005 Markus Schwab"
                      "\ne-mail: g17m0@lycos.com\n\nCompiled on %1 at %2"));
   ver.replace (ver.find ("%1"), 2, __DATE__);
   ver.replace (ver.find ("%2"), 2, __TIME__);

   XGP::XAbout* about (XGP::XAbout::create (ver, PACKAGE " V" VERSION));
   about->setIconProgram (xpmGame);
   about->setIconAuthor (xpmAuthor);
   about->get_window ()->set_transient_for (get_window ());
}

//-----------------------------------------------------------------------------
/// Callback to change the names of the playing people
//-----------------------------------------------------------------------------
void CardgameCollection::changePlayernames () {
   TRACE2 ("CardgameCollection::changePlayernames");
#ifdef HAVE_LIBPTHREAD
   if (cmgr.getMode () != YGP::ConnectionMgr::NONE)
      broadcastNames ();
#endif

   if (game)
      game->changeNames (aPlayer);
}

//-----------------------------------------------------------------------------
/// Callback to change the carddecks
/// \param cmd: Selected button of dialog
//-----------------------------------------------------------------------------
void CardgameCollection::changeDecks (const ICarddeckSelectDlg& dialog) {
   TRACE2 ("CardgameCollection::changeDecks (const ICarddeckSelectDlg&)");

   std::string deck (options.decks), back (options.back);
   dialog.getSelection (deck, back);
   unsigned int option (0);
   if (deck.size () && (deck != options.decks)) {
      option = 1;
      options.decks = deck;
   }
   if (back.size () && (back != options.back)) {
      option |= 2;
      options.back = back;
   }

   changeCards ((void*)option);
}

//-----------------------------------------------------------------------------
/// Loads the cards (from xpm-files)
/// \param opt: Actually a bit field! Option indicationg what to load
/// \returns \c void*: Status; Not NULL when loading was OK, NULL otherwise
//-----------------------------------------------------------------------------
void* CardgameCollection::changeCards (void* opt) {
   TRACE2 ("CardgameCollection::changeCards (void*) - Option: " << opt);

   // Cards need an realized (!) parent, so ensure that the window is already
   // shown
   Check3 (this->is_realized ());
   TRACE3 ("CardgameCollection::changeCards (void*) - Use " << options.decks
           << " and " << options.back);

   try {
      if ((unsigned int)opt & 1)
         cardFaces.loadDecks (options.decks, false);
      if ((unsigned int)opt & 2)
         cardFaces.loadBack (options.back, false);

       ((unsigned int)opt & 0x8000)
           ? cards.addPacket (cardFaces)
           : cards.update ();
      return this;
   }
   catch (std::string& e) {
      Glib::ustring msg ("Couldn't load the card images!\n\n"
                         "Reason: %1");
      msg.replace (msg.find ("%1"), 2, e);
      showMessage (msg);

      Check3 (apMenus[NEW]);
      apMenus[NEW]->set_sensitive (false);

#ifdef HAVE_LIBPTHREAD
      Check3 (apMenus[CONNECT]);
      apMenus[CONNECT]->set_sensitive (false);
#endif
   }
   return NULL;
}

//----------------------------------------------------------------------------
/// Shows an error from the communication thread
/// \param msg: Received message to handle
/// \returns bool: False
/// \remarks msg wil be deleted at the end
//----------------------------------------------------------------------------
bool CardgameCollection::showMessage (const std::string msg) {
   Gtk::MessageDialog* dlg (new Gtk::MessageDialog (msg, false, Gtk::MESSAGE_ERROR));
   dlg->set_title (PACKAGE);
   dlg->signal_response ().connect
       (bind (ptr_fun (&CardgameCollection::closeDialog), dlg));
   dlg->show ();
   return false;
}

//-----------------------------------------------------------------------------
/// Frees the passed dialog
/// \param int: Response of dialog (ignored)
/// \param dlg: Dialog to close additionally
//-----------------------------------------------------------------------------
void CardgameCollection::closeDialog (int, const Gtk::Dialog* dlg) {
   Check1 (dlg);
   delete dlg;
}

//-----------------------------------------------------------------------------
/// Checks the user-input after asking if he wants to end the game; depending
/// on the answer either stops or continues
/// \returns bool: Flag, if the game has already been started
//-----------------------------------------------------------------------------
bool CardgameCollection::restartGame () {
   TRACE8 ("CardgameCollection::restartGame () - Restart: " << restart);
   Check1 (game);

   if (game->isRunning ()) {
      status.pop ();
      status.push (_("User canceled"));

      if (game->canBeStopped ()) {
         TRACE9 ("CardgameCollection::restartGame () - Game can be stopped");
         game->stop ();
         if (restart)
            startGame ();
      }
      else {
         TRACE9 ("CardgameCollection::restartGame () - Delaying stop of game");
         game->end ((actGame == oldGame) ? restart : false);
         return false;
      }
   }
   else {
      restart = false;
      startGame ();
   }
   return true;
}

//-----------------------------------------------------------------------------
/// Loads the cards (from png-files)
/// \remarks Cards need an realized (!) parent, so make somehow sure, that the
///      window already exists
//-----------------------------------------------------------------------------
void CardgameCollection::loadCards () {
   Check3 (is_realized ());
   status.push (_("Loading cardimages ..."));

   // This code needs the game-IDs in a sequence starting with 0!
   if (GameTypes::LAST <= options.type)
      options.type = GameTypes::ROVHULT;
   Glib::RefPtr<Gtk::ToggleAction>::cast_dynamic (apMenus[ROVHULT + options.type])->set_active ();

   void* rc (changeCards ((void*)-1));
   if (rc) {
      Check3 (apMenus[NEW]);
      apMenus[NEW]->set_sensitive (true);

#ifdef HAVE_LIBPTHREAD
      Check3 (apMenus[CONNECT]);
      apMenus[CONNECT]->set_sensitive (true);
#endif

      status.pop ();
      if (cmgr.getMode () != YGP::ConnectionMgr::CLIENT) {
         status.push (_("Start a new game with Ctrl+N (or Game -> New)"));
      }
   }

   Check3 (cardFaces.size ());
}

//-----------------------------------------------------------------------------
/// Handling of game-events
/// \param status: New status of game
//-----------------------------------------------------------------------------
void CardgameCollection::gameEvents (unsigned int status) {
   TRACE8 ("CardgameCollection::gameEvents (unsigned int) const - New status: "
           << status << "; Restart: " << restart);

   switch (status) {
   case Game::PLAYING:
      Check3 (apMenus[END]);
      apMenus[END]->set_sensitive (true);
#ifdef HAVE_LIBPTHREAD
      Check3 (apMenus[CONNECT]);
      apMenus[CONNECT]->set_sensitive (false);
#endif
      break;

   case Game::STOPPED:
      Check3 (apMenus[END]);
      apMenus[END]->set_sensitive (false);
#ifdef HAVE_LIBPTHREAD
      Check3 (apMenus[CONNECT]);
      apMenus[CONNECT]->set_sensitive (true);
#endif

      if (restart == 1) {
         // (Re)start the (new) game, when the event queue is empty (and
         // therefore the old game has ended).
         Glib::signal_idle ().connect
             (bind_return (mem_fun (*this, &CardgameCollection::doStartGame), false));
      }
      else if (restart == -1U)
         Glib::signal_idle ().connect
             (bind_return (mem_fun (*this, &CardgameCollection::hide), false));

      restart = false;
      break;
   }
}

//----------------------------------------------------------------------------
/// Starts the game and unlocks a (locked) msg-handling mutex
//----------------------------------------------------------------------------
void CardgameCollection::doStartGame () {
   startGame ();
#ifdef HAVE_LIBPTHREAD
   mxThreadCmd.unlock ();
#endif
}
