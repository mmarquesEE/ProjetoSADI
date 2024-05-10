import React from "react";

function ConnectionConfig({
    COM,
    setCOM,
    baud,
    setBaud,
    status,
    connectToCOMPort,
    disconnectFromCOMPort,
}) {
    return (
        <>
            <h1>Port / Baud</h1>
            <input
                className=" bg-transparent px-5 w-20 h-8"
                type="text" // Define o tipo do input como texto
                value={COM} // Liga o estado do React ao valor do input
                onInput={(e) => {
                    setCOM(e.target.value);
                }} // Atualiza o estado sempre que o input mudar
                placeholder="Digite a porta serial do equipamento..." // Texto de ajuda quando o input está vazio
            />
            <input
                className=" bg-transparent px-5 w-24 h-8"
                type="text" // Define o tipo do input como texto
                value={baud} // Liga o estado do React ao valor do input
                onChange={(e) => {
                    setBaud(e.target.value);
                }} // Atualiza o estado sempre que o input mudar
            />
            <button
                className={`${status ? "bg-red-500" : "bg-green-500"}
                    w-28 p-2 m-2 text-slate-300 bg-opacity-75 hover:bg-opacity-100`}
                onClick={status ? disconnectFromCOMPort : connectToCOMPort}
            >
                {status ? "Close" : "Connect"}
            </button>
        </>
    );
}

export default ConnectionConfig;
