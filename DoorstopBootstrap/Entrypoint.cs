namespace Doorstop;

public static class Entrypoint
{
    private static int _once;

    public static void Start()
    {
        if (System.Threading.Interlocked.Exchange(ref _once, 1) != 0)
            return;

        string dll = System.IO.Path.Combine(System.AppContext.BaseDirectory, "Lizard.dll");
        if (System.IO.File.Exists(dll))
            System.Runtime.InteropServices.NativeLibrary.Load(dll);
    }
}
