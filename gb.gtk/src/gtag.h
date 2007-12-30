#ifndef __GTAG_H
#define __GTAG_H

class gTag
{
public:
	gTag() {}
	gTag(void *v) { value = v; }
	virtual ~gTag() {}
	virtual void ref(void *v) {};
	virtual void unref(void *v) {};
	void *get() { return value; }
	void ref() { ref(value); }
	void unref() { unref(value); }
private:
	void *value;
};

#endif
