https://script.google.com/macros/s/AKfycbxCOxSWChuxhSUxsv7hja2ZTlmrTG_CZcJnTvCfQmbsBAEKajdum19a8Q-ke3XjvdGx/exec
var ss = SpreadsheetApp.openById('1jayMPFpwc67dSZMukarB025ejhom4PEUYDvuERR6uNI');
var sheet = ss.getSheetByName('Sheet1');

function doPost(e) {
  var val = e.parameter.value;
  
  if (e.parameter.value !== undefined){
    var range = sheet.getRange('A2');
    range.setValue(val);
  }
}

function doGet(e){
  var read = e.parameter.read;

  if (read !== undefined){
    return ContentService.createTextOutput(sheet.getRange('C2').getValue());
  }
}
