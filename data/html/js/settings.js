window.onload = function () {
    var formOne = document.getElementById("countdown-set");
    var formTwo = document.getElementById("warn-time-set");
    var formThree = document.getElementById("idle-set");

    var formOneWidth = formOne.offsetWidth + "px";
    var formTwoWidth = formTwo.offsetWidth + "px";

    if (formTwoWidth > formOneWidth) {
        formOne.style.width = formTwoWidth;
        formThree.style.width = formTwoWidth;
    }
    else {
        formOne.style.width = formOneWidth;
        formThree.style.width = formOneWidth;
    }
}

function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

function removeCountdownConfirm() {
    document.getElementById("confirm-countdown").style.visibility = "hidden";
}

function removeWarningConfirm() {
    document.getElementById("confirm-warn-time").style.visibility = "hidden";
}

function removeIdleConfirm() {
    document.getElementById("confirm-idle").style.visibility = "hidden";
}

/*
** Prevents the page from reloading on button
*/
async function handleCountdownButton(event) {
    event.preventDefault();
    var cur_time = String(Date.now());
    var countdown_min = parseInt(document.getElementById("countdown-time-min"));
    var countdown_sec = parseInt(document.getElementById("countdown-time-sec"));

    countdown_time = ((countdown_min*60)+countdown_sec);

    var enabled = String(document.getElementById("countdown-enable").checked)
    var password = String(sessionStorage.getItem("password"));
    var verification = String(document.getElementById("verification").innerText);
    var tohash = cur_time + String(countdown_time) + enabled + verification + password;
    var hash = sjcl.hash.sha256.hash(tohash);
    var hashBits = sjcl.codec.hex.fromBits(hash);
    var data = {
        "time": cur_time,
        "countdown_time": countdown_time,
        "enabled": enabled,
        "hash": hashBits.toUpperCase()
    };
    var xhrRequest = postData(data, "/countdown");
    xhrRequest.then(
        async function () {
            document.getElementById("confirm-countdown").style.visibility = "visible";
            await sleep(1500);
            document.getElementById("confirm-countdown").style.visibility = "hidden";
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

var countdown = document.getElementById("countdown-set");
countdown.addEventListener('submit', handleCountdownButton);


/*
** Prevents the page from reloading on button
*/
async function handleWarningButton(event) {
    event.preventDefault();
    var cur_time = String(Date.now());
    var warn_hrs = parseInt(document.getElementById("warn-time-hrs"));
    var warn_min = parseInt(document.getElementById("warn-time-min"));
    var warn_sec = parseInt(document.getElementById("warn-time-sec"));

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
    var idle_color = String(document.getElementById("idle-color"));
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
idle.addEventListener('submit', handleidleButton);

var setHostname = getHostname();
setHostname.then(
    (response) => {
        document.getElementById("hostname-display").innerText = response.responseText;
    },
    function () {
        errorAlert(response);
    }
)