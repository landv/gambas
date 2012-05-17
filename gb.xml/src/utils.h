#ifndef UTILS_H
#define UTILS_H

#include "main.h"


typedef unsigned int uint;

#define DELETE(_ob) if(_ob) {delete _ob; _ob = 0;}
#define UNREF(_ob) if(_ob) GB.Unref(POINTER(&(_ob)))

#define SCHAR_N 0xA // \n
#define SCHAR_R 0xD // \r

#define CHAR_STARTTAG 0x3C // <
#define CHAR_ENDTAG 0x3E // >

#define CHAR_SINGLEQUOTE 0x27 // '
#define CHAR_DOUBLEQUOTE 0x22 // "

#define CHAR_SLASH 0x2F
#define CHAR_BACKSLASH 0x5C
#define CHAR_EQUAL 0x3D // =
#define CHAR_AND 0x26 // &
#define CHAR_PI 0x3F // ?
#define CHAR_EXCL 0x21 // !
#define CHAR_DASH 0x2D // -

#define CHAR_a 0x61 // a
#define CHAR_z 0x7A // z

#define CHAR_ERROR 0xFFFD // �



class fwstring
{
public:
    fwstring();
    fwstring(char *src, size_t length);
    fwstring(char *src, int &length);
    fwstring(const char *src);
    fwstring(const char src);
    fwstring(const fwstring &other);
    fwstring(int num, const char *s);
    fwstring(int num, const char);
    fwstring(const string &other);
    typedef size_t size_type;

    static const size_type	npos = static_cast<size_type>(-1);

    size_t rfind(const char s);
    size_t rfind(const char *s);

    size_t rfind(const fwstring &s);


    ~fwstring();



    char at(size_t i){return data[i];}
    char operator[](size_t i){return data[i];}

    wchar_t increment(size_t &cur)const;

    string toStdString() const;
    fwstring toString() const;
    fwstring* copyString(uint start, uint end) const;
    fwstring* copy();

    size_t length()const {return len;}
    size_t size()const {return len;}
    char* c_str() const {return data;}
    void erase();

    bool operator==(const char *other);

    fwstring& operator=(char s);
    fwstring& operator=(const fwstring other);
    fwstring& operator+=(const char* other);
    fwstring& operator+=(const fwstring other);
    fwstring& operator+=(const char other);

    fwstring substr(size_t start, size_t nlen = npos);
    fwstring* ssubstr(size_t start, size_t nlen);
    //fwstring substr(size_t start, size_t nlen = npos){return *(substr(start, len));}



    char *data;
    size_t len;

};



template<class T>
string toString(T i)
{
    std::stringstream ss;
    string s;
    ss << i;
    s = ss.str();
    return s;
}

template<class T>
inline T Maxi(T a, T b)
{
    if(a > b)
        return a;
    return b;
}
template<class T>
inline T Mini(T a, T b)
{
    if(a < b)
        return a;
    return b;
}
/*
fwstring (const fwstring& s);

fwstring (const char *src, const uint &len);
fwstring (const fwstring& s);*/

fwstring Html$(fwstring text);


#undef STRING
#define STRING(_arg) (CSTRING(_arg) != 0 ? fwstring(CSTRING(_arg), VARG(_arg).len) : fwstring())
#define CSTRING(_arg) (VARG(_arg).addr + VARG(_arg).start)

#undef PSTRING
#define PSTRING(_arg) (CPSTRING(_arg) != 0 ? fwstring(CPSTRING(_arg), VPROP(GB_STRING).len) : fwstring())
#define CPSTRING(_arg) (VPROP(GB_STRING).addr + VPROP(GB_STRING).start)

#define STRINGOPT(arg, def) (MISSING(arg) ? def : STRING(arg))

#define VARGOBJ(type, arg) reinterpret_cast<type*>(VARG(arg))

bool isNameStartChar(fwstring &s);
bool isNameChar(fwstring &s);
bool isNameStartChar(const wchar_t s);
bool isNameChar(const wchar_t s);
bool isWhiteSpace(fwstring &s);
bool isWhiteSpace(const wchar_t s);
bool isWhiteSpace(const char s);

inline const void* memchrs(const char *source, size_t lensource, const char *comp, size_t lencomp)
{
    const char *pos = source - 1;
    register size_t i = 0;
    do
    {
        pos = (char*)(memchr((void*)(pos + 1), ((comp))[0], lensource));
        if(!pos) return 0;
        if(pos + lencomp > source + lensource) return 0;
        for(i = 1; i < lencomp; i++)
        {
            if(*((pos + i)) == (comp)[i]) return pos;
        }

    }while(1);
}
const void* memrchrs(const void *source, size_t lensource, const void *comp, size_t lencomp);


template<typename T>
class fvector
{
public:
    fvector();
    void push_back(T newelement);
    void push_front(T newelement);

    size_t size(){return len;}

    T at(size_t i);

    size_t len;

    T *data;

};

template<typename T>
class flist
{
public:
    flist();
    void push_back(T value);
    void push_front(T value);


    typedef struct element
    {
        T value;
        element *next;
        element *previous;

    }element;


    void remove(element *elmt);
    void insertAfter(element *elmt, T value);
    void insertBefore(element *elmt, T value);
    void clear();

    size_t len;
    element *firstElement;
    element *lastElement;
};

template<typename T>
flist<T>::flist()
{
    //DEBUG << "new " << this << endl;
    len = 0;
    firstElement = 0;
    lastElement = 0;
}

template<typename T>
void flist<T>::clear()
{
    for(element *it = firstElement; it != 0; it = it->next)
    {
        if(it->previous) free(it->previous);
    }

    free(lastElement);
    firstElement = 0; lastElement = 0; len = 0;

}

template<typename T>
void flist<T>::insertAfter(element *elmt, T value)
{
    element *newElement = (element*)malloc(sizeof(element));
    newElement->value = value;
    newElement->next = elmt->next;
    newElement->previous = elmt;
    if(elmt->next)
    {
        elmt->next->previous = newElement;
    }
    if(elmt == lastElement)
    {
        lastElement = newElement;
    }
    elmt->next = newElement;
    len++;
}

template<typename T>
void flist<T>::insertBefore(element *elmt, T value)
{
    element *newElement = (element*)malloc(sizeof(element));
    newElement->value = value;
    newElement->next = elmt;
    newElement->previous = elmt->previous;
    if(elmt->previous)
    {
        elmt->previous->next = newElement;
    }
    if(elmt == firstElement)
    {
        firstElement = newElement;
    }
    elmt->previous = newElement;
    len++;
}

template<typename T>
void flist<T>::remove(element *elmt)
{
    if(elmt == firstElement) firstElement = elmt->next;
    if(elmt == lastElement) lastElement = elmt->previous;
    elmt->next->previous = elmt->previous;
    elmt->previous->next = elmt->next;
    free(elmt);
    len--;
}

template<typename T>
void flist<T>::push_back(T value)
{
    len++;
    if(!lastElement)//La liste est vide
    {
        firstElement = (element*)malloc(sizeof(flist<T>::element));
        lastElement = firstElement;
        lastElement->value = value;
        lastElement->next = 0;
        lastElement->previous = 0;
        return;
    }

    flist<T>::element *newElement =  (element*)malloc(sizeof(flist<T>::element));
    newElement->previous = lastElement;
    newElement->value = value;
    newElement->next = 0;
    lastElement->next = newElement;
    lastElement = newElement;

}

template<typename T>
void flist<T>::push_front(T value)
{
    len++;
    if(!firstElement)//La liste est vide
    {
        lastElement = (element*)malloc(sizeof(flist<T>::element));
        firstElement = lastElement;
        firstElement->value = value;
        firstElement->next = 0;
        firstElement->previous = 0;
        return;
    }

    flist<T>::element *newElement =  (element*)malloc(sizeof(flist<T>::element));
    newElement->next = firstElement;
    newElement->value = value;
    newElement->previous = 0;
    firstElement->previous = newElement;
    firstElement = newElement;

}


fwstring operator+(fwstring char1, const char *char2);
fwstring operator+(const char char1, fwstring char2);
fwstring operator+(const char *char1, fwstring char2);
fwstring operator+(fwstring char1, fwstring char2);
bool operator<(fwstring a, fwstring b);
bool operator>(fwstring a, fwstring b);

bool operator==(fwstring const &a, fwstring const& b);
bool operator==(fwstring const &a, const char* b);
bool operator!=(fwstring const &a, fwstring const& b);

std::ostream &operator<<( std::ostream &out, fwstring str );


template<typename T>
fvector<T>::fvector()
{
    len = 0;
    data = 0;
}

template<typename T>
void fvector<T>::push_back(T newelement)
{
    //DEBUG << "pushback " << newelement << endl;

    if(!len)
    {
        len = 1;
        data = (T*)malloc(sizeof(T));
        data[0] = newelement;
        //DEBUG << "pushbacked " << data[0] << endl;
        return;
    }


    T* newdata = (T*)malloc(sizeof(T) * (len+1));
    //DEBUG << "oldpushbacked " << data[0] << endl;
    memcpy(newdata, data, sizeof(T)* len);
    //DEBUG << "oldpushbacked " << data[0] << endl;
    //DEBUG << "oldpushbacked " << newdata[0] << endl;
    //free(data);
    data = newdata;
    data[len] = newelement;
    //DEBUG << "pushbacked " << data[len] << endl;
    len++;
}

template<typename T>
T fvector<T>::at(size_t i)
{
    return data[i];
}




#endif // UTILS_H
