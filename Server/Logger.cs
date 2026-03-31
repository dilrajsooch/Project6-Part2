namespace AircraftFuelServer;

/// <summary>
/// Simple static logger that writes timestamped messages to both
/// the console and a persistent log file (server_log.txt).
/// </summary>
public static class Logger
{
    private static readonly string LogFilePath = "server_log.txt";
    private static readonly object FileLock = new object();

    /// <summary>
    /// Writes a timestamped log message to the console and appends it to server_log.txt.
    /// </summary>
    public static void Log(string message)
    {
        string timestamped = $"[{DateTime.Now:yyyy-MM-dd HH:mm:ss}] {message}";
        Console.WriteLine(timestamped);

        lock (FileLock)
        {
            try
            {
                File.AppendAllText(LogFilePath, timestamped + Environment.NewLine);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[Logger] Failed to write to log file: {ex.Message}");
            }
        }
    }
}
