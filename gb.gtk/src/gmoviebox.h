#ifndef __GMOVIEBOX_H
#define __GMOVIEBOX_H

class gMovieBox : public gControl
{
public:
	gMovieBox(gContainer *parent);
	~gMovieBox();

//"Properties"
	long foreground();
	long background();
	int getBorder() { return getFrameBorder(); }
	bool playing();

	void setForeground(long vl);
	void setBackground(long vl);
	void setBorder(int vl) { setFrameBorder(vl); }
	void setPlaying(bool vl);

//"Methods"
	bool loadMovie(char *buf,long len);

//"Private"
	bool pl;
	guint timeout;
	GdkPixbufAnimation *animation;
	GdkPixbufAnimationIter *iter;


};

#endif
