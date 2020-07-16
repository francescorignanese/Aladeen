const sql = require('mssql');

module.exports = async function(context, obj) {
    context.log('JavaScript ServiceBus queue trigger function processed message', obj);

    // config database
    

    
    //context.log(obj.data_carriers);
    var carriers=obj.data_carriers;
    let query = "INSERT INTO [dbo].[Veicoli] ([Strada_id],[Tipologia_veicolo],[Conteggio],[Data],[Ora]) VALUES";
    for( let i=0; i<carriers.length;i++){
        var vehicles= carriers[i].data_vehicles;
        for(let j=0;j<vehicles.length;j++){
            query+="("+carriers[i].id_road+",'"+vehicles[j].type+"',"+vehicles[j].value+",'"+obj.date+"','"+obj.time+"')";
            if(i== (carriers.length-1) && j== (vehicles.length-1)) 
            {}
            else{
                query+=",";
            }
            
        }
    }
    context.log(query);
    doQuery();

    async function doQuery(){
        try{
            let pool= await sql.connect(config);
            let result = await pool.request().query(query);
            sql.close();
        }
        catch(errore){
            sql.close();
            context.log("Error: ", errore);
        }
    }

};