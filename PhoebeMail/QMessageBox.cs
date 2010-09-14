using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;

namespace PhoebeMail
{
    public class QMessageBox
    {
        private static readonly String m_caption = "PhoebeMail";

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
    }
}
