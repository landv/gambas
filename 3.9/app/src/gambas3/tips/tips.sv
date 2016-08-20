[WELCOME]

<p>Välkommen till <b>Gambas</b> !</p>

<p><b>Gambas</b> är en grafiskt utvecklingsmiljö
grundad på en avancerad <i>Basic</i> interpretator.</p>

<p><b>Gambas</b> mål är att få dig att göra
kraftfulla program, enkelt och snabbt. Men rena strukturerade
program är ditt <i>eget</i> ansvar...</p>

<p>Lycka till!</p>

<p align=right>Beno&icirc;t Minisini<br>
<u>gambas@users.sourceforge.net</u></p>


[STARTUP]

<p>Varje projekt måste ha en <i>startup class</i>. Denna
startup-class måste definiera en statisk publik metod kallad <i>Main</i>
utan argument, som kommer att fungera som uppstartmetod för
ditt program.</p>

<p>Du kan definiera startup-classen genom att klicka på den med
höger musknapp i projektträdet och genom att välja 
<i>Startup class</i> i popup-menyn.</p>

<p>Det är inte nödvändigt att definiera en <i>Main</i>-metod i ett startupp-
formulär, ty det har redan en fördefinierad sådan.</p>

<p>Denna fördefinierade uppstartmetod instansierar formuläret och visar det,
som i <i>Visual Basic&trade;</i>.</p>


[OPEN]

<p><b>OPEN</b>-instruktionen i <b>Gambas</b> fungerar inte på
samma sätt som den i <i>Visual Basic</i>. Den lämnar inte filen,
som ett heltal, utan som ett <i>File</i>-objekt.</p>

<p>Så i stället för att skriva:</p>

<pre>DIM handle AS Integer
...
OPEN "myfile" FOR READ AS #handle</pre>

<p>måste du skriva:</p>

<pre>DIM handle AS File
...
handle = OPEN "myfile" FOR READ</pre>


[CATDIR]

<p>Vet du att du kan slå ihop mappnamn och
filnamn med <b><tt>&/</tt></b>-operatorn? Denna operator
tar hand om avslutande snedstreck "/" så att den resulterande sökvägen
blir perfekt.</p>

<p>Till exempel:</p>

<pre>PRINT "/home/gambas" &/ ".bashrc"
/home/gambas/.bashrc

PRINT "/home/gambas/" &/ "/tmp" &/ "foo.bar"
/home/gambas/tmp/foo.bar
</pre>

<p>Är det inte förunderligt?</p>


[EXEC]

<p>Du kan göra en körbar fil utgående från ditt hela projekt. Välj
<i>Gör exekverbar</i> i <i>Projekt</i>-menyn.</p>

<p>När <b>Gambas</b> gör en körbar fil, läggs 
resultatet i din projektmapp som standard. 
Den körbara filen får samma namn som ditt projekt.</p>


[PATH]
<p>
Relativa sökvägar har speciell betydelse i <b><i>Gambas</i></b>.
De refererar alltid till filer inuti ditt projekt.
<p>
Det finns ingen innebörd i <i>current directory</i> och inget nyckelord som
<tt>CHDIR</tt> att ändra det.
<p>
<b>Var försiktig:</b> du måste använda relativ sökväg endast för att komma åt
projektfiler, ty absoluta sökvägar fungerar inte längre, när du gör en körbar fil.


[GLOBAL]

Det finns <u>inga globala variabler</u> i <b><i>Gambas</i></b>!
<p>
Ett sätt att kringgå detta är att placera dem i din huvud/main-modul
och deklarera dem som <tt>PUBLIC</tt>.
<p>
Om du inte har en huvud/main-modul i ditt projekt, men ett huvud/main-
formulär, deklarera dem som <tt>STATIC PUBLIC</tt>.
<p>
För att komma åt dessa variabler, måste du använda namnet på huvudmodulen
eller huvudformuläret: <tt>MinMainModul.MinGlobalaVariabel</tt> or
<tt>MinMainForm.MinGlobalaVariabel</tt>.


[EMPTY]

<p>För att veta om en sträng är tom, är det inte nödvändigt att använda
<b>Len()</b>-funktionen. Du kan testa den direkt, eftersom en tom sträng
är <b>FALSE</b> och en icke-tom sträng är <b>TRUE</b>.</p>

<p>Till exempel, i stället för att göra:</p>

<pre>IF Len(MyString) > 0 THEN ...
IF Len(MyString) = 0 THEN ...</pre>

<p>Bör du göra:</p>

<pre>IF MyString THEN ...
IF NOT MyString THEN ...</pre>


[TRANSLATE]

<p>Gambastillämpningar är helt och hållet översättningsbara, förutsatt att du
talar om vilka strängar som skall översättas.</p>
<p>För att markera att en sträng är översättningsbar, så sätt den bara inom parantester:<p>

<pre>PRINT ("Översätt mig")
PRINT "Men inte mig!"</pre>

[EVENT]

<p>Varje kontroll och varje objekt, som kan ge avbrott, har en
<i>avbrottshanterare</i> och ett avbrotts-<i>group name</i>.</p>

<p>Avbrottshanteraren fångar varje avbrott åstadkommet av objektet och
avbrottsgruppnamnet är prefix för den procedur, som anropas för att hantera
avbrottet.</p>

<p>Som standard är denna avbrottshanterare det objekt där du har
skapat kontrollen och gruppnamnet är samma som kontrollens.</p>

<p>På detta sätt får ett formulär mottaga alla avbrott som åstadkommits
av de kontroller du skapade inuti.</p>

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

<p>I <b><i>Gambas</i></b> är ett formulär sin egen avbrottshanterare, så att
du direkt kan hantera dess avbrott (som <i>Resize</i>, 
<i>Activate</i>, ...) in till dess egen klasskod.</p>

<p>På detta sätt blir nybörjare, som kommer från <i>Visual Basic</i> inte
disorienterade :-).</p>


[EMBED]

<p>Du kan inbädda vilket formulär som helst i andra formulär med <b><i>Gambas</i></b>
&nbsp;!</p>

<p>För att åstadkomma en så kraftfull sak, bara instanciera formuläret genom att skicka
ett förälderbehållare (parent container) som sista argument hos konstruktorn (contructor).</p>

<p>Till exempel&nbsp;:</p>
<p><tt>DIM hForm AS MyDialog<br>
DIM hSuperControl AS MyForm<br><br>
' Skapa en dialog<br>
hForm = NEW MyDialog<br>
' Tag in ett formulär i denna dialog<br>
' Notera att detta formulär tar två parametrar före behållaren (container)<br>
hSuperControl = NEW MyForm(Param1, Param2, MyDialog)<br>
' Flytta och justera storlek på formuläret<br>
hSuperControl.Move(8, 8, 128, 64)<br>
</tt></p>

<p>Var uppmärksam: ett formulär inbäddat i ett annat är fortfarande ett formulär och
dessutom sin egen avbrottshanterare.</p>


[GROUP]

<p>Varje kontroll har en <i>(Group)</i>-egenskap. När denna egenskap
sätts, är prefixet för avbrottshanterarens namn, namnet på gruppen
och inte namnet på kontrollen.</p>

<p>Låt oss anta att vi har en <i>Knapp</i> kallad <b>btnAction</b>
med följande <i>Click</i> avbrottshanterare:</p>

<pre>PUBLIC SUB btnAction_Click()</pre>

<p>Om du sätter <i>(Group)</i>-egenskapen hos <b>btnAction</b> till
<i>MyGroup</i>, så kommer avbrotsshanteraren att mottaga avbrott från
knappen att se ut som:</p>

<pre>PUBLIC SUB MyGroup_Click()</pre>

<p>Denna egenskap låter dig hantera avbrott från skilda kontroller med
en enkel funktion och kontrollerna i samma grupp behöver inte ha samma typ!</p>

<p><b>Notera :</b> Den gamle <i>Visual Basic</i>-veteranen kan kanske känna igen
konceptet i <i>kontroll array</i>, men i en kraftfullare implementering. :-)</p>


[TAG]

<p>Varje kontroll har en <i>Tag</i>-egenskap. Denna är till för
programmeraren och kan innehålla vilken <b>VARIANT</b>-data som du tycker är relevant.</p>

<p>Detta är mycket användbart, när du vill åtskilja kontroller i
samma grupp i en gemensam avbrottshanterare.</p>



[LAST]

<p>Nyckelordet <b>LAST</b> returnerar den sista kontroll, som
har mottagit ett avbrott. Detta är mycket användbart om du vill skriva en
avbrottshanterare, som är oberoende av namn på kontroller.</p>

<p>Till exempel, låt oss anta du vill skriva ett kalkylatorprogram.
Du har definierat tio knappar, en för varje siffra, var och en i 
samma <i>group</i> "Digit". <i>Tag</i>-en för varje kontroll sätts till
siffran ritad i knappen. Din avbrottshanterare skulle kunna se ut som:</p>

<p><tt>PUBLIC SUB Digit_Click()<br><br>
&nbsp;&nbsp;Display = Display & LAST.Tag<br>
&nbsp;&nbsp;RefreshDisplay<br><br>
END</tt></p>


[LEFT]

<p>De välkända <i>BASIC</i> rutinerna <b>Left$</b>, <b>Right$</b>
och <b>Mid$</b> har användbara uppträdande i <b><i>Gambas</i></b></p>

<p>Andra parametern i <b>Left$</b> och <b>Right$</b> är
frivillig och sätts till ett som standard.</p>

<p><tt>Left$("Gambas")</tt> returnerar <tt>"G"</tt><br>
<tt>Right$("Gambas")</tt> returnerar <tt>"s"</tt></p>

<p>Andra parametern kan vara negativ och betyder då antal
tecken som ej skall tas ut.</p>

<p><tt>Left$("Gambas", -2)</tt> returnerar <tt>"Gamb"</tt><br>
<tt>Right$("Gambas", -2)</tt> returnerar <tt>"mbas"</tt></p>

<p>På samma sätt med tredje argumentet till <b>Mid$</b> kan vara negativt och
ger då antal tecken från slutet på strängen som ej skall tas ut.</p>

<p><tt>Mid$("Gambas", 2, -2)</tt> returnerar <tt>"amb"</tt>

[OBSERVER]

<p><b>Observer</b>-klassen tillåter dig att fånga upp alla händelser skapade
av ett objekt innan de verkligen sänds.</p>

<pre>MyTextBox = NEW TextBox(ME) AS "MyTextBox"
MyObserver = NEW Observer(MyTextBox) AS "MyObserver"
...
PUBLIC SUB MyObserver_KeyPress()
  DEBUG "Got it first"
END

PUBLIC SUB MyTextBox_KeyPress()
  DEBUG "Got it next"
END</pre>

"Observatören" kan avbryta händelsen för att förhindra att objektet att åstadkommer den (händelsen).


[STRING]

<p>Gambas använder <b>UTF-8</b>-teckensätt för att representera strängar i minnet.</p>

<p>Men alla Gambas standardsträngfunktioner hanterar <b>ASCII</b>: 
<tt>Left</tt>, <tt>Mid</tt>, <tt>Right</tt>, <tt>UCase</tt>...

<p>Om du vill manipulera UTF-8-strängar, måste du använda metoder hörande till
<b>String</b>-static klass, vilka har samma namn som deras motsvarande standard.

<pre>PRINT Len("bébé");; Left$("bébé", 3)
6 bé
PRINT String.Len("bébé");; String.Left("bébé", 3)
4 béb</pre>


[ASSIGNMENT]

<p>Gambas implementerar förenklad tilldelning, som C/C++ programmerare är vana vid.

<p><tt>MyVariable += 2</tt> är samma sak som <tt>MyVariable = MyVariable + 2</tt>

<p><tt>MyVariable &= "Great"</tt> är samma sak som <tt>MyVariable = MyVariable & "Great"</tt>

<p>och så vidare...


[DEBUG]

<p>Du kan använda <b>DEBUG</b>-instruktionen till att skriva ut avlusningsmeddelandem
till konsolen (nämligen standard error output). Den uppför sig precis som <tt>PRINT</tt>-
instruktionen.

<p>Dessa meddelanden föregås av klassnamn, metodnamn och radnummer för
<tt>DEBUG</tt>-instruktionen.

<p>Debugmeddelandena tas automatiskt bort när en exekverbar fil
utan avlusningsinformation görs.


[TRY]

<p>Felhantering i Gambas görs med följande instruktioner: 
<b><tt>TRY</tt></b>, <b><tt>ERROR</tt></b>, <tt>CATCH</tt> och <tt>FINALLY</tt>.

<p><tt>TRY</tt> försöker utföra en sats utan att åstadkomma ett fel. <tt>ERROR</tt>-
instruktionen används just för att ta reda på om satsen utfördes korrekt.

<pre>TRY MyFile = OPEN "/etc/password" FOR WRITE
IF ERROR THEN PRINT "I cannot do what I want!"</pre>


[CATCH]

<p>Felhantering i Gambas görs med följande instruktioner: 
<tt>TRY</tt>, <tt>ERROR</tt>, <b><tt>CATCH</tt></b> och <tt>FINALLY</tt>.

<p><tt>CATCH</tt> utvisar början på felhanteringsdelen i en funktion eller procedur. 
Den placeras i slutet på funktionskoden.

<p>Catch-delen utförs när ett fel har åstadkommits mellan början på funktionen och dess slut.


<p>Om ett fel inträffar under utförande av catch-delen, fångas den. 

<pre>SUB ProcessFile(FileName AS STRING)
  ...
  OPEN FileName FOR READ AS #hFile
  ...
  CLOSE #hFile
CATCH ' Utförs endast om det finns ett fel
  PRINT "Cannot process file "; FileName
END</pre>


[FINALLY]

<p>Felhantering i Gambas görs med följande instruktioner: 
<tt>TRY</tt>, <tt>ERROR</tt>, <tt>CATCH</tt> och <b><tt>FINALLY</tt></b>.

<p><tt>FINALLY</tt> inleder den kod, som utförs vid slutet av funktionen, även om ett fel åstadkomms under funktionsutförandet. 

<p>Finally-delen är inte obligatorisk. Om det finns en catch-del, så måste finally-delen föregå den. 
 
<p>Om ett fel inträffar under utförande av finally-delen, fortsättes normalt.

<pre>SUB ProcessFile(FileName AS STRING)
  ...
  OPEN FileName FOR READ AS #hFile
  ...
FINALLY ' Utförs alltid, även om fel inträffat
  CLOSE #hFile
CATCH ' Utförs endast om fel inträffat
  PRINT "Cannot print file "; FileName
END</pre>



[END]

<p>Du har nu lästa alla Dagens Tips. Hoppas att du blivit
en <b>Gambas</b>-expert nu! :-)</p>

<p>Om du vill bidraga, sänd nya tips till följande
adress&nbsp;:</p>
<p><u>gambas@users.sourceforge.net</u></p>

<p>Tack på förhand!</p>



