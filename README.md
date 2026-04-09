# Aircraft Fleet Fuel Consumption Monitor

A real-time distributed client/server system that monitors aircraft fleet fuel consumption using TCP socket communication. Built in C++ with Winsock2.

---

## Requirements

- Windows 10 or later
- [Visual Studio 2022](https://visualstudio.microsoft.com/) (Community or Enterprise) with the **Desktop development with C++** workload installed

---

## Project Structure

The Server and Client are **two independent Visual Studio projects**. Each has its own `.sln` and can be opened, built, and deployed separately.

```
Project6-Part2/
  Server/
    Server.sln
    Server.vcxproj
    main.cpp
    ClientHandler.h / .cpp
    FleetDataManager.h / .cpp
    AirplaneRecord.h
    FuelCalculator.h / .cpp
    Logger.h / .cpp
    TelemetryDataPoint.h
  Client/
    Client.sln
    Client.vcxproj
    main.cpp
    TelemetryFileReader.h / .cpp
    PacketBuilder.h / .cpp
    TcpTransmitter.h / .cpp
    TelemetryDataPoint.h
    LoadTest_Batch.bat
    EnduranceTest_Batch.bat
    katl-kefd-B737-700.txt
    Telem_2023_3_12 14_56_40.txt
    Telem_2023_3_12 16_26_4.txt
    Telem_czba-cykf-pa28-w2_2023_3_1 12_31_27.txt
```

---

## Building

### Server

1. Open `Server/Server.sln` in Visual Studio 2022
2. Set configuration to **Release**
3. Build the solution (`Ctrl+Shift+B`)
4. Output: `Server/Release/Server.exe`

### Client

1. Open `Client/Client.sln` in Visual Studio 2022
2. Set configuration to **Release**
3. Build the solution (`Ctrl+Shift+B`)
4. Output: `Client/Release/Client.exe`

---

## Running the Application

### 1. Start the Server

On the server PC, open a Command Prompt and run:

```bat
Server.exe
```

By default the server listens on port **5000**. To use a different port:

```bat
Server.exe 6000
```

The server will display active connections and fleet fuel summaries in real time. All events are also logged to `server_log.txt`.

---

### 2. Start a Client

On a client PC, open a Command Prompt, navigate to the folder containing `Client.exe` and the telemetry `.txt` files, then run:

```bat
Client.exe <serverIP> <port> <telemetryFile>
```

**Example (same machine):**
```bat
Client.exe 127.0.0.1 5000 katl-kefd-B737-700.txt
```

**Example (across network):**
```bat
Client.exe 192.168.1.50 5000 "Telem_2023_3_12 14_56_40.txt"
```

> **Note:** The IP address is a command line argument — never hardcode it. Use the actual IP address of the server PC when running across multiple machines.

#### Available Telemetry Files

| File | Aircraft | Duration |
|------|----------|----------|
| `katl-kefd-B737-700.txt` | Boeing 737-700 | ~3 hours |
| `Telem_2023_3_12 14_56_40.txt` | Small aircraft | ~1 hour |
| `Telem_2023_3_12 16_26_4.txt` | Small aircraft | ~30 min |
| `Telem_czba-cykf-pa28-w2_2023_3_1 12_31_27.txt` | PA28 | ~46 min |

---

## Performance Testing

Copy `Client.exe`, the telemetry `.txt` files, and the batch scripts to each client PC. Before running, edit the `SERVER_IP` variable at the top of each `.bat` file to match the server PC's IP address.

### Load Testing

Use `LoadTest_Batch.bat` to spawn multiple clients at once. Edit the variables at the top of the file:

```bat
SET SERVER_IP=192.168.1.100
SET SERVER_PORT=5000
SET DATA_FILE=katl-kefd-B737-700.txt
SET /A "count = 25"
```

Change `count` to control how many clients launch. Run the script multiple times to increase load.

### Endurance/Spike Testing

Use `EnduranceTest_Batch.bat` to continuously spawn waves of clients. It will keep spawning `count` clients every `timeout` seconds until manually terminated. Edit the variables at the top:

```bat
SET SERVER_IP=192.168.1.100
SET SERVER_PORT=5000
SET DATA_FILE=katl-kefd-B737-700.txt
SET /A "count = 100"
timeout 250
```

---

## Firewall Note

If clients cannot connect to the server, make sure **port 5000 (or your chosen port) is open** on the server PC's Windows Firewall:

1. Open **Windows Defender Firewall with Advanced Security**
2. Click **Inbound Rules → New Rule**
3. Select **Port**, click Next
4. Enter port `5000`, click Next through the rest and save

---

## Finding Your Server IP Address

On the server PC, open Command Prompt and run:

```bat
ipconfig
```

Look for the **IPv4 Address** under your active network adapter (e.g., `192.168.1.50`). Use this address in all client commands and batch scripts.
