using System;
using System.Collections.Generic;
using System.Text;
using System.ComponentModel;
using System.IO;

namespace PhoebeMail
{
    public class MailJob
    {
        private EmailAccount m_account;

        public object[] m_addresses;

        //状态及统计信息
        public DateTime m_begin;
        public DateTime m_end;

        public int m_done = 0;
        public int m_fail = 0;
        public string m_current = string.Empty;

        public MailJob(EmailAccount account, object[] addresses)
        {
            m_account = account;
            m_addresses = addresses;
        }

        public void Send(BackgroundWorker worker)
        {
            m_begin = DateTime.Now;
            foreach (object item in m_addresses)
            {
                if (worker.CancellationPending)
                {
                    break;
                }

                m_current = item.ToString();

                try
                {
                    worker.ReportProgress((int)(m_done / m_addresses.Length), this);
                    m_account.Send(m_current);
                }
                catch (System.Exception ex)
                {
                    ++m_fail;

                    File.AppendAllText("log.txt", string.Format("send to {0} fail: \n{1}\n", m_current, ex.ToString()));
                }
                finally
                {
                    ++m_done;
                }
            }
            m_end = DateTime.Now;
        }
    }
}
