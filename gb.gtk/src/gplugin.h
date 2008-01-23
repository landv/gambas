#ifndef __GPLUGIN_H
#define __GPLUGIN_H

class gPlugin : public gControl
{
public:
	gPlugin(gContainer *parent);
	void plug(long id,bool prepared=true);
	void discard();
//"Properties"
	long client();

	int getBorder() { return getFrameBorder(); }
	void setBorder(int vl) { setFrameBorder(vl); }

//"Signals"
	void (*onPlug)(gControl *sender);
	void (*onUnplug)(gControl *sender);

};

#endif
