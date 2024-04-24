import React, { useState } from 'react';
const { ipcRenderer } = window.require('electron');

function App() {
    const [COM, setCOM] = useState('');
    const [baud, setBaud] = useState(9600);
    const [status, setStatus] = useState(false);

    const connectToCOMPort = async () => {
        const response = await ipcRenderer.invoke(
            'send-json',{cmd: "connect", port: COM, baud: baud}
        );
        setStatus(response?.connected == "yes");
    };

    const disconnectFromCOMPort = async () => {
        const response = await ipcRenderer.invoke(
            'send-json',{cmd:"disconnect"}
        );
        setStatus(response?.connected == "yes");
    }

    return (
        <div>
            <input
            type="text"  // Define o tipo do input como texto
            value={COM}  // Liga o estado do React ao valor do input
            onChange={(e)=>{setCOM(e.target.value)}}  // Atualiza o estado sempre que o input mudar
            placeholder="Digite a porta serial do equipamento..."  // Texto de ajuda quando o input está vazio
            />
            <input
            type='number'  // Define o tipo do input como texto
            value={baud}  // Liga o estado do React ao valor do input
            onChange={(e)=>{setBaud(e.target.value)}}  // Atualiza o estado sempre que o input mudar
            />
            <button onClick={status ? disconnectFromCOMPort:connectToCOMPort}>
                {status ? "Desconectar":"Conectar"}
            </button>
        </div>
    );
}

export default App;
