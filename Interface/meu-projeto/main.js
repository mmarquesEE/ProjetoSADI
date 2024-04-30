const zmq = require("zeromq");
const { app, BrowserWindow, ipcMain } = require("electron");

let mainWindow;

async function createWindow() {
    mainWindow = new BrowserWindow({
        width: 800,
        height: 450,
        webPreferences: {
            nodeIntegration: true,
            contextIsolation: false,
        },
    });

    mainWindow.loadURL("http://localhost:3000");
    mainWindow.setMenu(null);
}

app.on("ready", () => {
    createWindow();
    setupIpcListeners();
    setupZmqSubscriptions();
});

const client = new zmq.Request();
const subs = new zmq.Subscriber();

client.connect("tcp://localhost:5560");
console.log("Connected to server");

subs.connect("tcp://localhost:5561");
console.log("Connected to topic 'readings'");
subs.subscribe("");

function setupIpcListeners() {
    ipcMain.handle("send-json", async (event, args) => {
        console.log("Sending JSON to Python", args);
        await client.send(JSON.stringify(args));
        const [reply] = await client.receive();
        console.log("Reply received from server:", reply.toString());
        return JSON.parse(reply.toString());
    });
}

async function setupZmqSubscriptions() {
    for await (const [reading] of subs) {
        console.log(reading.toString());
        // Convertendo a mensagem de JSON para objeto
        let msgData;
        try {
            msgData = JSON.parse(reading.toString());
        } catch (error) {
            console.error("Error parsing JSON message:", error);
            continue; // Pula para a próxima iteração do loop em caso de erro
        }

        if (mainWindow) {
            mainWindow.webContents.send("zmq-message", msgData);
        }
    }
}
