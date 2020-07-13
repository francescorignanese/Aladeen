﻿


// dati grafico
/*
var ctxL = document.getElementById("lineChart").getContext('2d');
var myLineChart = new Chart(ctxL, {
    type: 'line',
    data: {
        labels: [],
        datasets: [{
            label: "Auto",
            data: [random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250)],
            backgroundColor: [
                'rgb(77,189,116,0.4)',
            ],
            borderColor: [
                'rgba(77,189,116,1)',
            ],
            borderWidth: 2
        },
        {
            label: "Camion",
            data: [random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250)],
            backgroundColor: [
                'rgb(32,168,216,0.4)',
            ],
            borderColor: [
                'rgba(32,168,216,1)',
            ],
            borderWidth: 2
        },
        {
            label: "Moto",
            data: [random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250), random(0, 250)],
            backgroundColor: [
                'rgb(248,108,107,0.4)',
            ],
            borderColor: [
                'rgba(248,108,107,1)',
            ],
            borderWidth: 2
        }
    ]
    },
    options: {
        responsive: true
    }
});
*/

var ctxL = document.getElementById("lineChart").getContext('2d');
var myLineChart = new Chart(ctxL, {
    type: 'line',
    data: {
        labels: [],
        datasets: [{
            label: "Auto",
            data: [messagesList],
            backgroundColor: [
                'rgb(77,189,116,0.4)',
            ],
            borderColor: [
                'rgba(77,189,116,1)',
            ],
            borderWidth: 2
        },
        {
            label: "Camion",
            data: [messagesList],
            backgroundColor: [
                'rgb(32,168,216,0.4)',
            ],
            borderColor: [
                'rgba(32,168,216,1)',
            ],
            borderWidth: 2
        },
        {
            label: "Moto",
            data: [messagesList],
            backgroundColor: [
                'rgb(248,108,107,0.4)',
            ],
            borderColor: [
                'rgba(248,108,107,1)',
            ],
            borderWidth: 2
        }
        ]
    },
    options: {
        responsive: true
    }
});



// funzione random

    function random(min, max) {
        min = Math.ceil(min);
        max = Math.floor(max);
        return Math.floor(Math.random() * (max - min + 1)) + min;  
}

var somma = document.getElementById("Somma");
somma = random + random;
   function somma() {
    if (typeof Array.prototype.sum !== 'function') {

        Array.prototype.sum = function () {

            var total = 0;

            for (var i = 0; i < this.length; i += 1) {

                total += this[i];

            }

            return total;

        };

    }
}


var timerId;
function Aggiornamento() {
    if (timerId == null) {
        timerId = setInterval(random, 1000);
    }
}
Aggiornamento();


// Signalr
var connection = new signalR.HubConnectionBuilder().withUrl("/smartcross").build();


connection.on("ReceiveMessage", function (message) {

    var msg = message.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
    var encodedMsg = " says " + msg;
    var li = document.createElement("li");
    li.textContent = encodedMsg;
    document.getElementById("messagesList").appendChild(li);

    
});
connection.start()
    .then(function () {
        onConnected(connection);
    })
    .catch(function (error) {
        console.error(error.message);
    });



