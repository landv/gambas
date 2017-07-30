
function getAbsoluteParent(obj)
{
  for(;;)
  {
    obj = obj.offsetParent;
    if (obj == null)
      break;
    if (obj.style.position == 'absolute')
      break;
  }
  
  return obj;
}

function __getIEVersion() {
    var rv = -1; // Return value assumes failure.
    if (navigator.appName == 'Microsoft Internet Explorer') {
        var ua = navigator.userAgent;
        var re = new RegExp("MSIE ([0-9]{1,}[\.0-9]{0,})");
        if (re.exec(ua) != null)
            rv = parseFloat(RegExp.$1);
    }
    return rv;
}

function __getOperaVersion() {
    var rv = 0; // Default value
    if (window.opera) {
        var sver = window.opera.version();
        rv = parseFloat(sver);
    }
    return rv;
}

var __userAgent = navigator.userAgent;
var __isIE =  navigator.appVersion.match(/MSIE/) != null;
var __IEVersion = __getIEVersion();
var __isIENew = __isIE && __IEVersion >= 8;
var __isIEOld = __isIE && !__isIENew;

var __isFireFox = __userAgent.match(/firefox/i) != null;
var __isFireFoxOld = __isFireFox && ((__userAgent.match(/firefox\/2./i) != null) || 
  (__userAgent.match(/firefox\/1./i) != null));
var __isFireFoxNew = __isFireFox && !__isFireFoxOld;

var __isWebKit =  navigator.appVersion.match(/WebKit/) != null;
var __isChrome =  window.chrome != null;
var __isOpera =  window.opera != null;
var __operaVersion = __getOperaVersion();
var __isOperaOld = __isOpera && (__operaVersion < 10);

function __parseBorderWidth(width) {
    var res = 0;
    if (typeof(width) == "string" && width != null && width != "" ) {
        var p = width.indexOf("px");
        if (p >= 0) {
            res = parseInt(width.substring(0, p));
        }
        else {
         //do not know how to calculate other values 
    //(such as 0.5em or 0.1cm) correctly now
        //so just set the width to 1 pixel
            res = 1; 
        }
    }
    return res;
}

//returns border width for some element
function __getBorderWidth(element) {
  var res = new Object();
  res.left = 0; res.top = 0; res.right = 0; res.bottom = 0;
  if (window.getComputedStyle) {
    //for Firefox
    var elStyle = window.getComputedStyle(element, null);
    res.left = parseInt(elStyle.borderLeftWidth.slice(0, -2));  
    res.top = parseInt(elStyle.borderTopWidth.slice(0, -2));  
    res.right = parseInt(elStyle.borderRightWidth.slice(0, -2));  
    res.bottom = parseInt(elStyle.borderBottomWidth.slice(0, -2));  
  }
  else {
    //for other browsers
    res.left = __parseBorderWidth(element.style.borderLeftWidth);
    res.top = __parseBorderWidth(element.style.borderTopWidth);
    res.right = __parseBorderWidth(element.style.borderRightWidth);
    res.bottom = __parseBorderWidth(element.style.borderBottomWidth);
  }
   
  return res;
}

//returns the absolute position of some element within document
function getElementAbsolutePos(element) {
  var res = new Object();
  res.x = 0; res.y = 0;
  
  if (element !== null) { 
    if (element.getBoundingClientRect) {
      var viewportElement = document.documentElement;  
           var box = element.getBoundingClientRect();
        var scrollLeft = viewportElement.scrollLeft;
         var scrollTop = viewportElement.scrollTop;

        res.x = box.left + scrollLeft;
        res.y = box.top + scrollTop;

    }
    else { //for old browsers
      res.x = element.offsetLeft;
      res.y = element.offsetTop;

      var parentNode = element.parentNode;
      var borderWidth = null;
      var offsetParent = element.offsetParent;

      while (offsetParent != null) {
        res.x += offsetParent.offsetLeft;
        res.y += offsetParent.offsetTop;
        
        var parentTagName = 
          offsetParent.tagName.toLowerCase();  

        if ((__isIEOld && parentTagName != "table") || 
          ((__isFireFoxNew || __isChrome) && 
            parentTagName == "td")) {        
          borderWidth = kGetBorderWidth
              (offsetParent);
          res.x += borderWidth.left;
          res.y += borderWidth.top;
        }
        
        if (offsetParent != document.body && 
        offsetParent != document.documentElement) {
          res.x -= offsetParent.scrollLeft;
          res.y -= offsetParent.scrollTop;
        }


        //next lines are necessary to fix the problem 
        //with offsetParent
        if (!__isIE && !__isOperaOld || __isIENew) {
          while (offsetParent != parentNode && 
            parentNode !== null) {
            res.x -= parentNode.scrollLeft;
            res.y -= parentNode.scrollTop;
            if (__isFireFoxOld || __isWebKit) 
            {
                borderWidth = 
                 kGetBorderWidth(parentNode);
                res.x += borderWidth.left;
                res.y += borderWidth.top;
            }
            parentNode = parentNode.parentNode;
          }    
        }

        if (offsetParent.style.position == 'absolute')
          break;
        parentNode = offsetParent.parentNode;
        offsetParent = offsetParent.offsetParent;
      }
    }
  }
    return res;
}


function getLeftPos(obj)
{
  return getElementAbsolutePos(obj).x
}

function getTopPos(obj)
{
  return getElementAbsolutePos(obj).y
}

function moveElementUnder(obj, under, dx, dy, right)
{
  var pos, opos, x, y;
  var absUnder, absObj;
  
  pos = getElementAbsolutePos(under);
  
  obj.style.position = 'absolute';
  obj.style.left = '0';
  obj.style.top = '0';  
  
  opos = getElementAbsolutePos(obj);

  x = pos.x - opos.x + dx; //- getLeftPos(under.offsetParent);
  y = pos.y - opos.y + under.offsetHeight + dy; //- getTopPos(under.offsetParent);
  
  if (right)
    x -= obj.offsetWidth - under.offsetWidth;

  obj.style.left = x + 'px';
  obj.style.top = y + 'px';  
}

/************************************************************************************************************ 
JS Calendar 
Copyright (C) September 2006  DTHMLGoodies.com, Alf Magne Kalleland 
 
This library is free software; you can redistribute it and/or 
modify it under the terms of the GNU Lesser General Public 
License as published by the Free Software Foundation; either 
version 2.1 of the License, or (at your option) any later version. 
 
This library is distributed in the hope that it will be useful, 
but WITHOUT ANY WARRANTY; without even the implied warranty of 
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
Lesser General Public License for more details. 
 
You should have received a copy of the GNU Lesser General Public 
License along with this library; if not, write to the Free Software 
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA 
 
Dhtmlgoodies.com., hereby disclaims all copyright interest in this script 
written by Alf Magne Kalleland. 
 
Alf Magne Kalleland, 2006 
Owner of DHTMLgoodies.com 
 
************************************************************************************************************/ 
 
var turnOffYearSpan = false;     // true = Only show This Year and Next, false = show +/- 5 years 
var weekStartsOnSunday = false;  // true = Start the week on Sunday, false = start the week on Monday 
var showWeekNumber = true;  // true = show week number,  false = do not show week number 
 
var calendar_display_time = true; 
 
// Format of current day at the bottom of the calendar 
// [todayString] = the value of todayString 
// [dayString] = day of week (example: mon, tue, wed...) 
// [UCFdayString] = day of week (example: Mon, Tue, Wed...) ( First letter in uppercase) 
// [day] = Day of month, 1..31 
// [monthString] = Name of current month 
// [year] = Current year 
var todayStringFormat = '[todayString] [UCFdayString]. [day] [monthString] [year]'; 
var pathToImages = $root;  // Relative to your HTML file 
 
var speedOfSelectBoxSliding = 50;  // Milliseconds between changing year and hour when holding mouse over "-" and "+" - lower value = faster 
var intervalSelectBox_minutes = 5;  // Minute select box - interval between each option (5 = default) 
 
var calendar_offsetTop = 3;    // Offset - calendar placement - You probably have to modify this value if you're not using a strict doctype 
var calendar_offsetLeft = 4;  // Offset - calendar placement - You probably have to modify this value if you're not using a strict doctype 
var calendarDiv; 
var must_submit = false;
 
var MSIE = false; 
var Opera = false; 
//if(navigator.userAgent.indexOf('MSIE')>=0 && navigator.userAgent.indexOf('Opera')<0)MSIE=true; 
if(navigator.userAgent.indexOf('Opera')>=0)Opera=true; 
 
var monthArray = ['Janvier','F&eacute;vrier','Mars','Avril','Mai','Juin','Juillet','Ao&ucirc;t','Septembre','Octobre','Novembre','D&eacute;cembre']; 
var monthArrayShort = ['Jan','F&eacute;v','Mar','Avr','Mai','Jun','Jul','Ao&ucirc;','Sep','Oct','Nov','D&eacute;c']; 
var dayArray = ['Lun','Mar','Mer','Jeu','Ven','Sam','Dim']; 
var weekString = 'Sem'; 
var todayString = ""; 
 

if (weekStartsOnSunday) { 
   var tempDayName = dayArray[6]; 
   for(var theIx = 6; theIx > 0; theIx--) { 
      dayArray[theIx] = dayArray[theIx-1]; 
   } 
   dayArray[0] = tempDayName; 
} 
 
var daysInMonthArray = [31,28,31,30,31,30,31,31,30,31,30,31]; 
var currentMonth; 
var currentYear; 
var currentHour; 
var currentMinute; 
var calendarContentDiv; 
var returnDateTo; 
var returnFormat; 
var activeSelectBoxMonth; 
var activeSelectBoxYear; 
var activeSelectBoxHour; 
var activeSelectBoxMinute; 
 
var iframeObj = false; 
//// fix for EI frame problem on time dropdowns 09/30/2006 
var iframeObj2 =false;

function EIS_FIX_EI1(where2fixit)
{ 
  if (!iframeObj2)
    return; 
  iframeObj2.style.display = 'block'; 
  iframeObj2.style.height = $(where2fixit).offsetHeight+1; 
  iframeObj2.style.width= $(where2fixit).offsetWidth; 
  iframeObj2.style.left=getLeftPos($(where2fixit))+1-calendar_offsetLeft; 
  iframeObj2.style.top=getTopPos($(where2fixit))-$(where2fixit).offsetHeight-calendar_offsetTop; 
} 
 
function EIS_Hide_Frame() 
{
  if(iframeObj2)
    iframeObj2.style.display = 'none';
} 

//// fix for EI frame problem on time dropdowns 09/30/2006 
var returnDateToYear; 
var returnDateToMonth; 
var returnDateToDay; 
var returnDateToHour; 
var returnDateToMinute; 
 
var inputYear; 
var inputMonth; 
var inputDay; 
var inputHour; 
var inputMinute; 
var calendarDisplayTime = false; 
 
var selectBoxHighlightColor = '#FF0000'; // Highlight color of select boxes 
var selectBoxRolloverBgColor = '#F0F0F0'; // Background color on drop down lists(rollover) 
 
var selectBoxMovementInProgress = false; 
var activeSelectBox = false; 
 
function cancelCalendarEvent() 
{ 
  return false; 
} 

function isLeapYear(inputYear) 
{ 
  return inputYear % 400 == 0 || (inputYear % 4 == 0 && inputYear % 100 != 0);
} 

var activeSelectBoxMonth = false; 
var activeSelectBoxDirection = false; 

function selectElt(elt, value)
{
  elt.style.fontWeight = value ? 'bold' : '';
}

function scrollMonthYear() 
{ 
  activeSelectBox = this; 
 
  if (this.id.indexOf('UpDiv') >= 0 || this.id.indexOf('DownDiv') >= 0)
  { 
    //if (this.className=='monthYearActive') 
      selectBoxMovementInProgress = true; 
    //else 
    //  selectBoxMovementInProgress = false;
      
    if (this.id.indexOf('UpDiv') >=0 )
      activeSelectBoxDirection = -1; 
    else 
      activeSelectBoxDirection = 1; 
    setTimeout(slideCalendarSelectBox, speedOfSelectBoxSliding);
  }
  else 
    selectBoxMovementInProgress = false; 
}
 
function showMonthDropDown() 
{ 
  if($('monthDropDown').style.display=='block'){ 
    $('monthDropDown').style.display='none'; 
    //// fix for EI frame problem on time dropdowns 09/30/2006 
        EIS_Hide_Frame(); 
  }else{ 
    $('monthDropDown').style.display='block'; 
    $('yearDropDown').style.display='none'; 
    $('hourDropDown').style.display='none'; 
    $('minuteDropDown').style.display='none'; 
      if (MSIE) 
    { EIS_FIX_EI1('monthDropDown')} 
    //// fix for EI frame problem on time dropdowns 09/30/2006 
  } 
} 
 
function showYearDropDown() 
{ 
  if($('yearDropDown').style.display=='block'){ 
    $('yearDropDown').style.display='none'; 
    //// fix for EI frame problem on time dropdowns 09/30/2006 
        EIS_Hide_Frame(); 
  }else{ 
    $('yearDropDown').style.display='block'; 
    $('monthDropDown').style.display='none'; 
    $('hourDropDown').style.display='none'; 
    $('minuteDropDown').style.display='none'; 
      if (MSIE) 
    { EIS_FIX_EI1('yearDropDown')} 
    //// fix for EI frame problem on time dropdowns 09/30/2006 
 
  } 
 
} 
function showHourDropDown() 
{ 
  if($('hourDropDown').style.display=='block'){ 
    $('hourDropDown').style.display='none'; 
    //// fix for EI frame problem on time dropdowns 09/30/2006 
        EIS_Hide_Frame(); 
  }else{ 
    $('hourDropDown').style.display='block'; 
    $('monthDropDown').style.display='none'; 
    $('yearDropDown').style.display='none'; 
    $('minuteDropDown').style.display='none'; 
        if (MSIE) 
    { EIS_FIX_EI1('hourDropDown')} 
    //// fix for EI frame problem on time dropdowns 09/30/2006 
  } 
 
} 
function showMinuteDropDown() 
{ 
  if($('minuteDropDown').style.display=='block'){ 
    $('minuteDropDown').style.display='none'; 
    //// fix for EI frame problem on time dropdowns 09/30/2006 
        EIS_Hide_Frame(); 
  }else{ 
    $('minuteDropDown').style.display='block'; 
    $('monthDropDown').style.display='none'; 
    $('yearDropDown').style.display='none'; 
    $('hourDropDown').style.display='none'; 
        if (MSIE) 
    { EIS_FIX_EI1('minuteDropDown')} 
    //// fix for EI frame problem on time dropdowns 09/30/2006 
  } 
 
} 
 
function selectMonth() 
{ 
  var elt;
  
  $('calendar_month_txt').innerHTML = this.innerHTML 
  currentMonth = this.id.replace(/[^\d]/g,''); 
 
  $('monthDropDown').style.display='none'; 
  //// fix for EI frame problem on time dropdowns 09/30/2006 
        EIS_Hide_Frame(); 
  for(var no=0;no<monthArray.length;no++){ 
    elt = $('monthDiv_'+no);
    selectElt(elt, false);
  }
  
  //if (activeSelectBoxMonth)
  //  selectElt(activeSelectBoxMonth, false);
  
  activeSelectBoxMonth = this; 
  selectElt(this, true);
  writeCalendarContent(); 
} 
 
function selectHour() 
{ 
  $('calendar_hour_txt').innerHTML = this.innerHTML 
  currentHour = this.innerHTML.replace(/[^\d]/g,''); 
  $('hourDropDown').style.display='none'; 
  //// fix for EI frame problem on time dropdowns 09/30/2006 
        EIS_Hide_Frame(); 
  if(activeSelectBoxHour)
    selectElt(activeSelectBoxHour, false);
  
  activeSelectBoxHour=this; 
  selectElt(this, true);
} 
 
function selectMinute() 
{ 
  $('calendar_minute_txt').innerHTML = this.innerHTML 
  currentMinute = this.innerHTML.replace(/[^\d]/g,''); 
  $('minuteDropDown').style.display='none'; 
  //// fix for EI frame problem on time dropdowns 09/30/2006 
        EIS_Hide_Frame(); 
  if(activeSelectBoxMinute)
    selectElt(activeSelectBoxMinute, false);
 
  activeSelectBoxMinute=this; 
  selectElt(this, true);
} 
 
 
function selectYear() 
{ 
  $('calendar_year_txt').innerHTML = this.innerHTML 
  currentYear = this.innerHTML.replace(/[^\d]/g,''); 
  $('yearDropDown').style.display='none'; 
  //// fix for EI frame problem on time dropdowns 09/30/2006 
        EIS_Hide_Frame(); 
  if(activeSelectBoxYear)
    selectElt(activeSelectBoxYear, false);
   
  activeSelectBoxYear=this; 
  selectElt(this, true);
  writeCalendarContent(); 
} 
 
function switchMonth() 
{ 
  if(this.src.indexOf('left')>=0){ 
    currentMonth=currentMonth-1;; 
    if(currentMonth<0){ 
      currentMonth=11; 
      currentYear=currentYear-1; 
    } 
  }else{ 
    currentMonth=currentMonth+1;; 
    if(currentMonth>11){ 
      currentMonth=0; 
      currentYear=currentYear/1+1; 
    } 
  } 
 
  updateMonthDiv();
  writeCalendarContent(); 
} 
 
function createMonthDiv(){ 
  var div = document.createElement('DIV'); 
  div.className='monthYearPicker'; 
  div.id = 'monthPicker'; 
 
  for(var no=0;no<monthArray.length;no++){ 
    var subDiv = document.createElement('DIV'); 
    subDiv.innerHTML = monthArray[no]; 
    //subDiv.onmouseover = scrollMonthYear; 
    //subDiv.onmouseout = scrollMonthYear; 
    subDiv.onclick = selectMonth; 
    subDiv.id = 'monthDiv_' + no; 
    subDiv.onselectstart = cancelCalendarEvent; 
    div.appendChild(subDiv); 
    if(currentMonth && currentMonth==no){ 
      selectElt(subDiv, true);
      activeSelectBoxMonth = subDiv; 
    } 
 
  } 
  return div; 
 
} 
 
function changeSelectBoxYear(e,inputObj) 
{ 
  if(!inputObj)inputObj =this; 
  var yearItems = inputObj.parentNode.getElementsByTagName('DIV'); 
  if(inputObj.id.indexOf('UpDiv')>=0){ 
    var startYear = yearItems[1].innerHTML/1 -1; 
    if(activeSelectBoxYear)
      selectElt(activeSelectBoxYear, false);
  }else{ 
    var startYear = yearItems[1].innerHTML/1 +1; 
    if(activeSelectBoxYear)
      selectElt(activeSelectBoxYear, false);
  } 
 
  for(var no=1;no<yearItems.length-1;no++){ 
    yearItems[no].innerHTML = startYear+no-1; 
    yearItems[no].id = 'yearDiv' + (startYear/1+no/1-1); 
 
  } 
  if(activeSelectBoxYear){ 
    selectElt(activeSelectBoxYear, false);
    if($('yearDiv'+currentYear)){ 
      activeSelectBoxYear = $('yearDiv'+currentYear); 
      selectElt(activeSelectBoxYear, true);
    } 
  } 
} 
function changeSelectBoxHour(e,inputObj) 
{ 
  if(!inputObj)inputObj = this; 
 
  var hourItems = inputObj.parentNode.getElementsByTagName('DIV'); 
  
  if (inputObj.id.indexOf('UpDiv') >= 0)
  { 
    var startHour = hourItems[1].innerHTML/1 - 1; 
    if(startHour < 0) startHour = 0; 
  }
  else
  { 
    var startHour = hourItems[1].innerHTML/1 + 1; 
    if(startHour >14 ) startHour = 14; 
  } 

  if(activeSelectBoxHour) 
    selectElt(activeSelectBoxHour, false);
    
  for (var no = 1; no < (hourItems.length - 1); no++)
  { 
    hourItems[no].innerHTML = padleft(startHour+no-1, '00'); 
    hourItems[no].id = 'hourDiv' + padleft(startHour+no-1, '00');
  } 
  
  if (activeSelectBoxHour)
  { 
    selectElt(activeSelectBoxHour, false);
    if ($('hourDiv'+currentHour))
    { 
      activeSelectBoxHour = $('hourDiv' + currentHour); 
      selectElt(activeSelectBoxHour, true);
    } 
  } 
} 
 
function updateYearDiv() 
{ 
    var yearSpan = 5; 
    if (turnOffYearSpan) { 
       yearSpan = 0; 
    } 
  var div = $('yearDropDown'); 
  var yearItems = div.getElementsByTagName('DIV'); 
  for(var no=1;no<yearItems.length-1;no++){ 
    yearItems[no].innerHTML = currentYear/1 -yearSpan + no; 
    if(currentYear==(currentYear/1 -yearSpan + no)){ 
      selectElt(yearItems[no], true);
      activeSelectBoxYear = yearItems[no]; 
    }else{ 
      selectElt(yearItems[no], false);
    } 
  } 
} 
 
function updateMonthDiv() 
{ 
  for(no=0;no<12;no++){ 
    selectElt($('monthDiv_' + no), false);
  } 
  activeSelectBoxMonth =   $('monthDiv_' + currentMonth); 
  selectElt(activeSelectBoxMonth, true); 
} 
 
 
function updateHourDiv() 
{ 
  var div = $('hourDropDown'); 
  var hourItems = div.getElementsByTagName('DIV'); 
 
  var addHours = 0; 

  if ((currentHour/1 - 6 + 1) < 0)
    addHours = -(currentHour/1 - 6 + 1); 
 
  for(var no=1;no<hourItems.length-1;no++)
  { 
    hourItems[no].innerHTML = padleft(currentHour/1 -6 + no + addHours, '00'); 
    if (currentHour == hourItems[no].innerHTML)
    { 
      selectElt(hourItems[no], true);
      activeSelectBoxHour = hourItems[no]; 
    }
    else
    { 
      selectElt(hourItems[no], false);
    } 
  } 
} 
 
function updateMinuteDiv() 
{ 
  for(no=0;no<60;no+=intervalSelectBox_minutes){ 
    var prefix = ''; 
    if(no<10)prefix = '0'; 
 
    selectElt($('minuteDiv_' + prefix + no), false);
  } 
  if($('minuteDiv_' + currentMinute)){ 
    selectElt($('minuteDiv_' + currentMinute), true);
    activeSelectBoxMinute = $('minuteDiv_' + currentMinute); 
  } 
} 
 
function createYearDiv() 
{ 
 
  if(!$('yearDropDown')){ 
    var div = document.createElement('DIV'); 
    div.className='monthYearPicker'; 
    div.align='center';
  }else{ 
    var div = $('yearDropDown'); 
    var subDivs = div.getElementsByTagName('DIV'); 
    for(var no=0;no<subDivs.length;no++){ 
      subDivs[no].parentNode.removeChild(subDivs[no]); 
    } 
  } 
 
  var d = new Date(); 
  if(currentYear){ 
    d.setFullYear(currentYear); 
  } 
 
  var startYear = d.getFullYear()/1 - 5; 
 
    var yearSpan = 10; 
  if (! turnOffYearSpan) { 
      var subDiv = document.createElement('DIV'); 
      subDiv.id = 'yearUpDiv';
      subDiv.style.height = '13px';
      subDiv.innerHTML = '<img src="' + pathToImages + '/gw-arrow-up.png" style="float:none;">';
      subDiv.onclick = changeSelectBoxYear; 
      subDiv.onmouseover = scrollMonthYear; 
      subDiv.onmouseout = function(){ selectBoxMovementInProgress = false;}; 
      subDiv.onselectstart = cancelCalendarEvent; 
      div.appendChild(subDiv); 
    } else { 
       startYear = d.getFullYear()/1 - 0; 
       yearSpan = 2; 
    } 
 
  for(var no=startYear;no<(startYear+yearSpan);no++){ 
    var subDiv = document.createElement('DIV'); 
    subDiv.innerHTML = no; 
    //subDiv.onmouseover = scrollMonthYear; 
    //subDiv.onmouseout = scrollMonthYear; 
    subDiv.onclick = selectYear; 
    subDiv.id = 'yearDiv' + no; 
    subDiv.onselectstart = cancelCalendarEvent; 
    div.appendChild(subDiv); 
    if(currentYear && currentYear==no){ 
      selectElt(subDiv, true);
      activeSelectBoxYear = subDiv; 
    } 
  } 
  if (! turnOffYearSpan) { 
      var subDiv = document.createElement('DIV'); 
      subDiv.id = 'yearDownDiv';
      subDiv.style.height = "13px";
      subDiv.innerHTML = '<img src="' + pathToImages + '/gw-arrow-down.png" style="float:none;">';
      subDiv.onclick = changeSelectBoxYear; 
      subDiv.onmouseover = scrollMonthYear; 
      subDiv.onmouseout = function(){ selectBoxMovementInProgress = false;}; 
      subDiv.onselectstart = cancelCalendarEvent; 
      div.appendChild(subDiv); 
  } 
  return div; 
} 
 
/* This function creates the hour div at the bottom bar */ 
 
function slideCalendarSelectBox() 
{ 
  if(selectBoxMovementInProgress){ 
    if(activeSelectBox.parentNode.id=='hourDropDown')
      changeSelectBoxHour(false,activeSelectBox); 
    else if(activeSelectBox.parentNode.id=='yearDropDown')
      changeSelectBoxYear(false,activeSelectBox); 
    setTimeout(slideCalendarSelectBox, speedOfSelectBoxSliding); 
  } 
} 
 
function createHourDiv() 
{ 
  if(!$('hourDropDown')){ 
    var div = document.createElement('DIV'); 
    div.className='monthYearPicker'; 
  }else{ 
    var div = $('hourDropDown'); 
    var subDivs = div.getElementsByTagName('DIV'); 
    for(var no=0;no<subDivs.length;no++){ 
      subDivs[no].parentNode.removeChild(subDivs[no]); 
    } 
  } 
 
  var startHour = currentHour/1; 
  if(startHour>14)startHour=14; 
 
  var subDiv = document.createElement('DIV'); 
  //subDiv.innerHTML = '&nbsp;&nbsp;- '; 
  subDiv.style.height = '13px';
  subDiv.id = 'hourUpDiv';
  subDiv.innerHTML = '<img src="' + pathToImages + '/gw-arrow-up.png" style="position:absolute;left:4px;">';
  subDiv.onclick = changeSelectBoxHour; 
  subDiv.onmouseover = scrollMonthYear; 
  subDiv.onmouseout = function(){ selectBoxMovementInProgress = false;}; 
  subDiv.onselectstart = cancelCalendarEvent; 
  div.appendChild(subDiv); 
 
  for(var no=startHour;no<startHour+10;no++){ 
    var prefix = ''; 
    if(no/1<10)prefix='0'; 
    var subDiv = document.createElement('DIV'); 
    subDiv.innerHTML = prefix + no; 
    //subDiv.onmouseover = scrollMonthYear; 
    //subDiv.onmouseout = scrollMonthYear; 
    subDiv.onclick = selectHour; 
    subDiv.id = 'hourDiv' + padleft(no, '00'); 
    subDiv.onselectstart = cancelCalendarEvent; 
    div.appendChild(subDiv); 
    if(currentHour && currentHour == no)
    { 
      selectElt(subDiv, true);
      activeSelectBoxYear = subDiv; 
    } 
  } 
  var subDiv = document.createElement('DIV'); 
  subDiv.style.height = '13px';
  subDiv.id = 'hourDownDiv';
  subDiv.innerHTML = '<img src="' + pathToImages + '/gw-arrow-down.png" style="position:absolute;left:4px;">';
  //subDiv.innerHTML = '&nbsp;&nbsp;+ '; 
  subDiv.onclick = changeSelectBoxHour; 
  subDiv.onmouseover = scrollMonthYear; 
  subDiv.onmouseout = function(){ selectBoxMovementInProgress = false;}; 
  subDiv.onselectstart = cancelCalendarEvent; 
  div.appendChild(subDiv); 
 
  return div; 
} 
/* This function creates the minute div at the bottom bar */ 
 
function createMinuteDiv() 
{ 
  if(!$('minuteDropDown')){ 
    var div = document.createElement('DIV'); 
    div.className='monthYearPicker'; 
  }else{ 
    var div = $('minuteDropDown'); 
    var subDivs = div.getElementsByTagName('DIV'); 
    for(var no=0;no<subDivs.length;no++){ 
      subDivs[no].parentNode.removeChild(subDivs[no]); 
    } 
  } 
  var startMinute = 0; 
  var prefix = ''; 
  for(var no=startMinute;no<60;no+=intervalSelectBox_minutes){ 
 
    if(no<10)prefix='0'; else prefix = ''; 
    var subDiv = document.createElement('DIV'); 
    subDiv.innerHTML = prefix + no; 
    //subDiv.onmouseover = scrollMonthYear; 
    //subDiv.onmouseout = scrollMonthYear; 
    subDiv.onclick = selectMinute; 
    subDiv.id = 'minuteDiv_' + prefix +  no; 
    subDiv.onselectstart = cancelCalendarEvent; 
    div.appendChild(subDiv); 
    if(currentMinute && currentMinute == no)
    { 
      selectElt(subDiv, true);
      activeSelectBoxYear = subDiv; 
    } 
  } 
  return div; 
} 
 
function closeCalendar()
{ 
  $('yearDropDown').style.display='none'; 
  $('monthDropDown').style.display='none'; 
  $('hourDropDown').style.display='none'; 
  $('minuteDropDown').style.display='none'; 
 
  calendarDiv.style.display='none'; 
  if(iframeObj){ 
    iframeObj.style.display='none'; 
     //// //// fix for EI frame problem on time dropdowns 09/30/2006 
      EIS_Hide_Frame();} 
  if(activeSelectBoxMonth)activeSelectBoxMonth.className=''; 
  if(activeSelectBoxYear)activeSelectBoxYear.className=''; 
  
  var hShadow = $('gw-modal');
  hShadow.onclick = null;
  hShadow.style.visibility = 'hidden';
  
  returnDateTo.focus();
} 
 
function writeTopBar() 
{ 
  var topBar = document.createElement('DIV'); 
  topBar.className = 'topBar'; 
  topBar.id = 'topBar'; 
  calendarDiv.appendChild(topBar); 
 
  // Left arrow 
  var leftDiv = document.createElement('DIV'); 
  leftDiv.style.marginRight = '1px'; 
  var img = document.createElement('IMG');
  img.src = pathToImages + '/gw-arrow-left.png'; 
  //img.onmouseover = highlightArrow; 
  //img.onmouseout = highlightArrow; 
  img.onclick = switchMonth; 
  img.width = 16;
  img.height = 16;
  leftDiv.appendChild(img); 
  topBar.appendChild(leftDiv); 
  if(Opera)leftDiv.style.width = '16px'; 
 
  // Right arrow 
  var rightDiv = document.createElement('DIV'); 
  rightDiv.style.marginRight = '1px'; 
  var img = document.createElement('IMG'); 
  img.src = pathToImages + '/gw-arrow-right.png'; 
  img.onclick = switchMonth; 
  //img.onmouseover = highlightArrow; 
  //img.onmouseout = highlightArrow; 
  img.width = 16;
  img.height = 16;
  rightDiv.appendChild(img); 
  if(Opera)rightDiv.style.width = '16px'; 
  topBar.appendChild(rightDiv); 
 
 
  // Month selector 
  var monthDiv = document.createElement('DIV'); 
  monthDiv.id = 'monthSelect'; 
  //monthDiv.onmouseover = highlightSelect; 
  //monthDiv.onmouseout = highlightSelect; 
  monthDiv.onclick = showMonthDropDown; 
 
  var span = document.createElement('SPAN'); 
  span.innerHTML = monthArray[currentMonth]; 
  span.id = 'calendar_month_txt'; 
  monthDiv.appendChild(span); 
   
  var img = document.createElement('IMG'); 
  img.src = pathToImages + '/gw-arrow-down.png'; 
  img.width = 14;
  img.height = 11;
  img.style.position = 'absolute'; 
  img.style.right = '0px';
  monthDiv.appendChild(img); 
  monthDiv.className = 'selectBox'; 
  if(Opera){ 
    img.style.cssText = 'float:right;position:relative'; 
    img.style.position = 'relative'; 
    img.style.styleFloat = 'right'; 
  } 
   
  topBar.appendChild(monthDiv); 
 
  var monthPicker = createMonthDiv(); 
  monthPicker.style.left = monthDiv.offsetLeft + 'px'; 
  monthPicker.style.top = (monthDiv.offsetTop + monthDiv.offsetHeight + 1) + 'px'; 
  monthPicker.style.width = monthDiv.offsetWidth + 'px'; 
  monthPicker.id = 'monthDropDown'; 
 
  calendarDiv.appendChild(monthPicker); 
 
  // Year selector 
  var yearDiv = document.createElement('DIV'); 
  yearDiv.id = 'yearSelect'; 
  //yearDiv.onmouseover = highlightSelect; 
  //yearDiv.onmouseout = highlightSelect; 
  yearDiv.onclick = showYearDropDown; 
  var span = document.createElement('SPAN'); 
  span.innerHTML = currentYear; 
  span.id = 'calendar_year_txt'; 
  yearDiv.appendChild(span); 
  topBar.appendChild(yearDiv); 
 
  var img = document.createElement('IMG'); 
  img.src = pathToImages + 'gw-arrow-down.png'; 
  img.width = 14;
  img.height = 11;
  img.style.position = 'absolute'; 
  img.style.right = '0px'; 
  yearDiv.appendChild(img); 
  yearDiv.className = 'selectBox'; 
 
  if(Opera){ 
    yearDiv.style.width = '50px'; 
    img.style.cssText = 'float:right'; 
    img.style.position = 'relative'; 
    img.style.styleFloat = 'right'; 
  } 
 
  var yearPicker = createYearDiv(); 
  yearPicker.style.left = yearDiv.offsetLeft + 'px'; 
  yearPicker.style.top = monthPicker.style.top; 
  yearPicker.style.width = yearDiv.offsetWidth + 'px'; 
  yearPicker.id = 'yearDropDown'; 
  calendarDiv.appendChild(yearPicker); 
 
  var img = document.createElement('IMG'); 
  img.src = pathToImages + '/gw-close.png'; 
  img.width = 16;
  img.height = 16;
  img.style.styleFloat = 'right'; 
  //img.onmouseover = highlightClose; 
  //img.onmouseout = highlightClose; 
  img.onclick = closeCalendar; 
  topBar.appendChild(img); 
  if(!document.all){ 
    img.style.position = 'absolute'; 
    img.style.right = '2px'; 
  } 
} 
 
function writeCalendarContent() 
{ 
  var calendarContentDivExists = true; 
  if(!calendarContentDiv){ 
    calendarContentDiv = document.createElement('DIV'); 
    calendarDiv.appendChild(calendarContentDiv); 
    calendarContentDivExists = false; 
  } 
  currentMonth = currentMonth/1; 
  var d = new Date(); 
 
  d.setFullYear(currentYear); 
  d.setDate(1); 
  d.setMonth(currentMonth); 
 
  var dayStartOfMonth = d.getDay(); 
  if (! weekStartsOnSunday) { 
      if(dayStartOfMonth==0)dayStartOfMonth=7; 
      dayStartOfMonth--; 
   } 
 
  $('calendar_year_txt').innerHTML = currentYear; 
  $('calendar_month_txt').innerHTML = monthArray[currentMonth]; 
  $('calendar_hour_txt').innerHTML = currentHour; 
  $('calendar_minute_txt').innerHTML = currentMinute; 
 
  var existingTable = calendarContentDiv.getElementsByTagName('TABLE'); 
  if(existingTable.length>0){ 
    calendarContentDiv.removeChild(existingTable[0]); 
  } 
 
  var calTable = document.createElement('TABLE'); 
  calTable.width = '100%'; 
  calTable.cellSpacing = '0'; 
  calendarContentDiv.appendChild(calTable); 
 
  var calTBody = document.createElement('TBODY'); 
  calTable.appendChild(calTBody); 
  var row = calTBody.insertRow(-1); 
  row.className = 'calendar_week_row'; 
  if (showWeekNumber) 
  { 
     var cell = row.insertCell(-1); 
     cell.innerHTML = weekString; 
     cell.className = 'calendar_week_column'; 
     cell.style.backgroundColor = selectBoxRolloverBgColor; 
  } 
 
  for(var no=0;no<dayArray.length;no++)
  { 
    var cell = row.insertCell(-1); 
    cell.style.width = '20px'; 
    cell.innerHTML = dayArray[no]; 
  } 
 
  var row = calTBody.insertRow(-1); 
 
   if (showWeekNumber) { 
     var cell = row.insertCell(-1); 
     cell.className = 'calendar_week_column'; 
     cell.style.backgroundColor = selectBoxRolloverBgColor; 
     var week = getWeek(currentYear,currentMonth,1); 
     cell.innerHTML = week;    // Week 
  } 
  for(var no=0;no<dayStartOfMonth;no++){ 
    var cell = row.insertCell(-1); 
    cell.innerHTML = '&nbsp;'; 
  } 
 
  var colCounter = dayStartOfMonth; 
  var daysInMonth = daysInMonthArray[currentMonth]; 
  if(daysInMonth==28){ 
    if(isLeapYear(currentYear))daysInMonth=29; 
  } 
 
  for(var no=1;no<=daysInMonth;no++){ 
    d.setDate(no-1); 
    if(colCounter>0 && colCounter%7==0){ 
      var row = calTBody.insertRow(-1); 
         if (showWeekNumber) { 
            var cell = row.insertCell(-1); 
            cell.className = 'calendar_week_column'; 
            var week = getWeek(currentYear,currentMonth,no); 
            cell.innerHTML = week;    // Week 
            cell.style.backgroundColor = selectBoxRolloverBgColor; 
         } 
    } 
    var cell = row.insertCell(-1); 
    if(currentYear==inputYear && currentMonth == inputMonth && no==inputDay)
      cell.className='activeDay'; 
    else
      cell.className='normalDay';
    cell.innerHTML = no; 
    if (calendarDisplayTime)
      cell.onclick = setDate; 
    else
      cell.onclick = pickDate; 
    colCounter++; 
  } 
 
  if(!document.all){ 
    if(calendarContentDiv.offsetHeight) 
      $('topBar').style.top = calendarContentDiv.offsetHeight + $('timeBar').offsetHeight + $('topBar').offsetHeight -1 + 'px'; 
    else{ 
      $('topBar').style.top = ''; 
      $('topBar').style.bottom = '0px'; 
    } 
 
  } 
 
  if(iframeObj){ 
    if(!calendarContentDivExists)setTimeout('resizeIframe()',350);else setTimeout('resizeIframe()',10); 
  } 
} 
 
function resizeIframe() 
{ 
  iframeObj.style.width = calendarDiv.offsetWidth + 'px'; 
  iframeObj.style.height = calendarDiv.offsetHeight + 'px' ; 
} 
 
function pickTodaysDate(n) 
{ 
  var d = new Date();
  if (n !== undefined)
    d.setDate(d.getDate() + n);
    
  currentMonth = d.getMonth(); 
  currentYear = d.getFullYear(); 
  inputDay = d.getDate();
  pickDate(null, inputDay);
} 

function setDate(e, day)
{
  inputDay = this.innerHTML;
  inputMonth = currentMonth;
  inputYear = currentYear;
  writeCalendarContent();
}

function pickDate(e, day) 
{ 
  var month;
  
  month = currentMonth/1 + 1;
  if (month < 10)
    month = '0' + month; 
  
  if (!day && this && this.innerHTML)
    day = this.innerHTML; 
  else 
    day = inputDay/1; 
 
  if (day/1 < 10)
    day = '0' + day; 
    
  if (returnFormat)
  { 
    returnFormat = returnFormat.replace('dd',day); 
    returnFormat = returnFormat.replace('mm',month); 
    returnFormat = returnFormat.replace('yyyy',currentYear); 
    returnFormat = returnFormat.replace('hh',currentHour); 
    returnFormat = returnFormat.replace('nn',currentMinute); 
    returnFormat = returnFormat.replace('d',day/1); 
    returnFormat = returnFormat.replace('m',month/1); 
 
    returnDateTo.value = returnFormat; 
  }
  else
  { 
    for(var no=0;no<returnDateToYear.options.length;no++){ 
      if(returnDateToYear.options[no].value==currentYear){ 
        returnDateToYear.selectedIndex=no; 
        break; 
      } 
    } 
    for(var no=0;no<returnDateToMonth.options.length;no++){ 
      if(returnDateToMonth.options[no].value==parseInt(month)){ 
        returnDateToMonth.selectedIndex=no; 
        break; 
      } 
    } 
    for(var no=0;no<returnDateToDay.options.length;no++){ 
      if(returnDateToDay.options[no].value==parseInt(day)){ 
        returnDateToDay.selectedIndex=no; 
        break; 
      } 
    } 
    if(calendarDisplayTime){ 
      for(var no=0;no<returnDateToHour.options.length;no++){ 
        if(returnDateToHour.options[no].value==parseInt(currentHour)){ 
          returnDateToHour.selectedIndex=no; 
          break; 
        } 
      } 
      for(var no=0;no<returnDateToMinute.options.length;no++){ 
        if(returnDateToMinute.options[no].value==parseInt(currentMinute)){ 
          returnDateToMinute.selectedIndex=no; 
          break; 
        } 
      } 
    } 
  } 
  
  if (must_submit)
    submit_form();
  else
    closeCalendar(); 
} 
 
// This function is from http://www.codeproject.com/csharp/gregorianwknum.asp 
// Only changed the month add 
function getWeek(year,month,day){ 
   if (! weekStartsOnSunday) { 
     day = (day/1); 
  } else { 
     day = (day/1)+1; 
  } 
  year = year /1; 
    month = month/1 + 1; //use 1-12 
    var a = Math.floor((14-(month))/12); 
    var y = year+4800-a; 
    var m = (month)+(12*a)-3; 
    var jd = day + Math.floor(((153*m)+2)/5) + 
                 (365*y) + Math.floor(y/4) - Math.floor(y/100) + 
                 Math.floor(y/400) - 32045;      // (gregorian calendar) 
    var d4 = (jd+31741-(jd%7))%146097%36524%1461; 
    var L = Math.floor(d4/1460); 
    var d1 = ((d4-L)%365)+L; 
    NumberOfWeek = Math.floor(d1/7) + 1; 
    return NumberOfWeek; 
} 
 
function writeTimeBar() 
{ 
  var timeBar = document.createElement('DIV'); 
  timeBar.id = 'timeBar'; 
  timeBar.className = 'timeBar'; 
 
  var subDiv = document.createElement('DIV'); 
  subDiv.innerHTML = 'Time:'; 
  //timeBar.appendChild(subDiv); 
 
  // Hour selector 
  var hourDiv = document.createElement('DIV'); 
  //hourDiv.onmouseover = highlightSelect; 
  //hourDiv.onmouseout = highlightSelect; 
  hourDiv.onclick = showHourDropDown; 
  hourDiv.style.width = '26px'; 
  var span = document.createElement('SPAN'); 
  span.innerHTML = currentHour; 
  span.id = 'calendar_hour_txt'; 
  hourDiv.appendChild(span); 
  timeBar.appendChild(hourDiv); 
 
  var img = document.createElement('IMG'); 
  img.src = pathToImages + '/gw-arrow-down.png';
  img.width = 14;
  img.height = 11;
  hourDiv.appendChild(img); 
  hourDiv.className = 'selectBoxTime'; 
 
  if(Opera){ 
    hourDiv.style.width = '26px'; 
    img.style.cssText = 'float:right'; 
    img.style.position = 'relative'; 
    img.style.styleFloat = 'right'; 
  } 
 
  var hourPicker = createHourDiv(); 
  hourPicker.style.left = '126px'; 
  //hourPicker.style.top = monthDiv.offsetTop + monthDiv.offsetHeight + 1 + 'px'; 
  hourPicker.style.width = '24px'; 
  hourPicker.id = 'hourDropDown'; 
  calendarDiv.appendChild(hourPicker); 
 
  // Add Minute picker 
 
  // Year selector 
  var minuteDiv = document.createElement('DIV'); 
  //minuteDiv.onmouseover = highlightSelect; 
  //minuteDiv.onmouseout = highlightSelect; 
  minuteDiv.onclick = showMinuteDropDown; 
  minuteDiv.style.width = '26px'; 
  var span = document.createElement('SPAN'); 
  span.innerHTML = currentMinute; 
 
  span.id = 'calendar_minute_txt'; 
  minuteDiv.appendChild(span); 
  timeBar.appendChild(minuteDiv); 
 
  var img = document.createElement('IMG'); 
  img.src = pathToImages + '/gw-arrow-down.png'; 
  img.width = 14;
  img.height = 11;
  minuteDiv.appendChild(img); 
  minuteDiv.className = 'selectBoxTime'; 
 
  if(Opera){ 
    minuteDiv.style.width = '26px'; 
    img.style.cssText = 'float:right'; 
    img.style.position = 'relative'; 
    img.style.styleFloat = 'right'; 
  } 
 
  var minutePicker = createMinuteDiv(); 
  minutePicker.style.left = '158px'; 
  //minutePicker.style.top = monthDiv.offsetTop + monthDiv.offsetHeight + 1 + 'px'; 
  minutePicker.style.width = '24px'; 
  minutePicker.id = 'minuteDropDown'; 
  calendarDiv.appendChild(minutePicker); 
 
  var img = document.createElement('IMG'); 
  img.src = pathToImages + 'ok.png'; 
  img.width = 16;
  img.height = 16;
  img.style.styleFloat = 'right'; 
  //img.onmouseover = highlightClose; 
  //img.onmouseout = highlightClose; 
  img.onclick = pickDate; 
  timeBar.appendChild(img); 

  return timeBar; 
} 
 
function writeBottomBar() 
{ 
  var d = new Date(); 
  var bottomBar = document.createElement('DIV'); 
 
  bottomBar.id = 'bottomBar'; 
 
  bottomBar.style.cursor = 'pointer'; 
  bottomBar.className = 'todaysDate'; 
  // var todayStringFormat = '[todayString] [dayString] [day] [monthString] [year]';  ;; 
 
  var subDiv = document.createElement('DIV'); 
  subDiv.onclick = function() { pickTodaysDate(); }
  subDiv.id = 'todaysDateString'; 
  //subDiv.style.width = (calendarDiv.offsetWidth - 95) + 'px'; 
  
  var day = d.getDay(); 
  if (!weekStartsOnSunday)
  { 
    if (day == 0)
      day = 7; 
    day--; 
  } 
 
  var bottomString = todayStringFormat; 
  bottomString = bottomString.replace('[monthString]',monthArrayShort[d.getMonth()]); 
  bottomString = bottomString.replace('[day]',d.getDate()); 
  bottomString = bottomString.replace('[year]',d.getFullYear()); 
  bottomString = bottomString.replace('[dayString]',dayArray[day].toLowerCase()); 
  bottomString = bottomString.replace('[UCFdayString]',dayArray[day]); 
  bottomString = bottomString.replace('[todayString]',todayString); 
 
  //subDiv.innerHTML = todayString + ': ' + d.getDate() + '. ' + monthArrayShort[d.getMonth()] + ', ' +  d.getFullYear() ; 
  subDiv.innerHTML = bottomString ; 
  bottomBar.appendChild(subDiv); 
 
  var dayPlus = document.createElement('DIV');
  dayPlus.className = 'dayPlus';
 
  var dayPlus1 = document.createElement('DIV');
  dayPlus1.id = 'dayPlus1';
  dayPlus1.innerHTML = 'J+1';
  dayPlus1.onclick = function() { pickTodaysDate(1); }
  dayPlus.appendChild(dayPlus1);
 
  var dayPlus2 = document.createElement('DIV');
  dayPlus2.id = 'dayPlus2';
  dayPlus2.innerHTML = 'J+2';
  dayPlus2.onclick = function() { pickTodaysDate(2); }
  dayPlus.appendChild(dayPlus2);
  
  bottomBar.appendChild(dayPlus); 
 
  var timeDiv = writeTimeBar(); 
  bottomBar.appendChild(timeDiv); 
 
  calendarDiv.appendChild(bottomBar); 
} 

function positionCalendar(inputObj) 
{ 
  //calendarDiv.style.left = (getLeftPos(inputObj, calendarDiv) + (navigator.userAgent.indexOf('Firefox') >= 0 ? 1 : 0)) + 'px'; 
  //calendarDiv.style.top = (getTopPos(inputObj), calendarDiv) + 'px'; 
  moveElementUnder(calendarDiv, inputObj, 0, 0);
  if(iframeObj){ 
    iframeObj.style.left = calendarDiv.style.left; 
    iframeObj.style.top =  calendarDiv.style.top; 
    //// fix for EI frame problem on time dropdowns 09/30/2006 
    iframeObj2.style.left = calendarDiv.style.left; 
    iframeObj2.style.top =  calendarDiv.style.top; 
  } 
 
} 
 
function initCalendar() 
{ 
  if(MSIE){ 
    iframeObj = document.createElement('IFRAME'); 
    iframeObj.style.filter = 'alpha(opacity=0)'; 
    iframeObj.style.position = 'absolute'; 
    iframeObj.border='0px'; 
    iframeObj.style.border = '0px'; 
    iframeObj.style.backgroundColor = '#FF0000'; 
    //// fix for EI frame problem on time dropdowns 09/30/2006 
    iframeObj2 = document.createElement('IFRAME'); 
    iframeObj2.style.position = 'absolute'; 
    iframeObj2.border='0px'; 
    iframeObj2.style.border = '0px'; 
    iframeObj2.style.height = '1px'; 
    iframeObj2.style.width = '1px'; 
    //// fix for EI frame problem on time dropdowns 09/30/2006 
    // Added fixed for HTTPS 
    iframeObj2.src = 'blank.html'; 
    iframeObj.src = 'blank.html'; 
    document.body.appendChild(iframeObj2);  // gfb move this down AFTER the .src is set 
    document.body.appendChild(iframeObj); 
  } 
 
  calendarDiv = $("calendarDiv");
  if (calendarDiv == null)
  {
    calendarDiv = document.createElement('div');
    calendarDiv.id = 'calendarDiv';
    document.getElementsByTagName("body")[0].appendChild(calendarDiv);
  }
  
  writeBottomBar(); 
  writeTopBar(); 
 
  /*if(!currentYear){ 
    var d = new Date(); 
    currentMonth = d.getMonth(); 
    currentYear = d.getFullYear(); 
  }*/

  writeCalendarContent();
} 
 
function setTimeProperties() 
{ 
  if (!calendarDisplayTime)
  { 
    $('timeBar').style.display='none'; 
    $('timeBar').style.visibility='hidden'; 
    //$('todaysDateString').style.width = '100%'; 
    $('dayPlus1').style.display = 'inline-table';
    $('dayPlus2').style.display = 'inline-table';
  }
  else
  { 
    $('timeBar').style.display='block'; 
    $('timeBar').style.visibility='visible'; 
    $('hourDropDown').style.top = $('calendar_minute_txt').parentNode.offsetHeight + calendarContentDiv.offsetHeight + $('topBar').offsetHeight + 'px'; 
    $('minuteDropDown').style.top = $('calendar_minute_txt').parentNode.offsetHeight + calendarContentDiv.offsetHeight + $('topBar').offsetHeight + 'px'; 
    $('minuteDropDown').style.right = '50px'; 
    $('hourDropDown').style.right = '50px'; 
    //$('todaysDateString').style.width = '';
    $('dayPlus1').style.display = 'none';
    $('dayPlus2').style.display = 'none';
  } 
} 
 
function calendarSortItems(a,b) 
{ 
  return a/1 - b/1;
} 
 
function trim(str)
{
  return str.replace(/^\s+|\s+$/g, '');
}

function padleft(str, fmt)
{
  str = str.toString();
  return fmt.substr(0, fmt.length - str.length) + str;
}
 
function displayCalendar(inputField, format, buttonObj, displayTime, submit) 
{
  var input, pos, date, time;
  var items, d;
  
  calendarDisplayTime = displayTime == true;
  must_submit = submit;
  
  d = new Date(); 
  d = new Date(d.getFullYear(), d.getMonth(), d.getDate()); 
  
  input = trim(inputField.value);

  if (input != '')
  {
    pos = input.indexOf(' ');
    if (pos > 0)
    {
      date = input.substr(0, pos);
      time = trim(input.substr(pos));
    }
    else
    {
      date = input;
      time = '00:00';
    }
    
    try
    {
      items = date.split(/[\/]/gi); 
    
      inputDay = items[0]/1;
      currentMonth = items[1]/1;
      currentYear = items[2]/1;
    
      items = time.split(/:/gi);
      
      currentHour = items[0]/1;
      currentMinute = items[1]/1;
      
      if (isFinite(currentYear) && isFinite(currentMonth) && isFinite(inputDay) && isFinite(currentHour) && isFinite(currentMinute))
        d = new Date(currentYear, currentMonth - 1, inputDay, currentHour, currentMinute, 0);
    }
    catch(e)
    {
    }
  }
  
  currentMonth = padleft(d.getMonth() + 1, '00'); 
  currentYear = padleft(d.getFullYear(), '0000'); 
  currentHour = padleft(d.getHours(), '00'); 
  currentMinute = padleft(d.getMinutes(), '00'); 
  
  currentMonth--;
  
  inputDay = d.getDate()/1; 
  inputYear = currentYear; 
  inputMonth = currentMonth; 
  
  if (!calendarDiv)
  { 
    initCalendar(); 
  }
  else
  { 
    if (calendarDiv.style.display == 'block')
    { 
      closeCalendar(); 
      return false; 
    } 
    
    writeCalendarContent(); 
  } 
 
  returnFormat = format; 
  returnDateTo = inputField; 
  
  calendarDiv.style.visibility = 'visible'; 
  calendarDiv.style.display = 'block'; 
  positionCalendar(buttonObj); 
  
  if(iframeObj){ 
    iframeObj.style.display = ''; 
    iframeObj.style.height = '140px'; 
    iframeObj.style.width = '195px'; 
        iframeObj2.style.display = ''; 
    iframeObj2.style.height = '140px'; 
    iframeObj2.style.width = '195px'; 
  } 
 
  setTimeProperties(); 
  updateYearDiv(); 
  updateMonthDiv(); 
  updateMinuteDiv(); 
  updateHourDiv(); 
  
  var hShadow = $('gw-modal');
  hShadow.style.visibility = 'visible';
  hShadow.onclick = closeCalendar;
} 
