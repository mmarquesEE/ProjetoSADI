import zmq.asyncio
import asyncio
import json
import serial
import sys
import time
import pandas as pd

if sys.platform == 'win32':
    asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())

ser = None

# Hardware constrained values!!!!
Q_NUMBER = 120

EVENT_P = Q_NUMBER + 1
EVENT_I = Q_NUMBER + 3
EVENT_S = Q_NUMBER + 5
EVENT_A = Q_NUMBER + 7

IMAX = 2.0
PMAX = 10.0

df = None
child_process = None
list_channel = 0
list_mode = 'I'
playing = 0
rows_out = []

async def list_task():
    global df
    global ser
    global list_channel
    global list_mode
    global playing
    try:
        if type(df) is not type(None):
            playing = 1
            for lvl,duration in zip(list(df["Level"]),list(df["Duration"])):
                level = int(lvl/(IMAX if list_mode == "I" else PMAX)*Q_NUMBER)
                code = 0x7f & (1 if level <= 0 else (level if level <= Q_NUMBER else Q_NUMBER))
                byte = bytes([list_channel | code])
                ser.write(byte)
                await asyncio.sleep(duration)
                print("ola")
            ser.write(bytes([list_channel | 0x00]))
            playing = 0
        else:
            print("List not loaded!!")

    except Exception as e:
        print(e)
        print("Killing List")
    finally:
        print("Shutting down List Task")

list_task_handle = None

async def server():
    global ser
    global df
    global list_task_handle
    global list_channel
    global list_mode
    global playing
    global rows_out

    context = zmq.asyncio.Context()
    socket = context.socket(zmq.REP)
    socket.bind("tcp://127.0.0.1:5560")

    while True:
        try:
            message = await socket.recv()  # Assíncrono, não bloqueante
            data = json.loads(message.decode('utf-8'))
            response = {}
            print(data)
            match data["cmd"]:
                case "connect":
                    try:
                        ser = serial.Serial(data["port"], data["baud"])
                    except Exception as e:
                        print(e)
                        response = {"status": "not connected"}
                    else:
                        response = {"status": "connected"}

                case "disconnect":
                    try:
                        ser.close()
                    except Exception as e:
                        print(e)
                        response = {"error": "no connection to close"}
                    else:
                        response = {"status": "not connected"}

                case "set-channels":
                    try:
                        channel = 0 if data["channel"] == 1 else 1 << 7
                        code = 0x00
                        match data["option"]:
                            case "set-level":
                                mode = data["mode"]
                                level = int(data["level"]/(IMAX if mode == "I" else PMAX)*Q_NUMBER)
                                code = 0x7f & (1 if level <= 0 else (level if level <= Q_NUMBER else Q_NUMBER))
                                byte = bytes([channel | level])
                                ser.write(byte)
                            
                            case "channel-off":
                                byte = bytes([channel | 0x00])
                                ser.write(byte)
                            
                            case "set-mode":
                                code = 0x7f & (EVENT_P if data["mode"] == "P" else EVENT_I)
                                byte = bytes([channel | code])
                                ser.write(byte)
                            
                            case "set-sync":
                                code = 0x7f & (EVENT_S if data["mode"] == "sync" else EVENT_A)
                                byte = bytes([channel | code])
                                ser.write(byte)
                        
                        print(code)

                    except Exception as e:
                        print(e)
                        response = {"error": "invalid command"}
                    else:
                        response = {"status" : "command sended"}
            
                case "open-file":
                    df = pd.read_csv(data["filepath"])
                    try:
                        response = {"time":list(df["Duration"]),"level":list(df["Level"]),"error":"none"}
                        print(response)
                    except Exception as e:
                        print(e)
                        response = {"error": "invalid table"}
                
                case "start-discharge":
                    list_channel = 0 if data["channel"] == 1 else 1 << 7
                    list_mode = data["mode"]
                    if list_task_handle is None or list_task_handle.done():
                        list_task_handle = asyncio.create_task(list_task())
                        rows_out = []
                        response = {"status": "playing"}
                    else:
                        response = {"status": "not playing"}
                case "stop-discharge":
                    list_channel = 0 if data["channel"] == 1 else 1 << 7
                    if list_task_handle is not None:
                        list_task_handle.cancel()
                        try:
                            await list_task_handle  # Apenas espera aqui para limpeza.
                        except asyncio.CancelledError:
                            pass
                        response = {"status": "finished"}
                    else:
                        response = {"status": "not playing"}
                    playing = 0
                    rows_out = []
                    ser.write(bytes([list_channel | 0x00]))
                
                case "save-data":
                    if len(rows_out) != 0:
                        df_out = pd.DataFrame(rows_out)
                        df_out.to_csv(data["filepath"])
                        response = {"status": "saved"}
                    else:
                        response = {"status": "no data"}


        except Exception as e:
            print(e)
            response = {"error": "invalid json"}
        finally:
            await socket.send_json(response)  # Assíncrono, não bloqueante

async def publisher():
    global ser
    global playing
    global rows_out
    data_available = 0
    context = zmq.asyncio.Context()
    publisher = context.socket(zmq.PUB)
    publisher.bind("tcp://127.0.0.1:5561")
    
    while True:
        if ser:
            try:
                reading = ser.readline().decode()
                v1, v2, i1, i2, m1, m2, lvl1, lvl2, sync = [el for el in reading.split(',')]
                json = {
                    "V1": float(v1),
                    "I1": float(i1),
                    "V2": float(v2),
                    "I2": float(i2),
                    "M1": m1,
                    "M2": m2,
                    "LVL1": float(lvl1)/Q_NUMBER * (IMAX if m1 == 'I' else PMAX),
                    "LVL2": float(lvl2)/Q_NUMBER * (IMAX if m1 == 'I' else PMAX),
                    "SYNC": int(sync),
                    "PLAYING": playing,
                    "DATA": data_available
                }
                if playing == 1:
                    data_available = 0
                    rows_out.append(json)
                elif len(rows_out) != 0:
                    data_available = 1
                elif len(rows_out) == 0:
                    data_available = 0

                await publisher.send_json(json)
            except Exception as e:
                print(e)
        await asyncio.sleep(0.01)  # Um breve delay para não sobrecarregar o loop

async def main():
    print("Both services are running indefinitely. Press Ctrl+C to stop.")
    await asyncio.gather(
        server(),
        publisher()
    )

if __name__ == "__main__":
    asyncio.run(main())
