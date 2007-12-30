[WELCOME]

<p>Bienvenido a <b>Gambas</b> !<img src="img/gambas.png" align=right></p>

<p><b>Gambas</b> es un entorno gráfico de desarrollo basado en un intérprete <i>Basic</i> avanzado.</p>

<p>El objetivo de <b>Gambas</b> es el de permitir la concepción de programas potentes, de forma fácil y sencilla.
 Pero la limpieza de estos programas queda bajo <i>vuestra</> entera responsabilidad...</p>

<p>Disfrutadlo !</p>

<p align=right>Beno&icirc;t Minisini<br>
<u>gambas@users.sourceforge.net</u></p>


[STARTUP]

<p>Each project must have a <i>startup class</i>. This
startup class must define a static public method named <i>Main</i>
with no arguments, that will act as the startup method of
your program.</p>

<p>You can define the startup class by clicking on it with the
right mouse button in the project tree, and by selecting 
<i>Startup class</i> in the popup menu.</p>

<p>It is not necessary to define a <i>Main</i> method in a startup
form, because it already has a predefined one.</p>

<p>This predefined startup method instanciates the form and shows it, like in
<i>Visual Basic&trade;</i>.</p>


[EXEC]

<p>Puedes realizar un archivo ejecutable de tu proyecto entero.
 Selecciona <i>Make executable</i> en el menu  <i>Project</i>.</p>

<p>Cuando <b>Gambas</b> construye un archivo ejecutable,
 coloca el resultado directamente en el directorio de tu proyecto.
 El nombre del archivo ejecutable será el mismo que el de tu proyecto.</p>


[OPEN]

<p>La Instrucción <b>OPEN</b> de <b>Gambas</b> no trabaja de la misma forma que en <i>Visual Basic</i>.
 No devuelve el archivo como un entero, sino como un objeto <i>File</i>.</p>

<p>Es decir, en vez de teclear :</p>

<pre>DIM handle AS Integer
...
OPEN "miarchivo" FOR READ AS #handle</pre>

<p>Debes teclear :</p>

<pre>DIM handle AS File
...
handle = OPEN "miarchivo" FOR READ</pre>


[CATDIR]

<p>¿ Sabes que puedes concatenar nombres de directorios y nombres de archivos con el operador <b><tt>&/</tt></b>?
 Este operador complementa si es necesario con el separador de directorios de forma que la ruta resultante sea perfecta.</p>

<p>Por ejemplo :</p>

<pre>PRINT "/home/gambas" &/ ".bashrc"
/home/gambas/.bashrc

PRINT "/home/gambas/" &/ "/tmp" &/ "foo.bar"
/home/gambas/tmp/foo.bar
</pre>

<p>¿ No es maravilloso ?</p>


[EMPTY]

<p>Para averiguar si una cadena está vacía, no es necesario usar la función <b>Len()</b> .
 Puedes probarlo directamente, ya que una cadena vacía es <b>FALSE</b> y una no-vacía es <b>TRUE</b>.</p>

<p>Por ejemplo, en vez de :</p>

<pre>IF Len(MiCadena) > 0 THEN ...
IF Len(MiCadena) = 0 THEN ...</pre>

<p>Deberías hacer :</p>

<pre>IF MiCadena THEN ...
IF NOT MiCadena THEN ...</pre>



[EVENT]

<p>Todos los controles y todos los objetos que pueden recibir eventos,
 tienen un <i>Observador de eventos</i> y un <i>nombre de grupo</i> del evento.</p>

<p>El observador de evetos captura todos los eventos que se produzcan sobre el objeto,
 y el nombre de grupo del evento es el prefijo del procedimiento encargado de gestionar el evento.</p>

<p>Por defecto, este observador de eventos es el objeto donde se ha creado el control,
 y el nombre de grupoes el nombre del control.</p>

<p>De este modo, un formulario recibe todos los eventos producidos sobre los controles que se hallan creado en su interior.</p>

<pre>' Gambas form

DIM hButton AS Button

PUBLIC SUB _new()
&nbsp;&nbsp;...
&nbsp;&nbsp;hButton = NEW Button(ME) AS "MyButton"
&nbsp;&nbsp;...
END

PUBLIC SUB MyButton_Click()
&nbsp;&nbsp;...
END
</pre>


[FORM]

<p>Un formulario es su propio observador de eventos, de esta forma puedes gestionar directamente
sus eventos (<i>Resize</i>, <i>Activate</i>, ...) dentro de su propio código de clase.</p>

<p>De este modo, los novatos que procedan de <i>Visual Basic</i> no estarán perdidos :-).</p>


[EMBED]

<p>¡ Con <b><i>Gambas</i></b> puedes hacer que cualquier formulario esté embebido en cualquier otro&nbsp;!</p>

<p>Para utilizar este poderosa herramienta,
 sólo debes instanciar el formulario pasando un contenedor padre como último argumento del constructor.</p>

<p>Por ejemplo&nbsp;:</p>
<p><tt>DIM hForm AS MyDialog<br>
DIM hSuperControl AS MyForm<br><br>
' Create a dialog<br>
hForm = NEW MyDialog<br>
' Inserta un formulario en este dialogo<br>
' Observa que este formulario recibe dos parámetros antes del contenedor<br>
hSuperControl = NEW MyForm(Param1, Param2, MyDialog)<br>
' Mueve y ajusta el tamaño del formulario<br>
hSuperControl.Move(8, 8, 128, 64)<br>

<p>Ten en cuenta: un formulario embebido en otro sigue siendo un formulario,
 y por lo tanto observador de sus eventos.</p>


[GROUP]

<p>Todos los controles tienen una propiedad <i>(Group)</i>.
 Cuando esta propiedad es utilizada, el prefijo del gestor de eventos es el nombre del grupo,
 y no el nombre del control.</p>

<p>Supongamos que tienes un <i>Button</i> denominado <b>btnAccion</b> con el gestor de eventos <i>Click</i> siguiente :</p>

<pre>PUBLIC SUB btnAccion_Click()</pre>

<p>Si defines la propiedad <i>(Group)</i> del <b>btnAction</b> como <i>MiGrupo</i>,
 Entonces el gestor de eventos que recibirá los eventos del botón será el siguiente :</p>

<pre>PUBLIC SUB MiGrupo_Click()</pre>

<p>Esta propiedad permite manejar eventos de diferentes controles en una función sencilla.
 Y los controles del mismo grupo no necesitan ser del mismo tipo !</p>

<p><b>Nota :</b> Los veteranos del viejo <i>Visual Basic</i> reconocerán el concepto de <i>control array</i>,
 pero en una implementación más potente. :-)</p>


[TAG]

<p>Todos los controles tienen una propiedad denominada <i>Tag</i>
 Esta propiedad está pensada para ser utilizada por el programador
 y puede contener cualquier dato <b>VARIANT</b> que consideres relevante.</p>

<p>Esto es muy útil cuando quieres distinguir controles del mismo grupo
en un mismo gestor de eventos.</p>



[LAST]

<p>La palabra clave <b>LAST</b> devuelve el último control que ha recibido un evento.
 Es muy útil cuando quieras escribir un gestor de eventos que sea independiente del nombre del control.</p>

<p>Supongamos, por ejemplo, que quieres escribir un programa de una calculadora.
Has definido diez botones, uno por cada dígito, y todos con el mismo <i>group</i> "Digit".
 El valor del <i>Tag</i> de cada control será el que se visualiza en el botón.
 Tu gestor de eventos tendría un aspecto similar a esto :</p>

<p><tt>PUBLIC SUB Digit_Click()<br><br>
&nbsp;&nbsp;Display = Display & LAST.Tag<br>
&nbsp;&nbsp;RefreshDisplay<br><br>
END</tt></p>


[LEFT]

<p>Las bien conocidas rutinas <b>Left$</b>, <b>Right$</b>, <b>Mid$</b> de <i>BASIC</i>
 tienen en <b><i>Gambas</i></b> un nuevo y útil comportamiento.</p>

<p>El segundo parámetro de <b>Left$</b> y <b>Right$</b> es opcional, y es uno por defecto.</p>

<p><tt>Left$("Gambas")</tt> devuelve <tt>"G"</tt><br>
<tt>Right$("Gambas")</tt> devuelve <tt>"s"</tt></p>

<p>Este segundo parámetro puede ser negativo, y entonces representa el número de caracteres que no se extraerán.</p>

<p><tt>Left$("Gambas", -2)</tt> devuelve <tt>"Gamb"</tt><br>
<tt>Right$("Gambas", -2)</tt> devuelve <tt>"mbas"</tt></p>

<p>Así mismo, el tercer argumento de <b>Mid$</b> puede ser negativo,
 y entonces representa el número de caracteres desde el final de la cadena que no se extraerán.</p>

<p><tt>Mid$("Gambas", 2, -2)</tt> devuelve <tt>"amb"</tt>


[END]

<p>Has leído todos los consejos del día. Espero que seas ahora
un experto en <b>Gambas</b> ! :-)</p>

<p>Si quieres contribuir, envía nuevos consejos a la siguiente
dirección&nbsp;:</p>
<p><u>gambas@users.sourceforge.net</u></p>

<p>Gracias anticipadas !</p>

