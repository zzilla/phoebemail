using System;
using System.Collections.Generic;
using System.Text;
using System.Net.Mail;
using System.Net;

namespace PhoebeMail
{
    public class EmailAccount
    {
        private string m_username = "why11221220@gmail.com";

        public string Username
        {
            get { return m_username; }
            set { m_username = value; }
        }

        private string m_password = "******";

        public string Password
        {
            get { return m_password; }
            set { m_password = value; }
        }

        public string DisplayPassword()
        {
            StringBuilder b = new StringBuilder();
            for (int i = 0; i < m_password.Length; ++i )
            {
                b.Append("*");
            }
            return b.ToString();
        }

        private string m_nickname = "Phoebe";

        public string Nickname
        {
            get { return m_nickname; }
            set { m_nickname = value; }
        }

        private string m_server = "smtp.gmail.com";

        public string Server
        {
            get { return m_server; }
            set { m_server = value; }
        }

        private int m_port = 587;

        public int Port
        {
            get { return m_port; }
            set { m_port = value; }
        }

        private bool m_enableSsl = true;

        public bool EnableSsl
        {
            get { return m_enableSsl; }
            set { m_enableSsl = value; }
        }

        public EmailAccount()
        {

        }

        public EmailAccount(string[] parts)
        {
            int i = 0;
            m_username = parts[i++];
            m_password = Decrypt(parts[i++]);
            m_nickname = parts[i++];

            m_server = parts[i++];
            m_port = int.Parse(parts[i++]);
            m_enableSsl = bool.Parse(parts[i++]);
        }

        private string Encrypt(string text)
        {
            return Convert.ToBase64String(Encoding.Unicode.GetBytes(text));
        }

        private string Decrypt(string text)
        {
            return Encoding.Unicode.GetString(Convert.FromBase64String(text));
        }

        public override string ToString()
        {
            StringBuilder sb = new StringBuilder();

            sb.AppendFormat("{0},{1},{2},{3},{4},{5}\n",
                   m_username, Encrypt(m_password), m_nickname, m_server, m_port, m_enableSsl);

            return sb.ToString();
        }

        private SmtpClient smtpClient = new SmtpClient();
        private MailMessage mailMessage = new MailMessage();

        public void Init(string subject, string body)
        {
            smtpClient.DeliveryMethod = SmtpDeliveryMethod.Network;
            smtpClient.EnableSsl = EnableSsl;
            smtpClient.Credentials = new NetworkCredential(Username, Password);
            smtpClient.Host = Server;
            smtpClient.Port = Port;

            mailMessage = new MailMessage();
            mailMessage.From = new MailAddress(Username, Nickname);
            mailMessage.SubjectEncoding = Encoding.GetEncoding("GB2312");
            mailMessage.BodyEncoding = Encoding.GetEncoding("GB2312");
            mailMessage.Subject = subject;
            mailMessage.Body = body;
            mailMessage.Headers.Add("ReturnReceipt", "1");
            //mailMessage.Headers.Add("Disposition-Notification-To", Username);
            //mailMessage.DeliveryNotificationOptions = DeliveryNotificationOptions.OnSuccess;
        }

        public void Send(string address)
        {
            mailMessage.To.Clear();
            mailMessage.To.Add(address);
            smtpClient.Send(mailMessage);
        }
    }
}
