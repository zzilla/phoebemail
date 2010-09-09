using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Net.Mail;
using System.Net;
using EasyMail.Properties;

namespace EasyMail
{
    public partial class MainForm : Form
    {
        private static readonly String m_caption = "EasyMail";

        public static void ShowInfomation(String message)
        {
            MessageBox.Show(message, m_caption + " Information", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        public static void ShowWarning(String message)
        {
            MessageBox.Show(message, m_caption + " Warning", MessageBoxButtons.OK, MessageBoxIcon.Warning);
        }

        public static void ShowError(String message)
        {
            MessageBox.Show(message, m_caption + " Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
        }

        public static DialogResult ShowQuestion(String message)
        {
            return MessageBox.Show(message, m_caption + " Question", MessageBoxButtons.YesNo, MessageBoxIcon.Question);
        }

        private SmtpClient m_client = new SmtpClient();

        public MainForm()
        {
            InitializeComponent();

            m_client.DeliveryMethod = SmtpDeliveryMethod.Network;

            m_client.Host = Settings.Default.mailserver;
            m_client.Port = Settings.Default.port;
            m_client.EnableSsl = true;
            string mailUsername = Settings.Default.username;
            string mailPassword = Settings.Default.password;
            m_client.Credentials = new NetworkCredential(mailUsername, mailPassword);

            textBoxUsername.Text = Settings.Default.username;
            textBoxPassword.Text = Settings.Default.password;
            textBoxMailServer.Text = Settings.Default.mailserver;
            textBoxNickname.Text = Settings.Default.nickname;
        }

        public void SendMail(string to, string subject, string body)
        {
            MailMessage mail = new MailMessage();
            mail.From = new MailAddress(Settings.Default.username, Settings.Default.nickname);
            mail.To.Add(new MailAddress(to));
            mail.SubjectEncoding = Encoding.GetEncoding("GB2312");
            mail.BodyEncoding = Encoding.GetEncoding("GB2312");
            mail.Subject = subject;
            mail.Body = body;
            m_client.Send(mail);
        }

        private void buttonOk_Click(object sender, EventArgs e)
        {
            SendMail(textBoxReceipient.Text, textBoxSubject.Text, richTextBoxBody.Text);

            ShowInfomation("send mail ok!");
        }

        private void buttonQuit_Click(object sender, EventArgs e)
        {
            this.Close();
        }
    }
}