window.onload = function () {
    var formOne = document.getElementById("warn-time-set");
    var formTwo = document.getElementById("idle-set");

    var formOneWidth = formOne.offsetWidth;
    var formTwoWidth = formTwo.offsetWidth;

    if (formTwoWidth > formOneWidth) {
        formOne.style.width = (formTwoWidth + "px");
        document.getElementById("warn-color-set").style.width = (formTwoWidth + "px");
        document.getElementById("norm-color-set").style.width = (formTwoWidth + "px");
    }
    else {
        formTwo.style.width = (formOneWidth + "px");
        document.getElementById("warn-color-set").style.width = (formOneWidth + "px");
        document.getElementById("norm-color-set").style.width = (formOneWidth + "px");
    }
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

function removeNormColorConfirm() {
    document.getElementById("confirm-normal-color").style.visibility = "hidden";
}

function removeWarnColorConfirm() {
    document.getElementById("confirm-warn-color").style.visibility = "hidden";
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
            await sleep(1500);
            document.getElementById("confirm-warn-time").style.visibility = "hidden";
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
            // await sleep(1500);
            // document.getElementById("confirm-idle").style.visibility = "hidden";
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

/*
** Prevents the page from reloading on button
*/
async function handleNormColorButton(event) {
    event.preventDefault();
    var cur_time = String(Date.now());
    var norm_color = String(document.getElementById("norm-color").value);
    var password = String(sessionStorage.getItem("password"));
    var verification = String(document.getElementById("verification").innerText);
    var tohash = cur_time + norm_color + verification + password;
    var hash = sjcl.hash.sha256.hash(tohash);
    var hashBits = sjcl.codec.hex.fromBits(hash);
    var data = {
        "time": cur_time,
        "norm-color": norm_color,
        "hash": hashBits.toUpperCase()
    };
    var xhrRequest = postData(data, "/normcolor");
    xhrRequest.then(
        async function () {
            document.getElementById("confirm-norm-color").style.visibility = "visible";
            // await sleep(1500);
            // document.getElementById("confirm-idle").style.visibility = "hidden";
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

var norm_color_set = document.getElementById("norm-color-set");
norm_color_set.addEventListener('submit', handleNormColorButton);

/*
** Prevents the page from reloading on button
*/
async function handleWarnColorButton(event) {
    event.preventDefault();
    var cur_time = String(Date.now());
    var warn_color = String(document.getElementById("warn-color").value);
    var password = String(sessionStorage.getItem("password"));
    var verification = String(document.getElementById("verification").innerText);
    var tohash = cur_time + warn_color + verification + password;
    var hash = sjcl.hash.sha256.hash(tohash);
    var hashBits = sjcl.codec.hex.fromBits(hash);
    var data = {
        "time": cur_time,
        "warn-color": warn_color,
        "hash": hashBits.toUpperCase()
    };
    var xhrRequest = postData(data, "/warncolor");
    xhrRequest.then(
        async function () {
            document.getElementById("confirm-warn-color").style.visibility = "visible";
        },
        (response) => {idle
            if (response.status === 401) {
                alert("Invalid password.")
                window.location.replace("/login.html");
            }
            else {
                alert("The server replied " + response.status + ". \n Please try again.");
            }
        })
};

var warn_color_set = document.getElementById("warn-color-set");
warn_color_set.addEventListener('submit', handleWarnColorButton);

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

        document.getElementById("norm-color").value = settings[4].padStart(6, "0");
        document.getElementById("warn-color").value = settings[5].padStart(6, "0");
    },
    function () {
        errorAlert(response);
    }
)