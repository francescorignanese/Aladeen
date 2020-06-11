const redis = require('redis');

const client = redis.createClient();

client.on('error', (err) => {
    console.log("Error " + err);
});

client.on("message",(channel,message)=>{
    //dati ricevuti
});

client.subscribe("");



/**
 * 1. prendo il dato dalla coda di redis
 * 2. provo ad inviarlo
 * 3. se risultato 'ok' --> tolgo dato dalla cosa
 *    se risultato 'err' --> rimango fermo e poi riprovo l'invio
 */