using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO.Ports;
using Crc;


namespace RelaisSwitchBox
{
    public partial class Form1 : Form
    {

        public Form1()
        {
            InitializeComponent();
        }

        bool SerialState = false;

        SerialPort port = new SerialPort();

        private void Form1_Load(object sender, EventArgs e)
        {
            this.Text += " Ver.1.0.2909.0702 by J.H - Elec.";

            foreach( string port in System.IO.Ports.SerialPort.GetPortNames())
            {
                comboBox1.Items.Add(port);
            }
            comboBox1.SelectedItem = "COM1";
            comboBox1.Select();

            comboBox2.Items.Add(300);
            comboBox2.Items.Add(600);
            comboBox2.Items.Add(1200);
            comboBox2.Items.Add(2400);
            comboBox2.Items.Add(9600);
            comboBox2.Items.Add(14400);
            comboBox2.Items.Add(19200);
            comboBox2.Items.Add(38400);
            comboBox2.Items.Add(57600);
            comboBox2.Items.Add(115200);
            comboBox2.SelectedIndex = 6;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if( port.IsOpen )
            {
                button1.Text = "Öffnen";
                port.Close();
                return;
            }

            port.BaudRate = Convert.ToInt32(comboBox2.SelectedItem);
            port.PortName = comboBox1.SelectedItem.ToString();

            try
            {
                port.Open();
            }
            catch
            {
                MessageBox.Show("Port konnte nicht geöffnet werden!", "RelaisSwitchBox", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            

            if( port.IsOpen )
            {
                button1.Text = "Schließen";
            }

        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            string rx = null;
            if ( port.IsOpen )
            {
                rx = port.ReadExisting();
                richTextBox1.AppendText(rx);
            }
        }


        private void numericUpDown1_ValueChanged(object sender, EventArgs e)
        {
            Crc8_CCITT calcCrc = new Crc8_CCITT();

            if( !port.IsOpen)
            {
                return;
            }

            string cmd = "-k:" + numericUpDown1.Value.ToString() + "," + "2" + ";";

            if( checkBox2.Checked == true )
            {
                cmd += "#" + calcCrc.cmdCrc8StrCCITT(cmd,0x07,0).ToString();
            }

            if ( checkBox1.Checked == true )
            {
                MessageBox.Show(cmd, "Debug", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
            port.WriteLine( cmd );
        }

        private void richTextBox1_TextChanged(object sender, EventArgs e)
        {
            richTextBox1.SelectionStart = richTextBox1.Text.Length;
            richTextBox1.ScrollToCaret();
        }

        private void richTextBox1_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            richTextBox1.Clear();
        }
    }
}
