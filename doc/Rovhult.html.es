<!-- -*-HTML-*- -->
<!--
  Copyright (C) 2002 - 2026 Markus Schwab (pelotudo.gringo@gmail.com)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
-->

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"
        "http://www.w3.org/TR/html4/strict.dtd">

<html>
  <head>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
    <title>R&oslash;vhult - Un juego de cartas conocido en todo el mundo (como Shithead, Asshole, ...)</title>
    <meta name="description" content="Documentaci&oacute;n del juego de cartas R&oslash;vhult">
    <meta name="keywords" content="documentacion, documentaci&oacute;n, docu, R&oslash;vhult, Rovhult; Shithead, game, juego, card, cardgame, carta, juego de cartas">

    <meta name="DC.Creator" content="Markus Schwab">
    <meta name="DC.Date" content="2003-02-11">
    <meta name="DC.Rights" content="Copyright (C) 2003 - 2026, distributed under the GNU Free Documentation License">
  </head>

  <body>
    <h1 align="center">R&oslash;vhult</h1>
    <hr size=2>

    <p>R&oslash;vhult es un juego de cartas que aprend&iacute; de tres
      daneses; a ellos se debe el nombre y esa letra linda y inefable (parece
      como una vaca mugiendo mientras vomita). Otros nombres son Shithead
      (Gran Breta&ntilde;a) o Asshole (Australia, EE.UU.).</p>

    <p>La meta del juego es <b>no</b> ser el &uacute;ltimo en
      liberarse de todas sus cartas, de acuerdo con las siguientes
      reglas - por supuesto son f&aacute;ciles como tambi&eacute;n los
      yankis lo juegan:</p>

    <h2>Reparto</h2>
    <p>Todos los jugadores empiezan con tres cartas en su mano y tres
      cartas boca abajo con una carta boca arriba por encima
      en la mesa. Las dem&aacute;s cartas se ponen en el mont&oacute;n
      de reserva.</p>

    <p>Las cartas boca arriba de la mesa se pueden cambiar con las
      de la mano (con tirar y depositar - drag and drop<a
      href="#Note1"><sup>1</sup></a>). Generalmente se trata de poner
      cartas &quot;buenas&quot; (de acuerdo con las reglas siguientes) en
      la mesa, pero eso es una cuesti&oacute;n de gusto.</p>

    <p>Hacer clic en una carta en la mano empieza el juego. Se ordenan
      las cartas y un jugador casual empieza.</p>

    <h2>El juego</h2>
    <p>Se juega contra la direcci&oacute;n de las agujas del
      reloj. Todos los jugadores ponen sus cartas, que deben tener un
      n&uacute;mero igual o mayor que la &uacute;ltima carta
      jugada. Tenga en cuenta que se puede poner cualquier cantidad de
      cartas con n&uacute;meros iguales (como el &spades; 5 y el
      &clubs; 5). Despu&eacute;s se tiene que sacar cartas de la
      reserva hasta de nuevo tener tres cartas en la mano (o hasta que la
      reserva est&eacute; vac&iacute;a).</p>

    <p>Hay unas cartas especiales y excepciones:</p>
    <dl>
      <dt><b>2's</b></dt>
      <dd><p>Se puede jugar siempre; el pr&oacute;ximo jugador puede seguir con
          cualquier carta</p></dd>
      <dt><b>La &uacute;ltima carta jugada es la carta de la reversi&oacute;n (7)<a href="#Note2"><sup>2</sup></a></b></dt>
      <dd><p>La pr&oacute;xima carta tiene que ser igual o menor</p></dd>
      <dt><b>Carta de salto (8)<a href="#Note2"><sup>2</sup></a></b></dt>
      <dd><p>El pr&oacute;ximo jugador es saltado</p></dd>
      <dt><b>Carta de borrar (10)<a href="#Note2"><sup>2</sup></a></b></dt>
      <dd><p>Se puede jugar siempre (como los 2's), pero adem&aacute;s las
          cartas jugadas son retiradas. El mismo jugador puede
          continuar, aunque no puede sacar cartas de la reserva
          (excepto si ya no tiene cartas).</p></dd>
      <dt><b>Cuatro cartas con n&uacute;meros iguales jugadas</b></dt>
      <dd><p>Entonces todas las cartas son retiradas y el
          jugador puede continuar (semejante a jugar una carta
          de borrar (10), excepto que s&iacute; se puede sacar de la
          reserva). Tenga en cuenta que las cartas tampoco
          tienen "efecto especial" (como cuatro 8's no saltan al
          pr&oacute;ximo jugador).</p></dd>
    </dl>

    <p>Si un jugador no puede continuar de acuerdo con estas
      reglas, tiene que recoger todas las cartas jugadas. Por
      supuesto lo puede hacer con intenci&oacute;n, entonces sigue el
      pr&oacute;ximo.</p>

    <h2>El juego desde la mesa</h2>
    <p>Si un jugador ya no tiene cartas en su mano (y tampoco hay cartas
      en la reserva), puede seguir con las de la mesa. Se
      juegan de acuerdo con las reglas anteriores; comenzando con las cartas
      de boca arriba y despu&eacute;s con las de boca abajo.</p>

    <p>Tenga en cuenta que si uno necesita recoger las cartas jugadas
      en su mano no se puede recoger tambi&eacute;n una carta de la
      mesa. En lugar de eso uno tiene que seguir con esas (nuevas)
      cartas en su mano antes de volver a usar las cartas de la
      mesa.</p>

    <p>Las cartas de boca abajo se juegan casualmente. Si la
      carta elegida no vale de acuerdo con las reglas, se tiene que
      recogerla tal cual como las dem&aacute;s jugadas.</p>

    <p>&iexcl;La barra de estado muestra lo que est&aacute; pasando!</p>

    <h2>Notas sobre el layout</h2>
    <p>Despu&eacute;s de iniciar se encuentran desplazadas las cartas (de
      izquierda a derecha y de arriba a abajo):</p>

    <ul>
      <li>La reserva: Una colecci&oacute;n de cartas boca abajo, de donde se sacan
        cartas nuevas (si se pone el mouse por encima, aparece una ventana
        mostrando el n&uacute;mero de cartas).</li>
      <li>Las cartas del jugador humano. Por arriba sus cartas de mano; por
        abajo las de la mesa.</li>
      <li>En la l&iacute;nea pr&oacute;xima est&aacute; lo mismo con los jugadores 1 y 3. Entre ellos
        se encuentra el puesto de las cartas jugadas.</li>
      <li>En la l&iacute;nea &uacute;ltima est&aacute;n las cartas del jugador 2.</li>
    </ul>

    <hr size=1 noshade>
    <p style="text-indent:-0.3cm;margin-left:0.3cm"><a name="Note1"></a>1)
      <i>Tirar y depositar</i> significa hacer clic con un bot&oacute;n del
      mouse en una carta, se sigue presionando el bot&oacute;n y se tira la
      carta a su destino. El fin de esa actividad se ejecuta
      con soltar el bot&oacute;n.</p>

    <p style="text-indent:-0.3cm;margin-left:0.3cm"><a name="Note2"></a>2)
      Se puede cambiar esa carta en las propiedades.</p>

    <hr size=3 noshade>
    <table width="100%">
      <tr>
        <td width="*">
          <address>
            <a href="mailto:pelotudo.gringo@gmail.com">Markus Schwab (pelotudo.gringo@gmail.com)</a><br>
          </address>
        </td>
        <td><a href="Machiavelli.html.es">Anterior</a><br>(Machiavelli)</td>
        <td><a href="CardCol.html.es">Contenido</a></td>
        <td><a href="Sgt.Mayor.html.es">Pr&oacute;ximo</a><br>(Sgt. Mayor)</td>
      </tr>
    </table>
  </body>
</html>
