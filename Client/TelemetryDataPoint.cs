namespace AircraftFuelClient;

/// <summary>
/// Simple DTO representing a single telemetry reading parsed from the data file.
/// </summary>
public class TelemetryDataPoint
{
    public DateTime Timestamp { get; set; }
    public double FuelRemaining { get; set; }
}
