function $(a)
{
  return document.getElementById(a);
}

gw = {

  timers: {},
  windows: [],
  form: '',
  debug: false,

  log: function(msg)
  {
    if (gw.debug)
      console.log(msg);
  },

  send: function(command)
  {
    var xhr = new XMLHttpRequest();
    gw.log(command);
    xhr.open('GET', $root + '/' + encodeURIComponent(gw.form) + '/x?c=' + encodeURIComponent(JSON.stringify(command)), true);
    xhr.onreadystatechange = function() {
      //console.log('state = ' + xhr.readyState + ' / status = ' + xhr.status);
      if (xhr.readyState == 4 && xhr.status == 200 && xhr.responseText)
      {
        gw.active = document.activeElement.id;
        if (gw.active)
          gw.selection = gw.getSelection($(gw.active));
        else
          gw.selection = undefined;
        
        if (gw.debug)
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
    zIndex: 0,
    
    open: function(id, resizable, modal, minw, minh)
    {
      gw.window.close(id);

      if (gw.windows.length == 0)
      {
        document.addEventListener('mousemove', gw.window.onMove);
        document.addEventListener('mouseup', gw.window.onUp);
        gw.log('document.addEventListener');
      }
      
      gw.windows.push(id);
      
      $(id).addEventListener('mousedown', gw.window.onMouseDown);
      
      $(id).gw_resizable = resizable;
      $(id).gw_modal = modal;
      
      if (minw != undefined)
      {
        $(id).gw_minw = minw;
        $(id).gw_minh = minh;
      }
      else
      {
        $(id).gw_minw = $(id).offsetWidth;
        $(id).gw_minh = $(id).offsetHeight;
      }
      
      //console.log('gw.window.open: minw = ' + $(id).gw_minw + ' minh = ' + $(id).gw_minh);
      
      // Touch events 
      //pane.addEventListener('touchstart', onTouchDown);
      //document.addEventListener('touchmove', onTouchMove);
      //document.addEventListener('touchend', onTouchEnd);
      
      gw.window.updateTitleBars();
      gw.window.refresh();
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
          $(gw.windows[i]).style.zIndex = 11 + i * 2;
          i++;
        }
        else
          gw.windows.splice(i, 1);
      }
      
      gw.window.updateModal();
      
      if (gw.windows.length == 0)
      {
        gw.log('document.removeEventListener');
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
        $(gw.windows[i]).style.zIndex = 11 + i * 2;
        
      gw.window.updateTitleBars();
        
      if (send)
        gw.update('', '#windows', gw.windows);
    },
    
    updateModal: function()
    {
      var i, elt = $('gw-modal');
      
      for (i = gw.windows.length - 1; i >= 0; i--)
      {
        if ($(gw.windows[i]).gw_modal)
        {
          gw.window.zIndex = 10 + i * 2;
          elt.style.zIndex = 10 + i * 2;
          elt.style.display = 'block';
          return;
        }
      }
      
      gw.window.zIndex = 0;
      elt.style.display = 'none';
    },
    
    center: function(id)
    {
      $(id).style.left = ((window.innerWidth - $(id).offsetWidth) / 2 + 0) + 'px';
      $(id).style.top = ((window.innerHeight - $(id).offsetHeight) / 2 + 0) + 'px';
      gw.window.updateGeometry(id);
    },
    
    onMouseDown: function(e)
    {
      gw.window.onDown(e);
    },
    
    onDown: function(e)
    {
      var c;
      
      gw.window.context = undefined;
      
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
        
        if (elt.style.zIndex < gw.window.zIndex)
          continue;
        
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
    
    updateGeometry: function(id)
    {
      var b = $(id).getBoundingClientRect();
      gw.update(id, '#geometry', [ b.left + 'px', b.top + 'px', b.width + 'px', b.height + 'px']);
    },
    
    onUp: function(e)
    {
      var c = gw.window.context;
      
      gw.window.downEvent = undefined;
      
      if (c && (c.isMoving || c.isResizing))
      {
        var id = gw.window.context.id;
        gw.window.context = undefined;
        gw.window.raise(id, true);
        gw.window.updateGeometry(id);
      }
    },

    animate: function() 
    {
      var id, elt, c, e, x, y, w, h;
      var minWidth;
      var minHeight;
      
      //requestAnimationFrame(gw.window.animate);
      
      c = gw.window.context;
      if (!c) return;
    
      elt = $(c.id);
      minWidth = elt.gw_minw;
      minHeight = elt.gw_minh; //$(c.id + '-titlebar').offsetHeight + 2 + elt.gw_minh;
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
  },
  
  getTargetId: function(elt)
  {
    for(;;)
    {
      if (elt.id)
        return elt.id;
      elt = elt.parentNode;
      if (!elt)
        return;
    }
  },

  menu:
  {
    hide: function(elt)
    {
      elt.style.display = 'none';
      setTimeout(function() { elt.style.display = ''; }, 150);
    },
    
    click: function(name, event)
    {
      var id = gw.getTargetId(event.target);
      gw.update(name, '#click', id);
      event.stopPropagation();
    }
  },
  
  table:
  {
    onScroll: function(id, more)
    {
      var elt = $(id);
      var sw = elt.firstChild;
      
      if (more)
      {
        //if ((sw.scrollHeight - sw.scrollTop) === (sw.clientHeight))
        if (sw.scrollTop >= (sw.scrollHeight - sw.clientHeight - 16))
        {
          /*var wait = document.createElement('div');
          wait.className = 'gw-waiting';
          elt.appendChild(wait);*/
          if (elt.gw_scroll)
          {
            clearTimeout(elt.gw_scroll); 
            elt.gw_scroll = undefined;
          }
          
          gw.update(id, '#more', [sw.scrollLeft, sw.scrollTop]);
          return;
        }
      }

      if (elt.gw_noscroll)
      {
        elt.gw_noscroll = undefined;
        return;
      }
      
      if (elt.gw_scroll)
        return;
      
      elt.gw_scroll = setTimeout(function()
        { 
          gw.update(elt.id, '#scroll', [sw.scrollLeft, sw.scrollTop]); 
          clearTimeout(elt.gw_scroll); 
          elt.gw_scroll = undefined;
        }, 250);
    },
    
    scroll: function(id, x, y)
    {
      var sw = $(id).firstChild
      if (x != sw.scrollLeft)
      {
        $(id).gw_noscroll = true;
        sw.scrollLeft = x;
      }
      if (y != sw.scrollTop)
      {
        $(id).gw_noscroll = true;
        sw.scrollTop = y;
      }
      if (x != sw.scrollLeft || y != sw.scrollTop)
        gw.update(id, '#scroll', [sw.scrollLeft, sw.scrollTop]); 
    }
  }
}
