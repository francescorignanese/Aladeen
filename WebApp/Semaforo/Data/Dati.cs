using Microsoft.Extensions.Configuration;
using Data.Models;
using System;
using System.Collections.Generic;
using System.Data.SqlClient;
using Dapper.Contrib.Extensions;
using Dapper;

namespace Data
{
    public class Dati : IDati       
    {
        
        string connectionString;
        public Dati(IConfiguration config)
        {
            connectionString = ("STRINGA DI CONNESSIONE"); // Percorso del DataBase
        }

        public IEnumerable<DataSem> GetAll()
        {
            using (var connection = new SqlConnection(connectionString))
            {
                string query = @" INSERIMENTO QUERY";

                return connection.Query<DataSem>(query);
            }
        }

        public DataSem GetById(int id)
        {
            using (var connection = new SqlConnection(connectionString))
            {
                return connection.Get<DataSem>(id);
            }
            
        }

        IEnumerable<DataSem> IDati.GetById(int id)
        {
            throw new NotImplementedException();
        }
    }
}
