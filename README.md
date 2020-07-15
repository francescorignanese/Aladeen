# Annunci / Status del progetto

# Titolo
SMARTCROSS

# Indice

- [Descrizione del progetto](#descrizione-del-progetto)
- [Come iniziare](#come-iniziare)
- [Avvio del progetto](#avvio-del-progetto)
- [Manutenzione](#manutenzione)
- [Licenza](#licenza)

# Descrizione del progetto
Realizzazione di un prototipo di Smart City con la progettazione di un sistema di incrocio di tipo semaforico con tecnologie IOT che permetta di rilevare diverse tipologie di dato a fine statistico e valutativo grazie alla presenza di un database per la raccolta dei dati.

# Come iniziare
Scaricare da GitHub tutto il la crtella di progetto Aladeen-master.
Installare i seguenti trumenti:
- MPLAB X
- PicsimLab
- Node.js
- Visual Studio Code (o editor di codice alternativo)
- Macchina virtuale Debian con Redis
- Azure services con sottoscrizione attiva

# Avvio del progetto
### Embedded
Aprire il PicsimLab -> Modules -> Spare parts.
Nella finestra che compare fare click su Add -> aggiungere almeno 12 RGB LED (ogni 3 LED rappresentano un semaforo)
Tasto destro sopra i LED -> Properties -> settare i LED come segue:

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

Nella finestra principale del PicsimLab caricare il file .hex File -> Load Hex presente nella directory: \Aladeen-master\Embedded\Semaforo.X\dist\default\production

### Gateway
Aprire la cartella di progetto con Visual Studio Code (o editor alternativo) e da terminale installare i node_modules utilizzando il comando *npm install*.
Nel file index.js configurare la porta per il seriale e immettere l'indirizzo IP della macchina virtuale per la connessione con Redis
(const port = new SerialPort('/PORTA'); const client = redis.createClient(6379, 'INDIRIZZO IP');)
Eseguire il file *index.js* con il comando *node index.js*.
Verificare la ricezione di dati da terminale e l'invio di questi a Redis.

### Azure
Installare le seguenti estensioni su Visual Studio Code: Azure Account, Azure IoT Hub, Azure Functions.
Configurare i file config delle Azure Function con la connessione ai Database Sql di Azure e all'IoT Hub.
Fare il deploy delle Azure Functions: da Visual Studio Code -> Azure Functions -> Deploy to Function App

## Documentazione
### Link a documentazione esterna 
[Google Drive](https://drive.google.com/drive/folders/1CRRsaPtyuaZ3H-2uDrHZevjmXLm522T4)
[GitHub](https://github.com/Francesco-Rignanese/Aladeen)
[Trello](https://trello.com/b/Abhk5jvA/aladeen)

## Community

### Code of conduct

### Responsible Disclosure

### Segnalazione bug e richieste di aiuto

# Manutenzione 

# Licenza 

## Licenza generale 

## Autori e Copyright
Powered by Aladeen Group
- Ciancia Simone
- Filippi Andrea
- Garolla Nicolò
- Pojoga Georgeta
- Rignanese Francesco
- Zuppel Alessandro

## Licenze software dei componenti di terze parti
