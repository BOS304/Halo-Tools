using Airyz;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace HaloTools
{
    public partial class MainForm : Form
    {
        public MainForm()
        {
            InitializeComponent();
        }

        private void button1_Click(object sender, EventArgs e)
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
