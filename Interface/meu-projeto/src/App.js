import React, { useEffect, useState } from "react";
import ToggleButton from "./ToggleButton";
import ConnectionConfig from "./ConnectionConfig";
import ChannelConfig from "./ChannelConfig";
import DischargeList from "./DischargeList";
const { ipcRenderer } = window.require("electron");

function App() {
    const [COM, setCOM] = useState("COM1");
    const [baud, setBaud] = useState("9600");
    const [status, setStatus] = useState(false);
    const [mode, setMode] = useState(["I", "I"]);
    const [crrLevel, setCrrLevel] = useState([0.0, 0.0]);
    const [level, setLevel] = useState([0.0, 0.0]);
    const [sync, setSync] = useState(false);
    const [voltage, setVoltage] = useState([0.0, 0.0]);
    const [current, setCurrent] = useState([0.0, 0.0]);
    const [list, setList] = useState([]);
    const [onList, setOnList] = useState(-1);
    const [origin, setOrigin] = useState(0);
    const [playing, setPlaying] = useState(false);
    const [dataAvailable, setDataAvailable] = useState(false);

    useEffect(() => {
        const zmqListener = (event, data) => {
            console.log("Received ZMQ message:", data);
            setVoltage([data.V1, data.V2]);
            setCurrent([data.I1, data.I2]);
            setMode([data.M1, data.M2]);
            setCrrLevel([data.LVL1, data.LVL2]);
            setSync(data.SYNC);
            setPlaying(data.PLAYING == 0 ? false : true);
            setDataAvailable(data.DATA == 0 ? false : true);
        };

        ipcRenderer.on("zmq-message", zmqListener);

        return () => {
            ipcRenderer.removeListener("zmq-message", zmqListener);
        };
    }, []);

    useEffect(() => {
        if (list.length == 0) setOnList(-1);
        else setOnList(sync ? 3 : origin + 1 == 1 ? 2 : 1);
    }, [list.length != 0, sync]);

    const setChannelMode = async (channel, mode) => {
        const response = await ipcRenderer.invoke("send-json", {
            cmd: "set-channels",
            channel: channel,
            option: "set-mode",
            mode: mode,
        });
        console.log(response);
    };
    const setChannelLevel = async (channel, level) => {
        const response = await ipcRenderer.invoke("send-json", {
            cmd: "set-channels",
            channel: channel,
            option: "set-level",
            mode: mode[channel - 1],
            level: level,
        });
        console.log(response);
    };
    const setChannelSync = async (channel, sync) => {
        let val = sync ? "sync" : "async";
        const response = await ipcRenderer.invoke("send-json", {
            cmd: "set-channels",
            channel: channel,
            option: "set-sync",
            mode: val,
        });
        return response;
    };
    const setChannelOff = async (channel) => {
        const response = await ipcRenderer.invoke("send-json", {
            cmd: "set-channels",
            channel: channel,
            option: "channel-off",
        });
        return response;
    };

    return (
        <div className=" w-screen h-full min-h-screen  bg-slate-700 text-slate-400 font-thin p-10 flex flex-col items-center">
            <h1 className="text-3xl mr-auto">SDCB</h1>
            <h2 className="text-sm mr-auto">
                Sistema de Descarga Controlada de Baterias
            </h2>
            <div className="mt-10 w-full max-w-[850px] flex flex-col items-center gap-5">
                <div className="w-full flex flex-row gap-5 items-center">
                    <h1>Sync Channels</h1>
                    <ToggleButton
                        id="toggle_sync"
                        on={sync}
                        onToggle={() => {
                            setChannelSync(1, !sync);
                        }}
                    />
                    <ConnectionConfig
                        COM={COM}
                        setCOM={setCOM}
                        baud={baud}
                        setBaud={setBaud}
                        status={status}
                        connectToCOMPort={async () => {
                            const response = await ipcRenderer.invoke(
                                "send-json",
                                {
                                    cmd: "connect",
                                    port: COM,
                                    baud: baud,
                                }
                            );
                            setStatus(response?.status == "connected");
                        }}
                        disconnectFromCOMPort={async () => {
                            const response = await ipcRenderer.invoke(
                                "send-json",
                                {
                                    cmd: "disconnect",
                                }
                            );
                            setStatus(response?.status == "connected");
                        }}
                    />
                </div>
                <div className=" text-white bg-slate-600 w-full flex flex-col items-center justify-between gap-10">
                    {["a", "b"].map((v, idx) => {
                        return (
                            <ChannelConfig
                                key={v}
                                sync={sync}
                                onList={onList}
                                onToggle={async () => {
                                    if (sync) {
                                        await setChannelSync(1, false);
                                        setOrigin(idx);
                                    } else setOnList(onList == 1 ? 2 : 1);
                                }}
                                channel={idx + 1}
                                mode={mode}
                                onModeToggle={() => {
                                    if (mode[idx] == "I")
                                        setChannelMode(idx + 1, "P");
                                    else setChannelMode(idx + 1, "I");
                                }}
                                level={level}
                                onLevelChange={(e) => {
                                    const newLevels = [...level]; // Cria uma nova cópia do array
                                    newLevels[idx] = e.target.value; // Atualiza o valor no novo array
                                    setLevel(newLevels); // Atualiza o estado com o novo array
                                }}
                                onLevelConfirm={(e) => {
                                    if (e.key == "Enter")
                                        setChannelLevel(
                                            idx + 1,
                                            parseFloat(level[idx])
                                        );
                                }}
                                onKill={() => {
                                    setChannelOff(idx + 1);
                                }}
                                voltage={voltage}
                                current={current}
                                crrLevel={crrLevel}
                            />
                        );
                    })}
                </div>
                <DischargeList
                    list={list}
                    playing={playing}
                    openFileDialog={async () => {
                        const response = await ipcRenderer.invoke(
                            "open-csv-dialog"
                        );
                        if (response) {
                            const lists = await ipcRenderer.invoke(
                                "send-json",
                                {
                                    cmd: "open-file",
                                    filepath: response,
                                }
                            );
                            if (lists.error == "none") {
                                let newList = lists.level.map(
                                    (element, index) => [
                                        element,
                                        lists.time[index],
                                    ]
                                );
                                setList(newList);
                            }
                        }
                    }}
                    onStart={async () => {
                        ipcRenderer.invoke("send-json", {
                            cmd: "start-discharge",
                            channel: onList,
                            mode: onList > 2 ? mode[0] : mode[onList - 1],
                        });
                    }}
                    onKill={async () => {
                        ipcRenderer.invoke("send-json", {
                            cmd: "stop-discharge",
                            channel: onList,
                            mode: onList > 2 ? mode[0] : mode[onList - 1],
                        });
                    }}
                    saveResults={async () => {
                        const response = await ipcRenderer.invoke(
                            "save-csv-dialog"
                        );
                        if (response) {
                            const res1 = await ipcRenderer.invoke("send-json", {
                                cmd: "save-data",
                                filepath: response,
                            });
                        }
                    }}
                    onClear={async () => {
                        setList([]);
                        ipcRenderer.invoke("send-json", {
                            cmd: "stop-discharge",
                            channel: onList,
                            mode: onList > 2 ? mode[0] : mode[onList - 1],
                        });
                    }}
                    dataAvailable={dataAvailable}
                />
            </div>
        </div>
    );
}

export default App;
