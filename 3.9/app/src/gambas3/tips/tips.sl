[WELCOME]

<p>Dobrodošli v <b>Gambas</b>!</p>

<p><b>Gambas</b> je grafično razvojno okolje
temelječe na <i>Basic</i> interpreterju.</p>

<p>Cilj <b>Gambas</b>a je omogočiti uporabniku hitro in
enostavno izdelavo zmogljivih programov. Čista in lepa koda seveda
ostaja <i>tvoja</i> odgovornost...</p>

<p>Uživaj!</p>

<p align=right>Beno&icirc;t Minisini<br>
<u>gambas@users.sourceforge.net</u></p>


[STARTUP]

<p>Vsak program mora imeti <i>zagonski razred</i>. V njem mora biti
določena javna metoda z imenom <i>Main</i> brez argumentov, ki bo
služila kot zagonska metoda programa.</p>

<p>Zagonski razred lahko določiš z desnim klikom na razred v drevesu projekta
in nato v meniju izbereš <i>Zagonski razred</i>.</p>

<p>Določitev metode <i>Main</i> v zagonski formi ni nujna, ker je ta metoda že vnaprej določena.</p>

<p>Ta vnaprej določena metoda instancira in prikaže formo, prav kakor v
<i>Visual Basicu&trade;</i>.</p>


[EXEC]

<p>Iz celotnega projekta lahko izdelaš izvršljivo datoteko. V meniju
<i>Projekt</i> izberi <i>Izdelaj izvršljivo datoteko</i>.</p>

<p><b>Gambas</b> bo izdelal izvršlivo datoteko in jo zapisal v mapo, kjer je shranjen projekt.
Izvršljiva datoteka ima enako ime kot projekt.</p>

[PATH]
<p>
Relativne poti imajo v <b>Gambas</i>u poseben pomen.
Vedno se nanašajo na datoteke znotraj projektov.
<p>
Koncepta <i>trenutne mape</i> ni, kot tudi ni ključne besede <tt>CHDIR</tt> za njeno spreminjanje.
<p>
<b>Pozor:</b> relativne poti uporabljaj samo za dostop do datotek projekta, ker absolutne poti ne bodo več delovale, ko bo izdelana izvršljiva datoteka.


[GLOBAL]

V <b><i>Gambas</i></b>u <u>ni globalnih spremenljivk</u>!
<p>
To lahko obideš tako, da spremenljivke najaviš v glavnem modulu kot <tt>PUBLIC</tt>.
<p>
Če projekt nima glavnega modula, ima pa glavno formo, jih najavi tam kot <tt>STATIC PUBLIC</tt>.
<p>
Za dostopanje do takšnih spremenljivk moraš uporabiti ime modula ali forme: <tt>MojGlavniModule.MojaGlobalnaSpremenljivka</tt> ali
<tt>MojaGlavnaForma.MojaGlobalnaSpremenljivka</tt>.


[OPEN]

<p>Ukaz <b>OPEN</b> v <b>Gambas</b>u ne deluje popolnoma enako kot v <i>Visual Basicu</i>, 
ker ne vrača vrednosti tipa integer, ampak objekt tipa <i>File</i>.</p>

<p>Tako je treba namesto</p>

<pre>DIM handle AS Integer
...
OPEN "mojaDatoteka" FOR READ AS #handle</pre>

<p>napisati</p>

<pre>DIM handle AS File
...
handle = OPEN "mojaDatoteka" FOR READ</pre>


[CATDIR]

<p>Ali veš, da lahko združuješ imena map z operatorjem <b><tt>&/</tt></b>?
 Ta operator poskrbi za vodilne in sledilne poševnice tako, da je sestavljena pot vedno pravilna.</p>

<p>Na primer:</p>

<pre>PRINT "/home/gambas" &/ ".bashrc"
/home/gambas/.bashrc

PRINT "/home/gambas/" &/ "/tmp" &/ "foo.bar"
/home/gambas/tmp/foo.bar
</pre>

<p>Lepo, ne?</p>


[EMPTY]

<p>Za ugotavljanje ali je niz prazen, ni treba uporabiti funkcije <b>Len()</b>.
Ali je niz prazen, lahko preveriš neposredno, saj prazen niz vrne <b>FALSE</b>, neprazen pa <b>TRUE</b>.</p>

<p>Namesto</p>

<pre>IF Len(mojNiz) > 0 THEN ...
IF Len(mojNiz) = 0 THEN ...</pre>

<p>je tako bolje napisati</p>

<pre>IF mojNiz THEN ...
IF NOT mojNiz THEN ...</pre>


[EVENT]

<p>Vsaka kontrola in vsak objekt, ki lahko sproža dogodke ima svoj
<i>prestreznik dogodkov (event observer)</i> in svoje <i>ime skupine dogodkov (event group name)</i>.</p>

<p>Prestreznik dogodkov prestreže vsak dogodek, ki ga sproži objekt, ime skupine dogodkov pa je predpona procedure, ki jo kličemo za upravljanje dogodka.</p>

<p>Privzeto je, da je prestreznik dogodkov objekt, ki vsebuje kontrolo, ime skupine dogodkov pa je enako imenu kontrole.</p>

<p>Na ta način lahko npr. forma prestreže vse dogodke, ki jih sprožijo kontrole na njej.</p>

<pre>' Gambas forma
DIM hButton AS Button

PUBLIC SUB _new()
&nbsp;&nbsp;hButton = NEW Button(ME) AS "MyButton"
END

PUBLIC SUB MyButton_Click()
&nbsp;&nbsp;PRINT "MyButton pritisnjen!"
END
</pre>


[FORM]

<p>V <b><i>Gambas</i></b>u predstavlja forma svoj lastni prestreznik dogodkov. Tako je možno neposredno upravljanje njenih dogodkov (kot so <i>Resize</i>, <i>Activate</i> itd.) v kodi njenega razreda.</p>

<p>Tako se pribežniki iz <i>Visual Basica</i> ne počutijo izgubljene. :-)</p>


[EMBED]

<p>V <b><i>Gambas</i></b>u je možno katerokoli formo vstaviti v druge forme.&nbsp;</p>

<p>Za izvedbo tega je treba instancirati formo tako, da ji podamo nadvsebnik kot zadnji argument konstruktorja.</p>

<p>Na primer:&nbsp;</p>
<p><tt>DIM hForm AS MyDialog<br>
DIM hSuperControl AS MyForm<br><br>
' Ustvarimo pogovorno okno<br>
hForm = NEW MyDialog<br>
' Vstavimo formo v to pogovorno okno<br>
' (Forma ima pred vsebnikom še dva druga parametra)<br>
hSuperControl = NEW MyForm(Param1, Param2, MyDialog)<br>
' Premaknimo in razvlečimo formo<br>
hSuperControl.Move(8, 8, 128, 64)<br>
</tt></p>

<p>Pozor! Forma, vstavljena v drugo formo, je še vedno forma in je zato tudi svoj lastni prestreznik dogodkov.</p>


[GROUP]

<p>Vsaka kontrola ima lastnost <i>(Group)</i>. Ko nastavimo to lastnost, je predpona upravljalca dogodkov ime skupine in ne več ime kontrole.</p>

<p>Imejmo na primer <i>gumb</i> z imenom <b>btnAction</b>
z naslednjim upravljalcem dogodka <i>Click</i>:</p>

<pre>PUBLIC SUB btnAction_Click()</pre>

<p>Če postavimo lastnost <i>(Group)</i> kontrole <b>btnAction</b> na
<i>MyGroup</i>, bo upravljalec dogodkov, ki bo sprejemal dogodke z našega gumba naslednji:</p>

<pre>PUBLIC SUB MyGroup_Click()</pre>

<p>Ta lastnost omogoča upravljanje dogodkov z različnih kontrol v eni sami funkciji. Pri tem ni treba, da so kontrole istega tipa.</p>

<p><b>Opomba:</b> Izkušeni <i>Visual Basic</i> veteran bo omenjeno prepoznal kot koncept <i>polja kontrol (control array)</i>, vendar v mnogo močnejši izvedbi. :-)</p>


[TAG]

<p>Vsaka kontrola ima lastnost <i>Tag</i>. Ta lastnost je namenjena programerju in lahko vsebuje kakršenkoli podatek tipa <b>VARIANT</b>, ki je pomemben za program.</p>

<p>To je zelo uporabno, kadar je treba razlikovati kontrole znotraj skupine pri uporabi skupnega upravljalca dogodkov.</p>



[LAST]

<p>Ključna beseda <b>LAST</b> vrne zadnjo kontrolo, ki je prejela dogodek.
To je zelo uporabno, kadar želimo napisati upravljalca dogodkov, ki naj bo neodvisen od imena kontrole.</p>

<p>Vzemimo, da želimo izdelati kalkulator. Določimo deset gumbov, svojega za vsako številko, in jim določimo <i>skupino</i> "Digit". Lastnost <i>Tag</i> za vsak gumb je številka, napisana na njem. Upravljalec dogodka za takšno skupino gumbov je tedaj lahko videti takole:</p>

<p><tt>PUBLIC SUB Digit_Click()<br><br>
&nbsp;&nbsp;Display = Display & LAST.Tag<br>
&nbsp;&nbsp;RefreshDisplay<br><br>
END</tt></p>


[LEFT]

<p>Dobro znane <i>BASIC</i> funkcije <b>Left$</b>, <b>Right$</b>
in <b>Mid$</b> imajo v <b><i>Gambas</i></b>u dodatno uporabnost.</p>

<p>Drugi parameter pri <b>Left$</b> in <b>Right$</b> je neobvezen in ima privzeto vrednost 1:</p>

<p><tt>Left$("Gambas")</tt> vrne <tt>"G"</tt><br>
<tt>Right$("Gambas")</tt> vrne <tt>"s"</tt></p>

<p>Drugi parameter ima lahko negativno vrednost. Tedaj predstavlja število znakov, ki naj jih funkcija ne vrne:</p>

<p><tt>Left$("Gambas", -2)</tt> vrne <tt>"Gamb"</tt><br>
<tt>Right$("Gambas", -2)</tt> vrne <tt>"mbas"</tt></p>

<p>Negativen je lahko tudi tretji argument pri <b>Mid$</b>. Tedaj podaja število znakov s konca niza, ki naj jih funkcija ne vrne:</p>

<p><tt>Mid$("Gambas", 2, -2)</tt> vrne <tt>"amb"</tt>


[END]

<p>Tukaj se Nasveti dneva končajo. Upam, da si po njihovem branju izvedenec za <b>Gambas</b>! :-)</p>

<p>Če želiš prispevati, pošlji nove nasvete na naslov:&nbsp;</p>
<p><u>gambas@users.sourceforge.net</u></p>

<p>Hvala v naprej!</p>



