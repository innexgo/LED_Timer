function revealPassword(element, field_id) {
  var x = document.getElementById(field_id);
  if (x.type === "password" && element.checked) {
    x.type = "text";
  } else {
    x.type = "password";
  }
};

function postData(data, url) {
  return new Promise(function (resolve, reject) {
    var xhr = new XMLHttpRequest();
    xhr.open("POST", url, true);
    xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');

    let urlEncodedData = "",
      urlEncodedDataPairs = [],
      name;

    for(name in data) {
      urlEncodedDataPairs.push(encodeURIComponent(name) + '=' + encodeURIComponent(data[name]));
    }
    urlEncodedData = urlEncodedDataPairs.join( '&' )

    xhr.send(urlEncodedData);
    xhr.onload = function() {
      if (xhr.status >= 200 && xhr.status < 300) {
        resolve({
          response: xhr.response,
          status: xhr.status,
          url: xhr.responseURL
        });
      } else {
        reject({
          status: xhr.status,
          statusText: xhr.statusText,
          responseText: xhr.responseText
        });
      }
    }
  })
}

function errorAlert(response) {
    alert("The server replied: " + String(response.statusText) + ".\nPlease try again.\n" + 
            ((response.responseText == "") ? "" : ("In addition, the server said: " + String(response.responseText))));
}

function getHostname() {
  return new Promise(function (resolve, reject) {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/hostname", true);
    xhr.send("");
    xhr.onload = function() {
      if (xhr.status >= 200 && xhr.status < 300) {
        resolve({
          response: xhr.response,
          responseText: xhr.responseText,
          status: xhr.status,
          url: xhr.responseURL
        });
      } else {
        reject({
          status: xhr.status,
          statusText: xhr.statusText,
          responseText: xhr.responseText
        });
      }
    }
  })
}