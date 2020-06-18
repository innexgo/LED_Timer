window.onload = function () {
    var divOne = document.getElementById("timer");
    var divTwo = document.getElementById("timer-change");

    var divOneWidth = divOne.offsetWidth + "px";
    var divTwoWidth = divTwo.offsetWidth + "px";

    if (divTwoWidth > divOneWidth) {
        divOne.style.width = divTwoWidth;
    }
    else {
        divOne.style.width = divOneWidth;
    }
}

if ((Math.floor((sessionStorage.getItem("epoch_time") - Date.now()) / 1000)) > 0) {
    timer_enabled = true;
}
else {
    timer_enabled = false;
}

function setCountdown() {
    var hours = document.getElementById("timer-time-hrs").value;
    var minutes = document.getElementById("timer-time-min").value;
    var seconds = document.getElementById("timer-time-sec").value;
    var time = ((parseInt(hours) * 3600) + (parseInt(minutes) * 60) + parseInt(seconds));

    sessionStorage.setItem("time", time);
    end_time_epoch = String(Date.now() + time * 1000);
    sessionStorage.setItem("epoch_time", end_time_epoch);
    timer_enabled = true;

    sendToTimer();

    countdown();
}

function countdown() {
    if (timer_enabled) {
        var end_time_epoch = sessionStorage.getItem("epoch_time");
        var time = Math.floor((end_time_epoch - Date.now()) / 1000)

        if (isNaN(time)) {
            time = parseInt(sessionStorage.getItem("time"));
        }
        if (isNaN(time)) {
            alert("An error occured: time variable is not valid");
            expected = Date.now() // Prevents the clock from thinking it drifted off.
            return null;
        }

        if (time <= 0) {
            timer_enabled = false;
            alert("Time's up");
            expected = Date.now() // Prevents the clock from thinking it drifted off.
            return null;
        };

        document.getElementById('timeleft').innerText = formatTime(time);
        time--;
        sessionStorage.setItem("time", time);
        validateTimeDiff();

    }
    else {
        return null;
    }
}

function formatTime(time) {
    var hours = Math.floor(time / 3600);
    var minutes = Math.floor((time - hours * 3600) / 60);
    var seconds = time - ((hours * 3600) + (minutes * 60));

    var display = hours;

    if (String(minutes).length == 1) {
        display += ":0" + String(minutes);
    }
    else {
        display += ":" + String(minutes);
    };

    if (String(seconds).length == 1) {
        display += ":0" + String(seconds);
    }
    else {
        display += ":" + String(seconds);
    };

    return display;
}

function validateTime() {
    var hours = document.getElementById("timer-time-hrs").value;
    var minutes = document.getElementById("timer-time-min").value;
    var seconds = document.getElementById("timer-time-sec").value;

    if ((hours > 0) || (minutes > 0) || (seconds > 4)) {
        return null;
    }

    else {
        document.getElementById("timer-time-sec").value = 5;
        alert("Time can't be less than 5")
        expected = Date.now() // Prevents the clock from thinking it drifted off.
    }
};

function validateTimeDiff() {
    var end_time_epoch = sessionStorage.getItem("epoch_time");
    var time = Math.floor((end_time_epoch - Date.now()) / 1000)
    var rhours = Math.floor(time / 3600);
    var rminutes = Math.floor((time - (rhours * 3600)) / 60);
    var rseconds = time - ((rhours * 3600) + (rminutes * 60));
    var dhours = document.getElementById("timer-time-hrs-change").value;
    var dminutes = document.getElementById("timer-time-min-change").value;
    var dseconds = document.getElementById("timer-time-sec-change").value;

    if (((rhours + dhours) >= 0) && ((rminutes + dminutes) >= 0) && ((rseconds + dseconds) > 9)) {
        return true;
    }
    else {
        document.getElementById("timer-time-hrs-change").value = -rhours;
        document.getElementById("timer-time-min-change").value = -rminutes;
        document.getElementById("timer-time-sec-change").value = Math.max(0, -(rseconds - 15));
        expected = Date.now()  // Prevents the clock from thinking it drifted off.
        return false;
    }
};

/*
** Prevents the page from reloading on button
*/
function handleTimer(event) {
    event.preventDefault();
    setCountdown();
};

var timer = document.getElementById("timer");
timer.addEventListener('submit', handleTimer);

function sendToTimer() {
    var cur_time = String(Date.now())
    var epoch_end_time = String(end_time_epoch);
    var seconds = String(sessionStorage.getItem("time"));
    var password = String(sessionStorage.getItem("password"));
    var verification = String(document.getElementById("verification").innerText);
    var tohash = cur_time + epoch_end_time + seconds + verification + password;
    var hash = sjcl.hash.sha256.hash(tohash);
    var hashBits = sjcl.codec.hex.fromBits(hash);
    var data = {
        "verification": verification,
        "time": cur_time,
        "end": epoch_end_time,
        "duration": seconds,
        "hash": hashBits
    };
    var xhrRequest = postData(data, "/timer");
    xhrRequest.then(
        function () { },
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


interval = 1000;
expected = Date.now() + interval;
clock = setTimeout(step, interval);


function step() {
    var dt = Date.now() - expected; // the drift (positive for overshooting)
    // if (dt > interval) { // I don't really think this needs to be handled. Just speed through it and ignore.
    // }

    countdown();

    expected += interval;
    clock = setTimeout(step, Math.max(0, interval - dt)); // take into account drift
};

/*
** Prevents the page from reloading on button
*/

function handleAddTime(event) {
    event.preventDefault();
    addTime();
};

var timerChange = document.getElementById("timer-change");
timerChange.addEventListener('submit', handleAddTime);

function addTime() {
    var dhours = document.getElementById("timer-time-hrs-change").value;
    var dminutes = document.getElementById("timer-time-min-change").value;
    var dseconds = document.getElementById("timer-time-sec-change").value;

    var dtime = ((parseInt(dhours) * 3600) + (parseInt(dminutes) * 60) + parseInt(dseconds));

    var epoch_end_time = parseInt(sessionStorage.getItem("epoch_time"));
    var mod_epoch_end_time = epoch_end_time + dtime * 1000;
    sessionStorage.setItem("epoch_time", mod_epoch_end_time);
    mod_epoch_end_time = String(mod_epoch_end_time);

    var seconds = String(sessionStorage.getItem("time"));
    var mod_seconds = seconds + dtime;
    sessionStorage.setItem("time", mod_seconds);
    mod_seconds = String(mod_seconds);

    var subtle = String(document.getElementById("subtle-change").checked)
    var cur_time = String(Date.now());
    var verification = String(document.getElementById("verification").innerText);
    var password = String(sessionStorage.getItem("password"));
    var tohash = cur_time + mod_epoch_end_time + mod_seconds + subtle + verification + password;
    var hash = sjcl.hash.sha256.hash(tohash);
    var hashBits = sjcl.codec.hex.fromBits(hash);
    var data = {
        "verification": verification,
        "time": cur_time,
        "end": mod_epoch_end_time,
        "duration": mod_seconds,
        "subtle": subtle,
        "hash": hashBits
    };
    var xhrRequest = postData(data, "/timer");
    xhrRequest.then(
        function () { },
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
