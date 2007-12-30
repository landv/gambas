/***************************************************************************

  gstock.cpp

  (c) 2004-2005 - Daniel Campos Fernández <danielcampos@netcourrier.com>
  
  Gtkmae "GTK+ made easy" classes
  
  Realizado para la Junta de Extremadura. 
  Consejería de Educación Ciencia y Tecnología. 
  Proyecto gnuLinEx
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/
#include "widgets.h"
#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

static char * stock_gambas_xpm[] = {
"16 16 133 2",
"  	c None",
". 	c #C4C4C4",
"+ 	c #B2B2B2",
"@ 	c #A199AC",
"# 	c #96959F",
"$ 	c #898793",
"% 	c #636263",
"& 	c #8C8C8C",
"* 	c #A8A8A8",
"= 	c #505050",
"- 	c #A4A4A4",
"; 	c #636363",
"> 	c #8D8D8D",
", 	c #25597B",
"' 	c #19395A",
") 	c #2A769E",
"! 	c #286A8A",
"~ 	c #7F7F7F",
"{ 	c #E8E8E8",
"] 	c #EFEFEF",
"^ 	c #CBCBCB",
"/ 	c #545454",
"( 	c #F2F2F2",
"_ 	c #B0B0B0",
": 	c #7D7D7D",
"< 	c #355C7A",
"[ 	c #21577B",
"} 	c #3398C2",
"| 	c #3398C3",
"1 	c #2A6C87",
"2 	c #AFAFAF",
"3 	c #1B1B1B",
"4 	c #434343",
"5 	c #F5F5F5",
"6 	c #3E3E3F",
"7 	c #0F0F0F",
"8 	c #9A9A9A",
"9 	c #5D5D5D",
"0 	c #A1B5C3",
"a 	c #1C4769",
"b 	c #3084A8",
"c 	c #707172",
"d 	c #959595",
"e 	c #ACACAC",
"f 	c #BCBCBC",
"g 	c #101D2B",
"h 	c #1C1B20",
"i 	c #BEBEBE",
"j 	c #585858",
"k 	c #506178",
"l 	c #3190BA",
"m 	c #245469",
"n 	c #6E6E6E",
"o 	c #878787",
"p 	c #29404B",
"q 	c #3291BA",
"r 	c #204060",
"s 	c #5D5B61",
"t 	c #AAAAAA",
"u 	c #294867",
"v 	c #5AABCE",
"w 	c #42A0C7",
"x 	c #3188AE",
"y 	c #2F80A2",
"z 	c #3397C2",
"A 	c #3294BE",
"B 	c #233B58",
"C 	c #1A3F60",
"D 	c #3A9CC5",
"E 	c #E3F0F6",
"F 	c #3C9CC6",
"G 	c #308DB7",
"H 	c #344159",
"I 	c #274260",
"J 	c #6DB5D4",
"K 	c #FEFEFF",
"L 	c #4AA3CA",
"M 	c #2C7EA7",
"N 	c #57596C",
"O 	c #4C5166",
"P 	c #287198",
"Q 	c #79BBD8",
"R 	c #FFFFFF",
"S 	c #8AC4DC",
"T 	c #246388",
"U 	c #888793",
"V 	c #0F1734",
"W 	c #2C80A9",
"X 	c #43A0C8",
"Y 	c #EBF5F9",
"Z 	c #F0F7FA",
"` 	c #50A6CC",
" .	c #225C81",
"..	c #2D2053",
"+.	c #110937",
"@.	c #1A395D",
"#.	c #304F6C",
"$.	c #6A7180",
"%.	c #415368",
"&.	c #183C5D",
"*.	c #308BB5",
"=.	c #17083B",
"-.	c #14033E",
";.	c #13194A",
">.	c #134B6A",
",.	c #136D7E",
"'.	c #136C7C",
").	c #105265",
"!.	c #0E2E46",
"~.	c #1A4263",
"{.	c #2F89B2",
"].	c #379AC4",
"^.	c #141047",
"/.	c #1E2945",
"(.	c #130632",
"_.	c #14073F",
":.	c #140E46",
"<.	c #14395F",
"[.	c #13657A",
"}.	c #137080",
"|.	c #125E70",
"1.	c #0F2C44",
"2.	c #2F688B",
"3.	c #5B8694",
"4.	c #12253E",
"5.	c #122446",
"6.	c #14174B",
"7.	c #14335C",
"8.	c #13687B",
"9.	c #5B9BA6",
"0.	c #ABB1BA",
"a.	c #8EA2AC",
"b.	c #8E92AA",
"              . +               ",
"      @ # $ % & * = - ; >       ",
"    , ' ) ! ~ { ] ^ / ( _ :     ",
"  < [ } | 1 2 3 4 5 6 7 8 9     ",
"0 a | | | b c d e f g h i j     ",
"k l | | | | m n o p q r s t     ",
"u | | | v w | x y z | A B       ",
"C | | D E F | | | | | | G H     ",
"I | | J K L | | | | | | | M N   ",
"O P | Q R S | | | | | | | | T U ",
"  V W X Y Z ` | | | | | | | |  .",
"  ..+.@.#.$.%.&. .*.| | | |  .  ",
"    =.-.;.>.,.'.).!.~.{.| ].^.  ",
"      /.(._.:.<.[.}.|.1.2.^.    ",
"        3.4.5.6.^.7.8.9.^.      ",
"            0.a.b.^.7.          "};

static char * stock_linux_xpm[] = {
"16 16 109 2",
"  	c None",
". 	c #818181",
"+ 	c #656565",
"@ 	c #363636",
"# 	c #515151",
"$ 	c #505050",
"% 	c #181818",
"& 	c #050505",
"* 	c #58585D",
"= 	c #2F2F33",
"- 	c #84868A",
"; 	c #181819",
"> 	c #979171",
", 	c #B8B082",
"' 	c #A69A81",
") 	c #312E2B",
"! 	c #000000",
"~ 	c #D7C428",
"{ 	c #FFE948",
"] 	c #F1AF2E",
"^ 	c #37260C",
"/ 	c #0B0D11",
"( 	c #DFCC90",
"_ 	c #FFCC57",
": 	c #FFE2B5",
"< 	c #757576",
"[ 	c #727373",
"} 	c #FFFFFF",
"| 	c #D1D1D1",
"1 	c #090909",
"2 	c #0C0C0C",
"3 	c #D3D3D3",
"4 	c #FDFEFE",
"5 	c #FAFAFA",
"6 	c #FDFDFD",
"7 	c #636363",
"8 	c #F9F9F9",
"9 	c #F4F4F4",
"0 	c #F8F8F8",
"a 	c #BFBFBF",
"b 	c #121212",
"c 	c #060606",
"d 	c #000309",
"e 	c #A9AAAC",
"f 	c #F5F5F5",
"g 	c #EEEEEE",
"h 	c #EBEBEB",
"i 	c #D5D7DA",
"j 	c #272727",
"k 	c #101010",
"l 	c #5D4418",
"m 	c #C4BDB2",
"n 	c #EFEFEF",
"o 	c #E8E8E9",
"p 	c #E3E3E3",
"q 	c #CABDA9",
"r 	c #393838",
"s 	c #242425",
"t 	c #AD7B28",
"u 	c #FFDF97",
"v 	c #C1A36A",
"w 	c #B5B5B6",
"x 	c #F0F1F2",
"y 	c #E9E9E9",
"z 	c #E1E2E4",
"A 	c #DBD5CB",
"B 	c #DFB15E",
"C 	c #484234",
"D 	c #786747",
"E 	c #B88A23",
"F 	c #FFE97F",
"G 	c #FFE489",
"H 	c #F8D469",
"I 	c #79653B",
"J 	c #BABCC5",
"K 	c #EAEBEA",
"L 	c #DEDFE1",
"M 	c #CBC6BC",
"N 	c #EEC76D",
"O 	c #FCDF93",
"P 	c #FFE997",
"Q 	c #D0AF55",
"R 	c #BF9D0A",
"S 	c #FFEE2C",
"T 	c #FFDD25",
"U 	c #FFDA19",
"V 	c #EEC81D",
"W 	c #D1C6A2",
"X 	c #D2D3D9",
"Y 	c #B1B2B7",
"Z 	c #857C64",
"` 	c #F2CB2B",
" .	c #FFE141",
"..	c #FFE42A",
"+.	c #F4CC2F",
"@.	c #C0A300",
"#.	c #ECCC00",
"$.	c #FFED00",
"%.	c #FFF600",
"&.	c #907F25",
"*.	c #373947",
"=.	c #303239",
"-.	c #5D522A",
";.	c #FDEB02",
">.	c #FFF400",
",.	c #B49500",
"'.	c #615303",
").	c #6E6101",
"!.	c #6D5E03",
"              . +               ",
"            @ # $ %             ",
"          & * = - ;             ",
"            > , ' ) !           ",
"            ~ { ] ^ !           ",
"          / ( _ : < !           ",
"          [ } } } | 1 !         ",
"        2 3 } 4 5 6 7 !         ",
"        7 } } 8 9 0 a b c       ",
"      d e } 8 f g h i j k       ",
"      l m } f n o p q r s       ",
"    t u v w x y z A B C D       ",
"  E F G H I J K L M N O P Q     ",
"  R S T U V W X Y Z `  ...+.    ",
"    @.#.$.%.&.*.=.-.;.>.,.      ",
"        '.).        !.!.        "};


static char * stock_zoom_xpm[] = {
"24 24 127 2",
"  	c None",
". 	c #343434",
"+ 	c #2D2D2D",
"@ 	c #292929",
"# 	c #262626",
"$ 	c #2E2E2E",
"% 	c #303030",
"& 	c #737373",
"* 	c #A1A1A1",
"= 	c #B4B4B4",
"- 	c #B2B2B2",
"; 	c #9D9D9D",
"> 	c #676767",
", 	c #202020",
"' 	c #1C1C1C",
") 	c #272727",
"! 	c #616161",
"~ 	c #CACACA",
"{ 	c #CFCFCF",
"] 	c #D0D0D0",
"^ 	c #CECECE",
"/ 	c #C9C9C9",
"( 	c #C1C1C1",
"_ 	c #A7A7A7",
": 	c #4C4C4C",
"< 	c #131313",
"[ 	c #222222",
"} 	c #757575",
"| 	c #D3D3D3",
"1 	c #DBDBDB",
"2 	c #E7E7E7",
"3 	c #EFEFEF",
"4 	c #F3F3F3",
"5 	c #F1F1F1",
"6 	c #E5E5E5",
"7 	c #D2D2D2",
"8 	c #BCBCBC",
"9 	c #5E5E5E",
"0 	c #101010",
"a 	c #212121",
"b 	c #5B5B5B",
"c 	c #CCCCCC",
"d 	c #DADADA",
"e 	c #FEFEFE",
"f 	c #FBFBFB",
"g 	c #FAFAFA",
"h 	c #C2C2C2",
"i 	c #434343",
"j 	c #0F0F0F",
"k 	c #1F1F1F",
"l 	c #B9B9B9",
"m 	c #D6D6D6",
"n 	c #FDFDFD",
"o 	c #FCFCFC",
"p 	c #E4E4E4",
"q 	c #ABABAB",
"r 	c #0E0E0E",
"s 	c #1B1B1B",
"t 	c #6D6D6D",
"u 	c #E1E1E1",
"v 	c #F9F9F9",
"w 	c #E6E6E6",
"x 	c #575757",
"y 	c #090909",
"z 	c #141414",
"A 	c #A8A8A8",
"B 	c #D8D8D8",
"C 	c #EEEEEE",
"D 	c #F8F8F8",
"E 	c #DCDCDC",
"F 	c #9B9B9B",
"G 	c #060606",
"H 	c #111111",
"I 	c #C5C5C5",
"J 	c #DFDFDF",
"K 	c #F5F5F5",
"L 	c #F7F7F7",
"M 	c #F2F2F2",
"N 	c #EDEDED",
"O 	c #BFBFBF",
"P 	c #C6C6C6",
"Q 	c #E3E3E3",
"R 	c #ECECEC",
"S 	c #F4F4F4",
"T 	c #F0F0F0",
"U 	c #EAEAEA",
"V 	c #E0E0E0",
"W 	c #D7D7D7",
"X 	c #BABABA",
"Y 	c #050505",
"Z 	c #0B0B0B",
"` 	c #A5A5A5",
" .	c #F6F6F6",
"..	c #D1D1D1",
"+.	c #939393",
"@.	c #020202",
"#.	c #0A0A0A",
"$.	c #5F5F5F",
"%.	c #D9D9D9",
"&.	c #E9E9E9",
"*.	c #D4D4D4",
"=.	c #000000",
"-.	c #CBCBCB",
";.	c #3B3B3B",
">.	c #BDBDBD",
",.	c #515151",
"'.	c #C7C7C7",
").	c #CDCDCD",
"!.	c #C8C8C8",
"~.	c #B8B8B8",
"{.	c #454545",
"].	c #030303",
"^.	c #313131",
"/.	c #999999",
"(.	c #BBBBBB",
"_.	c #B6B6B6",
":.	c #909090",
"<.	c #2B2B2B",
"[.	c #010101",
"}.	c #414141",
"|.	c #7A7A7A",
"1.	c #9A9A9A",
"2.	c #777777",
"3.	c #3C3C3C",
"4.	c #686868",
"5.	c #797979",
"6.	c #3A3A3A",
"                                                ",
"              . + @ # # #                       ",
"          $ % & * = - ; > , '                   ",
"        ) ! = ~ { ] ^ / ( _ : <                 ",
"      [ } ~ | 1 2 3 4 5 6 7 8 9 0               ",
"    a b c d d d e f g e d d d h i j             ",
"    k l m d n n n n o n n n d p q r             ",
"  s t 7 u d n n e e n o v e d w 1 x y           ",
"  z A B C e n o e e n f D e e w E F G           ",
"  H I J 3 K D f n n o g L M N 6 1 O G           ",
"  r P Q R M K D g g g L S T U V W X Y           ",
"  Z ` u 2 d e S  . .K 4 3 e d 1 ..+.@.          ",
"  #.$.%.u d e C 3 3 C N &.e d *.~ : =.          ",
"    #.= B d d d d w 2 d d d d -.* @.            ",
"    G ;.c d d d d d 1 d d d d >.$ =.            ",
"      Y ,.h '.-.c ).-.-.!.h ~.{.=.              ",
"        ].^./.(.>.8 8 8 _.:.<.=.=.=.            ",
"          [.].}.|.1.1.2.3.=.=.  =.=.=.=.        ",
"              =.=.=.=.=.=.        , , =.=.      ",
"                                  =.4.. =.=.    ",
"                                    =.5.6.=.=.  ",
"                                      =.4.k =.  ",
"                                        =.=.    ",
"                                                "};




gImage *gStock_missing(char *vl)
{
	gImage *img=NULL,*img2;
	char **tmp=NULL;
	bool do_flip=false;
	
	if       (!strcasecmp(vl,"gambas"))        tmp=stock_gambas_xpm;          
	else if  (!strcasecmp(vl,"linux"))         tmp=stock_linux_xpm;   
	else if  (!strcasecmp(vl,"zoom/viewmag"))  tmp=stock_zoom_xpm;

	
	if (tmp)
	{
		img=new gImage(0,0);
		img->image=gdk_pixbuf_new_from_xpm_data((const char**)tmp); 
		
		if (do_flip)
		{
			if (gtk_widget_get_default_direction ()==GTK_TEXT_DIR_RTL)
			{
				img2=img->flip();
				delete img;
				img=img2;
			}
		}
	}   
	
	return img;
}


void gStock_parse(char *vl,char **resul)
{
	
	*resul=NULL;
	
	if       (!strcasecmp(vl,"device/cdrom"))       *resul=GTK_STOCK_CDROM;
	else if  (!strcasecmp(vl,"device/floppy"))      *resul=GTK_STOCK_FLOPPY;
	else if  (!strcasecmp(vl,"device/harddisk"))    *resul=GTK_STOCK_HARDDISK;
	else if  (!strcasecmp(vl,"device/printer"))     *resul=GTK_STOCK_PRINT;
	else if  (!strcasecmp(vl,"device/network"))     *resul=GTK_STOCK_NETWORK; // (*)
	
	else if  (!strcasecmp(vl,"dialog/auth"))        *resul=GTK_STOCK_DIALOG_AUTHENTICATION;
	else if  (!strcasecmp(vl,"dialog/error"))       *resul=GTK_STOCK_DIALOG_ERROR;
	else if  (!strcasecmp(vl,"dialog/info"))        *resul=GTK_STOCK_DIALOG_INFO;
	else if  (!strcasecmp(vl,"dialog/question"))    *resul=GTK_STOCK_DIALOG_QUESTION;
	else if  (!strcasecmp(vl,"dialog/warning"))     *resul=GTK_STOCK_DIALOG_WARNING;

	else if  (!strcasecmp(vl,"dnd/simple"))         *resul=GTK_STOCK_DND; // (*)
	else if  (!strcasecmp(vl,"dnd/multiple"))       *resul=GTK_STOCK_DND_MULTIPLE; // (*)
	
	else if  (!strcasecmp(vl,"go/bottom"))          *resul=GTK_STOCK_GOTO_BOTTOM;
	else if  (!strcasecmp(vl,"go/down"))            *resul=GTK_STOCK_GO_DOWN;
	else if  (!strcasecmp(vl,"go/first"))           *resul=GTK_STOCK_GOTO_FIRST;
	else if  (!strcasecmp(vl,"go/last"))            *resul=GTK_STOCK_GOTO_LAST;
	else if  (!strcasecmp(vl,"go/left"))            *resul=GTK_STOCK_GO_BACK;
	else if  (!strcasecmp(vl,"go/right"))           *resul=GTK_STOCK_GO_FORWARD;
	else if  (!strcasecmp(vl,"go/top"))             *resul=GTK_STOCK_GOTO_TOP;
	else if  (!strcasecmp(vl,"go/up"))              *resul=GTK_STOCK_GO_UP;
	else if  (!strcasecmp(vl,"go/home"))            *resul=GTK_STOCK_HOME;

	else if  (!strcasecmp(vl,"sort/ascending"))     *resul=GTK_STOCK_SORT_ASCENDING; // (*)
	else if  (!strcasecmp(vl,"sort/descending"))    *resul=GTK_STOCK_SORT_DESCENDING; // (*)
	
	else if  (!strcasecmp(vl,"text/bold"))          *resul=GTK_STOCK_BOLD;
	else if  (!strcasecmp(vl,"text/center"))        *resul=GTK_STOCK_JUSTIFY_CENTER;
	else if  (!strcasecmp(vl,"text/font"))          *resul=GTK_STOCK_SELECT_FONT;
	else if  (!strcasecmp(vl,"text/italic"))        *resul=GTK_STOCK_ITALIC;
	else if  (!strcasecmp(vl,"text/justify"))       *resul=GTK_STOCK_JUSTIFY_FILL;
	else if  (!strcasecmp(vl,"text/left"))          *resul=GTK_STOCK_JUSTIFY_LEFT;
	else if  (!strcasecmp(vl,"text/right"))         *resul=GTK_STOCK_JUSTIFY_RIGHT;
	else if  (!strcasecmp(vl,"text/strikethrough")) *resul=GTK_STOCK_STRIKETHROUGH;
	else if  (!strcasecmp(vl,"text/underline"))     *resul=GTK_STOCK_UNDERLINE;
	else if  (!strcasecmp(vl,"text/indent"))        *resul=GTK_STOCK_INDENT; // (*)
	else if  (!strcasecmp(vl,"text/unindent"))      *resul=GTK_STOCK_UNINDENT; // (*)
	
	else if  (!strcasecmp(vl,"media/forward"))      *resul=GTK_STOCK_MEDIA_FORWARD; 
	else if  (!strcasecmp(vl,"media/next"))         *resul=GTK_STOCK_MEDIA_NEXT; 
	else if  (!strcasecmp(vl,"media/pause"))        *resul=GTK_STOCK_MEDIA_PAUSE; 
	else if  (!strcasecmp(vl,"media/play"))         *resul=GTK_STOCK_MEDIA_PLAY; 
	else if  (!strcasecmp(vl,"media/previous"))     *resul=GTK_STOCK_MEDIA_PREVIOUS; 
	else if  (!strcasecmp(vl,"media/record"))       *resul=GTK_STOCK_MEDIA_RECORD; 
	else if  (!strcasecmp(vl,"media/rewind"))       *resul=GTK_STOCK_MEDIA_REWIND; 
	else if  (!strcasecmp(vl,"media/stop"))         *resul=GTK_STOCK_MEDIA_STOP; 

	else if  (!strcasecmp(vl,"zoom/zoomfit"))       *resul=GTK_STOCK_ZOOM_FIT;
	else if  (!strcasecmp(vl,"zoom/zoomin"))        *resul=GTK_STOCK_ZOOM_IN;
	else if  (!strcasecmp(vl,"zoom/zoomnormal"))    *resul=GTK_STOCK_ZOOM_100;
	else if  (!strcasecmp(vl,"zoom/zoomout"))       *resul=GTK_STOCK_ZOOM_OUT;

	else if  (!strcasecmp(vl,"add"))                *resul=GTK_STOCK_ADD; // (*)
	else if  (!strcasecmp(vl,"apply"))              *resul=GTK_STOCK_APPLY;
	else if  (!strcasecmp(vl,"cancel"))             *resul=GTK_STOCK_CANCEL;
	else if  (!strcasecmp(vl,"clear"))              *resul=GTK_STOCK_CLEAR; // (*)
	else if  (!strcasecmp(vl,"close"))              *resul=GTK_STOCK_CLOSE;
	else if  (!strcasecmp(vl,"color"))              *resul=GTK_STOCK_SELECT_COLOR;
	else if  (!strcasecmp(vl,"colorpicker"))        *resul=GTK_STOCK_COLOR_PICKER;
	else if  (!strcasecmp(vl,"convert"))            *resul=GTK_STOCK_CONVERT; // (*)

	else if  (!strcasecmp(vl,"connect"))            *resul=GTK_STOCK_CONNECT; 
	else if  (!strcasecmp(vl,"copy"))               *resul=GTK_STOCK_COPY;
	else if  (!strcasecmp(vl,"cut"))                *resul=GTK_STOCK_CUT;
	else if  (!strcasecmp(vl,"delete"))             *resul=GTK_STOCK_DELETE;
	else if  (!strcasecmp(vl,"directory"))          *resul=GTK_STOCK_DIRECTORY; 
	else if  (!strcasecmp(vl,"disconnect"))         *resul=GTK_STOCK_DISCONNECT; 
	else if  (!strcasecmp(vl,"edit"))               *resul=GTK_STOCK_EDIT; 
	else if  (!strcasecmp(vl,"file"))               *resul=GTK_STOCK_FILE; 
	else if  (!strcasecmp(vl,"execute"))            *resul=GTK_STOCK_EXECUTE; // (*)
	else if  (!strcasecmp(vl,"find"))               *resul=GTK_STOCK_FIND;
	else if  (!strcasecmp(vl,"findandreplace"))     *resul=GTK_STOCK_FIND_AND_REPLACE; // (*)
	else if  (!strcasecmp(vl,"help"))               *resul=GTK_STOCK_HELP;
	else if  (!strcasecmp(vl,"index"))              *resul=GTK_STOCK_INDEX; // (*)
	else if  (!strcasecmp(vl,"jumpto"))             *resul=GTK_STOCK_JUMP_TO; // (*)
	else if  (!strcasecmp(vl,"missingimage"))       *resul=GTK_STOCK_MISSING_IMAGE; // (*)
	else if  (!strcasecmp(vl,"new"))                *resul=GTK_STOCK_NEW;
	else if  (!strcasecmp(vl,"no"))                 *resul=GTK_STOCK_NO; // (*)
	else if  (!strcasecmp(vl,"ok"))                 *resul=GTK_STOCK_OK;
	else if  (!strcasecmp(vl,"open"))               *resul=GTK_STOCK_OPEN;
	else if  (!strcasecmp(vl,"paste"))              *resul=GTK_STOCK_PASTE;
	else if  (!strcasecmp(vl,"preferences"))        *resul=GTK_STOCK_PREFERENCES;
	else if  (!strcasecmp(vl,"print"))              *resul=GTK_STOCK_PRINT;
	else if  (!strcasecmp(vl,"printpreview"))       *resul=GTK_STOCK_PRINT_PREVIEW; // (*)
	else if  (!strcasecmp(vl,"properties"))         *resul=GTK_STOCK_PROPERTIES;
	else if  (!strcasecmp(vl,"quit"))               *resul=GTK_STOCK_QUIT;
	else if  (!strcasecmp(vl,"redo"))               *resul=GTK_STOCK_REDO;
	else if  (!strcasecmp(vl,"refresh"))            *resul=GTK_STOCK_REFRESH;
	else if  (!strcasecmp(vl,"remove"))             *resul=GTK_STOCK_REMOVE; // (*)
	else if  (!strcasecmp(vl,"revert"))             *resul=GTK_STOCK_REVERT_TO_SAVED; // (*)
	else if  (!strcasecmp(vl,"save"))               *resul=GTK_STOCK_SAVE;
	else if  (!strcasecmp(vl,"saveas"))             *resul=GTK_STOCK_SAVE_AS;
	else if  (!strcasecmp(vl,"spellcheck"))         *resul=GTK_STOCK_SPELL_CHECK;
	else if  (!strcasecmp(vl,"stop"))               *resul=GTK_STOCK_STOP;
	else if  (!strcasecmp(vl,"undelete"))           *resul=GTK_STOCK_UNDELETE; // (*)
	else if  (!strcasecmp(vl,"undo"))               *resul=GTK_STOCK_UNDO;
	else if  (!strcasecmp(vl,"yes"))                *resul=GTK_STOCK_YES;// (*)

}

	
gPicture* gStock::get(char *vl)
{
	GtkIconSet *item;
	GtkStyle *style;
	gImage *tmp=NULL,*tmp2=NULL;
	gPicture *ret=NULL;
	gchar **buf;
	long size;
	long r_type;
	bool adapt=false;
	char *stock_icon;
	
	if (!vl) return NULL;
	if (!strlen(vl)) return NULL;
	
	buf=g_strsplit((const gchar*)vl,"/",2);
	
	if (!buf[0]) { g_strfreev(buf); return NULL; }
	if (!buf[1]) { g_strfreev(buf); return NULL; }
	
	size=strtol(buf[0],NULL,10);
	if (errno)
	{
	  if     (!strcasecmp(buf[0],"menu"))         { size=16; r_type=GTK_ICON_SIZE_MENU;}
	  else if(!strcasecmp(buf[0],"smalltoolbar")) { size=20; r_type=GTK_ICON_SIZE_SMALL_TOOLBAR;}
	  else if(!strcasecmp(buf[0],"largetoolbar")) { size=24; r_type=GTK_ICON_SIZE_LARGE_TOOLBAR;}
	  else if(!strcasecmp(buf[0],"button"))       { size=20; r_type=GTK_ICON_SIZE_BUTTON;}
	  else if(!strcasecmp(buf[0],"dnd"))          { size=32; r_type=GTK_ICON_SIZE_DND;}
	  else if(!strcasecmp(buf[0],"dialog"))       { size=48; r_type=GTK_ICON_SIZE_DIALOG;}
	}
	else
	{
		if (size<1) size=1;
		if (size>128) size=128;
		
		if      (size<=16) r_type=GTK_ICON_SIZE_MENU;
		else if (size<=20) r_type=GTK_ICON_SIZE_SMALL_TOOLBAR;
		else if (size<=24) r_type=GTK_ICON_SIZE_LARGE_TOOLBAR;
		else if (size<=32) r_type=GTK_ICON_SIZE_DND;
		else               r_type=GTK_ICON_SIZE_DIALOG;
		
		adapt=true;
	}
	
	gStock_parse(buf[1],&stock_icon);
	if (stock_icon)
	{
		GtkSettings *set=gtk_settings_get_default();
	    style=gtk_rc_get_style_by_paths(set,NULL,"GtkButton",GTK_TYPE_BUTTON);
		item=gtk_style_lookup_icon_set(style,stock_icon);
		
		if (!item)
		{ 
			g_strfreev(buf);
			return NULL;
		}	
		tmp=new gImage(0,0);
		
	    tmp->image=gtk_icon_set_render_icon(item,style, \
		                                    gtk_widget_get_default_direction(), \
		                                    GTK_STATE_NORMAL,(GtkIconSize)r_type,NULL,NULL);
	}
	else
	{
		tmp=gStock_missing(buf[1]);
		adapt=true;
	}
	
	g_strfreev(buf);
	if (!tmp) return NULL;
	
	if (adapt)
	{
		tmp2=tmp->stretch(size,size,true);
		delete tmp;
		tmp=tmp2;
	}
	
	ret=tmp->getPicture();
	delete tmp;
	
	return ret;
}


