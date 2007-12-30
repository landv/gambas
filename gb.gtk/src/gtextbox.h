#ifndef __GTEXTBOX_H
#define __GTEXTBOX_H

class gTextBox : public gControl
{
public:
	gTextBox(gContainer *parent, bool combo = false);

//"Properties"
	int alignment();
	bool hasBorder();
	virtual int length();
	int maxLength();
	bool password();
	int position();
	virtual char *text();
	virtual bool readOnly();
	int selLength();
	int selStart();
	char* selText();
	bool isSelected();

	void setAlignment(int vl);
	void setBorder(bool vl);
	void setMaxLength(int len);
	void setPassword(bool vl);
	void setPosition(int pos);
	virtual void setReadOnly(bool vl);
	virtual void setText(char *vl);
	void setSelText(char *txt, int len);

//"Methods"
	virtual void clear();
	void insert(char* txt,int len);
	void selClear();
	void select(int start,int len);
	void selectAll();

//"Signals"
	void (*onChange)(gTextBox *sender);
	void (*onActivate)(gTextBox *sender);

//"Private"
  void updateCursor(GdkCursor *cursor);
  void initEntry();
  GtkWidget *entry;
	virtual int minimumHeight();
};

#endif
