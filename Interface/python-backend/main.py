import zmq
import json
import serial

context = zmq.Context()
socket = context.socket(zmq.REP)
socket.bind("tcp://127.0.0.1:5560")

print("Server is running...")

ser = None

while True:
    # Recebe a mensagem
    message = socket.recv()
    # Decodifica a mensagem para string e carrega como JSON
    data = json.loads(message.decode('utf-8'))
    response = {}
    
    try:
        match data["cmd"]:
            case "connect":
                try:
                    ser = serial.Serial(data["comport"],data["baud"])
                except:
                    response = {"status":"not connected"}
                else:
                    response = {"status":"connected"}
            
            case "set-channels":
                pass
        
    except:
        response = {"error":"invalid cmd"}
    
    # Envia resposta
    socket.send_json(response)