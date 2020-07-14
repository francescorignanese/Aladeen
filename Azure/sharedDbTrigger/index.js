var iothub = require('azure-iothub');
const sql = require('mssql');

module.exports = async function (context, myTimer) {
    var timeStamp = new Date().toISOString();
    /*
    if (myTimer.isPastDue)
    {
        context.log('JavaScript is running late!');
    }
    */
   
    //config Db
const config={
    user: 'aladeen',
    password: 'smartcross100%',
    server: 'semaforo.database.windows.net',
    database:'semaforodb',
    options:{
        encrypt: true
        }
};

    var registry = iothub.Registry.fromConnectionString("HostName=smartcrosshub.azure-devices.net;SharedAccessKeyName=iothubowner;SharedAccessKey=KBj8eIGdQzh+jjgdDVjMZw9wPrvWNx+IMTAenf7DIQs=");
    
    context.log('JavaScript timer trigger function ran!', timeStamp);  
    await doQuery();

    
    async function doQuery(){
        let selectQuery=`SELECT [Id_semaforo] as semafores_couples,Fascia_oraria as time_slot,[Valore_tempo] as value
        FROM Temporizzazione WHERE Id_incrocio=1 GROUP BY Fascia_oraria, Id_semaforo, Id_incrocio , Valore_tempo`
        try{
            let pool=await sql.connect(config);
            var result = await pool.request().query(selectQuery);
            sql.close();
        }
        catch(errore){
            sql.close();
            context.log("Error: ", errore);
        }

        var array= result.recordset; //andrà nella proprietà desired del device Twin --> Id device = "incrocio"+array[i].Id_incrocio
        context.log("SHARED DB: ");
        context.log(array);
        var idDevice= `incrocio1`
        //context.log(typeof idDevice);
        
        registry.getTwin(idDevice, async (err, twin) => {
            if (err) {
                context.log(err.constructor.name + ': ' + err.message);
            }
            else {
                var patch = {
                    properties: {
                        desired: {
                            timing:[]
                        }
                    }
                };

                array.forEach(element => {
                    patch.properties.desired.timing.push(element);
                });
                twin.update(patch, function (err) {
                    if (err) {
                        console.log('Could not update twin: ' + err.constructor.name + ': ' + err.message);
                    }
                    else {
                        console.log(twin.deviceId + ' twin updated successfully');
                    }
                });

            }
        });
        
    
        

    } 
};