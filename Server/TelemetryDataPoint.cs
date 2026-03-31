namespace AircraftFuelServer;

/// <summary>
/// Simple DTO representing a single telemetry reading from an aircraft.
/// </summary>
public class TelemetryDataPoint
{
    public DateTime Timestamp { get; set; }
    public double FuelRemaining { get; set; }
}
