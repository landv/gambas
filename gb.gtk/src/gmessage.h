#ifndef __GMESSAGE_H
#define __GMESSAGE_H

class gMessage
{
public:
	static int showDelete(char *msg,char *btn1,char *btn2,char *btn3);
	static int showError(char *msg,char *btn1,char *btn2,char *btn3);
	static int showInfo(char *msg,char *btn1);
	static int showQuestion(char *msg,char *btn1,char *btn2,char *btn3);
	static int showWarning(char *msg,char *btn1,char *btn2,char *btn3);

	static void setTitle(char *title);
	static char *title();
		
	static void exit();
};

#endif
