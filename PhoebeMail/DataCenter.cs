using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace PhoebeMail
{
    class DataCenter
    {
        private List<EmailAccount> m_accounts = new List<EmailAccount>();

        internal List<EmailAccount> Accounts
        {
            get { return m_accounts; }
        }

        private static DataCenter m_instance = new DataCenter();

        internal static DataCenter Instance
        {
            get { return m_instance; }
        }

        private DataCenter()
        {

        }

        private string m_accountsFile = "accounts.txt";

        public void LoadAccounts()
        {
            if (File.Exists(m_accountsFile))
            {
                string[] lines = File.ReadAllLines(m_accountsFile);

                foreach (string line in lines)
                {
                    if (!string.IsNullOrEmpty(line))
                    {
                        EmailAccount account = new EmailAccount(line.Split(','));
                        m_accounts.Add(account);
                    }
                }
            }   
        }

        public void SaveAccounts()
        {
            if (m_accounts.Count > 0)
            {
                StringBuilder sb = new StringBuilder();
                foreach (EmailAccount a in m_accounts)
                {
                    sb.Append(a.ToString());
                }
                File.WriteAllText(m_accountsFile, sb.ToString());
            }
        }

        public void Add(EmailAccount account)
        {
            m_accounts.Add(account);
        }

        public void Remove(EmailAccount account)
        {
            m_accounts.Remove(account);
        }
    }
}
