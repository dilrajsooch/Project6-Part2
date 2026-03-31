using System.Net.Sockets;
using AircraftFuelClient;

/// <summary>
/// Entry point for the Aircraft Fuel Monitoring Client.
/// Reads telemetry from a file and streams it to the server over TCP.
/// </summary>
class Program
{
    static void Main(string[] args)
    {
        string serverIp = "127.0.0.1";
        int port = 5000;
        string telemetryFilePath = "";

        // Parse arguments
        if (args.Length == 0)
        {
            // Use defaults: try to find first .txt file in current directory
            string[] txtFiles = Directory.GetFiles(Directory.GetCurrentDirectory(), "*.txt");
            if (txtFiles.Length > 0)
            {
                telemetryFilePath = txtFiles[0];
                Console.WriteLine($"No arguments provided. Using defaults:");
                Console.WriteLine($"  Server: {serverIp}:{port}");
                Console.WriteLine($"  Telemetry file: {telemetryFilePath}");
            }
            else
            {
                Console.WriteLine("Usage: Client <serverIp> <port> <telemetryFilePath>");
                Console.WriteLine("       Client [defaults: 127.0.0.1 5000 <first .txt in current dir>]");
                Console.WriteLine("Error: No .txt telemetry file found in current directory.");
                return;
            }
        }
        else if (args.Length == 3)
        {
            serverIp = args[0];
            if (!int.TryParse(args[1], out port) || port < 1 || port > 65535)
            {
                Console.WriteLine($"Error: Invalid port number '{args[1]}'. Must be between 1 and 65535.");
                return;
            }
            telemetryFilePath = args[2];
        }
        else
        {
            Console.WriteLine("Usage: Client <serverIp> <port> <telemetryFilePath>");
            Console.WriteLine("       Client  (no args: uses defaults 127.0.0.1, 5000, first .txt in current dir)");
            return;
        }

        // Generate unique airplane ID
        Random rng = new Random();
        string airplaneId = $"AC-{rng.Next(1000, 9999)}";

        Console.WriteLine($"Aircraft ID : {airplaneId}");
        Console.WriteLine($"Server      : {serverIp}:{port}");
        Console.WriteLine($"Data file   : {telemetryFilePath}");
        Console.WriteLine();

        TelemetryFileReader fileReader = new TelemetryFileReader();
        PacketBuilder packetBuilder = new PacketBuilder();
        TcpTransmitter transmitter = new TcpTransmitter();

        try
        {
            // Open telemetry file
            fileReader.Open(telemetryFilePath);
            Console.WriteLine("Telemetry file opened successfully.");

            // Connect to server
            transmitter.Connect(serverIp, port);
            Console.WriteLine($"Connected to server at {serverIp}:{port}.");
            Console.WriteLine("Streaming telemetry data...");
            Console.WriteLine();

            int packetCount = 0;
            TelemetryDataPoint? dataPoint;

            while ((dataPoint = fileReader.ReadNext()) != null)
            {
                string packet = packetBuilder.BuildPacket(airplaneId, dataPoint);
                transmitter.Send(packet);
                packetCount++;

                if (packetCount % 100 == 0)
                    Console.WriteLine($"  Sent {packetCount} packets...");
            }

            Console.WriteLine();
            Console.WriteLine($"Transmission complete. Total packets sent: {packetCount}");
            Console.WriteLine("Disconnecting from server...");

            transmitter.Disconnect();
            Console.WriteLine("Disconnected. Flight data upload finished.");
        }
        catch (FileNotFoundException ex)
        {
            Console.WriteLine($"Error: Telemetry file not found — {ex.Message}");
            Console.WriteLine($"Check that the file exists: '{telemetryFilePath}'");
        }
        catch (SocketException ex)
        {
            Console.WriteLine($"Error: Could not connect to server at {serverIp}:{port}.");
            Console.WriteLine($"Details: {ex.Message}");
            Console.WriteLine("Make sure the server is running and the address/port are correct.");
        }
        catch (InvalidDataException ex)
        {
            Console.WriteLine($"Error: Invalid telemetry file format — {ex.Message}");
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Unexpected error: {ex.GetType().Name}: {ex.Message}");
        }
        finally
        {
            fileReader.Close();
            transmitter.Dispose();
        }
    }
}
