function savePassword() {
    var password = document.getElementById("login-password").value;
    sessionStorage.setItem("password", password);
}

var login = document.getElementById("login");

/*
** Prevents the page from reloading on button, requires program to give ok to redirect.
*/
function handleLogin(event) {
    event.preventDefault();
    var time = String(Date.now());
    var password = document.getElementById("login-password").value;
    var verification = document.getElementById("verification").innerText;
    var tohash = time + verification + password;
    var hash = sjcl.hash.sha256.hash(tohash);
    var hashBits = sjcl.codec.hex.fromBits(hash);
    var data = {
        "time": time,
        "hash": hashBits.toUpperCase()
    };
    var xhrRequest = new postData(data, "/login");
    xhrRequest.then( function () {
        window.location.href = "timer.html";
    },
        (response) => {
            if (response.status === 401) {
                alert("Wrong password.");
            }
            else {
                alert("The server replied " + response.status + ". \n Please try again");
            }
        })
}

login.addEventListener('submit', handleLogin);

function loadSJCL() {
  var script = document.createElement('script');
  script.src = "js/sjcl.js";
  document.getElementsByTagName('head')[0].appendChild(script);
}

setTimeout(loadSJCL, 750);