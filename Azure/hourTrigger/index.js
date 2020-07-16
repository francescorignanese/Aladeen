const sql = require('mssql');

module.exports = async function (context, myTimer) {
    var timeStamp = new Date().toISOString();

    // config database
    


    if (myTimer.isPastDue)
    {
        context.log('JavaScript is running late!');
    }
    context.log('JavaScript timer trigger function ran!', timeStamp);

    await doQuery();
    
    async function doQuery(){
        var result;
        var date= new Date();
        var hour= date.getHours()-1;
        if (hour < 1){
            date.setDate(date.getDate()-1)
        }
        date=date.toISOString().slice(0,10);
        context.log(date);
        context.log(hour); 
        let selectQuery= "SELECT DISTINCT Dispositivo_id as Id_Dispositivo,Id_semaforo, Strada_id as Id_Strada, Tipologia_veicolo, SUM(Conteggio) as Conteggio, DATEPART(hour, Ora) as Ora "
        + "FROM [dbo].[Veicoli], [dbo].[Strade] "
        + "WHERE DATEPART(hour, Ora)="+hour+" AND [dbo].[Veicoli].Strada_id=[dbo].[Strade].id_strada AND Data='"+date+"' "
        + "GROUP BY Tipologia_veicolo, Strada_id,Ora, Dispositivo_id,Id_semaforo";
            // formattare: FOR JSON AUTO
        try{
            let pool=await sql.connect(configProv);
            var result = await pool.request().query(selectQuery);
            sql.close();
        }
        catch(errore){
            sql.close();
            context.log("Error: ", errore);
        }

        context.log(result.recordset);
        var array= result.recordset;

        
        let insertQuery = "INSERT INTO [dbo].[Traffico] ([Id_incrocio],[Id_semaforo],[Id_strada],[Fascia_oraria],[Data],[Tipologia_veicolo],[Conteggio]) VALUES";
        let ora;
        for (let i = 0; i < array.length; i++) {
            ora=array[i].Ora+1;
            insertQuery+="("+array[i].Id_Dispositivo+","+array[i].Id_semaforo+","+array[i].Id_Strada+","+ora+",'"+date+"','"+array[i].Tipologia_veicolo+"',"+array[i].Conteggio+")";
            if(i== array.length-1){}
            else{
                insertQuery+=",";
            }
        }
        
        context.log(insertQuery);
        
        try{
            let pool= await sql.connect(configDest);
            let result = await pool.request().query(insertQuery);
            sql.close();
        }
        catch(errore){
            sql.close();
            //context.log("Error: ", errore);
        }
        
        
    }

};