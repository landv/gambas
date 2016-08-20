[WELCOME]

<p>Vítej v <b>Gambas</b>u !</p>

<p><b>Gambas</b> je grafické vývojové
prostředí založené na pokročilém <i>Basic</i> interpretovy.</p>

<p>Cílem <b>Gambas</b>u je umožnit Vám, vytvářet
užitečné programy snadno a rychle. Ale čistota těchto programů
zůstává na <i>vaší</i> plné odpovědnosti...</p>

<p>Užijte si to !</p>

<p align=right>Beno&icirc;t Minisini<br>
<u>gambas@users.sourceforge.net</u></p>


[STARTUP]

<p>Každý projekt musí mít <i>hlavní třídu</i>. Hlavní třída definuje
veřejnou statickou metodu nazvanou <i>Main</i>, argumentů, 
která bude fungovat jako spouštěcí metoda vašeho programu.</p>

<p>Hlavní třídu můžete nastavit tak, že ve stromu projektu kliknete
pravým tlačítkem myši a v popup menu vyberete položku <i>Hlavní třída</i>.</p>

<p>Po startu není nutné nastavovat methodu <i>Main</i>, protože je již přednastavená.</p>

<p>Tato metoda je zodpovědná za vytváření instance a poté zobrazení formuláře, jako ve
<i>Visual Basic&trade;</i>.</p>


[OPEN]

<p>Instrukci <b>OPEN</b> nemůžete použít v <b>Gambas</b>u
stejně jako ve <i>Visual Basic</i>. Soubor totiž nevrácí integer,
ale objekt <i>File</i>.</p>

<p>Takže místo:</p>

<pre>DIM handle AS Integer
...
OPEN "myfile" FOR READ AS #handle</pre>

<p>musíte psát:</p>

<pre>DIM handle AS File
...
handle = OPEN "myfile" FOR READ</pre>


[CATDIR]

Víte, že můžete spojit názvy adresářů a souborů operatorem <b><tt>&/</tt></b> ?
<p>Tento operátor se v případě potřeby postará
o správné přidání oddělovače <tt>'/'</tt> do cesty.</p>

<p>Například:</p>

<pre>PRINT "/home/gambas" &/ ".bashrc"
/home/gambas/.bashrc

PRINT "/home/gambas/" &/ "/tmp" &/ "foo.bar"
/home/gambas/tmp/foo.bar
</pre>

<p>No není to úžasné ?</p>


[EXEC]

<p>Z celého projektu můžete vytvořit spustitelný soubor. Klikni v
menu <i>Projekt</i> a <i>Vytvořit/Spustitelný...</i>.</p>

<p>Když <b>Gambas</b> vytvoří spustitelný soubor, tak jej defaultně vloží
do stejné složky ve které se nachází projekt.
Spustitelný soubor má pak stejné jméno jako projekt.</p>


[PATH]

<p>Relativní cesta má v <b>Gambas</b>u zvláštní význam.
Vždy se totiž odkazuje na soubory uvnitř svého projektů.</p>

<p>Neexistuje žádný koncept <i>akuálního adresáře</i>,
ani žádné slovo pro změnu jako <tt>CHDIR</tt>.</p>

<p><b>Buť tedy opatrný:</b> musíš použít relativní cesty pouze pro přístup k
souborům projektu. Protože absolutní cesty již nefungují ve vytvořeném spustitelném programu.</p>


[GLOBAL]

<p>V <b>Gambas</b>u <u>nejsou globální proměnné</u>!</p>

<p>Jako dočasné řešení, je můžete vkládat do hlavního modulu a deklarovat je
jako <tt>PUBLIC</tt>.</p>

<p>Pokud nemáte hlavní modul, ale hlavní formulář,
pak deklarujte jako <tt>STATIC PUBLIC</tt>.</p>

<p>Pro přístup k těmto proměnným, musíte používat název modulu
nebo formuláře: <tt>MyMainModule.MyGlobalVariable</tt> nebo
<tt>MyMainForm.MyGlobalVariable</tt>.</p>


[EMPTY]

<p>Chcete-li jestli je řetězec prázdný, není nutné srovnání s <tt>""</tt> nebo používat
funkci <b>Len()</b>. Je možné testovat přímo, protože prázdný řetězec
je ekvivalentní k <b>FALSE</b> a neprázdný řetězec je ekvivalentní k <b>TRUE</b>.</p>

<p>Například, než psát:</p>

<pre>IF Len(MyString) > 0 THEN ...
IF Len(MyString) = 0 THEN ...</pre>

<p>Místo toho můžete psát:</p>

<pre>IF MyString THEN ...
IF NOT MyString THEN ...</pre>


[TRANSLATE]

<p>Program napsaný v <b>Gambas</b>u je přeložitelný do jakéhokoli jazyka,
za předpokladu, že uvedete, které řezezce musí být přeloženy, a které ne.</p>
<p>Chcete-li označit řetězec pro překlad, stačí je uzavřít mezi kulaté závorky:<p>

<pre>PRINT ("Mě přelož")
PRINT "Ale mě nepřekládej!"</pre>


[EVENT]

<p>Každý ovládací prvek či objekt, kterému lze přiřadit událost, má
<i>pozorovatele událostí</i> a událost <i>název skupiny</i>.</p>

<p>Pozorovatel události zachytává každou událost vyvolanou objektem. Název skupiny
událostí tvoří prefix volané procedury, která se stará o vykonání události.</p>

<p>Defaultně je pozorovatel události tvořen mateřským objektem kontrolního prvku
a název skupiny je tvořen názvem kontrolního prvku.</p>

<p>Touto cestou formulář dostává všechny události vyvolané vnitřními prvky.</p>

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

<p>V <b>Gambas</b>u, je formulář vlastní pozorovatel událostí, takže si
může svoje události spravovat přímo (jako <i>Resize</i>,
<i>Activate</i>, ...) ve svém vlastním kódu.</p>

<p>Tímto způsobem, nováčci přícházející z <i>Visual Basic</i>
a nejsou dezorientovaný :-).</p>


[EMBED]

<p>Do formuláře <b>Gambas</b>u můžete vložit jakýkoliv jiný formulář !</p>

<p>Stačí předat rodičovský kontejner formuláře jako poslední argument konstruktoru.</p>

<p>Například:</p>
<p><tt>DIM hForm AS MyDialog<br>
DIM hSuperControl AS MyForm<br><br>
' Vytvoření dialogu<br>
hForm = NEW MyDialog<br>
' Vložení formuláře do tohoto dialogu<br>
' Všimněte si, formulář má ještě dva parametry před rodičovským kontejner<br>
hSuperControl = NEW MyForm(Param1, Param2, MyDialog)<br>
' Nastavení polohy a velikosti formuláře<br>
hSuperControl.Move(8, 8, 128, 64)<br>
</tt></p>

<p><b>Buďte opatrní:</b> formulář vložený do jiného formuláře je stále formulářem,
a tak ma i vlastního pozorovatele událostí.</p>


[GROUP]

<p>Každý ovládací prvek ma vlastnost <i>(Group)</i> [skupinová akce].
Pokud je tato vlastnost nastavena, tak název skupiny je předpona pro zpracování událostí
a ne název ovladacího prvku.</p>

<p>Předpokládejme, že máte <i>Button</i> nazvaný <b>btnAction</b>
s následným zpracováním událostí <i>Click</i>:</p>

<pre>PUBLIC SUB btnAction_Click()</pre>

<p>Pokud nastavíte vlastnost <i>(Group)</i> v <b>btnAction</b> na
<i>MyGroup</i>, pak zpracování událostí bude od tlačítka příjímat
následující událost:</p>

<pre>PUBLIC SUB MyGroup_Click()</pre>

<p>Tato vlastnost umožňuje zpracovávat události z různých ovládacích prvků
v jedné funkci. A ovladací prvky vůbec nemusí být stejné!</p>

<p><b>Poznámka:</b> Zkušený veterán <i>Visual Basic</i>u možná pozná
pojem <i>control array</i>, ale v mnohem silnější implementaci. :-)</p>


[TAG]

<p>Kazdý ovládací prvek má vlastnost <i>Tag</i>. Tato vlastnost je pro
programátory, a může obsahovat jakoukoli <b>VARIANT</b>u dat,
kterou považujete za relevantní.</p>

<p>Je to velmi užitečné, když chcete odlišit různé ovládací prvky
stejné skupiny při společném zpracování událostí.</p>


[LAST]

<p>Klíčové slovo <b>LAST</b> vrací objekt posledního ovládacího prveku, od kterého
obdržel událost. Je to velmi užitečné, pokud chcete psát obsluhu událostí,
která je nezávislá na názvech prvků.</p>

<p>Například, předpokládejme, že chceme napsat program kalkulačky.
Máte nadefinovaná tlačítka pro každé z deseti čísel. Každé je
ve stejné skupině <i>(group)</i> "Digit". <i>Tag</i> každého prvku
má nastaveno číslo, které je na tlačítku. Zpracování události pak
může vypadat takhle:</p>

<p><tt>PUBLIC SUB Digit_Click()<br><br>
&nbsp;&nbsp;Display = Display & LAST.Tag<br>
&nbsp;&nbsp;RefreshDisplay<br><br>
END</tt></p>


[LEFT]

<p>Známé rutiny <i>BASIC</i>u jako <b>Left$</b>, <b>Right$</b>
a <b>Mid$</b> jsou i v <b>Gambas</b>u</p>

<p>Druhý parametr <b>Left$</b> a <b>Right$</b> je volitelný,
a defultni hodnota je 1.</p>

<p><tt>Left$("Gambas")</tt> vrací <tt>"G"</tt><br>
<tt>Right$("Gambas")</tt> vrací <tt>"s"</tt></p>

<p>Tento druhý parametr může být i záporný, a
pak udává počet znaků od konce řetězce.</p>

<p><tt>Left$("Gambas", -2)</tt> vrací <tt>"Gamb"</tt><br>
<tt>Right$("Gambas", -2)</tt> vrací <tt>"mbas"</tt></p>

<p>Podobně, tak třetí parametr v <b>Mid$</b> může být záporný, a
pak udává počet znaků od konce řetězce.</p>

<p><tt>Mid$("Gambas", 2, -2)</tt> vrací <tt>"amb"</tt></p>


[OBSERVER]

<p>Třída <b>Observer</b> (pozorovatel událostí) vám umožňuje zachytit všechny události
vyvolané objekty ještě před tím než jsou skutečně poslány.</p>

<pre>MyTextBox = NEW TextBox(ME) AS "MyTextBox"
MyObserver = NEW Observer(MyTextBox) AS "MyObserver"
...
PUBLIC SUB MyObserver_KeyPress()
  DEBUG "Spuštěno první"
END

PUBLIC SUB MyTextBox_KeyPress()
  DEBUG "Spuštěno další"
END</pre>

<p>Pozorovatel událostí může zastavit událost, v zájmu ochrany objektu.</p>


[STRING]

<p>Gambas používá pro reprezentaci řetězců v paměti znakovou sadu <b>UTF-8</b>.</p>

<p>Ale všechny standartní řetezcové funkce pracují s <b>ASCII</b>:
<tt>Left</tt>, <tt>Mid</tt>, <tt>Right</tt>, <tt>UCase</tt>...</p>

<p>Pokud chcete pracovat s UTF-8 řetězci, musíte používat metody statické třídy <b>String</b>,
které mají stejné názvy jako jejich standartní protějšky.</p>

<pre>PRINT Len("bébé");; Left$("bébé", 3)
6 bé
PRINT String.Len("bébé");; String.Left("bébé", 3)
4 béb</pre>


[ASSIGNMENT]

<p>Gambas implementuje zkrácené přiřazování na
které jsou zvykli programátoři z C/C++.</p>

<p><tt>MyVariable += 2</tt> je ekvivalent
<tt>MyVariable = MyVariable + 2</tt></p>

<p><tt>MyVariable *= 4</tt> je ekvivalent
<tt>MyVariable = MyVariable * 4</tt></p>

<p><tt>MyVariable &= "Great"</tt> je ekvivalent
<tt>MyVariable = MyVariable & "Great"</tt></p>

<p>A tak dále...</p>


[DEBUG]

<p>Také můžete používat instrukci <b>DEBUG</b> pro tisk ladících zpráv do
konzole (tedy standartní chybový výstup). Chová se stejně jako instrukce <tt>PRINT</tt>.</p>

<p>Tyto zprávy obsahuji název třídy, název metody a číslo řádku
z místa volání instrukce <tt>DEBUG</tt>.</p>

<p>Při vytváření spustitelného souboru jsou ladící zprávy automaticky odstraněny.</p>


[TRY]

<p>Správa řízení chyb je v Gambasu řízena skrze instrukce:
<b><tt>TRY</tt></b>, <b><tt>ERROR</tt></b>, <tt>CATCH</tt>, a <tt>FINALLY</tt>.

<p><tt>TRY</tt> se pokusí provést výraz bez chyb. Instrukce <tt>ERROR</tt>
se používá aby se vědělo, jestli byl výraz proveden správně.</p>

<pre>TRY MyFile = OPEN "/etc/password" FOR WRITE
IF ERROR THEN PRINT "Nemůžu dělat, to co chci!"</pre>


[CATCH]

<p>Správa řízení chyb je v Gambasu řízena skrze instrukce:
<tt>TRY</tt>, <tt>ERROR</tt>, <b><tt>CATCH</tt></b>, a <tt>FINALLY</tt>.</p>

<p><tt>CATCH</tt> označuje začátek řízení chyb části funkce nebo procedůry.
Píše se na konec kodu funkce.</p>

<p>Catch blok se provede jen tehdy, když se uvnitř výkoného bloku kodu, vyskytne chyba.</p>

<p>Vyskatne-li se chyba během provádění bloku catch, je běžně zobrazena.</p>

<pre>SUB ProcessFile(FileName AS STRING)
  ...
  OPEN FileName FOR READ AS #hFile
  ...
  CLOSE #hFile
CATCH ' Vykonán pouze v případě, že se vyskytla chyba
  PRINT "Nemůže zpracovat soubor "; FileName
END</pre>


[FINALLY]

<p>Správa řízení chyb je v Gambasu řízena skrze instrukce:
<tt>TRY</tt>, <tt>ERROR</tt>, <tt>CATCH</tt>, a <b><tt>FINALLY</tt></b>.</p>

<p><tt>FINALLY</tt> zavádí kód vykonání na konci funkce,
i když byla chyba vyvolána během jejího plnění.</p>

<p>Finally blok není povinný. Je-li tu blok Catch blok ve funkci,
tak musí finally blok předcházet.</p>

<p>Vyvstane-li chyba během provádění Finally bloku, je běžně zobrazena.</p>

<pre>SUB ProcessFile(FileName AS STRING)
  ...
  Open FileName For Read As #hFile
  ...
Finally ' Provedeno vždy, i když byla chyba vyvolána

  Close #hFile
  
Catch ' Vykonán pouze v případě, že se vyskytla chyba
  
  Print "Cannot print file "; FileName
  
End</pre>


[OPTIONAL]

<p>Funke a procedry v Gambasu mohou mít volitelné parametry.</p>

<p>Provádí se to tak že se před název proměnné napíše instrkukce <b>OPTIONAL</b>.
A ta oznamuje že je parametr volitelný.</p>

<pre>PRIVATE SUB MyFunction(param AS String, OPTIONAL optim AS String)
  ...
  PRINT "povinný " & param & ", nepovinný '" & optim "'"
  ...
END

'volání v kodu:

MyFunction("parametr") 'není-li za potřebí tak se nemusí volat
MyFunction("parametr", "param")
</pre>


[ARRAY]

<p>V Gambasu se dají procházet proměnné (typu array, collection) za pomocí <b>FOR EACH</b>
(pro collection a pole) nebo <b>FOR</b> cyklus (pro pole)</p>

<p>Například:</p>

<pre>DIM xml AS NEW XmlDocument
DIM node AS XmlNode
DIM i AS Integer

'otevre xml
xml.Open("pokus.xml")
'Children se musi indexovat přes [i]!, je to array
FOR i = 0 TO xml.Root.Children.Count - 1
  'Attributes se musí procházet přes FOR EACH, je to collection
  FOR EACH node IN xml.Root.Children[i].Attributes
    DEBUG node.Name;; Node.Value
  NEXT
NEXT</pre>


[ICON]

<p>Můžete použít i vestavěné ikony pro doplnění krásy své aplikace,
a máte zde i například na výběr rozměr od 16x16 až po 128x128</p>

<p>Například:</p>

<pre>Image1.Picture = Picture["icon:/32/warning"]
Image2.Picture = Picture["icon:/small/error"]
</pre>

<p><b>Pozor:</b> musíte mít zapnutou komponentu: <tt>gb.form</tt>.


[SETTINGS]

<p>Pokud si potřebujete uložit konfigurací programu,
jako je třeba velikost a poloha okna, je to jednodušší než jste čekaly.</p>

<p>Můžete ukládat konfiguraci celého objektu, nebo jen jeho dílčí hodnotu.</p>

<p>Například:</p>

<pre>'uložení polohy onjektu
Settings.Write(ME)

'Načtení objektu
Settings.Read(ME)

'Uložení hodnoty
Settings["win/x"] = ME.X

'Načtení hodnoty
'lze zadat i defaultni hodnotu: ["win/x", 0]
ME.X = Settings["win/x"]
</pre>

<p><b>Pozor:</b> musíte mít zapnutou komponentu: <i>gb.settings</i></p>


[MESSAGE]

<p>V Gambasu můžete pro informování uživatele užít následující třídy:
<b>Message</b> a <b>Balloon</b>.
Message je z komponenty (QT, GTK+), a Balloon z <i>gb.form</i>.</p>

<p>Třída <b>Message</b> a <b>Balloon</b> má možné typy:
Message.Delete(), .Error(), .Info(), .Question() a .Warning()</p>

<p>U každého <b>Message</b> jdou nastavit i vlastní popisky tlačítek, po zmáčknutí tlačítka v dialogu je
vrácena číselná hodnota tlačítka od 1 do X, dle typu a počtu tlačítek.</p>

<pre>DEBUG Message.Info("Ahoj světe!")  'vrátí číslo 1

Message("Ahoj světe!")  'To samé zkráceně</pre>

<p>Při vytváření <b>Balloon</b> se musí uvést rodičovský kontejner, volitelně jde nastavit i pozice X a Y,
lze nastavit i prodleva zobrazení v ms a font.</p>

<pre>Balloon.Info("Ahoj světe!", ME)  'zobrazí bublinu na hlavním formuláři

'lze zkátit na: Balloon("Ahoj světe!", ME) ale pak je bez ikony
</pre>


[SUBST]

<p>Pro jednoduší překlad textů v gambasu je možno použít substituci,
instrkukce <b>Subst()</b>.</p>

<p>Má minimalné 2 parametry, kde první je textová maska na kterou se substituce aplikuje,
další parametry jsou vaše proměnné, kde první vložený parametr má pořadové číslo <i>1</i>.
Jako substituční znak se užívá <b>&1, &2, ...</b>, číslo je dle pořadí vložené proměnné.</p>

<pre>Subst(("Nahrazeni za &1, &2 a &3"), "prvni", "druhou", "dalsi")

'vypise:
'Nahrazeni za prvni a za druhou, dalsi</pre>


[EDITOR]

<p>Několik typů v editoru:</p>

<p>Dva typy komentářů:</p>

<pre>'normální komentář</pre>
<b><pre>''tučný komentář</pre></b>

<p>Tučné komentáře jsou používány pro dokumentování kódu.</p>

<p>Jak používat úryvky kodu:<br>

'napíšete:
<pre>main</pre>
'a pak zmáčknete tabulátor, a doplní se útržek kódu,<br>
'podle toho jak je v nastavení definován
</p>

<p>Úryvky kodu jsou zcela konfigurovatelné z globálního dialogu nastavení IDE.</p>


[END]

<p>Přečetly jste všechny tipy dnů. Doufám, že jste se stal
nyní odborníkem na <b>Gambas</b> ! :-)</p>

<p>Pokud chcete přidat nové tipy z vaší vlastní zkušenosti,
neváhejte a pošlete je na adresu:</p>

<p><u>gambas@users.sourceforge.net</u></p>

<p>Díky předem !</p>

