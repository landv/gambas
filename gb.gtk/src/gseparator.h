#ifndef __GSEPARATOR_H
#define __GSEPARATOR_H

class gSeparator : public gControl
{
public:
	gSeparator(gContainer *parent);

	long foreground();
	long background();

	void setForeground(long vl);
	void setBackground(long vl);

};

#endif
