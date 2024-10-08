var sessionData = sessionStorage;
var config;
function GetControllerData() {
    config = JSON.parse(sessionData.getItem("config"));

    if (!config) {
        $.ajax({
            type: 'GET',
            url: 'config.json',
            dataType: 'json',
            contentType: 'application/json;charset=utf-8',
            success: function (data) {
                config = data;
                sessionData.setItem("config", JSON.stringify(config))
                SetFormDetails();
            },
            failure: function (error) {
                alert(error.d);
            }
        });
    } else SetFormDetails();

}

function SetFormDetails() {
    config = JSON.parse(sessionData.getItem("config"));
    document.getElementById("fBoatName").value=config.boatName;

    var controllers = config.Controllers;
    $.each(controllers, function (index, value) {
        $('#lstObjects').append($('<option>').text(value.description).val(value.ID));
    });
}
$(function () {
    $(".repeat").on('click', function (e) {
        var frm = $('.frm:first').clone();
        frm.find('input').val('');
        $('.frm:last').after(frm);
    });

    $(".save").on('click', function (e) {
        sessionData.setItem("config", JSON.stringify(config))
    });
});
