const zmq = require("zeromq");
const { app, BrowserWindow, ipcMain, dialog } = require("electron");

let mainWindow;

async function createWindow() {
    mainWindow = new BrowserWindow({
        width: 800,
        height: 550,
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
        await client.send(JSON.stringify(args));
        const [reply] = await client.receive();
        return JSON.parse(reply.toString());
    });
    ipcMain.handle("open-csv-dialog", async () => {
        const { canceled, filePaths } = await dialog.showOpenDialog({
            properties: ["openFile"],
            filters: [{ name: "CSV Files", extensions: ["csv"] }],
        });
        if (canceled) {
            return;
        } else {
            return filePaths[0]; // retorna o caminho do arquivo selecionado
        }
    });
    ipcMain.handle("save-csv-dialog", async () => {
        const { filePath } = await dialog.showSaveDialog({
            filters: [{ name: "CSV Files", extensions: ["csv"] }],
        });
        if (filePath.canceled) {
            return;
        } else {
            return filePath.toString(); // retorna o caminho do arquivo selecionado
        }
    });
}

async function setupZmqSubscriptions() {
    for await (const [reading] of subs) {
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
