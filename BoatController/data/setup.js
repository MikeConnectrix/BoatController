var STypeLoaded = false;
var servosLoaded = false;
var controllersLoaded = false;
var controllerData;
var STypeData;
function GetParamsData(sectionName) {
    $.ajax({
        type: 'GET',
        url: 'ConfigSection?Section=' + sectionName,
        dataType: 'json',
        contentType: 'application/json;charset=utf-8',
        success: function (data) {
            switch (sectionName) {
                case "Params":
                case "Comp": {
                    loadParams(data);
                    break;
                }
                case "Servo": {
                    if (!servosLoaded) loadServos(data);
                    servosLoaded = true;
                    break;
                }
                case "SType": {
                    if (!STypeLoaded) loadSType(data);
                    STypeLoaded = true;
                    break;
                }
                case "Cont": {
                    if (!controllersLoaded) loadControllers(data);
                    controllersLoaded = true;
                    break;
                }
            }            
        },
        failure: function (error) {
            alert(error.d);
        }
    });
}

function resetCompass() {
    document.getElementById("Offset").value = 0;
    document.getElementById("MMinX").value = 0;
    document.getElementById("MMaxX").value = 0;
    document.getElementById("MMinY").value = 0;
    document.getElementById("MMaxY").value = 0;
    document.getElementById("MMinZ").value = 0;
    document.getElementById("MMaxZ").value = 0;
}
function loadParams(params) {
    for (var key in params) {
        
        switch (key) {            
            case "STAMode":
                document.getElementById("STAMode1").checked = true;
                if (!(params[key] == "true"))
                    document.getElementById("STAMode2").checked = true;
                break;
            case "AutoUpdate":
                document.getElementById("AutoUpdate").checked = true;
                if (!(params[key] == "true"))
                    document.getElementById("AutoUpdate").checked = true;
                break;
            default:
                var element = document.getElementById(key)
                if (element != null) element.value = params[key];
        }        
    }
}

function loadControllers(params) {
    controllerData = params;
    for (var key in params) {
        controllerDetails = params[key];
        $("#controllerTable").append(GetControllerLine(controllerDetails));
    }
}

function GetControllerLine(controllerDetails) {
    rowDetails = "<tr>"
    rowDetails += "<td><input type='text' id='dscn' class='txtbox' value='" + (controllerDetails["dscn"] || 'New Controller') + "' style='width: 100%'/></td>";
    rowDetails += "<td><input type='number' id='I2C' value='" + (controllerDetails["I2C"] || '') + "' style='width: 100%'></td>";
    rowDetails += "<td><input type='number' id='ID' class='txtbox' value='" + controllerDetails["ID"] + "' readonly style='width: 100%'/></td>";
    rowDetails += "<td><input type='button' value='Delete Controller' onclick='SomeDeleteRowFunction(this)'/></td>"
    rowDetails += "</tr>"
    return rowDetails;
}


function loadServos(params) {    
    for (var key in params) {
        servoDetails = params[key];        
        $("#servoTable").append(GetServoLine(servoDetails));
    }    
}

function loadSType(params) {
    STypeData = params;
    for (var key in params) {
        STypeDetails = params[key];
        $("#STypeTable").append(GetSTypeLine(STypeDetails));
    }
}


function GetServoLine(servoDetails) {
    var controllerTypeOption = "" //"<option value='0'>Built In</option><option value='1'>Rear Controller</option>";

    for (var key in controllerData) {
        contDetails = controllerData[key];
        controllerTypeOption += "<option value='" + key + "'>" + contDetails["dscn"] + "</option>";        
    }    

    controllerTypeOption = controllerTypeOption.replace("value='" + servoDetails["Ctrl"] + "'", "value='" + servoDetails["Ctrl"] + "' selected");

    var servoTypeOption = "" //"<option value='0'>Built In</option><option value='1'>Rear Controller</option>";

    for (var key in STypeData) {
        STypeDetails = STypeData[key];
        servoTypeOption += "<option value='" + key + "'>" + STypeDetails["dscn"] + "</option>";
    }  

    servoTypeOption = servoTypeOption.replace("value='" + servoDetails["type"] + "'", "value='" + servoDetails["type"] + "' selected");

    rowDetails = "<tr>"
    rowDetails += "<td><input type='text' id='dscn' class='txtbox' value='" + (servoDetails["dscn"] || 'New Servo')  + "' style='width: 100%'/></td>";
    rowDetails += "<td class='selBox'><select id='Ctrl' style='width: 100%'>" + controllerTypeOption + "</select></td>";
    rowDetails += "<td><input type='number' id='Prt' class='txtbox' value='" + (servoDetails["Prt"] || '0') + "' style='width: 100%'/></td>";
    rowDetails += "<td class='selBox'><select id='type' style='width: 100%'>" + servoTypeOption + "</select></td>";
    rowDetails += "<td><input type='number' id='spd' class='txtbox' value='" + servoDetails["spd"] + "' style='width: 100%'/></td>";
    rowDetails += "<td><input type='number' id='ID' class='txtbox' value='" + servoDetails["ID"] + "' readonly style='width: 100%'/></td>";
    rowDetails += "<td><input type='button' value='Delete Servo' onclick='SomeDeleteRowFunction(this)'/></td>"
    rowDetails += "</tr>"
    return rowDetails;
}

function GetSTypeLine(servoDetails) {
    var controllerTypeOption = "" //"<option value='0'>Built In</option><option value='1'>Rear Controller</option>";

    for (var key in controllerData) {
        contDetails = controllerData[key];
        controllerTypeOption += "<option value='" + key + "'>" + contDetails["dscn"] + "</option>";
    }

    //var controllerTypeOption = "<option value='0'>Built In</option><option value='1'>Rear Controller</option>"; 
    controllerTypeOption = controllerTypeOption.replace("value='" + servoDetails["Ctrl"] + "'", "value='" + servoDetails["Ctrl"] + "' selected");

    var servoTypeOption = "<option value='1'>Speed Controller</option><option value='2'>Rudder control</option><option value='3'>Range Finder</option><option value='4'>Turret Rotation</option><option value='5'>Turret Elevation</option>";
    servoTypeOption = servoTypeOption.replace("value='" + servoDetails["type"] + "'", "value='" + servoDetails["type"] + "' selected");

    rowDetails = "<tr>"
    rowDetails += "<td><input type='text' id='dscn' class='txtbox' value='" + (servoDetails["dscn"] || 'New Servo') + "' style='width: 100%'/></td>";
    rowDetails += "<td><input type='number' id='min' class='txtbox' value='" + (servoDetails["min"] || '0') + "' style='width: 100%'/></td>";
    rowDetails += "<td><input type='number' id='max' class='txtbox' value='" + (servoDetails["max"] || '180') + "' style='width: 100%'/></td>";
    rowDetails += "<td><input type='number' id='spd' class='txtbox' value='" + (servoDetails["spd"] || '5') + "' style='width: 100%'/></td>";
    rowDetails += "<td><input type='number' id='homPos' class='txtbox' value='" + (servoDetails["homPos"] || '90') + "' style='width: 100%'/></td>";
    rowDetails += "<td><input type='number' id='batPos' class='txtbox' value='" + (servoDetails["batPos"] || '90') + "' style='width: 100%'/></td>";
    rowDetails += "<td><input type='number' id='ID' class='txtbox' value='" + servoDetails["ID"] + "' readonly style='width: 100%'/></td>";
    rowDetails += "<td><input type='button' value='Delete Servo' onclick='SomeDeleteRowFunction(this)'/></td>"
    rowDetails += "</tr>"
    return rowDetails;
}
function addServo() {
    $("#servoTable").append(GetServoLine([]));
}

function addSType() {
    $("#STypeTable").append(GetSTypeLine([]));
}

function addController() {
    $("#controllerTable").append(GetControllerLine([]));
}
function saveParameters(elementid, section) {
    var form = document.getElementById(elementid);
    var formData = new FormData(form);
    var postData;
    var submitData = true;
    switch (section) {
        case "Cont": postData = getTableData("controllerTable");
            break;
        case "SType": postData = getTableData("STypeTable");
            break;
        case "Servo": postData = getTableData("servoTable");
            break;
        default: postData=JSON.stringify(Object.fromEntries(formData));
    }

    if (submitData)
        $.ajax({
            type: "POST",
            url: "SaveConfigSection?Section=" + section,
            data: postData,
            success: function () {
                alert(section + " saved!");
            },
            failure: function (error) {
                alert(error.d);
            },
            dataType: "json",
            contentType: "application/json"
        });
    else
        alert(postData);
}

function getTableData(elementid) {
    const table = document.getElementById(elementid);
    const rows = table.rows;
    const headers = [];
    const jsonData = [];

    // Extract headers
    for (let i = 0; i < rows[0].cells.length; i++) {
        headers.push(rows[0].cells[i].id);
    }

    var ItemID = 0;
    // Extract data
    for (let i = 1; i < rows.length; i++) {
        const rowObject = {};
        const cells = rows[i].cells;
        for (let j = 0; j < cells.length; j++) {
            if (headers[j] != "") {
                if (headers[j] == "ID") {
                    rowObject[headers[j]] = String(ItemID);
                    ItemID++;

                }
                else 
                    rowObject[headers[j]] = cells[j].firstChild.value;
                
            }          
        }
        jsonData.push(rowObject);
    }

    return JSON.stringify(jsonData);
}

function SomeDeleteRowFunction(o) {
    var p = o.parentNode.parentNode;
    p.parentNode.removeChild(p);
}
function SetIndexFormDetails() {
    document.getElementById("defaultOpen").click();
}

function SetFormDetails() {    
    $('#parametersContent').load('parameters.html');
    $('#compassContent').load('compass.html');
    $('#controllersContent').load('controllers.html');
    $('#STypeContent').load('SType.html');
    $('#servosContent').load('servos.html');
    $('#uploadContent').load('upload.html');
    $('#updateContent').load('update.html');
    GetParamsData("Cont");
    document.getElementById("defaultOpen").click();      
}

function repeatController(targetForm,targetTable) {
    var clonedForm = $("." + targetForm + ":first").clone();
    clonedForm.appendTo("#" + targetTable);
}

function openSection(evt, sectionName,GetData) {
    // Declare all variables
    var i, tabcontent, tablinks;

    // Get all elements with class="tabcontent" and hide them
    tabcontent = document.getElementsByClassName("tabcontent");
    for (i = 0; i < tabcontent.length; i++) {
        tabcontent[i].style.display = "none";
    }

    // Get all elements with class="tablinks" and remove the class "active"
    tablinks = document.getElementsByClassName("tablinks");
    for (i = 0; i < tablinks.length; i++) {
        tablinks[i].className = tablinks[i].className.replace(" active", "");
    }

    // Show the current tab, and add an "active" class to the button that opened the tab
    document.getElementById(sectionName).style.display = "block";
    evt.currentTarget.className += " active";

    if (GetData) GetParamsData(sectionName);
}
function saveChanges(evt, sectionName) {
    // Declare all variables
    alert("Changes updated!");
}

function AutoUpdateChange() {
    var chkBox = document.getElementById("AutoUpdate");
    if (chkBox.checked)
        chkBox.attributes("Value") = "True";
    else
        chkBox.attributes("Value") = "False";
}
