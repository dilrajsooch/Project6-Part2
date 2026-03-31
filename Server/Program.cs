using System.Net;
using System.Net.Sockets;

namespace AircraftFuelServer;

/// <summary>
/// Entry point for the Aircraft Fuel Monitoring Server.
/// Listens for TCP connections from aircraft telemetry clients
/// and manages fleet-wide fuel consumption data.
/// </summary>
class Program
{
    private static int _activeConnections = 0;

    static void Main(string[] args)
    {
        int port = 5000;

        if (args.Length >= 1)
        {
            if (!int.TryParse(args[0], out port) || port < 1 || port > 65535)
            {
                Console.WriteLine($"Invalid port '{args[0]}'. Using default port 5000.");
                port = 5000;
            }
        }

        Logger.Log($"Aircraft Fuel Monitoring Server starting on port {port}...");

        TcpListener? listener = null;
        try
        {
            listener = new TcpListener(IPAddress.Any, port);
            listener.Start();
            Logger.Log($"Server listening on 0.0.0.0:{port}. Waiting for connections...");

            FleetDataManager fleetManager = new FleetDataManager();
            AcceptLoop(listener, fleetManager);
        }
        catch (SocketException ex)
        {
            Logger.Log($"[Server] SocketException during startup or accept: {ex.Message}");
        }
        finally
        {
            listener?.Stop();
            Logger.Log("Server shut down.");
        }
    }

    /// <summary>
    /// Continuously accepts incoming TCP client connections and spawns a handler thread for each.
    /// </summary>
    static void AcceptLoop(TcpListener listener, FleetDataManager fleetManager)
    {
        while (true)
        {
            TcpClient client;
            try
            {
                client = listener.AcceptTcpClient();
            }
            catch (SocketException ex)
            {
                Logger.Log($"[Server] Accept loop ending: {ex.Message}");
                break;
            }

            int count = Interlocked.Increment(ref _activeConnections);
            Logger.Log($"New connection accepted. Active connections: {count}");

            SpawnHandler(client, fleetManager);
        }
    }

    /// <summary>
    /// Creates and starts a new background thread to handle the given TCP client.
    /// Decrements the active connection count when the thread finishes.
    /// </summary>
    static void SpawnHandler(TcpClient client, FleetDataManager fleetManager)
    {
        Thread handlerThread = new Thread(() =>
        {
            try
            {
                ClientHandler.HandleClient(client, fleetManager);
            }
            finally
            {
                int count = Interlocked.Decrement(ref _activeConnections);
                Logger.Log($"Handler thread finished. Active connections: {count}");
            }
        });

        handlerThread.IsBackground = true;
        handlerThread.Start();
    }
}
