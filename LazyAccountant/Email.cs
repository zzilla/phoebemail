using System;
using System.Collections.Generic;
using System.Text;
using System.Net.Mail;
using System.Net;

namespace LazyAccountant
{
    class Email
    {
        private SmtpClient m_client = new SmtpClient();
        private string m_from = null;

        public Email()
        {
            m_client.DeliveryMethod = SmtpDeliveryMethod.Network;

            m_client.Host = DataCenter.Instance.MailServer;
            string mailUsername = DataCenter.Instance.MailUsername;
            string mailPassword = DataCenter.Instance.MailPassword;
            m_from = mailUsername;
            m_client.Credentials = new NetworkCredential(mailUsername, mailPassword);
        }

        public void SendMail(string to, string subject, string body)
        {
            MailMessage mail = new MailMessage(m_from, to, subject, body);
            mail.SubjectEncoding = Encoding.GetEncoding("GB2312");
            mail.BodyEncoding = Encoding.GetEncoding("GB2312");
            m_client.Send(mail);
        }
    }
}
