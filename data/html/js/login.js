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
    var time = Date.now();
    var password = document.getElementById("login-password").value;
    var verification = document.getElementById("verification").innerText;
    var tohash = time + verification + password;
    var hash = sjcl.hash.sha256.hash(tohash);
    var hashBits = sjcl.codec.hex.fromBits(hash);
    var data = JSON.stringify({
        time: time,
        hash: hashBits
    });
    var xhrRequest = new postData(data, "/login");
    xhrRequest.then(response => function () {
        window.location.href = response.url;
    },
        response => function () {
            if (response.status === 401) {
                alert("Wrong password.");
            }
            else {
                alert("The server replied " + response.status + ". \n Please try again");
            }
        })
}

login.addEventListener('submit', handleLogin);