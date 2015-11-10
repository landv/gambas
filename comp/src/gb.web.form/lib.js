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
        console.log(xhr.responseText);
        eval(xhr.responseText);
      }
    };
    xhr.send(null);
  },

  raise: function(form, id, event, args)
  {
    gw.send(form, ['raise', id, event, args]);
  },
};