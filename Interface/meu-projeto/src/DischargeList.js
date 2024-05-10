import React, { useState } from "react";

function DischargeList({
    list,
    openFileDialog,
    playing,
    onStart,
    onKill,
    saveResults,
    dataAvailable,
    onClear,
}) {
    const [showList, setShowList] = useState(false);

    return (
        <>
            <div className="w-full flex flex-row my-2 justify-between">
                <button
                    className=" bg-green-700 text-white h-10 flex items-center p-5 bg-opacity-75 hover:bg-opacity-100"
                    onClick={openFileDialog}
                >
                    {list.length != 0 ? "Change" : "Set"} discharge list
                </button>
                {list.length != 0 && (
                    <>
                        <button
                            className={` ${
                                showList ? "bg-inherit" : "bg-slate-400"
                            } w-20 text-white ml-1 h-10 flex items-center p-5 bg-opacity-75 hover:bg-opacity-100`}
                            onClick={() => {
                                setShowList(!showList);
                            }}
                        >
                            {showList ? "Hide" : "Show"}
                        </button>
                        <button
                            className=" bg-red-700 text-white ml-1 h-10 flex items-center mr-auto p-5 bg-opacity-75 hover:bg-opacity-100"
                            onClick={onClear}
                        >
                            Clear
                        </button>
                        {dataAvailable && (
                            <button
                                className=" bg-orange-500 text-white ml-1 h-10 flex items-center mr-auto p-5 bg-opacity-75 hover:bg-opacity-100"
                                onClick={saveResults}
                            >
                                Save Results
                            </button>
                        )}
                        <button
                            onClick={playing ? onKill : onStart}
                            className={` ${
                                playing ? "bg-red-500" : "bg-yellow-300"
                            } h-10 flex items-center text-black p-5 bg-opacity-75 hover:bg-opacity-100`}
                        >
                            {playing ? "Kill Process" : "Start Discharge"}
                        </button>
                    </>
                )}
            </div>
            {list.length != 0 && showList && (
                <div className="container mx-auto overflow-auto scrollable-area">
                    {" "}
                    {/* Container scrollable */}
                    <table className=" m-auto bg-transparent text-slate-400 table-auto w-4/5 text-center whitespace-no-wrap">
                        <thead>
                            <tr className="text-xs font-semibold tracking-wide uppercase border-b">
                                <th className="px-4 py-3">
                                    Level (verify channel mode before playing)
                                </th>
                                <th className="px-4 py-3">Duration (s)</th>
                            </tr>
                        </thead>
                        <tbody className="divide-y">
                            {list.map((pair, index) => (
                                <tr key={index} className="text-slate-200">
                                    <td className="px-4 py-3 border">
                                        {pair[0]}
                                    </td>
                                    <td className="px-4 py-3 border">
                                        {pair[1] !== null ? pair[1] : "N/A"}
                                    </td>
                                </tr>
                            ))}
                        </tbody>
                    </table>
                </div>
            )}
        </>
    );
}

export default DischargeList;
