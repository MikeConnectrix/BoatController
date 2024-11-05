function sendValue(key, value) {
    var data = key + "," + value;
    ws.send(data);
}

function DelayStartWebSocket() {
    SetBoatName();
    setTimeout(WebSocketBegin, 1000);
}

function SetBoatName() {
    varBoatName = ""
    document.title = "ESP32 Boat Controller - " + varBoatName;
    document.getElementById('BoatName').innerHTML = varBoatName;
}

function SendJoystickData(joyX, joyY) {
    if (LastSendX != joyX) {
        sendValue("Speed", joyX);
        LastSendX = joyX
    }
    if (LastSendY != joyY) {
        sendValue("Bearing", joyY);
        LastSendY = joyY
    }
}
function WebSocketBegin() {

    if ("WebSocket" in window) {
        // Let us open a web socket
        ws = new WebSocket("ws://" + location.hostname + "/HTTPInput");

        ws.onopen = function () {
            // Web Socket is connected
            Joy.setFillColor("#00AA00");
        };

        ws.onmessage = function (evt) {
            var myObj = JSON.parse(event.data);
            var keys = Object.keys(myObj);

            for (var i = 0; i < keys.length; i++) {
                var key = keys[i];
                if (keys[i] == "bearing") {
                    const compassDisc = document.getElementById('compassDiscImg');
                    compassDisc.style.transform = `rotate(${myObj[key]}deg)`;
                    compassDisc.style.webkitTransform = `rotate(${myObj[key]}deg)`;
                    compassDisc.style.MozTransform = `rotate(${myObj[key]}deg)`;
                }

            }

        };

        ws.onclose = function () {
            // websocket is closed.
            Joy.setFillColor("#AA0000");
        };
    } else {
        // The browser doesn't support WebSocket
        alert("WebSocket NOT supported by your Browser!");
    }
}
function SetIndexFormDetails() {
    document.getElementById("defaultOpen").click();
}

function SetFormDetails() {
    document.getElementById("defaultOpen").click();
}

function openSection(evt, sectionName, GetData) {
    // Get all elements with class="tablinks" and remove the class "active"
    tablinks = document.getElementsByClassName("tablinks");
    for (i = 0; i < tablinks.length; i++) {
        tablinks[i].className = tablinks[i].className.replace(" active", "");
    }

    // Show the current tab, and add an "active" class to the button that opened the tab
    document.getElementById(sectionName).style.display = "block";
    evt.currentTarget.className += " active";

}
