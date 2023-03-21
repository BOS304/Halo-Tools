using Airyz;
using System;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Interop;

namespace HaloTools
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {

        public AiryzMemory halo;

        public MainWindow()
        {
            InitializeComponent();
        }

        private void CloseButton_Click(object sender, RoutedEventArgs e)
        {
            Environment.Exit(0);
        }


        [DllImport("user32.dll")]
        static extern int SendMessage(IntPtr hWnd, uint msg, IntPtr wParam, IntPtr lParam);
        private void MinimiseButton_Click(object sender, RoutedEventArgs e)
        {
            SendMessage(new WindowInteropHelper(this).Handle, 0x0112, (IntPtr)0xF020, IntPtr.Zero);
        }

        private void InjectButton_Click(object sender, RoutedEventArgs e)
        {
            if (Process.GetProcessesByName("MCC-Win64-Shipping").Length > 0)
            {
                AiryzMemory mem = new AiryzMemory("MCC-Win64-Shipping");
                FileInfo info = new FileInfo("HaloDirector.dll");
                if (info.Exists)
                {
                    mem.InjectDLL64(info.FullName);
                }
                else
                {
                    MessageBox.Show("DLL File not found: \"HaloDirector.dll\"", "Error:");
                }
            }
            else
            {
                MessageBox.Show("MCC Process not found", "Error:");
            }
        }
    }
}
