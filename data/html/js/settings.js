"use strict"

window.onload = function () {
    var formOne = document.getElementById("warn-time-set");
    var formTwo = document.getElementById("idle-set");

    var formOneWidth = formOne.offsetWidth;
    var formTwoWidth = formTwo.offsetWidth;

    if (formTwoWidth > formOneWidth) {
        formOne.style.width = (formTwoWidth + "px");
        document.getElementById("op-colors-set").style.width = (formTwoWidth + "px");
    }
    else {
        formTwo.style.width = (formOneWidth + "px");
        document.getElementById("op-colors-set").style.width = (formOneWidth + "px");
    }

    document.getElementById("confirm-op-colors").style.width = ((parseInt(document.getElementById("confirm-warn-time").offsetWidth) - 2.666) + "px");
}

function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

function removeWarningConfirm() {
    document.getElementById("confirm-warn-time").style.visibility = "hidden";
}

function removeIdleConfirm() {
    document.getElementById("confirm-idle").style.visibility = "hidden";
}

function removeOpColorsConfirm() {
    document.getElementById("confirm-op-colors").style.visibility = "hidden";
}

/*
** Prevents the page from reloading on button
*/
async function handleWarningButton(event) {
    event.preventDefault();
    var cur_time = String(Date.now());
    var warn_hrs = parseInt(document.getElementById("warn-time-hrs").value);
    var warn_min = parseInt(document.getElementById("warn-time-min").value);
    var warn_sec = parseInt(document.getElementById("warn-time-sec").value);

    var warn_time = ((warn_hrs*3600)+(warn_min*60)+warn_sec);

    var enabled = String(document.getElementById("warn-enable").checked);
    var password = String(sessionStorage.getItem("password"));
    var verification = String(document.getElementById("verification").innerText);
    var tohash = cur_time + String(warn_time) + enabled + verification + password;
    var hash = sjcl.hash.sha256.hash(tohash);
    var hashBits = sjcl.codec.hex.fromBits(hash);
    var data = {
        "time": cur_time,
        "warn-time": warn_time,
        "enabled": enabled,
        "hash": hashBits.toUpperCase()
    };
    var xhrRequest = postData(data, "/warn");
    xhrRequest.then(
        async function () {
            document.getElementById("confirm-warn-time").style.visibility = "visible";
        },
        (response) => {
            if (response.status === 401) {
                alert("Invalid password.")
                window.location.replace("/login.html");
            }
            else {
                alert("The server replied " + response.status + ". \n Please try again.");
            }
        })
};

var warn = document.getElementById("warn-time-set");
warn.addEventListener('submit', handleWarningButton);

/*
** Prevents the page from reloading on button
*/
async function handleIdleButton(event) {
    event.preventDefault();
    var cur_time = String(Date.now());
    var idle_color = String(document.getElementById("idle-color").value);
    var enabled = String(document.getElementById("idle-enable").checked);
    var password = String(sessionStorage.getItem("password"));
    var verification = String(document.getElementById("verification").innerText);
    var tohash = cur_time + idle_color + enabled + verification + password;
    var hash = sjcl.hash.sha256.hash(tohash);
    var hashBits = sjcl.codec.hex.fromBits(hash);
    var data = {
        "time": cur_time,
        "idle-color": idle_color,
        "enabled": enabled,
        "hash": hashBits.toUpperCase()
    };
    var xhrRequest = postData(data, "/idle");
    xhrRequest.then(
        async function () {
            document.getElementById("confirm-idle").style.visibility = "visible";
        },
        (response) => {
            if (response.status === 401) {
                alert("Invalid password.")
                window.location.replace("/login.html");
            }
            else {
                alert("The server replied " + response.status + ". \n Please try again.");
            }
        })
};

var idle = document.getElementById("idle-set");
idle.addEventListener('submit', handleIdleButton);

function resetNormColor() {
   document.getElementById("op-colors-norm").value = "33bb22";
}

function resetWarnColor() {
    document.getElementById("op-colors-warn").value = "dddd11";
}

function resetStopColor() {
    document.getElementById("op-colors-stop").value = "880000";
}

/*
** Prevents the page from reloading on button
*/
async function handleOpColorsButton(event) {
    event.preventDefault();
    var cur_time = String(Date.now());
    var norm_color = String(document.getElementById("op-colors-norm").value);
    var warn_color = String(document.getElementById("op-colors-warn").value);
    var stop_color = String(document.getElementById("op-colors-stop").value);
    var password = String(sessionStorage.getItem("password"));
    var verification = String(document.getElementById("verification").innerText);
    var tohash = cur_time + norm_color + warn_color + stop_color + verification + password;
    var hash = sjcl.hash.sha256.hash(tohash);
    var hashBits = sjcl.codec.hex.fromBits(hash);
    var data = {
        "time": cur_time,
        "norm-color": norm_color,
        "warn-color": warn_color,
        "stop-color": stop_color,
        "hash": hashBits.toUpperCase()
    };
    var xhrRequest = postData(data, "/opcolors");
    xhrRequest.then(
        async function () {
            document.getElementById("confirm-op-colors").style.visibility = "visible";
        },
        (response) => {
            if (response.status === 401) {
                alert("Invalid password.")
                window.location.replace("/login.html");
            }
            else {
                alert("The server replied " + response.status + ". \n Please try again.");
            }
        })
};

var op_colors_set = document.getElementById("op-colors-set");
op_colors_set.addEventListener('submit', handleOpColorsButton);

var setHostname = getData("/hostname");
setHostname.then(
    (response) => {
        document.getElementById("hostname-display").innerText = response.responseText;
    },
    function () {
        errorAlert(response);
    }
)

var getSettings = getData("/getsettings");
getSettings.then(
    (response) => {
        var settings = response.responseText.split(" ");
        if (settings[0] == "true") {
            document.getElementById("idle-enable").checked = true;
        }
        document.getElementById("idle-color").value = settings[1].padStart(6, "0");
        if (settings[2] == "true") {
            document.getElementById("warn-enable").checked = true;
        }
        var time = parseInt(settings[3]);
        var hours = Math.floor(( time/3600));
        var minutes = Math.floor(((time-(hours*3600))/60));
        var seconds = Math.floor((time-((hours*3600)+(minutes*60))));
        
        document.getElementById("warn-time-hrs").value = hours;
        document.getElementById("warn-time-min").value = minutes;
        document.getElementById("warn-time-sec").value = seconds;

        document.getElementById("op-colors-norm").value = settings[4].padStart(6, "0");
        document.getElementById("op-colors-warn").value = settings[5].padStart(6, "0");
        document.getElementById("op-colors-stop").value = settings[6].padStart(6, "0");
    },
    function () {
        errorAlert(response);
    }
)