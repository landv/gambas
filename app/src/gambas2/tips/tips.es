[WELCOME]

<p>¡Bienvenido a <b>Gambas</b>!</p>

<p><b>Gambas</b> es un ambiente integrado de desarrollo basado en un intérprete <i>Basic</i> avanzado.</p>

<p><b>Gambas</b> pretende permitirle crear poderosos programas rápida y fácilmente. Pero la claridad de éstos programas es su <i>propia</i> responsabilidad...</p>

<p>¡Disfrútelo!</p>

<p align=right>Beno&icirc;t Minisini<br><u>gambas@users.sourceforge.net</u></p>

[STARTUP]

<p>Cada proyecto debe tener una <i>clase de inicio</i>. Ésta clase de inicio debe definir un método estático y público llamado <i>Main</i> sin argumentos, que actuará como el método de inicio de su programa.</p>

<p>Usted puede definir la clase de inicio haciendo click sobre ella con el botón derecho del ratón en el árbol del proyecto, y seleccionado <i>Clase de inicio</i> en el menú contextual.</p>

<p>No es necesario definir un método <i>Main</i> en un formulario de inicio, porque éste ya tiene uno predefinido.</p>

<p>Éste método predefinido de inicio instancia el formulario y lo muestra, como en <i>Visual Basic&trade;</i>.</p>

[OPEN]

<p>La instrucción <b>OPEN</b> de <b>Gambas</b> no trabaja como la de <i>Visual Basic&trade;</i>. Ésta no retorna el archivo como un entero, sino como un objeto <i>File</i>.</p>

<p>Así que, en lugar de escribir:</p>

<pre>DIM handle AS Integer
...
OPEN "myfile" FOR READ AS #handle</pre>

<p>debe escribir:</p>

<pre>DIM handle AS File
...
handle = OPEN "myfile" FOR READ</pre>

[CATDIR]

<p>¿Sabía que puede concatenar nombres de directorios y nombres de archivos con el operador <b><tt>&/</tt></b>? Éste operador maneja las barras inclinadas de tal forma que la ruta resultante es perfecta.</p>

<p>Por ejemplo:</p>

<pre>PRINT "/home/gambas" &/ ".bashrc"
/home/gambas/.bashrc

PRINT "/home/gambas/" &/ "/tmp" &/ "foo.bar"
/home/gambas/tmp/foo.bar
</pre>

<p>¿No es maravilloso?</p>

[EXEC]

<p>Usted puede crear un archivo ejecutable del proyecto entero. Seleccione <i>Crear ejecutable</i> en el menú <i>Proyecto</i>.</p>

<p>Cuando <b>Gambas</b> hace un archivo ejecutable, por defecto pone el resultado en el directorio del proyecto. El nombre del ejecutable tiene el mismo nombre de su proyecto.</p>

[PATH]

<p>Las rutas relativas tienen un significado especial en <b>Gambas</b>. Ellas siempre se refieren a archivos dentro de sus proyectos.</p>

<p>No existe el concepto de <i>directorio actual</i>, ni palabra reservada como <tt>CHDIR</tt> para cambiarlo.</p>

<p><b>Tenga cuidado:</b> debe utilizar rutas relativas sólo para acceder archivos del proyecto, porque las rutas absolutas no trabajarán más cuando usted cree un ejecutable.</p>

[GLOBAL]

<p>¡No hay <u>variables globales</u> en <b>Gambas</b>!</p>

<p>Como apoyo, póngalas en su módulo principal y declárelas como <tt>PUBLIC</tt>.</p>

<p>Si no tiene un módulo principal en su proyecto, pero sí un formulario principal, entonces declárelas como <tt>STATIC PUBLIC</tt>.</p>

<p>Para acceder a esas variables, debe utilizar el nombre del módulo o formulario principal: <tt>MyMainModule.MyGlobalVariable</tt> o <tt>MyMainForm.MyGlobalVariable</tt>.</p>


[EMPTY]

<p>Para saber si una cadena está vacía, no es necesario usar la función <b>Len()</b>. Usted puede probarla directamente, ya que una cadena vacía es <b>FALSE</b> y una no vacía es <b>TRUE</b>.</p>

<p>Por ejemplo, en lugar de hacer:</p>

<pre>IF Len(MyString) > 0 THEN ...
IF Len(MyString) = 0 THEN ...</pre>

<p>Usted podría hacer:</p>

<pre>IF MyString THEN ...
IF NOT MyString THEN ...</pre>

[TRANSLATE]

<p>Las aplicaciones <b>Gambas</b> son completamente traducibles, usted decide cual cadena debe ser traducida, y cual no.</p>

<p>Para marcar una cadena como traducible, sólo enciérrela entre paréntesis:</p>

<pre>PRINT ("Tradúceme")
PRINT "¡Pero no me traduzcas a mi!"</pre>

[EVENT]

<p>Cada control, y cada objeto que puede lanzar eventos, tiene un <i>observador de eventos</i> y un <i>nombre de grupo</i> de evento.</p>

<p>El observador de eventos atrapa cada evento lanzado por el objeto, y el grupo de nombre es el prefijo del procedimiento llamado para manejar el evento.</p>

<p>Por defecto, este observador de eventos es el objeto donde usted creó el control, y el nombre de grupo es el nombre del control.</p>

<p>De esta forma, un formulario recibe todos los eventos lanzados por los controles que usted creó dentro de él.</p>

<pre>' Gambas form
DIM hButton AS Button

PUBLIC SUB _new()
&nbsp;&nbsp;hButton = NEW Button(ME) AS "MyButton"
END

PUBLIC SUB MyButton_Click()
&nbsp;&nbsp;PRINT "¡Usted ha presionado MyButton!"
END
</pre>

[FORM]

<p>En <b>Gambas</b>, un formulario es su propio observador de eventos, así que usted puede manejar sus eventos directamente (como <i>Resize</i>, <i>Activate</i>, ...) dentro su propio código de clase.</p>

<p>De esta forma, los novatos que vengan de <i>Visual Basic&trade;</i> no estarán desorientados :-).</p>

[EMBED]

<p>¡Usted puede incrustar cualquier formulario dentro de otros con <b>Gambas</b>!</p>

<p>Para hacer una cosa tan poderosa, sólo instancie el formulario pasando un contenedor padre como último argumento del constructor.</p>

<p>Por ejemplo:</p>
<p><tt>DIM hForm AS MyDialog<br>
DIM hSuperControl AS MyForm<br><br>
' Crear un dialigo<br>
hForm = NEW MyDialog<br>
' Insertar un formulario dentro de éste dialogo<br>
' Note que éste formualario toma dos parámetros antes del contenedor<br>
hSuperControl = NEW MyForm(Param1, Param2, MyDialog)<br>
' Mover y redimensionar el formulario<br>
hSuperControl.Move(8, 8, 128, 64)<br>
</tt></p>

<p><b>Tenga cuidado:</b> un formulario incrustado dentro de otro es aún un formulario, y entonces es su propio observador de eventos.</p>

[GROUP]

<p>Cada control tiene una propiedad <i>(Group)</i>. Cuando ésta propiedad es configurada, el prefirjo del nombre del manejador de eventos es el nombre del grupo y no el nombre del control.</p>

<p>Supongamos que tiene un <i>Butón</i> llamado <b>btnAction</b> con el siguiente manejador de eventos <i>Click</i>:</p>

<pre>PUBLIC SUB btnAction_Click()</pre>

<p>Si usted configura la propiedad <i>(Group)</i> de <b>btnAction</b> a <i>MyGroup</i>, entonces el manejador de eventos que recibirá los eventos del botón será el siguiente:</p>

<pre>PUBLIC SUB MyGroup_Click()</pre>

<p>Esta propiedad permite manejar eventos de diferentes controles en una función. ¡Y los controles del mismo nombre de grupo no tienen que ser del mismo tipo!</p>

<p><b>Nota:</b> Los viejos veteranos de <i>Visual Basic&trade;</i> podrían reconocer el concepto de <i>arreglo de controles</i>, pero en una implementación más poderosa. :-)</p>

[TAG]

<p>Cada control tiene una propiedad <i>Tag</i>. Ésta propiedad es para el programador, y puede contener cualquier dato de tipo <b>VARIANT</b> que usted pueda encontrar relevante.</p>

<p>Ésto es muy útil, cuando quiere distinguir controles del mismo grupo en un manejador de eventos común.</p>

[LAST]

<p>La palabra clave <b>LAST</b> retorna el último control que ha recibido un evento. Ésto es muy últil cuando usted quiere escribir un manejador de eventos que es independiente de cada nombre de control.</p>

<p>Por ejemplo, supongamos que usted quiere escribir un programa de una calculadora. Usted ha definido diez botones, uno para cada dígito, cada uno en el mismo <i>group</i> "Digit". El <i>Tag</i> de cada control es configurado para el dígito dibujado en el botón. Su manejador de eventos podría lucir así:</p>

<p><tt>PUBLIC SUB Digit_Click()<br><br>
&nbsp;&nbsp;Display = Display & LAST.Tag<br>
&nbsp;&nbsp;RefreshDisplay<br><br>
END</tt></p>


[LEFT]

<p>Las bien conocidas rutinas <i>BASIC</i> <b>Left$</b>, <b>Right$</b> y <b>Mid$</b> tienen conductas útiles en <b>Gambas</b>.</p>

<p>El segundo parámetro de <b>Left$</b> y <b>Right$</b> es opcional, y es uno por defecto.</p>

<p><tt>Left$("Gambas")</tt> retorna <tt>"G"</tt><br> <tt>Right$("Gambas")</tt> retorna <tt>"s"</tt></p>

<p>El segundo parámetro puede ser negativo, y entonces da el número de caracteres a no extraer.</p>

<p><tt>Left$("Gambas", -2)</tt> retorna <tt>"Gamb"</tt><br> <tt>Right$("Gambas", -2)</tt> retorna <tt>"mbas"</tt></p>

<p>Asimismo, el tercer argumento de <b>Mid$</b> puede ser negativo, y entonces da el número de caracteres desde el final de la cadena a no extraer.</p>

<p><tt>Mid$("Gambas", 2, -2)</tt> retorna <tt>"amb"</tt>

[OBSERVER]

<p>El clase <b>Observer</b> le permite interceptar todos los eventos lanzados por un objeto antes que de que sean enviados.</p>

<pre>MyTextBox = NEW TextBox(ME) AS "MyTextBox"
MyObserver = NEW Observer(MyTextBox) AS "MyObserver"
...
PUBLIC SUB MyObserver_KeyPress()
  DEBUG "Tengo ésto primero"
END

PUBLIC SUB MyTextBox_KeyPress()
  DEBUG "Tengo ésto después"
END</pre>

<p>El observador puede cancelar el evento para prevenir que el objeto eventualmente lo lance.</p>

[STRING]

<p><b>Gambas</b> usa el juego de caracteres <b>UTF-8</b> para representar cadenas en la memoria.</p>

<p>Pero todas las funciones de cadena estándar de <b>Gambas</b> trabajan con <b>ASCII</b>: <tt>Left</tt>, <tt>Mid</tt>, <tt>Right</tt>, <tt>UCase</tt>...</p>

<p>Si quiere manipular una cadena UTF-8, tiene que usar los métodos de la clase estática <b>String</b>, la cual tiene el mismo nombre de sus homólogos estándar.</p>

<pre>PRINT Len("bébé");; Left$("bébé", 3)
6 bé
PRINT String.Len("bébé");; String.Left("bébé", 3)
4 béb</pre>

[ASSIGNMENT]

<p><b>Gambas</b> implementa los atajos de asignación a los que los programadores de C/C++ están acostumbrados.</p>

<p><tt>MyVariable += 2</tt> es equivalente a <tt>MyVariable = MyVariable + 2</tt></p>

<p><tt>MyVariable &= "Great"</tt> es equivalente a <tt>MyVariable = MyVariable & "Great"</tt></p>

<p>Así consecutivamente...</p>

[DEBUG]

<p>Usted puede usar la instrucción <b>DEBUG</b> para imprimir mensajes de depurado en la consola (llamada la salida de errores estándar). Ésta se comporta exactamente como la instrucción <tt>PRINT</tt>.</p>

<p>Los mensajes tienen el prefijo del nombre de la clase, nombre del método y número de línea de la instrucción <tt>DEBUG</tt>.</p>

<p>Los mensajes de depurado son automáticamente removidos cuando se crea un ejecutable sin la información del depurado.</p>

[TRY]

<p>El manejo de Errores en <b>Gambas</b> es echo con las siguientes instrucciones: <b><tt>TRY</tt></b>, <b><tt>ERROR</tt></b>, <tt>CATCH</tt>, y <tt>FINALLY</tt.</p>

<p><tt>TRY</tt> trata de ejecutar una sentencia sin lanzar un error. La instrucción <tt>ERROR</tt> es usada justo después para saber si la instrucción fue ejecutada correctamente.</p>

<pre>TRY MyFile = OPEN "/etc/password" FOR WRITE
IF ERROR THEN PRINT "¡No puedo hacer lo que quiero!"</pre>

[CATCH]

<p>El manejo de Errores en <b>Gambas</b> es echo con las siguientes instrucciones: <tt>TRY</tt>, <tt>ERROR</tt>, <b><tt>CATCH</tt></b>, y <tt>FINALLY</tt>.</p>

<p><tt>CATCH</tt> indica el inicio de la parte de manejo de errores de una función o procedimiento. Es puesto al final del código de la función.</p>

<p>La parte catch es ejecutada cuando un error es lanzado entre el inicio de la ejecución de la función y su final.</p>

<p>Si un error es lanzado durante la ejecución de la parte catch, éste es normalmente propagado.</p>

<pre>SUB ProcessFile(FileName AS STRING)
  ...
  OPEN FileName FOR READ AS #hFile
  ...
  CLOSE #hFile
CATCH ' Ejecutado sólo si hay un error
  PRINT "No se puede procesar el archivo "; FileName
END</pre>

[FINALLY]

<p>El manejo de Errores en <b>Gambas</b> es echo con las siguientes instrucciones: <tt>TRY</tt>, <tt>ERROR</tt>, <tt>CATCH</tt>, y <b><tt>FINALLY</tt></b>.</p>

<p><tt>FINALLY</tt> introduce el código ejecutado al final de la función, aunque un error haya sido lanzado durante su ejecución.</p>

<p>La parte finally no es mandatoria. Si hay una parte catch en la función, la parte finally debe de precederle.</p>

<p>Si un error es lanzado durante la ejecución de la parte finally, éste es normalmente propagado.</p>

<pre>SUB ProcessFile(FileName AS STRING)
  ...
  OPEN FileName FOR READ AS #hFile
  ...
FINALLY ' Siempre ejecutada, aunque un error sea lanzado
  CLOSE #hFile
CATCH ' Ejecutado sólo si hay un error
  PRINT "No se puede imprimir el archivo "; FileName
END</pre>

[END]

<p>Usted ha leído todos los consejos de los días. ¡Espero que ahora se haya convertido en en experto en <b>Gambas</b>! :-)</p>

<p>Si quiere contribuir, envíe nuevos consejos a la siguiente dirección:</p>

<p><u>gambas@users.sourceforge.net</u></p>