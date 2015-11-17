function $(a)
{
  return document.getElementById(a);
}

gw = {

  timers: {},
  windows: [],
  form: '',

  send: function(command)
  {
    var xhr = new XMLHttpRequest();
    console.log(command);
    xhr.open('GET', $root + '/' + encodeURIComponent(gw.form) + '/x?c=' + encodeURIComponent(JSON.stringify(command)), true);
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
          if ($(gw.active))
          {
            $(gw.active).focus();
            gw.setSelection($(gw.active), gw.selection);
          }
          else
            gw.active = document.activeElement.id;
        }
      }
    };
    xhr.send(null);
  },

  raise: function(id, event, args)
  {
    gw.send(['raise', id, event, args]);
  },
  
  update: function(id, prop, value)
  {
    gw.send(['update', id, prop, value]);
  },

  getSelection: function(o)
  {
    var start, end;
    
    try
    {
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
    }
    catch(e) {};
    
    return undefined;
  },
  
  setSelection: function(o, sel)
  {
    if (sel)
    {
      if (o.setSelectionRange)
        try { o.setSelectionRange(sel[0], sel[1]) } catch(e) {};
    }
  },
  
  setFocus: function(id)
  {
    $(id).focus();
    gw.active = document.activeElement.id;
    gw.selection = undefined;
  },
  
  resizeComboBox: function(id)
  {
    $(id + '-select').onmouseover = function() { $(id + '-select').style.width = $(id).offsetWidth + 'px'; }
  },
  
  addTimer: function(id, delay)
  {
    gw.removeTimer(id);
    gw.timers[id] = setInterval(function() { gw.raise(id, 'timer'); }, delay);
  },
  
  removeTimer: function(id)
  {
    var t = gw.timers[id];
    if (t)
    {
      clearInterval(gw.timers[id]);
      gw.timers[id] = undefined;
    }
  },
  
  window: 
  {
    open: function(id, resizable)
    {
      gw.window.close(id);

      if (gw.windows.length == 0)
      {
        document.addEventListener('mousemove', gw.window.onMove);
        document.addEventListener('mouseup', gw.window.onUp);
        console.log('document.addEventListener');
      }
      
      $(id).style.zIndex = 10 + gw.windows.length;
      gw.windows.push(id);
      
      $(id).addEventListener('mousedown', gw.window.onMouseDown);
      
      $(id).gw_resizable = resizable;
      // Touch events 
      //pane.addEventListener('touchstart', onTouchDown);
      //document.addEventListener('touchmove', onTouchMove);
      //document.addEventListener('touchend', onTouchEnd);
      
      gw.window.updateTitleBars();
    },
    
    close: function(id)
    {
      var i;
      
      $(id).removeEventListener('mousedown', gw.window.onMouseDown);
      
      i = gw.windows.indexOf(id);
      if (i >= 0)
      {
        gw.windows.splice(i, 1);
        gw.window.refresh();
      }
      
    },
    
    refresh: function()
    {
      var i = 0;
      
      while (i < gw.windows.length)
      {
        if ($(gw.windows[i]))
        {
          $(gw.windows[i]).style.zIndex = 10 + i;
          i++;
        }
        else
          gw.windows.splice(i, 1);
      }
      
      if (gw.windows.length == 0)
      {
        console.log('document.removeEventListener');
        document.removeEventListener('mousemove', gw.window.onMove);
        document.removeEventListener('mouseup', gw.window.onUp);
      }
    },
    
    updateTitleBars: function()
    {
      var i;
      
      for (i = 0; i < gw.windows.length - 1; i++)
        $(gw.windows[i] + '-titlebar').style.backgroundColor = '#C0C0C0';
      
      $(gw.windows[gw.windows.length - 1] + '-titlebar').style.backgroundColor = '';
    },
    
    raise: function(id, send)
    {
      var i = gw.windows.indexOf(id);
      if (i < 0)
        return;
      
      gw.windows.splice(i, 1);
      gw.windows.push(id);
      
      for (i = 0; i < gw.windows.length; i++)
        $(gw.windows[i]).style.zIndex = 10 + i;
        
      gw.window.updateTitleBars();
        
      if (send)
        gw.update('', '#windows', gw.windows);
    },
    
    onMouseDown: function(e)
    {
      gw.window.onDown(e);
    },
    
    onDown: function(e)
    {
      var c;
      
      if (e.target.className == 'gw-window-button')
        return;
      
      gw.window.onMove(e);
      
      c = gw.window.context;
      if (c && (c.isMoving || c.isResizing))
      {
        gw.window.raise(c.id);
        gw.window.downEvent = e;
        e.preventDefault();
      }
    },
    
    onMove(e) 
    {
      var i, id, elt, b, x, y, bx, by, bw, bh;
      var onTopEdge, onLeftEdge, onRightEdge, onBottomEdge, isResizing;
      var MARGINS = 6;
      
      if (gw.window.downEvent)
      {
        gw.window.context.cx = e.clientX;
        gw.window.context.cy = e.clientY;
        gw.window.animate();
        return;
      }
      
      gw.window.context = undefined;
      
      for (i = 0; i < gw.windows.length; i++)
      {
        id = gw.windows[gw.windows.length - i - 1];
        elt = $(id);
        b = elt.getBoundingClientRect();
        
        bx = b.left; // - MARGINS;
        by = b.top; // - MARGINS;
        bw = b.width; // + MARGINS * 2;
        bh = b.height; // + MARGINS * 2;
        
        x = e.clientX - bx;
        y = e.clientY - by;
        
        //console.log(x + ',' + y + ' : ' + bx + ',' + by + ',' + bw + ',' + bh);
        
        if (x >= 0 && x < bw && y >= 0 && y < bh)
        {
          if (elt.gw_resizable)
          {
            onTopEdge = y < MARGINS;
            onLeftEdge = x < MARGINS;
            onRightEdge = x >= (bw - MARGINS);
            onBottomEdge = y >= (bh - MARGINS);
            
            isResizing = onTopEdge || onLeftEdge || onRightEdge || onBottomEdge;
          }
          else
            onTopEdge = onLeftEdge = onRightEdge = onBottomEdge = isResizing = false;
            
          isMoving = !isResizing && y < ($(id + '-titlebar').offsetHeight + MARGINS);
          
          gw.window.context = {
            id: id,
            x: b.left + window.scrollX,
            y: b.top + window.scrollY,
            cx: e.clientX,
            cy: e.clientY,
            w: b.width,
            h: b.height,
            isResizing: isResizing,
            isMoving: isMoving,
            onTopEdge: onTopEdge,
            onLeftEdge: onLeftEdge,
            onRightEdge: onRightEdge,
            onBottomEdge: onBottomEdge
            };
          gw.window.animate();
          break;
        }
      }
    },
    
    onUp: function(e)
    {
      var c = gw.window.context;
      
      gw.window.downEvent = undefined;
      
      if (c && (c.isMoving || c.isResizing))
      {
        var id = gw.window.context.id;
        var b = $(id).getBoundingClientRect();
        gw.window.context = undefined;
        gw.window.raise(id, true);
        gw.update(id, '#geometry', [ b.left + 'px', b.top + 'px', b.width + 'px', b.height + 'px']);
      }
    },

    animate: function() 
    {
      var id, elt, c, e, x, y, w, h;
      var minWidth = 120;
      var minHeight;
      
      //requestAnimationFrame(gw.window.animate);
      
      c = gw.window.context;
      if (!c) return;
    
      elt = $(c.id);
      minHeight = $(c.id + '-titlebar').offsetHeight + 2;
      e = gw.window.downEvent;
    
      if (c && c.isResizing && e)
      {
        if (c.onRightEdge)
          elt.style.width = Math.max(c.w + c.cx - e.clientX, minWidth) + 'px';
        
        if (c.onBottomEdge)
          elt.style.height = Math.max(c.h + c.cy - e.clientY, minHeight) + 'px';
    
        if (c.onLeftEdge) 
        {
          x = c.x + c.cx - e.clientX;
          w = c.x + c.w - x;
          if (w >= minWidth) 
          {
            elt.style.width = w + 'px';
            elt.style.left = x + 'px'; 
          }
        }
    
        if (c.onTopEdge) 
        {
          y = c.y + c.cy - e.clientY;
          h = c.y + c.h - y;
          if (h >= minHeight) 
          {
            elt.style.height = h + 'px';
            elt.style.top = y + 'px'; 
          }
        }
    
        return;
      }
    
      if (c && c.isMoving && e)
      {
        elt.style.left = (Math.max(0, c.x + c.cx - e.clientX)) + 'px';
        elt.style.top = (Math.max(0, c.y + c.cy - e.clientY)) + 'px';
        return;
      }
    
      // This code executes when mouse moves without clicking
    
      // style cursor
      if (c.onRightEdge && c.onBottomEdge || c.onLeftEdge && c.onTopEdge)
        elt.style.cursor = 'nwse-resize';
      else if (c.onRightEdge && c.onTopEdge || c.onBottomEdge && c.onLeftEdge)
        elt.style.cursor = 'nesw-resize';
      else if (c.onRightEdge || c.onLeftEdge)
        elt.style.cursor = 'ew-resize';
      else if (c.onBottomEdge || c.onTopEdge)
        elt.style.cursor = 'ns-resize';
      else
        elt.style.cursor = '';
    }
  }
}
