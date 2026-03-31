using System.Net.Sockets;

namespace AircraftFuelServer;

/// <summary>
/// Handles communication with a single connected TCP client (aircraft).
/// Reads telemetry lines, parses packets, and feeds data to FleetDataManager.
/// </summary>
public static class ClientHandler
{
    /// <summary>
    /// Main handler loop for a connected client.
    /// Reads pipe-delimited telemetry packets line by line until disconnect or EOF.
    /// On completion, finalizes the flight record in the fleet manager.
    /// </summary>
    public static void HandleClient(TcpClient client, FleetDataManager manager)
    {
        string? airplaneId = null;
        NetworkStream? stream = null;
        StreamReader? reader = null;

        try
        {
            stream = client.GetStream();
            reader = new StreamReader(stream);

            Logger.Log($"Client connected from {client.Client.RemoteEndPoint}");

            string? line;
            while ((line = reader.ReadLine()) != null)
            {
                if (string.IsNullOrWhiteSpace(line))
                    continue;

                try
                {
                    var (id, dataPoint) = ParsePacket(line);
                    airplaneId = id;
                    manager.UpdateFuelReading(id, dataPoint.Timestamp, dataPoint.FuelRemaining);
                }
                catch (FormatException ex)
                {
                    Logger.Log($"[ClientHandler] Malformed packet: '{line}' — {ex.Message}");
                }
            }

            Logger.Log($"Client {airplaneId ?? "unknown"} disconnected (EOF).");
        }
        catch (IOException ex)
        {
            Logger.Log($"[ClientHandler] IO error for {airplaneId ?? "unknown"}: {ex.Message}");
        }
        catch (SocketException ex)
        {
            Logger.Log($"[ClientHandler] Socket error for {airplaneId ?? "unknown"}: {ex.Message}");
        }
        finally
        {
            if (airplaneId != null)
                manager.CompleteFlight(airplaneId);

            reader?.Dispose();
            stream?.Dispose();
            client?.Dispose();

            Logger.Log($"Connection for {airplaneId ?? "unknown"} fully closed.");
        }
    }

    /// <summary>
    /// Parses a pipe-delimited telemetry packet of the form:
    ///   AIRPLANE_ID|TIMESTAMP|FUEL_REMAINING
    /// Returns the airplane ID and a TelemetryDataPoint.
    /// Throws FormatException if the packet is malformed.
    /// </summary>
    public static (string AirplaneId, TelemetryDataPoint DataPoint) ParsePacket(string line)
    {
        string[] parts = line.Split('|');
        if (parts.Length != 3)
            throw new FormatException($"Expected 3 pipe-delimited fields, got {parts.Length}.");

        string id = parts[0].Trim();
        if (string.IsNullOrEmpty(id))
            throw new FormatException("Airplane ID is empty.");

        if (!DateTime.TryParse(parts[1].Trim(), out DateTime timestamp))
            throw new FormatException($"Cannot parse timestamp: '{parts[1]}'.");

        if (!double.TryParse(parts[2].Trim(), System.Globalization.NumberStyles.Any,
                System.Globalization.CultureInfo.InvariantCulture, out double fuelRemaining))
            throw new FormatException($"Cannot parse fuel value: '{parts[2]}'.");

        return (id, new TelemetryDataPoint { Timestamp = timestamp, FuelRemaining = fuelRemaining });
    }
}
