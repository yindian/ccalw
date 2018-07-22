var step = 1;
function prev (n) {
  n = n || 1;
  do {
    window.external.invoke('prev:' + step);
  } while (--n);
}
function next (n) {
  n = n || 1;
  do {
    window.external.invoke('next:' + step);
  } while (--n);
}
var timers = {};
function keep (f) {
  window.external.invoke('keep:' + f);
  if (timers[f]) {
    window.clearInterval(timers[f]);
  }
  timers[f] = window.setTimeout(function () {
    timers[f] = window.setInterval(f, 20);
    timers['new:' + f] = window.setInterval(function () {
      step *= 2;
    }, 1000);
  }, 200);
}
function stop (f) {
  window.external.invoke('stop:' + f);
  if (timers[f]) {
    window.clearInterval(timers[f]);
    delete timers[f];
    window.clearInterval(timers['new:' + f]);
    delete timers['new:' + f];
  } else if (f === undefined) {
    for (var k in timers) {
      window.clearInterval(timers[k]);
      timers[k] = undefined;
    }
  }
  step = 1;
}
function selyear () {
  if (true) {
    return;
  }
  var year = parseInt(document.getElementById('year').textContent);
  var n = window.prompt('请输入公历年份', year);
  if (n) {
    window.external.invoke('year:' + n);
  }
}
function setleap () {
  var n = document.getElementById('leap').value;
  window.external.invoke('leap:' + n);
}
var div = document.createElement('div');
div.style.cssText = 'text-align: center;';
div.innerHTML = '' +
'<button type="button" onclick="prev()" ondblclick="prev()" onmousedown="keep(prev)" onmouseup="stop(prev)">&lt;</button>\n' +
'<span id="year" onclick="selyear()"></span>\n' +
'<button type="button" onclick="next()" ondblclick="next()" onmousedown="keep(next)" onmouseup="stop(next)">&gt;</button>\n' +
'<div style="float: right"><select id="leap" onchange="setleap()"><option value="0">不筛选</option></select></div>\n' +
'';
var app = document.getElementById('app');
document.body.insertBefore(div, app);
document.body.insertBefore(document.createElement('hr'), app);
var leap = document.getElementById('leap');
var monthStr = '？一二三四五六七八九十冬腊';
for (var i = 1; i <= 12; i++) {
  var option = document.createElement('option');
  option.value = i;
  option.text = '闰' + monthStr.charAt(i) + '月';
  leap.add(option);
}
option = document.createElement('option');
option.value = i;
option.text = '有闰月';
leap.add(option);
window.prev = prev;
window.next = next;
window.keep = keep;
window.stop = stop;
window.selyear = selyear;
window.setleap = setleap;
