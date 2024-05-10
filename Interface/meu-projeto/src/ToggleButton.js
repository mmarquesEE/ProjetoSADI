import React from "react";

function ToggleButton({ id, on, onToggle }) {
    return (
        <div className="flex items-center justify-center mr-auto">
            <label htmlFor={id} className="flex items-center cursor-pointer">
                <div className="relative">
                    <input
                        id={id}
                        type="checkbox"
                        className="sr-only"
                        checked={on == 1}
                        onChange={onToggle}
                    />
                    <div
                        className={`block transition-colors ${
                            on == 1 ? "bg-green-600" : "bg-gray-600"
                        } w-10 h-5 rounded-full`}
                    ></div>
                    <div
                        className={`absolute left-0 top-0 bg-white w-5 h-5 rounded-full transition-transform ${
                            on == 1 ? "transform translate-x-5" : ""
                        }`}
                    ></div>
                </div>
            </label>
        </div>
    );
}

export default ToggleButton;
