<!-- -*-HTML-*- -->
<!-- $Id$ -->

<!--
  Anti-Copyright (A) 2003 Markus Schwab (g17m0@lycos.com)

  This is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
-->

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//ES">

<html>
  <head>
    <title>Cardgames - Una Collecti&oacute;n de juegos de cartas para varias personas</title>
    <meta name="description" content="Documentaci&oacute;n de los juegos de cartas">
    <meta name="keywords" content="documentacion, documentaci&oacute;n, docu, juego, game, card, carta, juego de cartas, cardgame">

    <meta name="DC.Creator" content="Markus Schwab">
    <meta name="DC.Date" content="2003-02-11">
    <meta name="DC.Rights" content="Anticopyright (A) 2003, distributed under the GNU Free Documentation License">
  </head>

  <body>
    <h1 align="center">Informaci&oacute;n General sobre los Juegos de Cartas</h1>
    <hr size=2>

    <p>Ese es una colecci&oacute;n  de juegos de cartas que aprend&iacute; de
      varias personas, generalmente durante viajar (&iquest;Qu&eacute; pudr&iacute;a
      matar mejor el tiempo durante un viaje de &gt;10 horas en
      bus/tren/qu&eacute;-s&eacute;-yo que und juego de cartas bueno?)</p>

    <p>Todos son para varias personas (los dem&aacute;s est&aacute;n controlado
      de la computadora). Por juegos de cartas solitarios puedo recomendar
      los excelentes juegos de cartas de KDE.</p>

    <p>Este programa se distribuye bajo los condicones de la GNU Licensia
      General P&uacute;blico, que principalmente expresa lo siguente (v&eacute;ase
      el archivo <tt>COPYING</tt> dentro de la distribuci&oacute; o
      la <a href="http://www.gnu.org">p&aacute;gina web de GNU
        (http://www.gnu.org)</a> para m&aacute;s detalles):</p>

    <blockquote><em><p>Este programa es software libre; puede
        redistribuirlo y/o modificarlo bajo los t&eacute;rminos de la
        GNU Licencia P&uacute;blica General seg&uacute;n se publica por la
        Free Software Foundation (la Fundaci&oacute;n para el Software
        Libre); tanto de la versi&oacute;n 2 de la Licencia, o (seg&uacute;n su
        elecci&oacute;n) de cualquier versi&oacute;n posterior.</p>
 
      <p>Este se destribuye con la esperanza de que sea &uacute;til,
        pero SIN NINGUNA GARANT&Iacute;A, ni siquiera la garant&iacute;a
        impl&iacute;cita de COMERCIABILIDAD o CONVENIENCIA PARA UN
        PROP&Oacute;SITO PARTICULAR.  V&eacute;ase la GNU Licensia General
        P&uacute;blico para m&aacute;s detalles).</p></em></blockquote>

    <h2>Autor</h2>
    <p>Ambos el programa y la documentaci&oacute;n han sido escrito de Markus
      Schwab.</p>

    <h2>Juegos</h2>
    <p>Actualmente la colecci&oacute;n consiste de los siguentes juegos:</p>

    <table>
      <!--#exec cmd='
      /usr/bin/awk \'/title.*title/ \
      { if (FILENAME !~ /CardCol\..*/) \
      { sub (/<\/title>/, "")
      sub (/<title>/, "")
      $1 = "<tr><td>&nbsp;&nbsp;<a href=\"" FILENAME "\">" $1 "</a></td><td>"
      print $0 "</td></tr>" } }\' *.html.es
      ' -->
    </table>

    <h2>Cr&eacute;ditos</h2>
    <p>Saludos a Andreas, Jakob y Janus, que me han ese&ntilde;ado los reglas de
      <tt>R&oslash;vhult</tt>.</p>

    <p>Gracias a Ann-Karin, Arne, B&aring;rn y Solveing por introducirme a
      <tt>Twopart</tt>.</p>

    <p>Corazones es distribuido con Windows 98 (y tal vez otras versiones).</p>

    <hr size=2 noshade>
    <address>
      <a href="mailto:g17m0@lycos.com">Markus Schwab (g17m0@lycos.com)</a><br>
    </address>
<!-- hhmts start -->
Last modified: Tue Feb 11 09:33:07 PET 2003
<!-- hhmts end -->
          - $Revision$
</body>
</html>
