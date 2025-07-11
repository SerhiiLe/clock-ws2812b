function ajaxRequest(url, aMethod, param, onSuccess, onFailure) {
	var aR = new XMLHttpRequest();
	aR.onreadystatechange = function() {
		if( aR.readyState == 4 && (aR.status == 200 || aR.status == 304))
			onSuccess(aR);
		else if (aR.readyState == 4 && aR.status != 200) onFailure(aR);
	};
	aR.open(aMethod, url, true);
	if (aMethod == 'POST') aR.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded; charset=utf-8');
	aR.send(param);
};
function $g(aID) {return (aID !== '' ? document.getElementById(aID) : null);}
function $gn(aID) {return (aID != '' ? document.getElementsByName(aID) : null);}
function $gt(str,m) {return (typeof m == 'undefined' ? document:m).getElementsByTagName(str);}
function $gc(str,m) {return (typeof m == 'undefined' ? document:m).getElementsByClassName(str);}
function $at(aElem, att) {if (att !== undefined) {for (var xi in att) {aElem.setAttribute(xi, att[xi]); if (xi.toUpperCase() == 'TITLE') aElem.setAttribute('alt', att[xi]);}}}
function $t(iHTML) {return document.createTextNode(iHTML);}
function $e(nElem, att) {var Elem = document.createElement(nElem); $at(Elem, att); return Elem;}
function $ee(nElem, oElem, att) {var Elem = $e(nElem, att); if (oElem !== undefined) if( typeof(oElem) == 'object' ) Elem.appendChild(oElem); else Elem.innerHTML = oElem; return Elem;}
function $am(Elem, mElem) {if (mElem !== undefined) for(var i = 0; i < mElem.length; i++) { if( typeof(mElem[i]) == 'object' ) Elem.appendChild(mElem[i]); else Elem.appendChild($t(mElem[i])); } return Elem;}
function $em(nElem, mElem, att) {var Elem = $e(nElem, att); return $am(Elem, mElem);}
function insertAfter(node, rN) {rN.parentNode.insertBefore(node, rN.nextSibling);}
function $ib(node, rN) {rN.parentNode.insertBefore(node, rN);}
function dummy() {return;}
jsVoid = 'javaScript:void(0)';
jsNone = 'return false;';

function httpGet(url) {
	var xhttp = new XMLHttpRequest();
	xhttp.open("GET", url, false);
	xhttp.send(null);
	return xhttp.responseText;
};

function toggle_by_class(cls, on) {
	var lst = $gc(cls);
	for(var i = 0; i < lst.length; ++i) {
		lst[i].style.display = on ? '' : 'none';
	}
};
function show_block(check_el, sw_class, check_form="settings") {
	if(!sw_class) sw_class = check_el;
	if(document.forms[check_form].elements[check_el].checked)
		toggle_by_class(sw_class,true);
	else
		toggle_by_class(sw_class,false);
};
function color_text(p,c) {
	var sp = $gt("span",p);
	sp[1].innerHTML = c.slice(1,3).toUpperCase();
	sp[2].innerHTML = c.slice(3,5).toUpperCase();
	sp[3].innerHTML = c.slice(5).toUpperCase();
};
function color_demo(o) {
	$g(o.name).style = "background-color:" + o.value;
	color_text($g(o.name).parentNode, o.value);
};
function init_color_demo() {
	var t = $gc("color_demo");
	for(var i=0; i<t.length; i++) {
		t[i].style = "background-color:" + $gn(t[i].id)[0].value;
		color_text(t[i].parentNode, $gn(t[i].id)[0].value);
	}
};
function show_color_block(cl="color0", el="color_mode", frm="settings") {
	if(document.forms[frm].elements[el].value == 0)
		toggle_by_class(cl,true);
	else
		toggle_by_class(cl,false);
};
function ret() {
	$g("return").removeAttribute("onclick");
	$g("list").style.display = "table";
	$g("edit").style.display = "none";
};
function toggle_edit(n,s) {
	var url = ["off_alarm","off_text"];
	var e = $g("e"+n);
	if( e.checked ) {
		show_edit(n);
		e.checked = false;
	} else {
		$g("t"+n).classList.add("off");
		ajaxRequest(url[s], "POST", "t="+n, dummy, dummy);
	}
};
var cur = 0;
var col = 0;
var vol = 15;
function show(a) {
	cur = a[0];
	if($g("cur").value != cur)
		$g("cur").value = cur;
	col = a[1];
	$g("col").innerHTML = col;
	vol = a[2];
	$g("volume").innerHTML = vol;
	$g("vol").value = vol;
	var status = $g("status");
	if(status) status.innerHTML = a[3] == "0" ? "":"*";
};
var stime;
function sendVol() {
	const timeout = 200; // задержка перед отсылкой
	stime = new Date().getTime();
	setTimeout(() => {
		var ctime = new Date().getTime();
		if( ctime-stime < timeout ) return; // произошло более новое событие
		send(9);
		stime = ctime;
	}, timeout);
};
var fl_busy = false;
function send(p) {
	if(fl_busy) return;
	fl_busy = true;
	$g("wait").innerHTML = "w";
	ajaxRequest("play","POST","p="+p+"&c="+$g("cur").value+"&r="+$g("repeat").value+"&v="+$g("vol").value, function(ajaxResp) {
		show(ajaxResp.responseText.split(":"));
		fl_busy = false;
		$g("wait").innerHTML = "";
	}, dummy);
};
function onoff(id,a=1) {
	ajaxRequest("onoff","POST","t="+id+"&a="+a, function(ajaxResp) {
		$g(id).innerHTML = ajaxResp.responseText=="1" ? "On": "Off";
	}, dummy);
};
function fill_settings(url,form_name,cbFunc=null) {
	ajaxRequest(url,"GET",null, function(ajaxResp) {
		var doc = JSON.parse(ajaxResp.responseText);
		var f = document.forms[form_name];
		for (var key in doc) {
			if(!f.elements[key]) continue;
			if(f.elements[key].type=="checkbox") {
				if( f.elements[key].checked && doc[key] == 0 )
					f.elements[key].checked = false;
				if( ! f.elements[key].checked && doc[key] != 0 )
					f.elements[key].checked = true;
			} else if(f.elements[key].type=="time") {
				var h = Math.floor(doc[key]/60);
				var m = doc[key]%60;
				f.elements[key].value = (h<10?"0"+h:h) + ":" + (m<10?"0"+m:m);
			} else
				f.elements[key].value = doc[key];
		}
		if(cbFunc !== null) cbFunc();
	}, dummy);
};
function localization(cbFunc1,cbFunc2) {
	ajaxRequest("status","GET",null, (ajaxResp) => {
		const info = JSON.parse(ajaxResp.responseText)
		if(cbFunc1) cbFunc1(info);
		if( info.lang != "en" ) {
			ajaxRequest("localization_"+info.lang+".json","GET",null, (ajaxResp) => {
				const DICT = JSON.parse(ajaxResp.responseText);
				const i18 = document.querySelectorAll("[i18]");
				i18.forEach((el) => {
					const txt = el.getAttribute("i18");
					if( typeof DICT === "object" )
						if( typeof DICT[txt] != 'undefined' )
							el.innerHTML = DICT[txt];
				});
				if(cbFunc2) cbFunc2();
			}, cbFunc2 ? () => cbFunc2(): dummy);
		} else if(cbFunc2) cbFunc2();
	}, cbFunc2 ? () => cbFunc2(): dummy);
};