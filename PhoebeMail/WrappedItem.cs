using System;
using System.Collections.Generic;
using System.Text;

namespace PhoebeMail
{
    class WrappedItem
    {
        public WrappedItem(EmailAccount account)
        {
            m_account = account;
        }

        private EmailAccount m_account;

        internal EmailAccount Account
        {
            get { return m_account; }
        }

        public override string ToString()
        {
            return m_account.Username;
        }
    }
}
