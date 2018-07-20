function prev (n) {
  n = n || 1;
  do {
    window.external.invoke('prev:');
  } while (--n);
}
function next (n) {
  n = n || 1;
  do {
    window.external.invoke('next:');
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
  }, 200);
}
function stop (f) {
  window.external.invoke('stop:' + f);
  if (timers[f]) {
    window.clearInterval(timers[f]);
    delete timers[f];
  } else if (f === undefined) {
    for (var k in timers) {
      window.clearInterval(timers[k]);
      timers[k] = undefined;
    }
  }
}
var div = document.createElement('div');
div.style.cssText = 'text-align: center;';
div.innerHTML = '' +
'<button type="button" onclick="prev()" ondblclick="prev()" onmousedown="keep(prev)" onmouseup="stop(prev)">&lt;</button>\n' +
'<span id="year"></span>\n' +
'<button type="button" onclick="next()" ondblclick="next()" onmousedown="keep(next)" onmouseup="stop(next)">&gt;</button>\n' +
'';
var app = document.getElementById('app');
document.body.insertBefore(div, app);
document.body.insertBefore(document.createElement('hr'), app);
window.prev = prev;
window.next = next;
window.keep = keep;
window.stop = stop;
