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

        public AccountForm(EmailAccount a)
        {
            InitializeComponent();

            textBoxUsername.Text = a.Username;
            textBoxPassword.Text = a.Password;
            textBoxNickname.Text = a.Nickname;
            textBoxServer.Text = a.Server;
            numericUpDownPort.Value = a.Port;
            checkBoxSsl.Checked = a.EnableSsl;
        }

        public EmailAccount GetAccount()
        {
            EmailAccount a = new EmailAccount();
            a.Username = textBoxUsername.Text;
            a.Password = textBoxPassword.Text;
            a.Nickname = textBoxNickname.Text;

            a.Server = textBoxServer.Text;
            a.Port = (int)numericUpDownPort.Value;
            a.EnableSsl = checkBoxSsl.Checked;
            return a;
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

        public bool EnabledSsl()
        {
            return checkBoxSsl.Checked;
        }

    }
}