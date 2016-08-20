function $(a)
{
  return document.getElementById(a);
}

if (!String.prototype.endsWith) 
{
  String.prototype.endsWith = function(searchString, position) 
  {
    var subjectString = this.toString();
    if (typeof position !== 'number' || !isFinite(position) || Math.floor(position) !== position || position > subjectString.length) {
      position = subjectString.length;
    }
    position -= searchString.length;
    var lastIndex = subjectString.indexOf(searchString, position);
    return lastIndex !== -1 && lastIndex === position;
  };
}

Element.prototype.hasClass = function(klass)
{
  if (this.classList)
    return this.classList.contains(klass);
  else
    return !!this.className.match(new RegExp('(\\s|^)' + klass + '(\\s|$)'));
};

Element.prototype.addClass = function(klass)
{
  if (this.classList)
    this.classList.add(klass);
  else if (!this.hasClass(klass))
    this.className += " " + klass;
};

Element.prototype.removeClass = function(klass)
{
  if (this.classList)
    this.classList.remove(klass);
  else if (this.hasClass(klass)) 
  {
    var reg = new RegExp('(\\s|^)' + klass + '(\\s|$)');
    this.className = this.className.replace(reg, ' ');
  }
};

function replaceInnerHTML(oldDiv, html) {
    var newDiv = oldDiv.cloneNode(false);
    newDiv.innerHTML = html;
    oldDiv.parentNode.replaceChild(newDiv, oldDiv);
};

gw = {

  version: '0',
  timers: {},
  windows: [],
  form: '',
  debug: false,
  loaded: {},
  uploads: {},
  autocompletion: [],
  
  log: function(msg)
  {
    if (gw.debug)
    {
      if (gw.startTime == undefined)
        gw.startTime = Date.now();
      console.log(((Date.now() - gw.startTime) / 1000) + ': ' + msg);
    }
  },
  
  load: function(lib)
  {
    var elt, src;
    
    if (gw.loaded[lib])
      return;
    
    if (lib.endsWith('.js'))
    {
      elt = document.createElement('script');
      elt.setAttribute("type", "text/javascript");
      src = $root + '/lib:' + lib.slice(0, -3) + ':' + gw.version + '.js';
      elt.setAttribute("src", src);
    }
    else if (lib.endsWith('.css'))
    {
      elt = document.createElement('link');
      elt.setAttribute("rel", "stylesheet");
      elt.setAttribute("type", "text/css");
      src = $root + '/style:' + lib.slice(0, -4) + ':' + gw.version + '.css';
      elt.setAttribute("href", src);
    }
    else
      return;
      
    document.getElementsByTagName("head")[0].appendChild(elt);
    gw.loaded[lib] = src;
    console.log('load: ' + src);
  },
  
  setInnerHtml : function(id, html)
  {
    var oldDiv = $(id);
    var newDiv = oldDiv.cloneNode(false);
    newDiv.innerHTML = html;
    oldDiv.parentNode.replaceChild(newDiv, oldDiv);
  },
  
  setOuterHtml : function(id, html)
  {
    if ($(id))
      $(id).outerHTML = html;
    else
      console.log('setOuterHtml: ' + id + '? ' + html);
  },
  
  removeElement : function(id)
  {
    var elt = $(id);
    //for (i = 0; i < id_list.length; i++)
    //{
      //elt = $(id_list[i]);
      if (!elt)
        return;
    
      console.log(id + " removed");
    
      elt.parentNode.removeChild(elt);
    //}
  },
  
  answer: function(xhr)
  {
    if (xhr.readyState == 4 && xhr.status == 200 && xhr.responseText)
    {
      gw.log('gw.answer (' + xhr.gw_command + ')');
      
      gw.active = document.activeElement.id;
      if (gw.active)
        gw.selection = gw.getSelection($(gw.active));
      else
        gw.selection = undefined;
      
      if (gw.debug)
        console.log('--> ' + xhr.responseText);
        
      eval(xhr.responseText);
      
      if (gw.active)
      {
        gw.log('active: ' + gw.active);
        if ($(gw.active))
        {
          $(gw.active).focus();
          gw.setSelection($(gw.active), gw.selection);
        }
        else
          gw.active = document.activeElement.id;
      }
      
      gw.log('answer done');
    }
  },

  send: function(command, after)
  {
    gw.log('gw.send: ' + command);
    var xhr = new XMLHttpRequest();
    xhr.gw_command = command;
    xhr.open('GET', $root + '/' + encodeURIComponent(gw.form) + '/x?c=' + encodeURIComponent(JSON.stringify(command)), true);
    if (after)
      xhr.onreadystatechange = function() { gw.answer(xhr); after(); };
    else
      xhr.onreadystatechange = function() { gw.answer(xhr); };
    xhr.send(null);
  },

  raise: function(id, event, args)
  {
    gw.send(['raise', id, event, args]);
  },
  
  update: function(id, prop, value, after)
  {
    gw.send(['update', id, prop, value], after);
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
    /*if (!$(id))
      return;*/
      
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

  getPos: function(elt)
  {
    var found, left = 0, top = 0, width = 0, height = 0;
    var offsetBase = gw.offsetBase;
   
    if (!offsetBase && document.body) 
    {
      offsetBase = gw.offsetBase = document.createElement('div');
      offsetBase.style.cssText = 'position:absolute;left:0;top:0';
      document.body.appendChild(offsetBase);
    }
    
    if (elt && elt.ownerDocument === document && 'getBoundingClientRect' in elt && offsetBase) 
    {
      var boundingRect = elt.getBoundingClientRect();
      var baseRect = offsetBase.getBoundingClientRect();
      found = true;
      left = boundingRect.left - baseRect.left;
      top = boundingRect.top - baseRect.top;
      width = boundingRect.right - boundingRect.left;
      height = boundingRect.bottom - boundingRect.top;
    }
    
    return { found: found, left: left, top: top, width: width, height: height, right: left + width, bottom: top + height };
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
      
      gw.window.refresh();
    },
    
    popup: function(id, resizable, control, alignment, minw, minh)
    {
      var pos;
      
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
      $(id).gw_modal = true;
      $(id).gw_popup = true;
      
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
      
      pos = gw.getPos($(control));
      //console.log(pos);
      
      /*$(id).style.left = pos.left + 'px';
      $(id).style.top = pos.bottom + 'px';*/
      $(id).style.transform = 'translate(' + pos.left + 'px,' + pos.bottom + 'px)';
      
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
      else
        gw.window.updateTitleBars();
    },
    
    updateTitleBars: function()
    {
      var i, win, last;
      
      for (i = 0; i < gw.windows.length - 1; i++)
      {
        win = gw.windows[i];
        if ($(win).gw_popup)
          continue;
        $(win).addClass('gw-deactivated');
        $(win + '-titlebar').addClass('gw-deactivated');
        last = win;
      }
      
      if (last && !$(last).gw_popup)
      {
        $(last).removeClass('gw-deactivated');
        $(last + '-titlebar').removeClass('gw-deactivated');
      }
        
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
          if ($(gw.windows[i]).gw_popup)
            elt.style.opacity = '0';
          else
            elt.style.opacity = '';
          return;
        }
      }
      
      gw.window.zIndex = 0;
      elt.style.display = 'none';
    },
    
    center: function(id)
    {
      $(id).style.transform = 'translate(' + ((window.innerWidth - $(id).offsetWidth) / 2 | 0) + 'px,' + ((window.innerHeight - $(id).offsetHeight) / 2 | 0) + 'px)';
      gw.window.updateGeometry(id);
    },
    
    maximize: function(id)
    {
      var geom = $(id).gw_save_geometry;
      if (geom != undefined)
      {
        //$(id).style.left = geom[0];
        //$(id).style.top = geom[1];
        $(id).style.transform = geom[0]
        $(id).style.width = geom[1];
        $(id).style.height = geom[2];
        $(id).gw_save_geometry = undefined;
      }
      else
      {
        $(id).gw_save_geometry = [$(id).style.transform, $(id).style.width, $(id).style.height];
        $(id).style.transform = '';
        $(id).style.width = '100%';
        $(id).style.height = '100%';
      }
      //gw.window.updateGeometry(id);
    },
    
    onMouseDown: function(e)
    {
      gw.window.onDown(e);
    },
    
    onDown: function(e)
    {
      var c, win;
      
      gw.window.context = undefined;
      
      if (e.target.className == 'gw-window-button')
        return;
        
      gw.window.onMove(e);
      
      c = gw.window.context;
      if (c == undefined)
        return; 
        
      if ($(c.id).gw_save_geometry)
        return;
        
      if (c.isMoving || c.isResizing)
      {
        gw.window.raise(c.id);
        gw.window.downEvent = e;
        e.preventDefault();
      }
    },
    
    onDownModal: function()
    {
      var win = gw.windows[gw.windows.length - 1];
      
      if ($(win).gw_popup)
        gw.update(win, '#close');
    },
    
    onMove(e) 
    {
      var i, id, elt, b, x, y, bx, by, bw, bh, th;
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
          
          if ($(id).gw_popup)
            th = 0;
          else
            th = $(id + '-titlebar').offsetHeight;
          isMoving = !isResizing && y < (th + MARGINS);
          
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
    
        x = c.x;
        y = c.y;
    
        if (c.onLeftEdge) 
        {
          x = c.x + c.cx - e.clientX;
          w = c.x + c.w - x;
          if (w >= minWidth) 
          {
            elt.style.width = w + 'px';
            //elt.style.left = x + 'px'; 
            elt.style.transform = 'translate(' + x + 'px,' + y + 'px)';
            //c.x = x;
          }
        }
    
        if (c.onTopEdge) 
        {
          y = c.y + c.cy - e.clientY;
          h = c.y + c.h - y;
          if (h >= minHeight) 
          {
            elt.style.height = h + 'px';
            //elt.style.top = y + 'px'; 
            elt.style.transform = 'translate(' + x + 'px,' + y + 'px)';
          }
        }
    
        return;
      }
    
      if (c && c.isMoving && e)
      {
        /*elt.style.left = (Math.max(0, c.x + c.cx - e.clientX)) + 'px';
        elt.style.top = (Math.max(0, c.y + c.cy - e.clientY)) + 'px';*/
        elt.style.transform = 'translate(' + (Math.max(0, c.x + c.cx - e.clientX)) + 'px,' + (Math.max(0, c.y + c.cy - e.clientY)) + 'px)';
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
    onScroll: function(id, more, timeout)
    {
      var elt = $(id);
      var sw = elt.firstChild;
      var last = elt.gw_last_scroll;
      
      if (last && last[0] == sw.scrollLeft && last[1] == sw.scrollTop)
        return;
      
      elt.gw_last_scroll = [sw.scrollLeft, sw.scrollTop];
      
      //console.log('onScroll: ' + id + ' ' + sw.scrollLeft + ',' + sw.scrollTop);
      
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
      
      if (elt.gw_headerh)
        $(elt.gw_headerh).firstChild.scrollLeft = sw.scrollLeft;

      if (elt.gw_headerv)
        $(elt.gw_headerv).firstChild.scrollTop = sw.scrollTop;

      if (elt.gw_noscroll)
      {
        elt.gw_noscroll = undefined;
        return;
      }
      
      if (elt.gw_scroll)
        clearTimeout(elt.gw_scroll); 
      
      elt.gw_scroll = setTimeout(function()
        { 
          //console.log("gw.table.onScroll: " + id + ": " + sw.scrollLeft + " " + sw.scrollTop);
          var pos = [sw.scrollLeft, sw.scrollTop];
          
          clearTimeout(elt.gw_scroll); 
          
          gw.update(elt.id, '#scroll', pos, function() 
            { 
              elt.gw_scroll = undefined; 
              if (pos[0] != sw.scrollLeft || pos[1] != sw.scrollTop)
                gw.table.onScroll(id, more, timeout);
            });
            
          //elt.gw_scroll = undefined;
        }, timeout || 250);
    },
    
    scroll: function(id, x, y)
    {
      var sw = $(id).firstChild
      
      //console.log("gw.table.scroll: " + id + ": " + x + " " + y);
      
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
  },
  
  scrollview:
  {
    setHeaders: function(id, hid, vid)
    {
      $(id).gw_headerh = hid;
      $(id).gw_headerv = vid;
    }
  },
  
  file: 
  {
    select: function(id) 
    {
      var elt = $(id + ':file');
      
      if ($(id).gw_uploading)
        return;
      
      elt.focus();
      elt.click();
    },
    
    upload: function(id)
    {
      var elt = $(id + ':file');
      var file = elt.files[0];
      var xhr = new XMLHttpRequest();
      var form = new FormData();
      
      if (gw.uploads[id])
        return;
      
      gw.uploads[id] = xhr;
      
      gw.log('gw.file.upload: ' + id + ': ' + file.name);
      
      form.append('file', file);
      form.append('name', file.name);
      form.append('id', id);
      
      //xhr.upload.addEventListener("loadstart", loadStartFunction, false);  
      //xhr.upload.addEventListener("load", transferCompleteFunction, false);
      
      xhr.upload.addEventListener("progress", 
        function(e) 
        {
          if (e.lengthComputable)
          {
            var t = (new Date()).getTime();
            
            if (xhr.gw_time == undefined || (t - xhr.gw_time) > 250)
            {
              gw.update(id, '#progress', e.loaded / e.total); 
              xhr.gw_time = t;
            }
          }
        },
        false);
      
      xhr.gw_command = 'upload ' + id;
      xhr.gw_id = id;
        
      xhr.open("POST", $root + '/' + encodeURIComponent(gw.form) + '/u', true);  
      
      xhr.onreadystatechange = function() 
        {
          if (xhr.readyState == 4)
          {
            gw.uploads[xhr.gw_id] = undefined;
            gw.answer(xhr); 
          }
        };
        
      xhr.send(form);  
    },
    
    abort: function(id)
    {
      if (gw.uploads[id])
        gw.uploads[id].abort();
    }
  },

  autocomplete: function(id)
  {
    new AutoComplete({
      selector: $(id),
      cache: false,
      /*source: function(term, response) 
        {
          gw.raise(id, 'completion', [term]);
          response($(id).gw_completion);
        }*/
      source: function(term, response) {
        var xhr = $(id).gw_xhr;
        if (xhr)
        {
          try { xhr.abort(); } catch(e) {}
        }
        
        $(id).gw_xhr = xhr = new XMLHttpRequest();
        
        xhr.open('GET', $root + '/' + encodeURIComponent(gw.form) + '/x?c=' + encodeURIComponent(JSON.stringify(['raise', id, 'completion', [term]])), true);
        xhr.onreadystatechange = function() {
          if (xhr.readyState == 4)
          {
            gw.autocompletion = [];
            gw.answer(xhr);
            response(gw.autocompletion);
          }
        };
        xhr.send();
      }
    });
  } 
}

