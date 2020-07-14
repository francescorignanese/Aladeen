const sql = require('mssql');

module.exports = async function(context, obj) {
    context.log('JavaScript ServiceBus queue trigger function processed message', obj);

    // config database
    

    var climate=obj.data_climate;
    let query = "INSERT INTO [dbo].[Clima] ([Dispositivo_id],[Temperatura],[Pressione],[Umidita],[Data],[Ora]) VALUES";
    for (let i = 0; i < climate.length; i++) {
        if(climate[i].type=='temperature'){
            var temperature = climate[i].value;
        }
        if(climate[i].type=='pressure'){
            var pressure = climate[i].value;
        }
        if(climate[i].type=='humidity'){
            var humidity = climate[i].value;
        }
        
    }
    query+="("+obj.id_cross+","+temperature+","+pressure+","+humidity+",'"+obj.date+"','"+obj.time+"')";
    
    try{
        let pool= await sql.connect(config);
        let result = await pool.request().query(query);
        sql.close();
    }
    catch(errore){
        sql.close();
        context.log("Error: ", errore);
    }

};