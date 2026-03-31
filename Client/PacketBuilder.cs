namespace AircraftFuelClient;

/// <summary>
/// Builds telemetry packets for transmission to the server.
/// </summary>
public class PacketBuilder
{
    /// <summary>
    /// Constructs a pipe-delimited telemetry packet string.
    /// Format: AIRPLANE_ID|TIMESTAMP|FUEL_REMAINING
    ///   - TIMESTAMP is ISO 8601 format (e.g. 2023-03-03T14:53:21.0000000)
    ///   - FUEL_REMAINING formatted to 6 decimal places
    /// </summary>
    public string BuildPacket(string airplaneId, TelemetryDataPoint data)
    {
        string timestamp = data.Timestamp.ToString("O"); // ISO 8601 round-trip format
        string fuel = data.FuelRemaining.ToString("F6", System.Globalization.CultureInfo.InvariantCulture);
        return $"{airplaneId}|{timestamp}|{fuel}";
    }
}
