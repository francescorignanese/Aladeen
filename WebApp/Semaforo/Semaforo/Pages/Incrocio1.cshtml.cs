using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Data;
using Data.Models;
using Microsoft.AspNetCore.Mvc;
using Microsoft.AspNetCore.Mvc.RazorPages;
using Semaforo.Data;

namespace Semaforo
{
    public class Incrocio1Model : PageModel
    {
       
        /*
        private readonly IDati _dati;

        public IEnumerable<DataSem> GetDati { get; set; }
        public Dati Dati { get; set; }
        public Incrocio1Model(IDati dati)
        {
            _dati = dati;
        }

        public void OnGet()
        {
            GetDati = _dati.GetAll();
        }
        public IActionResult OnGet(int id)
        {
            GetDati = _dati.GetById(id);
            if (Dati == null)
                return NotFound();

            return Page();
        }
        */
    }
}