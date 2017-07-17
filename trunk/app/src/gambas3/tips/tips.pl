[WELCOME]

<p>Witaj w <b>Gambasie</b> !</p>

<p><b>Gambas</b> jest zintegrowanym środowiskiem programistycznym
 bazującym na zaawansowanym interpreterze <i>Basica</i>.</p>

<p><b>Gambas</b> umożliwi Ci łatwe i szybkie tworzenie programów. 
Jednak to, czy programy będą ładne, funcjonalne i przejrzyste zależy tylko od Ciebie...
</p>

<p>Miłej pracy !</p>

<p align=right>Beno&icirc;t Minisini<br>
<u>gambas@users.sourceforge.net</u></p>


[STARTUP]

<p>Każdy projekt musi mieć <i>klasę startową</i>. Klasa
startowa musi definiować  statyczną metodę publiczną o nazwie <i>Main</i>
bez argumentów, która uruchomi Twój program.</p>

<p>Możesz zdefiniować klasę startową poprzez kliknięcie prawym klawiszem 
myszki w drzewie projektu i wybrać opcję <i>Klasa startowa</i> w menu podręcznym.</p>

<p>Nie jest konieczne definiowanie metody <i>Main</i> w formularzu 
startowym, ponieważ jest już w nim predefiniowana.</p>

<p>Ta predefiniowana metoda startowa inicjuje formularz i wyświetla go podobnie jak
w <i>Visual Basic&trade;</i>.</p>


[OPEN]

<p>W <b>Gambasie</b> instrukcja <b>OPEN</b> nie działa jak ta
w <i>Visual Basicu</i>. Nie zwraca pliku jako integer, ale jako obiekt  <i>File</i>.</p>

<p>Dlatego, zamiast pisać :</p>

<pre>DIM hPliku AS Integer
...
OPEN "MójPlik" FOR READ AS #hPliku</pre>

<p>musisz napisać :</p>

<pre>DIM hPliku AS File
...
hPliku = OPEN "MójPlik" FOR READ</pre>


[CATDIR]

<p>Czy wiesz, że możesz połączyć ścieżkę z nazwą pliku
używając operatora <b><tt>&/</tt></b> ? Ten operator
połączy dwa łańcuchy znaków dodając między nimi '/',
jeśli wcześniej go tam nie było, lub zredukuje jego wielokrotność.</p>

<p>Przykład:</p>

<pre>PRINT "/home/gambas" &/ ".bashrc"
/home/gambas/.bashrc

PRINT "/home/gambas/" &/ "/tmp" &/ "foo.bar"
/home/gambas/tmp/foo.bar
</pre>

<p>Czyż to nie wpaniałe ?</p>


[EXEC]

<p>Z projektu możesz utworzyć plik uruchamialny.
 Wybierz opcję <i>Utwórz plik uruchamialny</i> w menu <i>Projekt</i>.</p>

<p>Kiedy <b>Gambas</b> stworzy plik uruchamialny umieści go
w domyślnym katalogu projektu. 
Nazwa pliku uruchamialnego jest taka sama jak nazwa Twojego projektu.</p>


[PATH]
<p>
Ścieżki względne mają specjalne znaczenie w <b>Gambasie</i>.
Zawsze odwołują się do pliku wewnątrz Twojego projektu.

<p>
Nie istnieje idea <i>bieżącego katalogu</i> i nie ma słowa kluczowego
takiego jak <tt>CHDIR</tt>, które pozwalałoby na zmianę katalogu roboczego.
<p>
<b>Uwaga:</b> musisz używać tylko ścieżek względnych do
dostępu do plików wewnątrz projektu, ponieważ ścieżki bezwględne
 po utworzeniu pliku uruchamialnego nie będą działać poprawnie.


[GLOBAL]

W <b><i>Gambasie</i></b> <u>nie ma zmiennych globalnych</u>!
<p>
Aby to obejść, zadeklaruj zmienne jako <tt>PUBLIC</tt> w module startowym.
<p>
Jeśli nie masz w projekcie modułu startowego tylko formularz 
startowy, wtedy zadeklaruj zmienne jako <tt>STATIC PUBLIC</tt>.
<p>
Aby mieć dostęp do tych zmiennych musisz użyć nazwy modułu lub
formularza startowego przed nazwą zmiennej: 
<tt>MójModułStartowy.MojaZmiennaGlobalna</tt> lub
<tt>MójFormularzStartowy.MojaZmiennaGlobalna</tt>.


[EMPTY]

<p>Nie jest konieczne użycie funcji <b>Len()</b> aby się dowiedzieć czy 
łańcuch znaków jest pusty. Możesz przetestować go bezpośrednio;
jeśli łańcuch jest pusty, to zostanie zwrócone <b>FALSE</b>, a jeśli nie jest pusty
zostanie zwrócone <b>TRUE</b>.</p>

<p>Na przykład, zamiast :</p>

<pre>IF Len(MójŁańcuchZnaków) > 0 THEN ...
IF Len(MójŁańcuchZnaków) = 0 THEN ...</pre>

<p>Możesz wpisać :</p>

<pre>IF MójŁańcuchZnaków THEN ...
IF NOT MójŁańcuchZnaków THEN ...</pre>


[EVENT]

<p>Każda kontrolka i każdy obiekt, który może wywołać zdarzenie
posiada <i>obserwatora zdarzenia (event observer)</i> i 
<i>nazwę grupy</i> zdarzenia.</p>

<p>Obserwator zdarzenia wyłapuje każde zdarzenie wywołane przez obiekt, 
a nazwa grupy zdarzenia jest prefiksem procedury wywołanej do zarządzania 
zdarzeniem.</p>

<p>Domyślnie, obserwator zdarzenia jest obiektem 
utworzonej kontrolki, a nazwa grupy jest nazwą tej kontrolki.
</p>

<p>W ten sposób formularz otrzymuje wszystkie zdarzenia wywołane przez
kontrolki utworzone wewnątrz tego formularza.</p>

<pre>' Gambas form
DIM hPrzycisku AS Button

PUBLIC SUB _new()
&nbsp;&nbsp;hPrzycisku = NEW Button(ME) AS "MójPrzycisk"
END

PUBLIC SUB MójPrzycisk_Click()
&nbsp;&nbsp;PRINT "Kliknąłeś w MójPrzycisk !"
END
</pre>


[FORM]

<p>W <b><i>Gambasie</i></b> formularz jest obserwatorem 
własnych zdarzeń, w związku z czym możesz bezpośrednio zarządzać
jego zdarzeniami (takimi jak <i>Resize</i>, 
<i>Activate</i>, ...) w kodzie jego własnej klasy.</p>

<p>Dzięki temu początkujący w Gambasie, pracujący wcześniej w 
<i>Visual Basicu</i> nie są zdezorientowani :-).</p>


[EMBED]

<p>W <b><i>Gambasie</i></b> możesz osadzać dowolny formularz
 w innym formularzu&nbsp;!</p>

<p>Aby uzyskać tak wspaniałą funkcjonalność
wystarczy zainicjować formularz poprzez podanie 
nadrzędnego kontenera jako ostatniego argumentu konstruktora.</p>

<p>Przykład&nbsp;:</p>
<p><tt>DIM hFormularza AS MójFormularz<br>
DIM hPodformularza AS MójPodformularz<br><br>
' Tworzenie formularza głównego<br>
hFormularza = NEW MójFormularz<br>
' Wstawienie podformularza do formularza głównego<br>
'Zauważ, że ten formularz pobiera dwa parametry przed kontenerem<br>
hPodformularza = NEW MójPodformularz(Param1, Param2, MójFormularz)<br>
' Przenieś i zmień wielkość formularza<br>
hPodformularza.Move(8, 8, 128, 64)<br>
</tt></p>

<p>Uwaga: formularz osadzony w innym formularzu jest nadal formularzem
i tym samym jest własnym obserwatorem zdarzeń.</p>


[GROUP]

<p>Każda kontrolka posiada właściwość <i>(Group)</i>.
Kiedy właściwość ta jest ustawiona, prefiks nazwy uchwytu zdarzenia
jest nazwą grupy, a nie nazwą kontrolki.</p>

<p>Załóżmy, że masz <i>Przycisk</i> o nazwie <b>btnMójPrzycisk</b>
z następującym uchwytem zdarzenia <i>Click</i>:</p>

<pre>PUBLIC SUB btnMójPrzycisk_Click()</pre>

<p>Jesli ustawisz właściwość <i>(Group)</i> dla <b>btnMójPrzycisk</b>
na <i>MojaGrupa</i> wtedy uchwyt zdarzenia, który otrzyma
zdarzenia z przycisku będzie następujący:</p>

<pre>PUBLIC SUB MojaGrupa_Click()</pre>

<p>Ta właściwość pozwala Ci na wychwytywanie zdarzeń różnych kontrolek
w jednej funkcji, a kontrolki tej samej grupy nie muszą być tego samego typu !</p>

<p><b>Notatka :</b> Starzy weterani <i>Visual Basica</i> moga rozpoznać ideę
<i>tablicy kontrolek (control array)</i>, ale w bardziej funkcjonalnej implementacji. :-)</p>


[TAG]

<p>Każda kontrolka ma właściwość <i>Tag</i>. Ta właściwość jest przeznaczona
dla programistów i może zawierać dowolne dane typu <b>VARIANT</b>, które
uważa on za istotne.</p>

<p>To jest bardzo użyteczne kiedy chcesz rozróżnić kontrolki tej samej grupy we 
wspólnym uchwycie zdarzenia.</p>


[LAST]

<p>Słowo kluczowe <b>LAST</b> zwraca ostatnią kontrolkę, która odebrała
zdarzenie. Jest to bardzo użyteczne kiedy chcesz napisać uchwyt zdarzenia,
 który jest niezależny od nazwy kontrolki.</p>

<p>Przykładowo, przypuśćmy, że chcesz napisać program kalkulator.
Zdefiniowałeś przyciski, po jednym dla każdej cyfry, każdy w tej samej <i>grupie</i>
o nazwie "Cyfry". Właściwość <i>Tag</i> każdej kontrolki zawiera cyfrę widoczną 
na przycisku. Twój uchwyt zdarzenia może wyglądać tak:</p>

<p><tt>PUBLIC SUB Cyfry_Click()<br><br>
&nbsp;&nbsp;Display = Display & LAST.Tag<br>
&nbsp;&nbsp;RefreshDisplay<br><br>
END</tt></p>


[LEFT]

<p>Dobrze znane funkcje z <i>BASICa</i>: <b>Left$</b>, <b>Right$</b>
i <b>Mid$</b> mają dodatkową użyteczność w <b><i>Gambasie</i></b>.</p>

<p>Drugi parametr <b>Left$</b> i <b>Right$</b> jest opcjonalny i domyślnie zwraca
jeden znak.</p>

<p><tt>Left$("Gambas")</tt> zwraca <tt>"G"</tt><br>
<tt>Right$("Gambas")</tt> zwraca <tt>"s"</tt></p>

<p>Drugi parametr może być ujemny i wtedy funkcja zwraca wartość 
bez określonej ilości niechcianych znaków.</p>

<p><tt>Left$("Gambas", -2)</tt> zwraca <tt>"Gamb"</tt><br>
<tt>Right$("Gambas", -2)</tt> zwraca <tt>"mbas"</tt></p>

<p>Na podobnej zasadzie, trzeci argument funkcji <b>Mid$</b> 
może mieć również wartość ujemną.</p>

<p><tt>Mid$("Gambas", 2, -2)</tt> zwraca <tt>"amb"</tt>


[END]

<p>Przeczytałeś wszystkie porady dnia. Mam nadzieję, że dzięki nim stałeś się 
ekspertem programowania w <b>Gambasie</b> ! :-)</p>

<p>Jesli chcesz dodać nowe porady, prześlij je na adres
&nbsp;:</p>
<p><u>gambas@users.sourceforge.net</u></p>

<p>Z góry dziękuję !</p>



