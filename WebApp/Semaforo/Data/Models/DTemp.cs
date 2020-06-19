using Dapper.Contrib.Extensions;
using System;
using System.Collections.Generic;
using System.Text;

namespace Data.Models
{
    [Table("Sensori")]
    public class DTemp
    {
        [Key]
        public int Id_sensore { get; set; }
        public int Temperatura { get; set; }
        public int Pressione { get; set; }
        public int Umidita { get; set; }
    }
}
