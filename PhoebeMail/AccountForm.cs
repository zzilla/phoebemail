using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace PhoebeMail
{
    public partial class AccountForm : Form
    {
        public AccountForm()
        {
            InitializeComponent();
        }

        public AccountForm(string[] parts)
        {
            InitializeComponent();
            int i = 0;
            textBoxUsername.Text = parts[i++];
            textBoxPassword.Text = parts[i++];
            textBoxNickname.Text = parts[i++];
            textBoxServer.Text = parts[i++];
            numericUpDownPort.Value = (decimal)int.Parse(parts[i++]);
            checkBoxSsl.Checked = bool.Parse(parts[i++]);
        }

        public string GetUsername()
        {
            return textBoxUsername.Text;
        }

        public string GetPassword()
        {
            return textBoxPassword.Text;
        }

        public string GetNickname()
        {
            return textBoxNickname.Text;
        }

        public string GetServer()
        {
            return textBoxServer.Text;
        }

        public int GetPort()
        {
            return (int)numericUpDownPort.Value;
        }

        public bool SslEnabled()
        {
            return checkBoxSsl.Checked;
        }

    }
}