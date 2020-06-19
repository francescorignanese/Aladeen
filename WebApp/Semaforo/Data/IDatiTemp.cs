using Data.Models;
using System;
using System.Collections.Generic;
using System.Text;

namespace Data
{
    public interface IDatiTemp
    {
        DTemp Get(byte id);

        IEnumerable<DTemp> GetAll();
    }
}
