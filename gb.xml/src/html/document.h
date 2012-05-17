#ifndef HDOCUMENT_H
#define HDOCUMENT_H

#include "element.h"
#include "../document.h"


class HtmlDocument : public Document
{
public:


    virtual fwstring getContent(bool indent = false);


    Element* getBody();
    Element* getHead();

    Element* getElementById(fwstring id, int depth) { return root->getChildById(id, depth); }
    GBI::ObjectArray<Element>* getElementsByClassName(fwstring className, int depth = -1) {return root->getChildrenByClassName(className, depth);}

    bool getHtml5() {return html5;}
    void setHtml5(bool val) {html5 = val;}

    void setContent(fwstring str);

    fwstring getTitle();
    void setTitle(fwstring title);

    fwstring getFavicon();
    void setFavicon(fwstring url);

    fwstring getBase();
    void setBase(fwstring base);

    fwstring getLang() {return root->getAttribute("lang");}
    void setLang(fwstring lang) {root->setAttribute("lang", lang);}

    void AddStyleSheet(fwstring src, fwstring media = "screen");
    void AddStyleSheetIfIE(fwstring src, fwstring cond = "IE", fwstring media = "screen");
    void AddStyleSheetIfNotIE(fwstring src, fwstring media = "screen");
    void AddScript(fwstring src);
    void AddScriptIfIE(fwstring src, fwstring cond = "IE");
    void AddScriptIfNotIE(fwstring src);

    Element* getTitleElement();
    Element* getFaviconElement();
    Element* getBaseElement();
    bool html5;

    static GB_CLASS ClassName;

};

#endif
