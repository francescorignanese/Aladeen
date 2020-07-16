# SMARTCROSS

# Indice

- [Descrizione del progetto](#descrizione-del-progetto)
- [Come iniziare](#come-iniziare)
- [Avvio del progetto](#avvio-del-progetto)
- [Licenza](#licenza)

# Descrizione del progetto
Realizzazione di un prototipo di Smart City con la progettazione di un sistema di incrocio di tipo semaforico con tecnologie IOT che permetta di rilevare diverse tipologie di dato a fine statistico e valutativo grazie alla presenza di un database per la raccolta dei dati.

# Come iniziare
Scaricare da GitHub tutta la cartella del progetto Aladeen-master.<br/>
Installare i seguenti strumenti:
- MPLAB X
- PicsimLab
- Node.js
- Visual Studio Code (o editor di codice alternativo)
- Macchina virtuale Debian con Redis
- Portale Azure con sottoscrizione attiva

# Avvio del progetto
### Embedded
Aprire il PicsimLab -> Modules -> Spare parts.<br/>
Fare click su File -> Load configurations -> caricare il file configurazione_spare_parts.pcf presente nel percorso \Aladeen\Embedded\Semaforo.X\dist\default\production.<br/>
Oppure è possibile configurare manualemnte il led con il seguente procedimento: nella finestra che compare fare click su Add -> aggiungere 12 RGB LED (ogni 3 LED rappresentano un semaforo) e un Potentiometers.<br/>
Tasto destro sopra i LED -> Properties -> settare i LED come segue:<br/>

**Primo semaforo**
- Primo LED: Pin 1: RC0
- Secondo LED: Pin 1/2: RC2
- Terzo LED: Pin 2: RC1

**Secondo semaforo**
- Primo LED: Pin 1: RB7
- Secondo LED: Pin 1/2: RB6
- Terzo LED: Pin 2: RB2

**Terzo semaforo**
- Primo LED: Pin 1: RB7
- Secondo LED: Pin 1/2: RB6
- Terzo LED: Pin 2: RB2

**Quarto semaforo**
- Primo LED: Pin 1: RC0
- Secondo LED: Pin 1/2: RC2
- Terzo LED: Pin 2: RC1

Tasto destro sopra Potentiometers -> Properties -> settare il potenziometro come segue:<br/>

**Sensore pressione**
- Primo SLIDER: Pot.1: RE0

Nella finestra principale del PicsimLab caricare il file .hex (File -> Load Hex) presente nella directory \Aladeen-master\Embedded\Semaforo.X\dist\default\production.

### Gateway
Avviare il servizio di Redis da macchina virtuale Debian o da altro sistema operativo base Linux o al posto della macchina virtuale anche Docker.<br/>
Aprire la cartella di progetto con Visual Studio Code (o editor alternativo) e da terminale installare i node_modules utilizzando il comando *npm install*.<br/>
Nel file index.js configurare la porta per il seriale e immettere l'indirizzo IP della macchina virtuale per la connessione con Redis
(const port = new SerialPort('/PORTA'); const client = redis.createClient(6379, 'INDIRIZZO IP');).<br/>
Eseguire il file *index.js* con il comando *node index.js*.<br/>
Verificare la ricezione di dati da terminale e l'invio di questi a Redis.<br/>

### Azure
Installare le seguenti estensioni su Visual Studio Code: Azure Account, Azure IoT Hub, Azure Functions.<br/>
Configurare i file config delle Azure Function con la connessione ai Database Sql di Azure e all'IoT Hub.<br/>
Fare il deploy delle Azure Functions: da Visual Studio Code -> Azure Functions -> Deploy to Function App.<br/>

## Documentazione
### Link a documentazione esterna 
[Google Drive](https://drive.google.com/drive/folders/1CRRsaPtyuaZ3H-2uDrHZevjmXLm522T4)<br/>
[GitHub](https://github.com/Francesco-Rignanese/Aladeen)<br/>
[Trello](https://trello.com/b/Abhk5jvA/aladeen)<br/> 

# Licenza 
Licenza open source 

## Autori e Copyright
Powered by Aladeen Group
- Ciancia Simone
- Filippi Andrea
- Garolla Nicolò
- Pojoga Georgeta
- Rignanese Francesco
- Zuppel Alessandro
