function revealPassword(field_id) {
  var x = document.getElementById(field_id);
  if (x.type === "password") {
    x.type = "text";
  } else {
    x.type = "password";
  }
};

function postData(data, url) {
  return new Promise(function (resolve, reject) {
    var xhr = new XMLHttpRequest();
    xhr.open("POST", url, true);
    xhr.setRequestHeader('Content-Type', 'application/json');
    xhr.send(data);
    if (xhr.status >= 200 && xhr.status < 300) {
      resolve({
        response: xhr.response,
        status: xhr.status,
        url: xhr.responseURL
      });
    } else {
      reject({
        status: xhr.status,
        statusText: xhr.statusText
      });
    }
  })
}