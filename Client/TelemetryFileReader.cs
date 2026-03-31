namespace AircraftFuelClient;

/// <summary>
/// Reads telemetry data points from a CSV-style file.
///
/// Expected file format:
///   Line 1 (header+first data): FUEL TOTAL QUANTITY,D_M_YYYY HH:MM:SS,fuel_value,
///   Subsequent lines:           ,D_M_YYYY HH:MM:SS,fuel_value,
///
/// Timestamp format: D_M_YYYY HH:MM:SS
///   e.g. "3_3_2023 14:53:21" = March 3, 2023 at 14:53:21
/// </summary>
public class TelemetryFileReader
{
    private StreamReader? _reader;
    private TelemetryDataPoint? _bufferedPoint;

    /// <summary>
    /// Opens the telemetry file and parses the header line,
    /// buffering the first data point embedded in the header.
    /// </summary>
    public void Open(string filePath)
    {
        _reader = new StreamReader(filePath);

        // The header line contains both the label and the first data point:
        // Format: FUEL TOTAL QUANTITY,D_M_YYYY HH:MM:SS,fuel_value,
        string? headerLine = _reader.ReadLine();
        if (headerLine == null)
            throw new InvalidDataException("Telemetry file is empty.");

        string[] parts = headerLine.Split(',');
        // parts[0] = label (e.g. "FUEL TOTAL QUANTITY")
        // parts[1] = timestamp
        // parts[2] = fuel value
        // parts[3] = trailing empty (optional)

        if (parts.Length < 3)
            throw new InvalidDataException(
                $"Header line does not contain expected fields: '{headerLine}'");

        string timestampRaw = parts[1].Trim();
        string fuelRaw = parts[2].Trim();

        if (!string.IsNullOrEmpty(timestampRaw) && !string.IsNullOrEmpty(fuelRaw))
        {
            DateTime ts = ParseTimestamp(timestampRaw);
            if (double.TryParse(fuelRaw, System.Globalization.NumberStyles.Any,
                    System.Globalization.CultureInfo.InvariantCulture, out double fuel))
            {
                _bufferedPoint = new TelemetryDataPoint { Timestamp = ts, FuelRemaining = fuel };
            }
        }
    }

    /// <summary>
    /// Reads and returns the next telemetry data point from the file.
    /// Returns null at end of file.
    /// Skips blank or unparseable lines.
    /// </summary>
    public TelemetryDataPoint? ReadNext()
    {
        // Return the buffered first point from the header line first
        if (_bufferedPoint != null)
        {
            TelemetryDataPoint point = _bufferedPoint;
            _bufferedPoint = null;
            return point;
        }

        if (_reader == null)
            throw new InvalidOperationException("File not opened. Call Open() first.");

        while (true)
        {
            string? line = _reader.ReadLine();
            if (line == null)
                return null; // EOF

            if (string.IsNullOrWhiteSpace(line))
                continue;

            // Data lines: ,D_M_YYYY HH:MM:SS,fuel_value,
            // The first token before the comma may be blank (continuation) or label text
            string[] parts = line.Split(',');
            if (parts.Length < 3)
                continue;

            string timestampRaw = parts[1].Trim();
            string fuelRaw = parts[2].Trim();

            if (string.IsNullOrEmpty(timestampRaw) || string.IsNullOrEmpty(fuelRaw))
                continue;

            try
            {
                DateTime ts = ParseTimestamp(timestampRaw);
                if (!double.TryParse(fuelRaw, System.Globalization.NumberStyles.Any,
                        System.Globalization.CultureInfo.InvariantCulture, out double fuel))
                    continue;

                return new TelemetryDataPoint { Timestamp = ts, FuelRemaining = fuel };
            }
            catch (FormatException)
            {
                // Skip lines with unparseable timestamps
                continue;
            }
        }
    }

    /// <summary>
    /// Parses a timestamp in the format D_M_YYYY HH:MM:SS.
    /// Example: "3_3_2023 14:53:21" => March 3, 2023 14:53:21
    ///
    /// Parsing logic:
    ///   Split on '_': parts[0]=day, parts[1]=month, parts[2]="YYYY HH:MM:SS"
    ///   Split parts[2] on ' ': [0]=year, [1]="HH:MM:SS"
    /// </summary>
    public static DateTime ParseTimestamp(string raw)
    {
        // Split on underscore to separate D, M, and "YYYY HH:MM:SS"
        string[] underscoreParts = raw.Split('_');
        if (underscoreParts.Length != 3)
            throw new FormatException($"Expected D_M_YYYY HH:MM:SS format, got: '{raw}'");

        if (!int.TryParse(underscoreParts[0].Trim(), out int day))
            throw new FormatException($"Cannot parse day from: '{underscoreParts[0]}'");

        if (!int.TryParse(underscoreParts[1].Trim(), out int month))
            throw new FormatException($"Cannot parse month from: '{underscoreParts[1]}'");

        // parts[2] is "YYYY HH:MM:SS"
        string yearAndTime = underscoreParts[2].Trim();
        int spaceIndex = yearAndTime.IndexOf(' ');
        if (spaceIndex < 0)
            throw new FormatException($"Cannot find space separating year and time in: '{yearAndTime}'");

        string yearStr = yearAndTime.Substring(0, spaceIndex).Trim();
        string timeStr = yearAndTime.Substring(spaceIndex + 1).Trim();

        if (!int.TryParse(yearStr, out int year))
            throw new FormatException($"Cannot parse year from: '{yearStr}'");

        // Parse HH:MM:SS
        string[] timeParts = timeStr.Split(':');
        if (timeParts.Length != 3)
            throw new FormatException($"Expected HH:MM:SS, got: '{timeStr}'");

        if (!int.TryParse(timeParts[0], out int hour) ||
            !int.TryParse(timeParts[1], out int minute) ||
            !int.TryParse(timeParts[2], out int second))
            throw new FormatException($"Cannot parse time components from: '{timeStr}'");

        return new DateTime(year, month, day, hour, minute, second, DateTimeKind.Unspecified);
    }

    /// <summary>
    /// Closes and disposes the underlying file stream.
    /// </summary>
    public void Close()
    {
        _reader?.Dispose();
        _reader = null;
    }
}
