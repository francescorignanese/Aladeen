using Microsoft.AspNetCore.Mvc;
using Microsoft.AspNetCore.Mvc.RazorPages;

namespace Semaforo
{
    public class IndexModel : PageModel
    {
        public IActionResult OnGet()
        {
            return new RedirectResult("/Startseite");
        }
    }
}