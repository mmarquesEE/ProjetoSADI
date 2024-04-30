import React, { useEffect, useState } from "react";
const { ipcRenderer } = window.require("electron");

function App() {
    const [COM, setCOM] = useState("COM1");
    const [baud, setBaud] = useState(9600);
    const [status, setStatus] = useState(false);
    const [mode, setMode] = useState(["I", "I"]);
    const [crrLevel, setCrrLevel] = useState([0.0, 0.0]);
    const [level, setLevel] = useState([0.0, 0.0]);
    const [voltage, setVoltage] = useState([0.0, 0.0]);
    const [current, setCurrent] = useState([0.0, 0.0]);

    const connectToCOMPort = async () => {
        const response = await ipcRenderer.invoke("send-json", {
            cmd: "connect",
            port: COM,
            baud: baud,
        });
        setStatus(response?.status == "connected");
    };

    const disconnectFromCOMPort = async () => {
        const response = await ipcRenderer.invoke("send-json", {
            cmd: "disconnect",
        });
        setStatus(response?.status == "connected");
    };

    useEffect(() => {
        const zmqListener = (event, data) => {
            console.log("Received ZMQ message:", data);
            setVoltage([data.V1, data.V2]);
            setCurrent([data.I1, data.I2]);
            setMode([data.M1, data.M2]);
            setCrrLevel([data.LVL1, data.LVL2]);
        };

        ipcRenderer.on("zmq-message", zmqListener);

        return () => {
            ipcRenderer.removeListener("zmq-message", zmqListener);
        };
    }, []);

    const setChannelMode = async (channel, mode) => {
        const response = await ipcRenderer.invoke("send-json", {
            cmd: "set-channels",
            channels: [channel],
            option: "mode",
            value: [mode],
        });
        console.log(response);
    };

    return (
        <div className=" w-screen h-screen bg-slate-700 text-slate-400 font-thin p-10 flex flex-col items-center">
            <div className=" w-full max-w-screen-xl flex flex-col items-center gap-5">
                <div className=" ml-auto flex flex-row gap-5 items-center">
                    <h1>Port / Baud</h1>
                    <input
                        className=" bg-transparent px-5 w-20 h-8"
                        type="text" // Define o tipo do input como texto
                        value={COM} // Liga o estado do React ao valor do input
                        onChange={(e) => {
                            setCOM(e.target.value);
                        }} // Atualiza o estado sempre que o input mudar
                        placeholder="Digite a porta serial do equipamento..." // Texto de ajuda quando o input está vazio
                    />
                    <input
                        className=" bg-transparent px-5 w-24 h-8"
                        type="number" // Define o tipo do input como texto
                        value={baud} // Liga o estado do React ao valor do input
                        onChange={(e) => {
                            setBaud(e.target.value);
                        }} // Atualiza o estado sempre que o input mudar
                    />
                    <button
                        className={`${status ? "bg-red-500" : "bg-green-500"}
                     w-28 p-2 m-2 text-slate-300 bg-opacity-75 hover:bg-opacity-100`}
                        onClick={
                            status ? disconnectFromCOMPort : connectToCOMPort
                        }
                    >
                        {status ? "Close" : "Connect"}
                    </button>
                </div>
                <div className=" text-black bg-slate-600 w-full max-w-2xl flex flex-col items-start justify-around gap-10">
                    {[0, 1].map((v) => {
                        return (
                            <div className=" w-full bg-slate-500 flex flex-row items-center align-middle gap-5">
                                <h1 className=" ml-2 w-28">Channel {v + 1}</h1>
                                <button
                                    className={` w-20 ${
                                        mode[v] == "I"
                                            ? " bg-blue-500"
                                            : "bg-pink-400"
                                    } text-slate-900 bg-opacity-75 hover:bg-opacity-100`}
                                    onClick={() => {
                                        let val = mode;
                                        if (mode[v] == "I") {
                                            val[v] = "P";
                                            setChannelMode(v + 1, "P");
                                        } else {
                                            val[v] = "I";
                                            setChannelMode(v + 1, "I");
                                        }
                                        setMode(val);
                                    }}
                                >
                                    {mode[v] == "I" ? "Current" : "Power"}
                                </button>
                                <input
                                    className=" bg-transparent px-5 w-28"
                                    type="number" // Define o tipo do input como texto
                                    value={level[v]} // Liga o estado do React ao valor do input
                                    onChange={(e) => {
                                        if (e.target.value >= 0) {
                                            let val = level;
                                            val[v] = e.target.value;
                                            setLevel(val);
                                        }
                                    }} // Atualiza o estado sempre que o input mudar
                                />
                                <h2 className="w-10 text-center">
                                    {mode[v] == "I" ? "A" : "W"}
                                </h2>
                                <div className="ml-20 w-36 flex flex-col text-slate-400 bg-slate-800 p-2">
                                    <div className="flex flex-row justify-between">
                                        <h3>Voltage = </h3>
                                        <p>{voltage[v]} V</p>
                                    </div>
                                    <div className="flex flex-row justify-between">
                                        <h3>Current = </h3>
                                        <p>{current[v]} A</p>
                                    </div>
                                </div>
                            </div>
                        );
                    })}
                </div>
                <button className=" bg-green-700 text-white p-5 m-auto bg-opacity-75 hover:bg-opacity-100">
                    Add discharge list
                </button>
            </div>
        </div>
    );
}

export default App;
