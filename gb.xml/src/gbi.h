#ifndef GBI_H
#define GBI_H

#include "../gambas.h"
#include "../gb_common.h"
#include <string>
#include <iostream>
#include <vector>

#define CHAR_ERROR 0xFFFD // �


extern "C" GB_INTERFACE GB;

namespace GBI
{

    template<typename T>
    class ObjectArray
    {
    public:
        ObjectArray(const char* className, unsigned int siz = 0) : array(0)
        {
            GB.Array.New(&array, GB.FindClass(className), siz);
        }
        ObjectArray(const char *className, std::vector<T*> &vect);
        ~ObjectArray();

        T* at(unsigned int i) const;
        void push_back(T* value);
        void push_back(const ObjectArray<T> *otherArray);
        void push_back(const std::vector<T*> *otherArray);

        unsigned int size() const{return GB.Array.Count(array);}
        unsigned int length() const{return GB.Array.Count(array);}
        unsigned int count() const{return GB.Array.Count(array);}

        GB_ARRAY array;

    };

    template<typename T>
    ObjectArray<T>::ObjectArray(const char *className, std::vector<T*> &vect) : array(0)
    {
        GB.Array.New(&array, GB.FindClass(className), 0);
        for(unsigned int i = 0; i < vect.size(); i++)
        {
            push_back(vect.at(i));
        }
    }

    template<typename T>
    T* New(const std::string className, char* eventName = "", void* parent = 0)
    {
        return reinterpret_cast<T*>(GB.New(GB.FindClass(className.c_str()), eventName, parent));
    }

    void InitClasses();

    template<typename T>
    T* New()
    {
        if(!T::ClassName) InitClasses();
        return reinterpret_cast<T*>(GB.New(T::ClassName, "", 0));
    }


    template<typename T>
    void ObjectArray<T>::push_back(T* value)
    {
        *(reinterpret_cast<void **>((GB.Array.Add(this->array)))) = value;
        GB.Ref(value);
    }

    template<typename T>
    void ObjectArray<T>::push_back(const ObjectArray<T> *otherArray)
    {
        for(unsigned int i = 0; i < otherArray->length(); i++)
        {
            push_back(otherArray->at(i));
        }
    }

    template<typename T>
    void ObjectArray<T>::push_back(const std::vector<T*> *otherArray)
    {
        for(unsigned int i = 0; i < otherArray->size(); i++)
        {
            push_back(otherArray->at(i));
        }
    }

    template<typename T>
    T* ObjectArray<T>::at(unsigned int i) const
    {
        return *(reinterpret_cast<T**>(GB.Array.Get(this->array,i)));
    }

    template<typename T>
    ObjectArray<T>::~ObjectArray()
    {
        for(unsigned int i = 0; i < this->count(); i++)
        {
            //std::cout << "-1 tab "<< this->at(i) <<" (" << (this->at(i)->ref - 1) << ") ";
            GB.Unref(reinterpret_cast<void**>(GB.Array.Get(this->array,i)));
        }
//        GB.Unref(POINTER((void*)(&(this->array))));
    }

}

class fwstring
{
public:
    fwstring();
    fwstring(char *src, size_t &length);
    fwstring(char *src, int &length);
    fwstring(const fwstring &other);
    ~fwstring();


    wchar_t increment();
    wchar_t increment(size_t count);
    void resetCounter();

    wchar_t at(size_t i);
    wchar_t operator[](size_t i);

    std::string toString() const;

    fwstring& operator+(const fwstring &other);

    bool incrementCompare(const wchar_t* text, size_t len);
    inline bool neof(){return cur < len;}


    char *data;
    size_t len;
    size_t cur;
    size_t wcur;

};

#endif // GBI_H
