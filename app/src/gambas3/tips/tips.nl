[WELCOME]

<p>Welkom bij <b>Gambas</b> !</p>

<p><b>Gambas</b> is een grafisch ontwikkel omgeving gebaseerd op een
geavanceerdd <i>Basic</i> interpreter.</p>

<p><b>Gambas</b> is geschreven om eenvoudig en snel
krachtige applicaties te ontwikkelen. Het bouwen van de 
programmacode blijft je <i>eigen</i> verantwoordelijkheid...</p>

<p>Veel plezier ermee !</p>

<p align=right>Beno&icirc;t Minisini<br>
<u>gambas@users.sourceforge.net</u></p>


[STARTUP]

<p>Elk project moet een <i>startup class</i> hebben. Deze 
startup class moet een piblieke methode <i>Main</i> hebben,
zonder argumenten. Deze Main methode fungeerd als 
de opstart methode voor de applicatie.</p>

<p>Je kunt de startup class definieren door in de projectboom 
met de rechter muisknop op het object te klikken en 
startup class in het popup menu te selecteren</p>



<p>Het is niet nodig om een <i>Main</i> methode in een opstart
formulier te declareren, omdat het opstart formulier al een <i>Main</i> methode bevat.</p>

<p>Deze reeds gedefinieerde Main methode initieerd het formulier, net zoals in 
<i>Visual Basic&trade;</i>.</p>


[OPEN]

<p>De <b>OPEN</b> instructie van <b>Gambas</b> werkt niet zoals de <i>Visual Basic</i> variant.
Het retourneerd geen integer, maar een <i>File</i> object.</p>

<p>Dus in plaats van:</p>

<pre>DIM handle AS Integer
...
OPEN "mijnfile" FOR READ AS #handle</pre>

<p>gebruik je :</p>

<pre>DIM handle AS File
...
handle = OPEN "mijnfile" FOR READ</pre>


[CATDIR]

<p>Weet je dat je directory- en filenamen kunt koppelen met de <b><tt>&/</tt></b> operator ?
Deze operator lost alle problemen met eindigende slashes voor je op.</p>

<p>Bijvoorbeeld:</p>

<pre>PRINT "/home/gambas" &/ ".bashrc"
/home/gambas/.bashrc

PRINT "/home/gambas/" &/ "/tmp" &/ "foo.bar"
/home/gambas/tmp/foo.bar
</pre>

<p>Is dit niet fantastisch ?</p>


[EXEC]

<p>Je kunt een 'executable' file (binairy) maken van je gehele project.
Selecteer <i>Make executable</i> in het <i>Project</i> menu.</p>

<p>Als <b>Gambas</b> een binairy file voor je maakt, plaatst het
het resultaat in de standaard directory van je project.
Deze executable krijgt dezelfde naam als het project.</p>


[PATH]
<p>
Relatieve paden hebben altijd een speciale betekenis in <b><i>Gambas</i></b>.
Ze refereren altijd naar bestanden in je project.
<p>
Er is geen concept als <i>current directory</i>, en geen commando als
<tt>CHDIR</tt> om het te veranderen.
<p>
<b>Wees voorzichtig:</b> je moet altijd relatieve paden gebruiken voor verwijzingen 
naar andere files in het project, omdat volledige paden niet meer werken als er 
een executable van het project gemaakt wordt.</p>

[GLOBAL]

Er zijn <u>geen globale variabelen</u> in <b><i>Gambas</i></b>!
<p>
Als oplossing kun je ze in je main module plaatsen en declareren als <tt>PUBLIC</tt>.
<p>
Als je geen main module hebt, maar een main form, declareer ze dan als <tt>STATIC PUBLIC</tt>.
<p>
Om bij deze variabelen te komen gebruik je de naam van de module: <tt>MijnMainModule.MijnGlobaleVariable</tt> of
<tt>MijnMainForm.MijnGlobaleVariable</tt>.


[EMPTY]

<p>Om te kijken of een string leeg is hoef je geen gebruik te maken van de
<b>Len()</b> functie. Je kunt het direct testen: een lege string
is <b>FALSE</b> en een niet-lege string is <b>TRUE</b>.</p>

<p>Bijvoorbeeld, in plaats van :</p>

<pre>IF Len(MijnString) > 0 THEN ...
IF Len(MijnString) = 0 THEN ...</pre>

<p>Kun je sneller :</p>

<pre>IF MijnString THEN ...
IF NOT MijnString THEN ...</pre>


[EVENT]

<p>Elke controller, en elk object dat een gebeurtenis (event) kan oproepen, heeft een 
<i>event observer</i> en een event <i>group name</i>.</p>

<p>De event observer ondervangt ieder event opgeroepen door het object, en 
de 'event group name' is het voorvoegsel van de procedure die opgeroepen wordt om het
event af te handelen.</p>

<p>Standaard is de <i>event observer</i> het object waar je de controller gemaakt hebt, en de 
group name is de naam van de controller.</p>

<p>Op deze manier ontvangt een formulier alle gebeurtenissen die door controllers op dit 
formulier gemaakt zijn.</p>

<pre>' Gambas form
DIM hButton AS Button

PUBLIC SUB _new()
&nbsp;&nbsp;hButton = NEW Button(ME) AS "MyButton"
END

PUBLIC SUB MyButton_Click()
&nbsp;&nbsp;PRINT "You have clicked MyButton !"
END
</pre>


[FORM]

<p>In <b><i>Gambas</i></b>, is een formulier zijn eigen <i>event observer</i>,
zodat je gebeurtenissen als <i>Resize</i>, 
<i>Activate</i>, ...) kunt afhandelen in zijn eigen class code.</p>

<p>Op deze manier worden de newbies van <i>Visual Basic</i> niet teveel
afgeschrikt :-).</p>


[EMBED]

<p>Je kunt bij <b><i>Gambas</i></b> een formulier in een ander formulier invoegen
&nbsp;!</p>

<p>Om zo'n krachtige actie te doen hoef je enkel een ouder container (parent) mee te geven 
als laatste parameter bij het oproepen van het formulier.</p>

<p>Bij voorbeeld&nbsp;:</p>
<p><tt>DIM hForm AS MijnDialog<br>
DIM hSuperControl AS MijnForm<br><br>
' Create a dialog<br>
hForm = NEW MijnDialog<br>
' Insert a form into this dialog<br>
' Note that this form takes two parameters before the container<br>
hSuperControl = NEW MijnForm(Param1, Param2, MijnDialog)<br>
' Moves and resizes the form<br>
hSuperControl.Move(8, 8, 128, 64)<br>
</tt></p>

<p>Let op: een formulier ingevoegd in een ander is nog steeds een formulier, 
met zijn eigen event observer. </p>


[GROUP]

<p>Elke controller heeft een <i>(Group)</i> eigenschap. 
Als deze eigenschap wordt gebruikt wordt de naam van de groep gebuikt voor het
afhandelen van de events, en niet de naam van de controller.
</p>

<p>Stel: we hebben een <i>Button</i> genaamd <b>btnAction</b>
Met de volgende <i>Click</i> event handler :</p>

<pre>PUBLIC SUB btnAction_Click()</pre>

<p>Als je de <i>(Group)</i> eigenschap van <b>btnAction</b> zet op
<i>MijnGroup</i>, dan wordt de event handlerdie alle events ontvangt:</p>

<pre>PUBLIC SUB MijnGroup_Click()</pre>

<p>Op deze manier kun je verschillende controllers dezelfde functie toekennen.
De controllers hoeven hiervoor niet eens van hetzelfde type te zijn!</p>

<p><b>NB :</b> Een oude <i>Visual Basic</i> veteraan zal het systeem van een 
<i>control array</i> herkennen, maar dan in een krachtigere uitvoering :-). </p>


[TAG]

<p>Elke controlller heeft een <i>Tag</i> eigenschap. Deze eigenschap is voor
de programmeur, en kan elke gewenste <b>VARIANT</b> gegevens bevatten die je
relevant vindt.</p>

<p>Deze eigenschap is erg handig om verschillende controllers die bij elkaar 
in dezelfde <i>group</i> zitten te onderscheiden 
binnen de gezamenlijke event handler van deze group.</p>



[LAST]

<p>Het <b>LAST</b> commando retourneerd de laatst opgeroepen controller. 
Dit is erg handis als je een event handler wilt schrijven die 
onafhankelijk is van de controller naam.</p>

<p>Bijvoorbeeld: Stel je wilt een rekenmachine programmeren.
Je hebt tien knoppen gedefinieerd, elk voor een cijfer, en de knoppen gehangen 
aan de <i>group</i> "Digit". De <i>Tag</i> van elke controlller is gelijk aan het 
getal op de button. Je event handler kan er dan zo uit zien :</p>

<p><tt>PUBLIC SUB Digit_Click()<br><br>
&nbsp;&nbsp;Display = Display & LAST.Tag<br>
&nbsp;&nbsp;RefreshDisplay<br><br>
END</tt></p>


[LEFT]

<p>De bekende <i>BASIC</i> routines <b>Left$</b>, <b>Right$</b>
en <b>Mid$</b> hebben handige functies in <b><i>Gambas</i></b></p>

<p>De tweede parameter van <b>Left$</b> en <b>Right$</b> is
optioneel, en is standaard 1.</p>

<p><tt>Left$("Gambas")</tt> retourneerd <tt>"G"</tt><br>
<tt>Right$("Gambas")</tt> retourneerd <tt>"s"</tt></p>

<p>De tweede parameter mag negatief zijn, en geeft dan het aantal karakters dat
je <u>niet</u> wilt.</p>

<p><tt>Left$("Gambas", -2)</tt> retourneerd <tt>"Gamb"</tt><br>
<tt>Right$("Gambas", -2)</tt> retourneerd <tt>"mbas"</tt></p>

<p>Vanzelfsprekend, het derde argument van <b>Mid$</b> mag negatief zijn, en geeft 
dan het aantal karakters vanaf het eind van de string dat genegeerd moet worden.</p>

<p><tt>Mid$("Gambas", 2, -2)</tt> retourneerd <tt>"amb"</tt>


[END]

<p>Je hebt alle tips van de dag gelezen. Ik hoop dat je nu een <b>Gambas</b> 
expert geworden bent ! :-)</p>

<p>Als je tips toe te voegen hebt, zend je nieuwe tips dan naar het volgende 
adres&nbsp;:</p>
<p><u>gambas@users.sourceforge.net</u></p>

<p>Alvast bedankt !</p>



