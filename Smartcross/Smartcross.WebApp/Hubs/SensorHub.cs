using System;
using System.Threading.Tasks;
using Microsoft.AspNetCore.SignalR;

namespace Smartcross.WebApp.Hubs
{
    public class SensorHub : Hub
    {
        public async Task SendMessage(string sensor, string message)
        {
            await Clients.All.SendAsync("SensorsData", sensor, message);
        }

    }
}
