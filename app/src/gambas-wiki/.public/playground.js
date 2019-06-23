// Gambas Wiki - Javascript to Run Examples in Playground
// Written by: Matthew Collins - Feb 2019
// Modified by: Benoît Minisini - Feb 2019

// Add this into the wiki: <script type="text/javascript" src="http://gambas.one/playground/wiki.js"></script>

// Run this when Document Loaded
document.addEventListener("DOMContentLoaded", function() {

    var i;
    var REMOTE_URI = 'https://pg1.gambas.one/run-daily.php';

    // Hide all results

    /*var all = document.querySelectorAll('DIV.result');
    for (i = 0; i < all.length; i++) {
      all[i].style.display = 'none';
    }*/

    // Get and loop all "code gambas" elements
    
    var gcs = document.getElementsByClassName("code gambas");
    var next;
    
    for (i = 0; i < gcs.length; i++) {
    
        next = gcs[i].parentElement.nextElementSibling;
    
        if (!next || !next.firstElementChild || next.firstElementChild.className != 'result')
          continue;

        // Add Outer Div with look of Result Box
        var divOuter = document.createElement("div");
        divOuter.className = "playground";
        //divOuter.style = "border:solid 1px #D8D8D8; border-top:none; background-color:#F8F8F8";
        gcs[i].parentElement.insertBefore(divOuter, gcs[i].nextSibling);

        // Add From
        var formPlay = document.createElement("form");
        formPlay.action = "https://gambas.one/playground/ultra.php";
        formPlay.method = "POST";
        formPlay.target = '_blank';
        divOuter.appendChild(formPlay);

        // Add Pre and Code Elements same a Result Box
        var preResult = document.createElement("pre");
        //preResult.style = "padding:0.5em 1em; margin:0";
        var codeResult = document.createElement("code");
        //codeResult.innerText = "...";
        preResult.appendChild(codeResult);
        formPlay.appendChild(preResult);

        // Add Run Button
        var buttonRun = document.createElement("button");
        buttonRun.type = "button";
        buttonRun.innerText = (("Run"));
        formPlay.appendChild(buttonRun);

        // Add Run Click Event
        buttonRun.addEventListener('click', function() {
	        var xhr = new XMLHttpRequest();
          this[1].parentElement.style.display = 'block';
	        //this[1].innerText = (("Loading..."));
	        this[1].innerHTML = '<div class="waiting"></div>';
	        xhr.onreadystatechange = function() {
		        if(xhr.readyState !== XMLHttpRequest.DONE) return;
            this.innerHTML = '';
		        if(xhr.status === 200) {
			        this.innerText = xhr.responseText;
		        } else {
			        this.innerText = 'Error: ' + xhr.status + ' : \n\n' + xhr.responseText;
		        }
	        }.bind(this[1]);
	        xhr.open('POST', REMOTE_URI, true);
	        xhr.send(this[0]);
        }.bind([gcs[i].innerText,codeResult]));

        // Add Input Box
        var inputCode = document.createElement("textarea");
        inputCode.style.display = 'none';
        inputCode.name = "Code";
        inputCode.value = gcs[i].innerText;    
        formPlay.appendChild(inputCode);

        // Add Play Button
        var buttonPlay = document.createElement("button");
        //buttonPlay.style="margin-left: 3px;";
        buttonPlay.innerText = (("Play"));
        formPlay.appendChild(buttonPlay);

    }

});


