import zmq.asyncio
import asyncio
import json
import serial
import sys
import time

def hand_type(line: str,ser: serial.Serial):
    for l in line:
        time.sleep(0.07)
        ser.write(l.encode())


if sys.platform == 'win32':
    asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())

ser = None

async def server():
    global ser
    context = zmq.asyncio.Context()
    socket = context.socket(zmq.REP)
    socket.bind("tcp://127.0.0.1:5560")

    while True:
        try:
            message = await socket.recv()  # Assíncrono, não bloqueante
            data = json.loads(message.decode('utf-8'))
            print(data)
            response = {}

            if data["cmd"] == "connect":
                try:
                    ser = serial.Serial(data["port"], data["baud"])
                except Exception as e:
                    print(e)
                    response = {"status": "not connected"}
                else:
                    response = {"status": "connected"}

            elif data["cmd"] == "disconnect":
                try:
                    ser.close()
                except Exception as e:
                    print(e)
                    response = {"error": "no connection to close"}
                else:
                    response = {"status": "not connected"}

            elif data["cmd"] == "set-channels":
                try:
                    channels = ','.join([str(ch) for ch in data["channels"]])
                    match data["option"]:
                        case "mode" | "level":
                            values = ','.join([str(v) for v in data["value"]])
                            hand_type(f"SET {data['option'].upper()} ({channels}) [{values}];",ser)
                        case "list":
                            hand_type(f"SET LIST ({channels}) {data['levels']}{data['time']};",ser)
                        case "start"|"stop":
                            hand_type(f"{data['option']} LIST ({channels});",ser)
                except Exception as e:
                    print(e)
                    response = {"error": "invalid command"}
                else:
                    response = {"status" : "command sended"}

        except Exception as e:
            print(e)
            response = {"error": "invalid json"}
        finally:
            await socket.send_json(response)  # Assíncrono, não bloqueante

async def publisher():
    global ser
    context = zmq.asyncio.Context()
    publisher = context.socket(zmq.PUB)
    publisher.bind("tcp://127.0.0.1:5561")
    topic = "readings"
    
    while True:
        if ser:
            try:
                reading = ser.readline().decode()
                t, v1, v2, i1, i2, m1, m2, lvl1, lvl2 = [el for el in reading.split(',')]
                await publisher.send_json({
                    "time": int(t),
                    "V1": float(v1),
                    "I1": float(i1),
                    "V2": float(v2),
                    "I2": float(i2),
                    "M1": m1,
                    "M2": m2,
                    "LVL1": int(lvl1),
                    "LVL2": int(lvl2)
                })
                print(reading)
            except Exception as e:
                print(e)
        await asyncio.sleep(0.1)  # Um breve delay para não sobrecarregar o loop

async def main():
    print("Both services are running indefinitely. Press Ctrl+C to stop.")
    await asyncio.gather(
        server(),
        publisher()
    )

if __name__ == "__main__":
    asyncio.run(main())
