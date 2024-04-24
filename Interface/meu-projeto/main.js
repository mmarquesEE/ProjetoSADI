const zmq = require('zeromq');
const { app, BrowserWindow, ipcMain } = require('electron');

let mainWindow;

async function createWindow() {
    mainWindow = new BrowserWindow({
        width: 800,
        height: 600,
        webPreferences: {
            nodeIntegration: true,
            contextIsolation: false,
        }
    });

    mainWindow.loadURL('http://localhost:3000');
    mainWindow.setMenu(null);
}

app.on('ready', () => {
    createWindow();
    setupIpcListeners();
});

const sock = new zmq.Request();

sock.connect("tcp://localhost:5560");
console.log("Conectado ao servidor Python");

function setupIpcListeners() {
    ipcMain.handle('send-json', async (event, args) => {
        console.log("Enviando JSON para o Python", args);
        await sock.send(JSON.stringify(args));
        const [reply] = await sock.receive();
        console.log("Resposta recebida do servidor:", reply.toString());
        return JSON.parse(reply.toString());
    });
}