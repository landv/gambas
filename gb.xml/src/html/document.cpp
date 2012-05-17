
#include "document.h"
#include "../document.cpp"

GB_CLASS HtmlDocument::ClassName = 0;

Element* HtmlDocument::getTitleElement()
{
    Element *head = getHead();
    vector<Element*>* elmts = head->getChildrenByTagName("title");
    Element *elmt;
    if(elmts->size() <= 0)
    {
        elmt = new Element;
        elmt->setTagName("title");
        head->appendChild(elmt);
        return elmt;
    }
    else
    {
        elmt = elmts->at(0);
        delete elmts;
        return elmt;
    }
}

Element* HtmlDocument::getBaseElement()
{
    Element *head = getHead();
    vector<Element*>* elmts = head->getChildrenByTagName("base");
    Element *elmt;
    if(elmts->size() <= 0)
    {
        elmt = new Element;
        elmt->setTagName("base");
        head->prependChild(elmt);
    }
    else
    {
        elmt = elmts->at(0);
    }

    delete elmts;
    return elmt;
}

Element* HtmlDocument::getFaviconElement()
{
    Element *head = getHead();
    vector<Element*>* elmts = head->getChildrenByTagName("link");
    Element *elmt;

    for(unsigned int i = 0; i < elmts->size(); i++)
    {
        if(elmts->at(i)->getAttribute("re") == "icon")
        {
            elmt = elmts->at(i);
            delete elmts;
            return elmt;
        }
    }

    delete elmts;
    elmt = new Element;
    elmt->setTagName("link");
        elmt->setAttribute("rel", "icon");
        head->appendChild(elmt);
        return elmt;

}

fwstring HtmlDocument::getBase()
{
    return getBaseElement()->getAttribute("href");
}

void HtmlDocument::setBase(fwstring base)
{
    getBaseElement()->setAttribute("href", base);
}

fwstring HtmlDocument::getFavicon()
{
    return getFaviconElement()->getAttribute("href");
}

void HtmlDocument::setFavicon(fwstring url)
{
    getFaviconElement()->setAttribute("href", url);
}

void HtmlDocument::AddStyleSheet(fwstring src, fwstring media)
{
    Element *elmt = new Element;
    elmt->setTagName("link");
    elmt->setAttribute("rel", "stylesheet");
    elmt->setAttribute("href", src);
    elmt->setAttribute("type", "text/css");
    elmt->setAttribute("media", media);
    getHead()->appendChild(elmt);
}

void HtmlDocument::AddStyleSheetIfIE(fwstring src, fwstring cond, fwstring media)
{
    fwstring content;
    content = "[if "+cond+"]>";

    Element *elmt = new Element;
    elmt->setTagName("link");
    elmt->setAttribute("rel", "stylesheet");
    elmt->setAttribute("href", src);
    elmt->setAttribute("type", "text/css");
    elmt->setAttribute("media", media);

    content += elmt->toString();
    content += "<![endif]";
    GB.Unref(POINTER(&elmt));

    CommentNode *comment = new CommentNode;
    comment->setTextContent(content);
    getHead()->appendChild(comment);
}

void HtmlDocument::AddStyleSheetIfNotIE(fwstring src, fwstring media)
{
    Element *head = getHead();
    CommentNode *comment =  new CommentNode;
    comment->setTextContent("[if !IE]><");
    head->appendChild(comment);
    AddStyleSheet(src, media);
    comment =  new CommentNode;
    comment->setTextContent("><![endif]");
    head->appendChild(comment);
}

void HtmlDocument::AddScript(fwstring src)
{
    Element *elmt = new Element;
    elmt->setTagName("script");
    elmt->setAttribute("src", src);
    elmt->setAttribute("type", "text/javascript");
    getHead()->appendChild(elmt);
}

void HtmlDocument::AddScriptIfIE(fwstring src, fwstring cond)
{
    fwstring content;
    content = "[if "+cond+"]>";

    Element *elmt = new Element;
    elmt->setTagName("script");
    elmt->setAttribute("src", src);
    elmt->setAttribute("type", "text/javascript");

    content += elmt->toString();
    content += "<![endif]";
    GB.Unref(POINTER(&elmt));

    CommentNode *comment = new CommentNode;
    comment->setTextContent(content);
    getHead()->appendChild(comment);
}

void HtmlDocument::AddScriptIfNotIE(fwstring src)
{
    Element *head = getHead();
    CommentNode *comment = new CommentNode;
    comment->setTextContent("[if !IE]><");
    head->appendChild(comment);
    AddScript(src);
    comment = new CommentNode;
    comment->setTextContent("><![endif]");
    head->appendChild(comment);;
}

fwstring HtmlDocument::getTitle()
{
    return getTitleElement()->textContent();
}

void HtmlDocument::setTitle(fwstring text)
{
    getTitleElement()->setTextContent(text);
}

fwstring HtmlDocument::getContent(bool indent)
{
    fwstring doctype;
    if (html5)
    {
        doctype = "<!DOCTYPE html>\n";
    }
    else
    {
        doctype = "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n";
    }

    doctype += root->toString(indent ? 0 : -1);
    return doctype;
}

void HtmlDocument::setContent(fwstring str)
{
    //Doctype ?
    unsigned int i;
    fwstring s;
    for (i = 0; i < str.length(); i++)
    {
        s = str[i];
        if(!(s == " " || s == "\n" || s == "\r" || s == "\t")) break; //On enlève les espaces du début
    }

    fvector<Node*>* elements = 0;
    if(str.substr(i + 1, 9) == "!DOCTYPE " )//Il y a un doctype
    {
        i += 10;
        fwstring doctype;
        for(;i < str.length(); i++)//On prend le contenu
        {
            s = str[i];
            if(s == ">") break;
            doctype += s;
        }
        if(doctype == "htm")
        {
            html5 = true;
        }
        else
        {
            html5 = false;
        }
        i++;
        elements = Element::fromText(str, i);

    }
    else
    {
        elements = Element::fromText(str);
    }

    GB.Unref(POINTER(&root));
    root = 0;
    Node *node = 0;
    for(i = 0; i < elements->size(); i++)
    {
        node = elements->at(i);
        if(node->isElement() && !root)
        {
            root = node->toElement();
            root->setOwnerDocument(this);
            GB.Ref(root);
        }
        else
        {
            GB.Unref(POINTER(&node));
        }
    }

    delete elements;

    if(!root)throw HTMLParseException(0, 0, "somewhere", "No valid root element found.");

}

Element* HtmlDocument::getHead()
{
    Element *elmt = root->getFirstChildByTagName("head", 2);
    if(!elmt)
    {
        elmt = new Element;
        elmt->setTagName("head");
        root->appendChild(elmt);
    }
        return elmt;
}

Element* HtmlDocument::getBody()
{
    Element *elmt = root->getFirstChildByTagName("body", 2);
    if(!elmt)
    {
        elmt = new Element;
        elmt->setTagName("body");
        root->appendChild(elmt);
    }
        return elmt;
}
