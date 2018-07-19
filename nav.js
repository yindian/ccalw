function prev () {
  var year = parseInt(document.getElementById('year').textContent);
  --year;
  window.external.invoke('year:' + year);
}
function next () {
  var year = parseInt(document.getElementById('year').textContent);
  ++year;
  window.external.invoke('year:' + year);
}
var div = document.createElement('div');
div.style.cssText = 'text-align: center;';
div.innerHTML = '' +
'<button type="button" onclick="window.prev()">&lt;</button>\n' +
'<span id="year"></span>\n' +
'<button type="button" onclick="window.next()">&gt;</button>\n' +
'';
var app = document.getElementById('app');
document.body.insertBefore(div, app);
document.body.insertBefore(document.createElement('hr'), app);
window.prev = prev;
window.next = next;
