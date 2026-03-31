namespace AircraftFuelServer;

/// <summary>
/// Static utility class for fuel consumption calculations.
/// </summary>
public static class FuelCalculator
{
    /// <summary>
    /// Calculates instantaneous fuel consumption rate in gallons per second.
    /// Returns 0 if elapsed time is zero or negative (invalid interval).
    /// </summary>
    public static double CalculateInstantRate(double prevFuel, double currFuel, TimeSpan elapsed)
    {
        if (elapsed.TotalSeconds <= 0)
            return 0.0;

        double consumed = prevFuel - currFuel;
        return consumed / elapsed.TotalSeconds;
    }

    /// <summary>
    /// Computes a cumulative moving average given the current average,
    /// the new data rate, and the updated sample count.
    /// Formula: oldAvg + (newRate - oldAvg) / sampleCount
    /// </summary>
    public static double UpdateRunningAverage(double currentAvg, double newRate, int sampleCount)
    {
        if (sampleCount <= 0)
            return newRate;

        return currentAvg + (newRate - currentAvg) / sampleCount;
    }

    /// <summary>
    /// Returns the current flight's average fuel consumption rate from the record.
    /// </summary>
    public static double ComputeFlightAverage(AirplaneRecord record)
    {
        return record.CurrentFlightAvg;
    }

    /// <summary>
    /// Returns the historical average across all completed flights.
    /// Returns 0 if there are no completed flights.
    /// </summary>
    public static double ComputeHistoricalAverage(AirplaneRecord record)
    {
        if (record.CompletedFlightAverages.Count == 0)
            return 0.0;

        double sum = 0.0;
        foreach (double avg in record.CompletedFlightAverages)
            sum += avg;

        return sum / record.CompletedFlightAverages.Count;
    }
}
