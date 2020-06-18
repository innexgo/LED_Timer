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
    var countdown_min = String(document.getElementById("countdown-time-min"));
    var countdown_sec = String(document.getElementById("countdown-time-sec"));
    var enabled = String(document.getElementById("countdown-enable").checked)
    var password = String(sessionStorage.getItem("password"));
    var verification = String(document.getElementById("verification").innerText);
    var tohash = cur_time + countdown_min + countdown_sec + enabled + verification + password;
    var hash = sjcl.hash.sha256.hash(tohash);
    var hashBits = sjcl.codec.hex.fromBits(hash);
    var data = {
        "verification": verification,
        "time": cur_time,
        "countdown_min": countdown_min,
        "countdown_sec": countdown_sec,
        "enabled": enabled,
        "hash": hashBits
    };
    var xhrRequest = postData(data, "/countdown");
    xhrRequest.then(
        async function () {
            document.getElementById("confirm-countdown").style.visibility = "visible";
            await sleep(1500);
            document.getElementById("confirm-countdown").style.visibility = "hidden";
        },
        response => function () {
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
    var warn_hrs = String(document.getElementById("warn-time-hrs"));
    var warn_min = String(document.getElementById("warn-time-min"));
    var warn_sec = String(document.getElementById("warn-time-sec"));
    var enabled = String(document.getElementById("warn-enable").checked);
    var password = String(sessionStorage.getItem("password"));
    var verification = String(document.getElementById("verification").innerText);
    var tohash = cur_time + warn_hrs + warn_min + warn_sec + enabled + verification + password;
    var hash = sjcl.hash.sha256.hash(tohash);
    var hashBits = sjcl.codec.hex.fromBits(hash);
    var data = {
        "verification": verification,
        "time": cur_time,
        "warn_hrs": warn_hrs,
        "warn_min": warn_min,
        "warn_sec": warn_sec,
        "enabled": enabled,
        "hash": hashBits
    };
    var xhrRequest = postData(data, "/warn");
    xhrRequest.then(
        async function () {
            document.getElementById("confirm-warn-time").style.visibility = "visible";
            await sleep(1500);
            document.getElementById("confirm-warn-time").style.visibility = "hidden";
        },
        response => function () {
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
    var warn_hrs = String(document.getElementById("idle-color"));
    var enabled = String(document.getElementById("idle-enable").checked);
    var password = String(sessionStorage.getItem("password"));
    var verification = String(document.getElementById("verification").innerText);
    var tohash = cur_time + idle_color + enabled + verification + password;
    var hash = sjcl.hash.sha256.hash(tohash);
    var hashBits = sjcl.codec.hex.fromBits(hash);
    var data = {
        "verification": verification,
        "time": cur_time,
        "warn_hrs": warn_hrs,
        "warn_min": warn_min,
        "warn_sec": warn_sec,
        "enabled": enabled,
        "hash": hashBits
    };
    var xhrRequest = postData(data, "/idle");
    xhrRequest.then(
        async function () {
            document.getElementById("confirm-idle").style.visibility = "visible";
            // await sleep(1500);
            // document.getElementById("confirm-idle").style.visibility = "hidden";
        },
        response => function () {
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