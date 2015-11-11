function $(a)
{
  return document.getElementById(a);
}

gw = {

  send: function(form, command)
  {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', $root + '/' + form + '/x?c=' + encodeURIComponent(JSON.stringify(command)), true);
    xhr.onreadystatechange = function() {
      if (xhr.readyState == 4 && xhr.status == 200 && xhr.responseText)
      {
        gw.active = document.activeElement.id;
        if (gw.active)
          gw.selection = gw.getSelection($(gw.active));
        else
          gw.selection = undefined;
        
        console.log(xhr.responseText);
        eval(xhr.responseText);
        
        if (gw.active)
        {
          $(gw.active).focus();
          gw.setSelection($(gw.active), gw.selection);
        }
      }
    };
    xhr.send(null);
  },

  raise: function(form, id, event, args)
  {
    gw.send(form, ['raise', id, event, args]);
  },
  
  update: function(form, id, prop, value)
  {
    gw.send(form, ['update', id, prop, value]);
  },

  getSelection: function(o)
  {
    var start, end;
    
    if (o.createTextRange) 
    {
      var r = document.selection.createRange().duplicate();
      r.moveEnd('character', o.value.length)
      if (r.text == '')
        start = o.value.length;
      else
        start = o.value.lastIndexOf(r.text);
      r.moveStart('character', -o.value.length);
      end = r.text.length;
      return [start, end];
    }
    
    if (o.selectionStart && o.selectionEnd)
      return [o.selectionStart, o.selectionEnd];
    
    return null;
  },
  
  setSelection: function(o, sel)
  {
    if (sel)
    {
      if (o.setSelectionRange)
        o.setSelectionRange(sel[0], sel[1]);
    }
  }
}
