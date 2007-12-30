#ifndef __GTEXTAREA_H
#define __GTEXTAREA_H

class gTextArea : public gControl
{
public:
	gTextArea(gContainer *parent);

//"Properties"
	bool hasBorder();
	int column();
	int length();
	int line();
	int position();
	bool readOnly();
	char* text();
	bool wrap();
	bool isSelected();

	void setBorder(bool vl);
	void setColumn(int vl);
	void setLine(int vl);
	void setPosition(int vl);
	void setReadOnly(bool vl);
	void setText(char *txt);
	void setWrap(bool vl);
	
	int textWidth();
	int textHeight();

//"Methods"
	void copy();
	void cut();
	void ensureVisible();
	void paste();
	void insert(char *txt);
	int toLine(int pos);
	int toColumn(int pos);
	int toPosition(int line,int col);

//"Selection properties"
	int selStart();
	int selEnd();
	char* selText();

	void setSelText(char *vl);

//"Selection methods"
	void selDelete();
	void selSelect(int start,int length);

//"Signals"
	void (*onChange)(gTextArea *sender);
	void (*onCursor)(gTextArea *sender);

//"Private"
  void updateCursor(GdkCursor *cursor);
  void waitForLayout(int *tw, int *th);
};

#endif
