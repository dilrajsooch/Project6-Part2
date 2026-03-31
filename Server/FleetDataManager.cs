using System.Collections.Concurrent;
using System.Text;

namespace AircraftFuelServer;

/// <summary>
/// Thread-safe manager for all aircraft records in the fleet.
/// Handles incoming telemetry, computes running fuel averages,
/// and finalizes flight records on disconnect.
/// Periodically prints a fleet summary every 5 seconds.
/// </summary>
public class FleetDataManager
{
    private readonly ConcurrentDictionary<string, AirplaneRecord> _records =
        new ConcurrentDictionary<string, AirplaneRecord>(StringComparer.OrdinalIgnoreCase);

    private readonly Timer _summaryTimer;

    public FleetDataManager()
    {
        // Print fleet summary to console every 5 seconds
        _summaryTimer = new Timer(_ => PrintFleetSummary(), null,
            TimeSpan.FromSeconds(5), TimeSpan.FromSeconds(5));
    }

    /// <summary>
    /// Returns an existing record for the given airplane ID, or creates a new one.
    /// </summary>
    public AirplaneRecord GetOrCreateRecord(string id)
    {
        return _records.GetOrAdd(id, key => new AirplaneRecord(key));
    }

    /// <summary>
    /// Records a new telemetry data point for the specified airplane.
    /// Computes the instantaneous fuel burn rate and updates the running average.
    /// Thread-safe: locks on the individual AirplaneRecord.
    /// </summary>
    public void UpdateFuelReading(string id, DateTime time, double fuelRemaining)
    {
        AirplaneRecord record = GetOrCreateRecord(id);

        lock (record)
        {
            if (!record.IsFlightActive)
            {
                record.IsFlightActive = true;
                record.FlightStartTime = time;
                record.CurrentFlightAvg = 0.0;
                record.CurrentFlightSampleCount = 0;
                record.TotalFuelConsumed = 0.0;
                Logger.Log($"[{id}] Flight started at {time:yyyy-MM-dd HH:mm:ss}");
            }

            if (record.PreviousDataPoint != null)
            {
                TimeSpan elapsed = time - record.PreviousDataPoint.Timestamp;
                double instantRate = FuelCalculator.CalculateInstantRate(
                    record.PreviousDataPoint.FuelRemaining, fuelRemaining, elapsed);

                if (instantRate >= 0 && elapsed.TotalSeconds > 0)
                {
                    record.CurrentFlightSampleCount++;
                    record.CurrentFlightAvg = FuelCalculator.UpdateRunningAverage(
                        record.CurrentFlightAvg, instantRate, record.CurrentFlightSampleCount);

                    double consumed = record.PreviousDataPoint.FuelRemaining - fuelRemaining;
                    if (consumed > 0)
                        record.TotalFuelConsumed += consumed;
                }
            }

            record.PreviousDataPoint = new TelemetryDataPoint
            {
                Timestamp = time,
                FuelRemaining = fuelRemaining
            };
        }
    }

    /// <summary>
    /// Finalizes the current flight for the given airplane.
    /// Saves the flight average to the completed flights list and resets accumulators.
    /// Thread-safe: locks on the individual AirplaneRecord.
    /// </summary>
    public void CompleteFlight(string id)
    {
        if (!_records.TryGetValue(id, out AirplaneRecord? record))
            return;

        lock (record)
        {
            if (!record.IsFlightActive)
                return;

            double flightAvg = FuelCalculator.ComputeFlightAverage(record);
            record.CompletedFlightAverages.Add(flightAvg);

            Logger.Log($"[{id}] Flight completed. " +
                       $"Avg burn rate: {flightAvg:F6} gal/s, " +
                       $"Total consumed: {record.TotalFuelConsumed:F2} gal, " +
                       $"Duration: {(record.PreviousDataPoint?.Timestamp - record.FlightStartTime)?.TotalMinutes:F1} min");

            // Reset accumulators for next flight
            record.IsFlightActive = false;
            record.CurrentFlightAvg = 0.0;
            record.CurrentFlightSampleCount = 0;
            record.PreviousDataPoint = null;
            record.TotalFuelConsumed = 0.0;
            record.FlightStartTime = DateTime.MinValue;
        }
    }

    /// <summary>
    /// Builds and returns a summary string of all airplane records in the fleet.
    /// </summary>
    public string GetFleetSummary()
    {
        if (_records.IsEmpty)
            return "Fleet summary: No aircraft tracked yet.";

        var sb = new StringBuilder();
        sb.AppendLine("=== Fleet Summary ===");

        foreach (var kvp in _records)
        {
            AirplaneRecord record = kvp.Value;
            lock (record)
            {
                sb.AppendLine($"  Aircraft: {record.AirplaneId}");
                sb.AppendLine($"    Status: {(record.IsFlightActive ? "IN FLIGHT" : "GROUNDED")}");
                if (record.IsFlightActive)
                {
                    sb.AppendLine($"    Current avg burn: {record.CurrentFlightAvg:F6} gal/s");
                    sb.AppendLine($"    Total fuel consumed this flight: {record.TotalFuelConsumed:F2} gal");
                    sb.AppendLine($"    Samples received: {record.CurrentFlightSampleCount}");
                }

                if (record.CompletedFlightAverages.Count > 0)
                {
                    double histAvg = FuelCalculator.ComputeHistoricalAverage(record);
                    sb.AppendLine($"    Completed flights: {record.CompletedFlightAverages.Count}");
                    sb.AppendLine($"    Historical avg burn: {histAvg:F6} gal/s");
                }
            }
        }

        sb.Append("=====================");
        return sb.ToString();
    }

    private void PrintFleetSummary()
    {
        string summary = GetFleetSummary();
        Console.WriteLine(summary);
    }
}
