function $(a)
{return document.getElementById(a);}
gw={timers:{},windows:[],form:'',debug:false,log:function(d)
{if(gw.debug)
console.log(d);},send:function(f)
{var g=new XMLHttpRequest();gw.log(f);g.open('GET',$root+'/'+encodeURIComponent(gw.form)
+'/x?c='+encodeURIComponent(JSON.stringify(f)),true);g.onreadystatechange=function()
{if(g.readyState==4&&g.status==200&&g.responseText)
{gw.active=document.activeElement.id;if(gw.active)
gw.selection=gw.getSelection($(gw.active));else
gw.selection=undefined;if(gw.debug)
console.log(g.responseText);eval(g.responseText);if(gw.active)
{if($(gw.active))
{$(gw.active).focus();gw.setSelection($(gw.active),gw.selection);}
else
gw.active=document.activeElement.id;}}};g.send(null);},raise:function(j,k,l)
{gw.send(['raise',j,k,l]);},update:function(j,m,n)
{gw.send(['update',j,m,n]);},getSelection:function(o)
{var p,q;try
{if(o.createTextRange)
{var r=document.selection.createRange().duplicate();r.moveEnd('character',o.value.length)
if(r.text=='')
p=o.value.length;else
p=o.value.lastIndexOf(r.text);r.moveStart('character',-o.value.length);q=r.text.length;
return[p,q];}
if(o.selectionStart&&o.selectionEnd)
return[o.selectionStart,o.selectionEnd];}
catch(e){};return undefined;},setSelection:function(o,s)
{if(s)
{if(o.setSelectionRange)
try{o.setSelectionRange(s[0],s[1])}
catch(e){};}},setFocus:function(j)
{$(j).focus();gw.active=document.activeElement.id;gw.selection=undefined;},resizeComboBox:
function(j)
{$(j+'-select').onmouseover=function(){$(j+'-select').style.width=$(j).offsetWidth
+'px';}},addTimer:function(j,u)
{gw.removeTimer(j);gw.timers[j]=setInterval(function(){gw.raise(j,'timer');},
u);},removeTimer:function(j)
{var t=gw.timers[j];if(t)
{clearInterval(gw.timers[j]);gw.timers[j]=undefined;}},window:{zIndex:0,open:function(j,v,z,A,B)
{gw.window.close(j);if(gw.windows.length==0)
{document.addEventListener('mousemove',gw.window.onMove);document.addEventListener('mouseup',
gw.window.onUp);gw.log('document.addEventListener');}
gw.windows.push(j);$(j).addEventListener('mousedown',gw.window.onMouseDown);$(j).gw_resizable=
v;$(j).gw_modal=z;if(A!=undefined)
{$(j).gw_minw=A;$(j).gw_minh=B;}
else
{$(j).gw_minw=$(j).offsetWidth;$(j).gw_minh=$(j).offsetHeight;}
gw.window.updateTitleBars();gw.window.refresh();},close:function(j)
{var i;$(j).removeEventListener('mousedown',gw.window.onMouseDown);i=gw.windows.indexOf(j);
if(i>=0)
{gw.windows.splice(i,1);gw.window.refresh();}},refresh:function()
{var i=0;while(i<gw.windows.length)
{if($(gw.windows[i]))
{$(gw.windows[i]).style.zIndex=11+i*2;i++;}
else
gw.windows.splice(i,1);}
gw.window.updateModal();if(gw.windows.length==0)
{gw.log('document.removeEventListener');document.removeEventListener('mousemove',
gw.window.onMove);document.removeEventListener('mouseup',gw.window.onUp);}},updateTitleBars:function()
{var i;for(i=0;i<gw.windows.length-1;i++)
$(gw.windows[i]+'-titlebar').style.backgroundColor='#C0C0C0';$(gw.windows[gw.windows.length
-1]+'-titlebar').style.backgroundColor='';},raise:function(j,C)
{var i=gw.windows.indexOf(j);if(i<0)
return;gw.windows.splice(i,1);gw.windows.push(j);for(i=0;i<gw.windows.length;
i++)
$(gw.windows[i]).style.zIndex=11+i*2;gw.window.updateTitleBars();if(C)
gw.update('','#windows',gw.windows);},updateModal:function()
{var i,D=$('gw-modal');for(i=gw.windows.length-1;i>=0;i--)
{if($(gw.windows[i]).gw_modal)
{gw.window.zIndex=10+i*2;D.style.zIndex=10+i*2;D.style.display='block';return;}}
gw.window.zIndex=0;D.style.display='none';},center:function(j)
{$(j).style.left=((window.innerWidth-$(j).offsetWidth)/2+0)+'px';$(j).style.top=
((window.innerHeight-$(j).offsetHeight)/2+0)+'px';gw.window.updateGeometry(j);},onMouseDown:function(e)
{gw.window.onDown(e);},onDown:function(e)
{var c;gw.window.context=undefined;if(e.target.className=='gw-window-button')
return;gw.window.onMove(e);c=gw.window.context;if(c&&(c.isMoving||c.isResizing))
{gw.window.raise(c.id);gw.window.downEvent=e;e.preventDefault();}},onMove(e)
{var i,j,D,b,x,y,E,F,G,H;var I,J,K,L,M;var N=6;if(gw.window.downEvent)
{gw.window.context.cx=e.clientX;gw.window.context.cy=e.clientY;gw.window.animate();
return;}
gw.window.context=undefined;for(i=0;i<gw.windows.length;i++)
{j=gw.windows[gw.windows.length-i-1];D=$(j);if(D.style.zIndex<gw.window.zIndex)
continue;b=D.getBoundingClientRect();E=b.left;F=b.top;G=b.width;H=b.height;x=
e.clientX-E;y=e.clientY-F;if(x>=0&&x<G&&y>=0&&y<H)
{if(D.gw_resizable)
{I=y<N;J=x<N;K=x>=(G-N);L=y>=(H-N);M=I||J||K||L;}
else
I=J=K=L=M=false;isMoving=!M&&y<($(j+'-titlebar').offsetHeight+N);gw.window.context=
{id:j,x:b.left+window.scrollX,y:b.top+window.scrollY,cx:e.clientX,cy:e.clientY,
w:b.width,h:b.height,isResizing:M,isMoving:isMoving,onTopEdge:I,onLeftEdge:J,
onRightEdge:K,onBottomEdge:L};gw.window.animate();break;}}},updateGeometry:function(j)
{var b=$(j).getBoundingClientRect();gw.update(j,'#geometry',[b.left+'px',b.top
+'px',b.width+'px',b.height+'px']);},onUp:function(e)
{var c=gw.window.context;gw.window.downEvent=undefined;if(c&&(c.isMoving||c.isResizing))
{var j=gw.window.context.id;gw.window.context=undefined;gw.window.raise(j,true);
gw.window.updateGeometry(j);}},animate:function()
{var j,D,c,e,x,y,w,h;var O;var P;c=gw.window.context;if(!c)return;D=$(c.id);O=
D.gw_minw;P=D.gw_minh;e=gw.window.downEvent;if(c&&c.isResizing&&e)
{if(c.onRightEdge)
D.style.width=Math.max(c.w+c.cx-e.clientX,O)+'px';if(c.onBottomEdge)
D.style.height=Math.max(c.h+c.cy-e.clientY,P)+'px';if(c.onLeftEdge)
{x=c.x+c.cx-e.clientX;w=c.x+c.w-x;if(w>=O)
{D.style.width=w+'px';D.style.left=x+'px';}}
if(c.onTopEdge)
{y=c.y+c.cy-e.clientY;h=c.y+c.h-y;if(h>=P)
{D.style.height=h+'px';D.style.top=y+'px';}}
return;}
if(c&&c.isMoving&&e)
{D.style.left=(Math.max(0,c.x+c.cx-e.clientX))+'px';D.style.top=(Math.max(0,c.y
+c.cy-e.clientY))+'px';return;}
if(c.onRightEdge&&c.onBottomEdge||c.onLeftEdge&&c.onTopEdge)
D.style.cursor='nwse-resize';else if(c.onRightEdge&&c.onTopEdge||c.onBottomEdge
&&c.onLeftEdge)
D.style.cursor='nesw-resize';else if(c.onRightEdge||c.onLeftEdge)
D.style.cursor='ew-resize';else if(c.onBottomEdge||c.onTopEdge)
D.style.cursor='ns-resize';else
D.style.cursor='';}},getTargetId:function(D)
{for(;;)
{if(D.id)
return D.id;D=D.parentNode;if(!D)
return;}},menu:{hide:function(D)
{D.style.display='none';setTimeout(function(){D.style.display='';},150);},click:
function(Q,k)
{var j=gw.getTargetId(k.target);gw.update(Q,'#click',j);k.stopPropagation();}}}
