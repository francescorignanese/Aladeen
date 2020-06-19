using Microsoft.Extensions.Configuration;
using System;
using System.Collections.Generic;
using System.Text;
using Data.Models;
using System.Data.SqlClient;
using Dapper;

namespace Data
{
    public class DatiTemp : IDatiTemp
    { 
         string connectionString;
        public DatiTemp(IConfiguration config)
    {
        connectionString = ("STRINGA DI CONNESSIONE"); // Percorso del DataBase
    }


    
        public DTemp Get(byte id)
        {
            throw new NotImplementedException();
        }

        public IEnumerable<DTemp> GetAll()
        {
            using (var connection = new SqlConnection(connectionString))
            {
                string query = @" INSERIMENTO QUERY";

                return connection.Query<DTemp>(query);
            }
        }
    }
}
