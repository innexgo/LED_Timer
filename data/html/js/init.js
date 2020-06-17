window.onload = function () {
    var divOne = document.getElementById("wifi-select-div");
    var divTwo = document.getElementById("credentials-div");

    var divOneWidth = divOne.offsetWidth + "px";
    var divTwoWidth = divTwo.offsetWidth + "px";

    if (divTwoWidth > divOneWidth) {
        divOne.style.width = divTwoWidth;
    }
    else {
        divOne.style.width = divOneWidth;
    }
}

function wifi_type(object) {
    document.getElementById("none").className = "";
    document.getElementById("WPA2").className = "";
    document.getElementById("WPA2-E").className = "";
    document.getElementById("unsecured").className = "";

    var wifi_type = object.value;
    var selected_element = document.getElementById(wifi_type);

    selected_element.className = "active";

    if (wifi_type != "none") {
        document.getElementById("name").placeholder = "hostname";
        document.getElementById("name-label").innerText = "Hostname: "
    }
    else {
        document.getElementById("name").placeholder = "hostname/SSID";
        document.getElementById("name-label").innerText = "Hostname/SSID: "
    };
    document.getElementById("confirmation-prompt").style.visibility = "hidden";
};

form0 = document.getElementById("none");
form1 = document.getElementById("WPA2");
form2 = document.getElementById("WPA2-E");
form3 = document.getElementById("unsecured");
form4 = document.getElementById("credentials");


/*
** Prevents the page from reloading on button
*/
function handleNone(event) {
    event.preventDefault();
    var data = JSON.stringify({
        type: "none",
    });
    var xhrRequest = postData(data, "/wifi");
    xhrRequest.then(function () {
        document.getElementById("confirmation-prompt").style.visibility = "visible";
    }, response => function () {
        alert("The server replied " + String(response.statusText) + ".\n Please try again.")
    })
};

function handleWPA2(event) {
    event.preventDefault();
    var WPA2Pass = document.getElementById("WPA2-password").value;
    var SSID = document.getElementById("wifi-selection").value;
    var data = JSON.stringify({
        type: "WPA2",
        SSID: SSID,
        pass: WPA2Pass
    });
    var xhrRequest = postData(data, "/wifi");
    xhrRequest.then(function () {
        document.getElementById("confirmation-prompt").style.visibility = "visible";
    }, response => function () {
        alert("The server replied " + String(response.statusText) + ".\n Please try again.")
    })
};

function handleWPA2E(event) {
    event.preventDefault();
    var WPA2EUser = document.getElementById("WPA2-E-username").value;
    var WPA2EPass = document.getElementById("WPA2-E-password").value;
    var SSID = document.getElementById("wifi-selection").value;
    var data = JSON.stringify({
        type: "WPA2E",
        SSID: SSID,
        user: WPA2EUser,
        pass: WPA2EPass
    });
    var xhrRequest = postData(data, "/wifi");
    xhrRequest.then(function () {
        document.getElementById("confirmation-prompt").style.visibility = "visible";
    }, response => function () {
        alert("The server replied " + String(response.statusText) + ".\n Please try again.")
    })
};

function handleUnsec(event) {
    event.preventDefault();
    var SSID = document.getElementById("wifi-selection").value;
    var data = JSON.stringify({
        type: "unsecured",
        SSID: SSID
    });
    var xhrRequest = postData(data, "/wifi");
    xhrRequest.then(function () {
        document.getElementById("confirmation-prompt").style.visibility = "visible";
    }, response => function () {
        alert("The server replied " + String(response.statusText) + ".\n Please try again.")
    })
};

async function handleCreds(event) {
    event.preventDefault();
    var name = document.getElementById("name").value;
    var password = document.getElementById("password").value;
    var data = JSON.stringify({
        name: name,
        pass: password
    });
    var xhrRequest = postData(data, "/credentials");
    xhrRequest.then(function () {
        alert("Data recieved properly, please wait while the device restarts and switch to your desired method of connection, then try to connect. (~15-30 secs)")
    }, response => function () {
        if (response.status === 412) {
            alert("Precondition failed, please choose and confirm a wifi option.")
        }
        alert("The server replied " + String(response.statusText) + ".\n Please try again.")
    })
};

form0.addEventListener('submit', handleNone);
form1.addEventListener('submit', handleWPA2);
form2.addEventListener('submit', handleWPA2E);
form3.addEventListener('submit', handleUnsec);
form4.addEventListener('submit', handleCreds);
