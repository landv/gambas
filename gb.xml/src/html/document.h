#ifndef HDOCUMENT_H
#define HDOCUMENT_H

#include "element.h"
#include "../document.h"


class HtmlDocument : public Document
{
public:

    class Virtual : public Document::Virtual
    {
    public:
        Virtual(HtmlDocument *doc) : Document::Virtual(doc), parent(doc) {}
        Virtual(const HtmlDocument::Virtual &copie) : Document::Virtual(copie.parent), parent(copie.parent) {}
        HtmlDocument::Virtual &operator=(const HtmlDocument::Virtual &copie) {parent = copie.parent; return *this;}
        virtual wstring getContent(bool indent = false);

        HtmlDocument *parent;

    };

    Element* getBody();
    Element* getHead();

    Element* getElementById(wstring id, int depth) { return root->getChildById(id, depth); }
    GBI::ObjectArray<Element>* getElementsByClassName(wstring className, int depth = -1) {return root->getChildrenByClassName(className, depth);}

    bool getHtml5() {return html5;}
    void setHtml5(bool val) {html5 = val;}

    void setContent(wstring str);

    wstring getTitle();
    void setTitle(wstring title);

    wstring getFavicon();
    void setFavicon(wstring url);

    wstring getBase();
    void setBase(wstring base);

    wstring getLang() {return root->getAttribute(L"lang");}
    void setLang(wstring lang) {root->setAttribute(L"lang", lang);}

    void AddStyleSheet(wstring src, wstring media = L"screen");
    void AddStyleSheetIfIE(wstring src, wstring cond = L"IE", wstring media = L"screen");
    void AddStyleSheetIfNotIE(wstring src, wstring media = L"screen");
    void AddScript(wstring src);
    void AddScriptIfIE(wstring src, wstring cond = L"IE");
    void AddScriptIfNotIE(wstring src);

    Element* getTitleElement();
    Element* getFaviconElement();
    Element* getBaseElement();
    bool html5;

};

#endif
