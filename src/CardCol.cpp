//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Common
//REFERENCES  :
//TODO        : 
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 9.9.2002
//COPYRIGHT   : Anticopyright (A) 2002, 2003

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

#include <stdio.h>

#include <fstream>

#include <glib.h>

#include <Check.h>
#include <Trace_.h>

#include <File.h>
#include <INIFile.h>
#include <PathSrch.h>

#include <XAbout.h>
#include <XMessageBox.h>
#include <DeckSelect.h>

#include <CardWidget.h>
#include "Hearts.h"
#include "Rovhult.h"
#include "Twopart.h"
#include "Options.h"
#include <PlayerDlg.h>

#include "CardCol.h"


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


// With a very ugly trick initialize I18n before the first use of gettext)
XApplication::MenuEntry CardgameCollection::menuItems[] = {
    { (initI18n (PACKAGE, LOCALEDIR),
      _("_Game")),            _("<alt>G"), 0,        BRANCH },
    { _("_New"),              _("<ctl>N"), NEW,      ITEM },
    { _("_End"),              _("<ctl>E"), END,      ITEM },
    { "",                     "",          0,        SEPARATOR },
    { _("E_xit"),             _("<ctl>Q"), EXIT,     ITEM },
    { _("_Options"),          _("<alt>O"), 0,        BRANCH },
    { _("_Change game"),      _("<alt>C"), 0,        SUBMENU },
    {    _("_Røvhult"),       _("<ctl>R"), ROVHULT,  RADIOITEM },
    {    _("_Twopart"),       _("<ctl>T"), TWOPART,  RADIOITEM },
    {    _("_Hearts"),        _("<ctl>H"), HEARTS,   LASTRADIOITEM },
    { "",                     "",          0,        SUBMENUEND },
    { _("Change _decks ..."), _("<ctl>D"), CHGDECKS, ITEM },
    { _("Change _names ..."), _("<ctl>C"), CHGNAMES, ITEM },
    { _("_Save settings"),    _("<ctl>S"), SAVESET,  ITEM },
#if TRACELEVEL >= 1
    { "_Debug",               "<ctl>G",    DEBUG,    CHECKITEM }
#endif
};


// VIO-Application part of the Cardgames; cares about reading the INI-file and
// processing the options
class CardgameAppl : public IVIOApplication {
 public:
   CardgameAppl (const int argc, const char* argv[])
      : IVIOApplication (argc, argv, lo) { }
   ~CardgameAppl () { }

 protected:
   virtual void readINIFile (const char* pFile);
   virtual bool handleOption (const char option);

   // Program-handling
   virtual bool        shallShowInfo () const { return false; }
   virtual int         perform (int argc, const char* argv[]);
   virtual const char* name () const { return PACKAGE; }
   virtual const char* description () const {
      static string version =
         (PACKAGE " V" VERSION " - "
          + std::string (_("Compiled on"))
          + std::string (" " __DATE__ " - " __TIME__ "\n\n")
          + std::string (_("Author: Markus Schwab; e-mail: g17m0@lycos.com"
                           "\nDistributed under the terms of the GNU General "
                           "Public License")));
      return version.c_str (); }

   // Help-handling
   virtual void showHelp () const;

 private:
   // Prohobited manager functions
   CardgameAppl ();
   CardgameAppl (const CardgameAppl&);
   const CardgameAppl& operator= (const CardgameAppl&);

   Options options;

   static CardgameCollection::games convertToGameType (const char* pText);

   static const longOptions lo[];
};

const IVIOApplication::longOptions CardgameAppl::lo[] = {
   { IVIOAPPL_HELP_OPTION },
   { "browser", 'b' },
   { "help-dir", 'd' },
   { "file", 'f' },
   { "version", 'V' },
   { NULL, '\0' } };


/*--------------------------------------------------------------------------*/
//Purpose   : Defaultconstructor; all widget are created
//Parameters: type: Type of game to start with
/*--------------------------------------------------------------------------*/
CardgameCollection::CardgameCollection (Options& opts)
   : XApplication (PACKAGE " V" PRG_RELEASE), status ()
     , cardFaces (USED_CARDS), cards (), pThread (NULL), game (NULL)
     , options (opts), oldGame (NONE), restart (false) {
   set_usize (WIDTH, HEIGHT);

   helpBrowser = options.browser;

   // Create controls
   addMenus (menuItems, sizeof (menuItems) / sizeof (menuItems[0]));
   showHelpMenu ();
   Check3 (apMenus[NEW]);
   apMenus[NEW]->set_sensitive (false);

   status.show ();
   getClient ().pack_end (status, false);

   show ();

   // Load cards in background
   try {
      pThread = THRDAPPL::create (this, (THRDAPPL::THREAD_OBJMEMBER)&CardgameCollection::loadCards,
                               NULL);
      TRACE9 ("CardgameCollection::CardgameCollection () - Thread-ID = " << pThread->getID ());
   }
   catch (std::string& e) {
      XMessageBox::Show (e, _("Error starting thread"), XMessageBox::ERROR);
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
CardgameCollection::~CardgameCollection () {
   TRACE9 ("CardgameCollection::~CardgameCollection ()");
}


/*--------------------------------------------------------------------------*/
//Purpose   : Starts the game
/*--------------------------------------------------------------------------*/
void CardgameCollection::startGame () {
   TRACE6 ("CardgameCollection::startGame () - Old game type " << oldGame
           << " -> New: " << options.type);

   // Check if game has been changed; if so destroy the old one
   if (oldGame != options.type) {
      if (game) {
         game->clean ();
         getClient ().remove (*game);
         delete game;
      }

      oldGame = CardgameCollection::games (options.type);
      switch (oldGame) {
      case GROVHULT:
         game = new TGame<Rovhult, CardgameCollection>
            (*this, &CardgameCollection::gameEvents);
         break;

      case GTWOPART:
         game = new TGame<Twopart, CardgameCollection>
            (*this, &CardgameCollection::gameEvents);
         break;

      case GHEARTS:
         game = new TGame<Hearts, CardgameCollection>
            (*this, &CardgameCollection::gameEvents);
         break;

      default:
         Check (0);
      }
   }

   Check3 (game);
   string name (PACKAGE " V" PRG_RELEASE " - ");
   name += game->name ();
   set_title (name);

   game->start ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Returns the names of the players
//Returns   : The names of the players
//Remarks   : Can't be inline because of cyclic dependencies to Options
/*--------------------------------------------------------------------------*/
const vector<string>& CardgameCollection::getNames () const {
   return options.names;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Command-handler
//Parameters: menu: ID of command (menu)
/*--------------------------------------------------------------------------*/
void CardgameCollection::command (int menu) {
   switch (menu) {
   case NEW:
      if (game && game->isRunning ()) {
         restart = true;
         XMessageDialog<CardgameCollection>
            ::Show (*this, &CardgameCollection::userWants2End,
                    _("A game is already running. Do you really"
                      " want to end it and start another?"),
                    PACKAGE, XMessageBox::QUESTION | XMessageBox::YESNO);
      }
      else
         startGame ();
      break;

   case END:
      Check3 (game && game->isRunning ());
      restart = false;
      XMessageDialog<CardgameCollection>
         ::Show (*this, &CardgameCollection::userWants2End,
                 _("Do you really want to end the game?"),
                 PACKAGE, XMessageBox::QUESTION | XMessageBox::YESNO);
      break;

   case TWOPART:
      options.type = GTWOPART;
      break;

   case ROVHULT:
      options.type = GROVHULT;
      break;

   case HEARTS:
      options.type = GHEARTS;
      break;

   case CHGDECKS:
      CarddeckSelectDlg<CardgameCollection>
         ::create (*this, &CardgameCollection::changeDecks,
                   CARDSET_PATH, options.decks, options.back);
      break;

   case CHGNAMES:
      PlayerDlg<CardgameCollection>
         ::create (*this, &CardgameCollection::changePlayernames, options.names);
      break;

   case SAVESET: {
      TRACE2 ("CardgameCollection::command (int) - Save file");
      ofstream inifile (options.pNameINIFile);
      if (inifile) {
         options.strType = options.type + '0';
         INIFile::write (inifile, "Game", options);
         INIList<string>::write (inifile, "Players", options.names);
      }
      break;
   }

   case EXIT:
      if (game && game->isRunning ()
          && (XMessageBox::Show (_("A game is running. Do you really want to quit?"),
                                 PACKAGE, XMessageBox::QUESTION | XMessageBox::YESNO)
              != XMessageBox::YES))
            break;

      Main::quit ();
      break;

#if TRACELEVEL >= 0
   case DEBUG: {
      static bool open = false;
      open = !open;
      if (game)
         game->playOpen (open);
      break; }
#endif

   default:
      XApplication::command (menu);
   } // end-switch
}

/*--------------------------------------------------------------------------*/
//Purpose   : Returns the name of the file to display in the help
//Returns   : Name of file to display
/*--------------------------------------------------------------------------*/
const char* CardgameCollection::getHelpfile () {
   string file (options.helpPath);
   if (file[file.size () - 1] != File::DIRSEPARATOR)
      file += File::DIRSEPARATOR;
   file += game ? (string (game->name ()) + ".html") : "CardCol.html";
   return file.c_str ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Shows the about box for the program
/*--------------------------------------------------------------------------*/
void CardgameCollection::showAboutbox () {
   string ver (_("Anticopyright (A) 2002, 2003 Markus Schwab"
                 "\ne-mail: g17m0@lycos.com\n\nCompiled on %1 at %2"));
   ver.replace (ver.find ("%1"), 2, __DATE__);
   ver.replace (ver.find ("%2"), 2, __TIME__);

   XAbout* about (new XAbout (ver, PACKAGE " V" VERSION));
   about->setIconProgram (xpmGame);
   about->setIconAuthor (xpmAuthor);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback to change the names of the playing people
/*--------------------------------------------------------------------------*/
void CardgameCollection::changePlayernames () {
   TRACE2 ("CardgameCollection::changePlayernames");
   if (game)
      game->changeNames (options.names);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback to change the carddecks
//Parameters: cmd: Selected button of dialog
/*--------------------------------------------------------------------------*/
void CardgameCollection::changeDecks (const ICarddeckSelectDlg& dialog) {
   TRACE2 ("CardgameCollection::changeDecks (const ICarddeckSelectDlg&)");

   std::string deck, back;
   dialog.getSelection (deck, back);
   unsigned int option (0);
   if (deck != options.decks) {
      option = 1;
      options.decks = deck;
   }
   if (back != options.back) {
      option |= 2;
      options.back = back;
   }

   pThread = THRDAPPL::create (this,
                               (THRDAPPL::THREAD_OBJMEMBER)&CardgameCollection::changeCards,
                               (void*)option);
   TRACE9 ("CardgameCollection::changeDecks (const ICarddeckSelectDlg) - Thread-ID = "
           << pThread->getID ());
}

/*--------------------------------------------------------------------------*/
//Purpose   : Loads the cards (from xpm-files)
//Parameters: opt: Actually a bit field! Option indicationg what to load
/*--------------------------------------------------------------------------*/
void CardgameCollection::changeCards (void* opt) {
   TRACE2 ("CardgameCollection::changeCards (void*) - Option: " << opt);
   Check1 (opt);

   // Cards need an realized (!) parent, so ensure that the window is already
   // shown
   Check3 (this->is_realized ());
   
   TRACE3 ("CardgameCollection::changeCards (void*) - Use " << options.decks
           << " and " << options.back);

   if ((unsigned int)opt & 1)
      cardFaces.loadDecks (get_window (), options.decks);
   if ((unsigned int)opt & 2)
      cardFaces.loadBack (get_window (), options.back);

   gdk_threads_enter ();
   cards.update ();
   gdk_threads_leave ();
   pThread = NULL;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Checks the user-input after asking if he wants to end the game;
//             depending on the answer either stops or continues
//Parameters: input: Button pressed by the user
/*--------------------------------------------------------------------------*/
void CardgameCollection::userWants2End (unsigned int input) {
   if (input == XMessageBox::YES) {
      Check3 (game);
      if (game->isRunning ()) {
         status.pop (1);
         status.push (1, _("User canceled"));

         if (game->canBeStopped ()) {
            game->stop ();
            if (restart)
               startGame ();
         }
         else {
            game->end ((options.type == oldGame) ? restart : false);
            if (options.type == oldGame)
               restart = false;
         }
      }
      else
         startGame ();
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Loads the cards (from xpm-files)
/*--------------------------------------------------------------------------*/
void CardgameCollection::loadCards () {
   // Cards need an realized (!) parent, so make somehow sure, that the window
   // already exists
   Check3 (is_realized ());

   gdk_threads_enter ();
   status.push (1, _("Loading cardimages ..."));
   gdk_threads_leave ();

   // This code needs the game-IDs in a sequence starting with 0!
   if (GLAST <= (unsigned int)options.type)
      options.type = GROVHULT;
   dynamic_cast<CheckMenuItem*> (apMenus[ROVHULT + options.type])->set_active ();

   cardFaces.load (get_window (), options.decks, options.back);
   cards.addPacket (cardFaces);

   gdk_threads_enter ();
   apMenus[NEW]->set_sensitive (true);
   status.pop (1);
   status.push (1, _("Start a new game with Ctrl+N (or Game -> New)"));
   gdk_threads_leave ();

   assert (cardFaces.numberOfCards ());
   pThread = NULL;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Handling of game-events
//Parameters: status: New status of game
/*--------------------------------------------------------------------------*/
void CardgameCollection::gameEvents (unsigned int status) {
   TRACE8 ("CardgameCollection::gameEvents (unsigned int) const - New status: "
           << status);

   switch (status) {
   case Game::PLAYING:
      Check3 (apMenus[END]);
      apMenus[END]->set_sensitive (true);
      break;

   case Game::STOPPED:
      Check3 (apMenus[END]);
      apMenus[END]->set_sensitive (false);

      if (restart)
         startGame ();
      restart = false;
      break;
   }
}


/*--------------------------------------------------------------------------*/
//Purpose   : Displays the help
/*--------------------------------------------------------------------------*/
void CardgameAppl::showHelp () const {
   cout << "Collection of cardgames\n\nUsage: " PACKAGE " [OPTIONS]\n\n"
      "  -g, --game ....... [GAME] Select game to start (default: Rovhult)\n"
      "  -f, --file ....... [FILE] Use file as INI file\n"
      "  -b, --browser .... [NAME] Browser to use to display the help\n"
      "  -d, --help-dir ... [DIR] Directory to search for help\n"
      "  -V, --version .... Output version information and exit\n"
      "  -h, -?, --help ... Displays this help and exit\n\n"
      "Valid values for GAME are Rovhult, Røvhult, Twopart and Hearts or the numbers\n"
      "0, 1 and 2 (corresponding to the games in the above order).\n\n"
      "The INI file can have the following entries:\n\n"
      "  [Game]\n"
      "  Type=Twopart\n"
      "  Helpbrowser=galeon\n"
      "  Helpdir=/usr/share/doc/Cardgames/\n"
      "  CardFront=/usr/local/share/Cardsets/Deck1\n"
      "  CardBack=/usr/local/share/Cardsets/back1.xpm\n\n"
      "  [Players]\n"
      "  0=Human\n"
      "  1=Computer 1\n"
      "  2=Computer 2\n"
      "  3=Computer 3\n";
}

/*--------------------------------------------------------------------------*/
//Purpose   : Checks the validity of the passed option
//Parameters: option: Actual option
//Returns   : bool: Status; false: Invalid option/option-value
//Require   : option not '\0´'
/*--------------------------------------------------------------------------*/
bool CardgameAppl::handleOption (const char option) {
   Check3 (option != '\0');

   switch (option) {
   case 'g': {
      const char* game (getOptionValue ());
      if (game) {
         CardgameCollection::games type (convertToGameType (game));
         if (type != CardgameCollection::NONE)
            options.type = type;
         else
            cerr << PACKAGE "-warning: INI-file contains invalid game type `"
                 << game << "'\n";
      }
      else
         cerr << PACKAGE "-warning: No game specified! Ignoring option `g'\n";
      break; }

   case 'd': {
      const char* pDir (getOptionValue ());
      if (pDir)
         options.helpPath = pDir;
      else
         cerr << PACKAGE "-warning: No directory specified! Ignoring option `d'\n";
      break; }

   case 'b': {
      const char* pBrowser (getOptionValue ());
      if (pBrowser)
         options.browser = pBrowser;
      else
         cerr << PACKAGE "-warning: No browser specified! Ignoring option `b'\n";
      break; }

   case 'f': {
      const char* pFile (getOptionValue ());
      if (pFile)
         readINIFile (pFile);
      else
         cerr << PACKAGE "-warning: No file specified! Ignoring option `f'\n";
      break; }

   case 'V':
      cout << description () << '\n'; exit (0);
      break;
   }

   return true;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Converts a text to a game type
//Parameters: pText: Text to convert
//Returns   : Type of game as understood by the CardgameCollection
/*--------------------------------------------------------------------------*/
CardgameCollection::games CardgameAppl::convertToGameType (const char* pText) {
   static struct {
      const char* pText;
      CardgameCollection::games value;
   } values[] = { { "Rovhult", CardgameCollection::GROVHULT },
                  { "Røvhult", CardgameCollection::GROVHULT },
                  { "Twopart", CardgameCollection::GTWOPART },
                  { "Hearts", CardgameCollection::GHEARTS },
                  { "0", CardgameCollection::GROVHULT },
                  { "1", CardgameCollection::GTWOPART },
                  { "2", CardgameCollection::GHEARTS } };

   for (unsigned int i (0); i < (sizeof (values) / sizeof (values[0])); ++i)
      if (!strcmp (values[i].pText, pText))
         return values[i].value;

   return CardgameCollection::NONE;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Reads the options of the INI-file
//Parameters: pFile: Pointer to filename
//Requieres : pFile not NULL
/*--------------------------------------------------------------------------*/
void CardgameAppl::readINIFile (const char* pFile) {
   TRACE5 ("CardgameAppl::readINIFile (const char*) - " << pFile);
   Check3 (pFile);

   options.names.push_back ("Human");
   options.names.push_back ("Player 1");
   options.names.push_back ("Player 2");
   options.names.push_back ("Player 3");

   options.pNameINIFile = pFile;

   std::string Style;
   try {
      INIFILE (pFile);
      INIOBJ (options, Game);
      INILIST2 (Players, std::string, options.names);

      unsigned int rc (INIFILE_READ ());
   }
   catch (std::string& error) {
      cerr << PACKAGE "-warning: Error reading INI-file '" << pFile << "'\n"
           << error << '\n';
   }

   CardgameCollection::games type (convertToGameType (options.strType.c_str ()));
   if (type != CardgameCollection::NONE)
      options.type = type;
   else
      cerr << PACKAGE "-warning: INI-file contains invalid game type `"
           << options.strType << "'\n";
}

/*--------------------------------------------------------------------------*/
//Purpose   : Performs the job of the applications
//Parameters: int: Number of parameters (without options)
//            const char*: Array with pointer to arguments
//Returns   : int: Status
/*--------------------------------------------------------------------------*/
int CardgameAppl::perform (int, const char**) {
   srand (time (NULL));              // Initialize the random number generator

   g_thread_init (NULL);

   // Pass real (unprocessed) options to gtkmm/GTK+
   Main appl (static_cast<int> (args), const_cast <char**> (ppArgs));
   CardgameCollection win (options);

   gdk_threads_enter ();
   appl.run ();
   gdk_threads_leave ();
   return 0;
}


/*--------------------------------------------------------------------------*/
//Purpose   : Entrypoint of application
//Parameters: argc: Number of parameters
//            argv: Array with pointer to parameter
//Returns   : int: Status
/*--------------------------------------------------------------------------*/
int main (int argc, const char* argv[]) {
   CardgameAppl appl (argc, argv);
   return appl.run ();
}
