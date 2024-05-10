import React from "react";
import ToggleButton from "./ToggleButton";

function ChannelConfig({
    channel,
    sync,
    mode,
    onModeToggle,
    level,
    onLevelChange,
    onLevelConfirm,
    onList,
    onToggle,
    voltage,
    current,
    crrLevel,
}) {
    return (
        <div className="pl-4 w-full bg-slate-500 flex flex-row items-center justify-between align-middle gap-5">
            {onList != -1 && (
                <ToggleButton
                    id={"toggle" + String(channel)}
                    on={(channel == onList) | (sync && onList == 3)}
                    onToggle={onToggle}
                />
            )}
            <h1 className=" ml-2 w-32">Channel {channel - 1 + 1}</h1>
            <button
                className={` w-28 ${
                    mode[channel - 1] == "I" ? " bg-blue-500" : "bg-pink-400"
                } bg-opacity-75 hover:bg-opacity-100 p-2`}
                onClick={onModeToggle}
            >
                {mode[channel - 1] == "I" ? "Current" : "Power"}
            </button>
            <h1 className="w-40">Set Point</h1>
            <div className="flex flex-col justify-between">
                <input
                    className=" bg-transparent px-5 w-28"
                    type="number" // Define o tipo do input como texto
                    value={level[channel - 1]} // Liga o estado do React ao valor do input
                    onInput={onLevelChange} // Atualiza o estado sempre que o input mudar
                    onKeyDown={onLevelConfirm}
                />
                <h3 className=" text-xs">
                    (currentlyt: {crrLevel[channel - 1].toFixed(2)}{" "}
                    {mode[channel - 1] == "I" ? "A" : "W"})
                </h3>
            </div>
            <h2 className="w-10 text-center">
                {mode[channel - 1] == "I" ? "A" : "W"}
            </h2>

            <div className=" w-56 flex flex-col text-slate-400 bg-slate-800 p-2">
                <div className="flex flex-row justify-between">
                    <h3>Voltage = </h3>
                    <p>{voltage[channel - 1]} V</p>
                </div>
                <div className="flex flex-row justify-between">
                    <h3>Current = </h3>
                    <p>{current[channel - 1]} A</p>
                </div>
            </div>
        </div>
    );
}

export default ChannelConfig;
