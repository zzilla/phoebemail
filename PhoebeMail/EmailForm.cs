using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace PhoebeMail
{
    public partial class EmailForm : Form
    {
        public EmailForm()
        {
            InitializeComponent();
        }

        public string GetAddress()
        {
            return this.textBoxEmail.Text;
        }

        public void SetAddress(string address)
        {
            this.textBoxEmail.Text = address;
        }
    }
}