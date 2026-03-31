# Aircraft Fleet Fuel Consumption Monitor

A real-time distributed client/server system that monitors aircraft fleet fuel consumption using TCP socket communication. Built in C++ with Winsock2.

---

## Requirements

- Windows 10 or later
- [Visual Studio 2022](https://visualstudio.microsoft.com/) (Community or Enterprise) with the **Desktop development with C++** workload installed
- [CMake 3.16+](https://cmake.org/download/) (included with Visual Studio or install separately)

---

## Project Structure

```
Project6-Part2/
  CMakeLists.txt
  Server/
    main.cpp
    ClientHandler.h / .cpp
    FleetDataManager.h / .cpp
    AirplaneRecord.h
    FuelCalculator.h / .cpp
    Logger.h / .cpp
    TelemetryDataPoint.h
  Client/
    main.cpp
    TelemetryFileReader.h / .cpp
    PacketBuilder.h / .cpp
    TcpTransmitter.h / .cpp
    TelemetryDataPoint.h
    katl-kefd-B737-700.txt
    Telem_2023_3_12 14_56_40.txt
    Telem_2023_3_12 16_26_4.txt
    Telem_czba-cykf-pa28-w2_2023_3_1 12_31_27.txt
```

---

## Building the Project

### Option A — Visual Studio (Recommended)

1. Open **Visual Studio 2022**
2. Click **Open a local folder** and select the `Project6-Part2` folder
3. Visual Studio will automatically detect the `CMakeLists.txt` and configure the project
4. In the toolbar, set the build configuration to **Release**
5. Go to **Build → Build All** (`Ctrl+Shift+B`)
6. Executables will be output to a path like:
   ```
   out\build\x64-Release\Server\Server.exe
   out\build\x64-Release\Client\Client.exe
   ```

### Option B — Command Line (Developer Command Prompt)

Open **Developer Command Prompt for VS 2022** and run:

```bat
cd Project6-Part2
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

Executables will be in:
```
build\Server\Release\Server.exe
build\Client\Release\Client.exe
```

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

> **Note:** Use the actual IP address of the server PC when running across multiple machines. Never use `127.0.0.1` across a network.

#### Available Telemetry Files

| File | Aircraft | Duration |
|------|----------|----------|
| `katl-kefd-B737-700.txt` | Boeing 737-700 | ~3 hours |
| `Telem_2023_3_12 14_56_40.txt` | Small aircraft | ~1 hour |
| `Telem_2023_3_12 16_26_4.txt` | Small aircraft | ~30 min |
| `Telem_czba-cykf-pa28-w2_2023_3_1 12_31_27.txt` | PA28 | ~46 min |

---

## Running Multiple Clients (Load Testing)

Use the provided batch script to launch multiple clients at once. Create a file called `LoadTest_Batch.bat`:

```bat
@echo off
SET /A "index = 1"
SET /A "count = 25"
:while
if %index% leq %count% (
    START /MIN Client.exe 192.168.1.50 5000 katl-kefd-B737-700.txt
    SET /A index = %index% + 1
    @echo %index%
    goto :while
)
```

Change `count` to control how many clients launch. Run the script multiple times to increase load.

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

Look for the **IPv4 Address** under your active network adapter (e.g., `192.168.1.50`). Use this address in all client commands.
