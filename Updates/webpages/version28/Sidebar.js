function openNav() {
    document.getElementById("sideMenu")
        .style.width = "50%";
    document.getElementById("contentArea")
        .style.marginLeft = "50%";
    document.getElementById("myHamburger")
        .style.width = "0px";
}

function closeNav() {
    document.getElementById("sideMenu")
        .style.width = "0";
    document.getElementById("contentArea")
        .style.marginLeft = "0";
    document.getElementById("myHamburger")
        .style.width = "100px";
}

function showContent(content) {
    document.getElementById("contentTitle")
        .textContent = content + " page";

    closeNav();
}

function openNav() {
    document.getElementById("myNav").style.width = "10%";
    document.getElementById("myHamburger").style.width = "0px";
}

function closeNav() {
    document.getElementById("myNav").style.width = "0%";
    document.getElementById("myHamburger").style.width = "50px";
}