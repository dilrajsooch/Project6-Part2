namespace AircraftFuelServer;

/// <summary>
/// Holds all state for a single aircraft's telemetry tracking,
/// including current flight accumulators and historical flight averages.
/// </summary>
public class AirplaneRecord
{
    public string AirplaneId { get; set; }
    public double CurrentFlightAvg { get; set; }
    public int CurrentFlightSampleCount { get; set; }
    public TelemetryDataPoint? PreviousDataPoint { get; set; }
    public List<double> CompletedFlightAverages { get; set; }
    public bool IsFlightActive { get; set; }
    public DateTime FlightStartTime { get; set; }
    public double TotalFuelConsumed { get; set; }

    public AirplaneRecord(string airplaneId)
    {
        AirplaneId = airplaneId;
        CurrentFlightAvg = 0.0;
        CurrentFlightSampleCount = 0;
        PreviousDataPoint = null;
        CompletedFlightAverages = new List<double>();
        IsFlightActive = false;
        FlightStartTime = DateTime.MinValue;
        TotalFuelConsumed = 0.0;
    }
}
