
#include "document.h"
#include "../document.cpp"

Element* HtmlDocument::getTitleElement()
{
    Element *head = getHead();
    vector<Element*>* elmts = head->getChildrenByTagName(L"title");
    Element *elmt;
    if(elmts->size() <= 0)
    {
        elmt = GBI::New<Element>("XmlElement");
        elmt->setTagName(L"title");
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
    vector<Element*>* elmts = head->getChildrenByTagName(L"base");
    Element *elmt;
    if(elmts->size() <= 0)
    {
        elmt = GBI::New<Element>("XmlElement");
        elmt->setTagName(L"base");
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
    vector<Element*>* elmts = head->getChildrenByTagName(L"link");
    Element *elmt;

    for(unsigned int i = 0; i < elmts->size(); i++)
    {
        if(elmts->at(i)->getAttribute(L"rel") == L"icon")
        {
            elmt = elmts->at(i);
            delete elmts;
            return elmt;
        }
    }

    delete elmts;
    elmt = GBI::New<Element>("XmlElement");
    elmt->setTagName(L"link");
        elmt->setAttribute(L"rel", L"icon");
        head->appendChild(elmt);
        return elmt;

}

wstring HtmlDocument::getBase()
{
    return getBaseElement()->getAttribute(L"href");
}

void HtmlDocument::setBase(wstring base)
{
    getBaseElement()->setAttribute(L"href", base);
}

wstring HtmlDocument::getFavicon()
{
    return getFaviconElement()->getAttribute(L"href");
}

void HtmlDocument::setFavicon(wstring url)
{
    getFaviconElement()->setAttribute(L"href", url);
}

void HtmlDocument::AddStyleSheet(wstring src, wstring media)
{
    Element *elmt = GBI::New<Element>("XmlElement");
    elmt->setTagName(L"link");
    elmt->setAttribute(L"rel", L"stylesheet");
    elmt->setAttribute(L"href", src);
    elmt->setAttribute(L"type", L"text/css");
    elmt->setAttribute(L"media", media);
    getHead()->appendChild(elmt);
}

void HtmlDocument::AddStyleSheetIfIE(wstring src, wstring cond, wstring media)
{
    wstring content;
    content = L"[if "+cond+L"]>";

    Element *elmt = GBI::New<Element>("XmlElement");
    elmt->setTagName(L"link");
    elmt->setAttribute(L"rel", L"stylesheet");
    elmt->setAttribute(L"href", src);
    elmt->setAttribute(L"type", L"text/css");
    elmt->setAttribute(L"media", media);

    content += elmt->toString();
    content += L"<![endif]";
    GB.Unref(POINTER(&elmt));

    CommentNode *comment = GBI::New<CommentNode>("XmlCommentNode");
    comment->setTextContent(content);
    getHead()->appendChild(comment);
}

void HtmlDocument::AddStyleSheetIfNotIE(wstring src, wstring media)
{
    Element *head = getHead();
    CommentNode *comment = GBI::New<CommentNode>("XmlCommentNode");
    comment->setTextContent(L"[if !IE]><");
    head->appendChild(comment);
    AddStyleSheet(src, media);
    comment = GBI::New<CommentNode>("XmlCommentNode");
    comment->setTextContent(L"><![endif]");
    head->appendChild(comment);
}

void HtmlDocument::AddScript(wstring src)
{
    Element *elmt = GBI::New<Element>("XmlElement");
    elmt->setTagName(L"script");
    elmt->setAttribute(L"src", src);
    elmt->setAttribute(L"type", L"text/javascript");
    getHead()->appendChild(elmt);
}

void HtmlDocument::AddScriptIfIE(wstring src, wstring cond)
{
    wstring content;
    content = L"[if "+cond+L"]>";

    Element *elmt = GBI::New<Element>("XmlElement");
    elmt->setTagName(L"script");
    elmt->setAttribute(L"src", src);
    elmt->setAttribute(L"type", L"text/javascript");

    content += elmt->toString();
    content += L"<![endif]";
    GB.Unref(POINTER(&elmt));

    CommentNode *comment = GBI::New<CommentNode>("XmlCommentNode");
    comment->setTextContent(content);
    getHead()->appendChild(comment);
}

void HtmlDocument::AddScriptIfNotIE(wstring src)
{
    Element *head = getHead();
    CommentNode *comment = GBI::New<CommentNode>("XmlCommentNode");
    comment->setTextContent(L"[if !IE]><");
    head->appendChild(comment);
    AddScript(src);
    comment = GBI::New<CommentNode>("XmlCommentNode");
    comment->setTextContent(L"><![endif]");
    head->appendChild(comment);;
}

wstring HtmlDocument::getTitle()
{
    return getTitleElement()->textContent();
}

void HtmlDocument::setTitle(wstring text)
{
    getTitleElement()->setTextContent(text);
}

wstring HtmlDocument::Virtual::getContent(bool indent)
{
    wstring doctype;
    if (parent->html5)
    {
        doctype = L"<!DOCTYPE html>\n";
    }
    else
    {
        doctype = L"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n";
    }

    return doctype + parent->root->toString(indent ? 0 : -1);
}

void HtmlDocument::setContent(wstring str)
{
    //Doctype ?
    unsigned int i;
    wstring s;
    for (i = 0; i < str.length(); i++)
    {
        s = str[i];
        if(!(s == L" " || s == L"\n" || s == L"\r" || s == L"\t")) break; //On enlève les espaces du début
    }

    vector<Node*>* elements = 0;
    if(str.substr(i + 1, 9) == L"!DOCTYPE " )//Il y a un doctype
    {
        i += 10;
        wstring doctype;
        for(;i < str.length(); i++)//On prend le contenu
        {
            s = str[i];
            if(s == L">") break;
            doctype += s;
        }
        if(doctype == L"html")
        {
            html5 = true;
        }
        else
        {
            html5 = false;
        }
        i++;
        elements = Element::fromText(str, i, i + 1, 1);

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

    if(!root)throw HTMLParseException(0, 0, L"somewhere", L"No valid root element found.");

}

Element* HtmlDocument::getHead()
{
    Element *elmt = root->getFirstChildByTagName(L"head", 2);
    if(!elmt)
    {
        elmt = GBI::New<Element>("XmlElement");
        elmt->setTagName(L"head");
        root->appendChild(elmt);
    }
        return elmt;
}

Element* HtmlDocument::getBody()
{
    Element *elmt = root->getFirstChildByTagName(L"body", 2);
    if(!elmt)
    {
        elmt = GBI::New<Element>("XmlElement");
        elmt->setTagName(L"body");
        root->appendChild(elmt);
    }
        return elmt;
}
