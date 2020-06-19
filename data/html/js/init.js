window.onload = function () {
    var divOne = document.getElementById("wifi-select-div");
    var divTwo = document.getElementById("credentials-div");

    var divOneWidth = divOne.offsetWidth;
    var divTwoWidth = divTwo.offsetWidth;

    if (divTwoWidth > divOneWidth) {
        divOne.style.width = divTwoWidth + "px";
    }
    else {
        divOne.style.width = divOneWidth + "px";
    }
}

function wifi_type() {
    document.getElementById("none").className = "";
    document.getElementById("WPA2").className = "";
    document.getElementById("WPA2-E").className = "";
    document.getElementById("unsecured").className = "";

    var wifi_selector = document.getElementById("wifi-selection");
    var wifi_type = wifi_selector.value;
    if (wifi_type == "auto") {
        var second_select = document.getElementById("wifi-selection-two");
        second_select.style.display = "";
        document.getElementById("wifi-selection-two-label").style.display = "";
        wifi_type = second_select.value;
    }
    else {
        document.getElementById("wifi-selection-two").style.display = "none";
        document.getElementById("wifi-selection-two-label").style.display = "none";
    }
    var selected_element = document.getElementById(wifi_type);

    selected_element.className = "active";

    document.getElementById("confirmation-prompt").style.visibility = "hidden";

    if (wifi_selector.options[wifi_selector.selectedIndex].text == "Hidden Network") {
        document.getElementById("hidden-non-guarantee").style.display = "inline";
    }
    else {
        document.getElementById("hidden-non-guarantee").style.display = "none";
    }
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
    var data = {
        type: "none",
    };
    var xhrRequest = postData(data, "/wifi");
    xhrRequest.then(function () {
        document.getElementById("confirmation-prompt").style.visibility = "visible";
    }, response => function () {
        errorAlert(response);
    })
};

function handleWPA2(event) {
    event.preventDefault();
    var WPA2Pass = document.getElementById("WPA2-password").value;
    var wifi_selector = document.getElementById("wifi-selection");
    var SSID = wifi_selector.options[wifi_selector.selectedIndex].text;
    var data = {
        "type": "WPA2",
        "SSID": SSID,
        "pass": WPA2Pass
    };
    var xhrRequest = postData(data, "/wifi");
    xhrRequest.then(function () {
        document.getElementById("confirmation-prompt").style.visibility = "visible";
    }, response => function () {
        errorAlert(response);
    })
};

function handleWPA2E(event) {
    event.preventDefault();
    var WPA2EUser = document.getElementById("WPA2-E-username").value;
    var WPA2EPass = document.getElementById("WPA2-E-password").value;
    var wifi_selector = document.getElementById("wifi-selection");
    var SSID = wifi_selector.options[wifi_selector.selectedIndex].text;
    var data = {
        "type": "WPA2E",
        "SSID": SSID,
        "user": WPA2EUser,
        "pass": WPA2EPass
    };
    var xhrRequest = postData(data, "/wifi");
    xhrRequest.then(function () {
        document.getElementById("confirmation-prompt").style.visibility = "visible";
    }, response => function () {
        errorAlert(response);
    })
};

function handleUnsec(event) {
    event.preventDefault();
    var wifi_selector = document.getElementById("wifi-selection");
    var SSID = wifi_selector.options[wifi_selector.selectedIndex].text;
    var data = {
        "type": "unsecured",
        "SSID": SSID
    };
    var xhrRequest = postData(data, "/wifi");
    xhrRequest.then(function () {
        document.getElementById("confirmation-prompt").style.visibility = "visible";
    }, response => function () {
        errorAlert(response);
    })
};

async function handleCreds(event) {
    event.preventDefault();
    var name = document.getElementById("name").value;
    var password = document.getElementById("password").value;
    var data = {
        "name": name,
        "pass": password
    };
    var xhrRequest = postData(data, "/credentials");
    xhrRequest.then(function () {
        alert("Data recieved properly, please wait while the device restarts and switch to your desired method of connection, then try to connect. (~15-30 secs)");
    }, (response) => {
        if (response.status == 412) {
            alert("Precondition failed, please choose and confirm a wifi option.");
        }
        errorAlert(response);
    })
};

form0.addEventListener('submit', handleNone);
form1.addEventListener('submit', handleWPA2);
form2.addEventListener('submit', handleWPA2E);
form3.addEventListener('submit', handleUnsec);
form4.addEventListener('submit', handleCreds);
