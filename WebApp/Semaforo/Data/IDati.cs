using Data.Models;
using System;
using System.Collections.Generic;
using System.Text;

namespace Data
{
    public interface IDati
    {
        IEnumerable<DataSem> GetData();
        DataSem GetDataById(int id);
    }
}
