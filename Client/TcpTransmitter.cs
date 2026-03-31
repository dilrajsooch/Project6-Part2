using System.Net.Sockets;

namespace AircraftFuelClient;

/// <summary>
/// Manages the TCP connection to the server and handles packet transmission.
/// </summary>
public class TcpTransmitter : IDisposable
{
    private TcpClient? _client;
    private StreamWriter? _writer;
    private bool _disposed = false;

    /// <summary>
    /// Returns true if currently connected to the server.
    /// </summary>
    public bool IsConnected => _client != null && _client.Connected && !_disposed;

    /// <summary>
    /// Establishes a TCP connection to the specified server.
    /// </summary>
    public void Connect(string serverIp, int port)
    {
        _client = new TcpClient();
        _client.Connect(serverIp, port);
        NetworkStream stream = _client.GetStream();
        _writer = new StreamWriter(stream) { AutoFlush = false };
    }

    /// <summary>
    /// Sends a packet as a newline-terminated line to the server.
    /// </summary>
    public void Send(string packet)
    {
        if (_writer == null || !IsConnected)
            throw new InvalidOperationException("Not connected. Call Connect() first.");

        _writer.WriteLine(packet);
        _writer.Flush();
    }

    /// <summary>
    /// Flushes any buffered data and closes the connection gracefully.
    /// </summary>
    public void Disconnect()
    {
        if (_writer != null)
        {
            try
            {
                _writer.Flush();
            }
            catch { /* ignore flush errors on disconnect */ }
        }

        Dispose();
    }

    public void Dispose()
    {
        if (_disposed)
            return;

        _disposed = true;
        _writer?.Dispose();
        _client?.Dispose();
        _writer = null;
        _client = null;
    }
}
