using System;
using Dapper;
using Dapper.Contrib.Extensions;

namespace Data.Models
{
    [Table("Veicoli")]
    public class DataSem
    {
        [Key]
        public int Id_veicolo { get; set; }
        public int Strada_id { get; set; }
        public string Tipologia_veicolo { get; set; }
        public int Conteggio { get; set; }
        public DateTime Data { get; set; }
        public DateTime Ora { get; set; }
    }
}
