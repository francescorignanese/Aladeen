document.addEventListener('DOMContentLoaded', function () {
    // Real-time Chart Example written by Simon Brunel (Plunker: https://plnkr.co/edit/Imxwl9OQJuaMepLNy6ly?p=info)
    var samples = 20;
    var speed = 250;
    var values = [];
    var listCar = [];
    var listTruck = [];
    var listMoto = [];
    var labels = [];
    var charts = [];
    var value = 0;

    listCar.length = samples;
    listTruck.length = samples;
    listMoto.length = samples;
    labels.length = samples;
    
    listCar.push(0);
    listTruck.push(0);
    listMoto.push(0);
    labels.fill('');

    var chart = new Chart(document.getElementById("lineChart"),
        {
            type: 'line',
            data: {
                labels: labels,
                datasets: [
                    {
                        data: listCar,
                        label: 'Automobili',
                        backgroundColor: 'rgba(255, 99, 132, 0.1)',
                        borderColor: 'rgb(255, 99, 132)',
                        borderWidth: 2,
                        lineTension: 0.25,
                        pointRadius: 2,
                        spanGaps: false,
                        fill: false
                    },
                    {
                        data: listTruck,
                        label: 'Mezzi Pesanti',
                        backgroundColor: 'rgba(23, 162, 180, 0.1)',
                        borderColor: 'rgb(23, 162, 180)',
                        borderWidth: 2,
                        lineTension: 0.25,
                        pointRadius: 2,
                        spanGaps: false,
                        fill: false
                    },
                    {
                        data: listMoto,
                        label: 'Moto',
                        backgroundColor: 'rgba(40, 167, 69, 0.1)',
                        borderColor: 'rgb(40, 167, 69)',
                        borderWidth: 2,
                        lineTension: 0.25,
                        pointRadius: 2,
                        spanGaps: false,
                        fill: false
                    }
                ]
            },
            options: {
                responsive: true,
                animation: {
                    duration: speed * 1,
                    easing: 'linear'
                },
                scales: {
                    xAxes: [
                        {
                            display: true
                        }
                    ],
                    yAxes: [
                        {
                            ticks: {
                                max: 100,
                                min: 0
                            }
                        }
                    ]
                }
            }
        });

    
    var myVar = setInterval(myTimer, 1000);

    function myTimer() {
        var d = new Date();
        document.getElementById("oraAgg").innerHTML = d.toLocaleTimeString();
    }
    
    //SignalR

    var connection = new signalR.HubConnectionBuilder().withUrl('/telemetry').build();
    
    connection.start().then(function () {
        console.log("connected")
    })
        .catch(function (error) {
            console.error(error.message);
        });

    bindConnectionMessage(connection);

    function bindConnectionMessage(connection) {
        var messageCallback = function (sensor, message) {
            //console.log("qui");
            console.log("sensor: ", sensor);
            console.log("message: ", message);
            var cars = 0, moto = 0, trucks = 0;
            var obj = JSON.parse(message);
            labels.push(obj.time);
            labels.shift();
            if (sensor == 'traffic') {
                obj.data_carriers.forEach(carrier => {
                    //console.log("foreach");
                    carrier.data_vehicles.forEach(vehicles => {
                        if (vehicles.type === 'Automobile') {
                            cars += vehicles.value;
                        }
                        else if (vehicles.type === 'Motociclo') {
                            moto += vehicles.value;
                        }
                        else {
                            trucks += vehicles.value;
                        }
                    })
                })
                console.log("Auto: ", cars);
                console.log("Moto: ", moto);
                console.log("Trucks: ", trucks);

                listCar.push(cars);
                listCar.shift();

                listTruck.push(trucks);
                listTruck.shift();

                listMoto.push(moto);
                listMoto.shift();

                //console.log("truck: ", listTruck);
                //console.log("car: ", listCar);

                chart.update();
            }
            if (sensor == 'climate') {
                console.log("here");
                obj.data_climate.forEach(climate => {
                    if (climate.type === 'temperature') {
                        //console.log("value", climate.value);
                        var temp = document.getElementById("temperature")
                        temp.innerText = climate.value;
                        temp.innerText += '°C';
                          
                    }
                    if (climate.type === 'pressure') {
                        var press = document.getElementById("pressure")
                        press.innerText = climate.value;
                        press.innerText += ' Pa';
                    }
                    if (climate.type === 'humidity') {
                        var hum = document.getElementById("humidity")
                        hum.innerText = climate.value;
                        hum.innerText += ' Us';
                    }
                })
                
            }
            


        };


        connection.on('SensorsData', messageCallback);

    }
});