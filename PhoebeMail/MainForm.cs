using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Net.Mail;
using System.Net;
using PhoebeMail.Properties;
using System.IO;
using System.Text.RegularExpressions;

namespace PhoebeMail
{
    public partial class MainForm : Form
    {
        private string m_email = "email.txt";
        private static readonly String m_caption = "PhoebeMail";

        public MainForm()
        {
            InitializeComponent();        
        }

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

        private void MainForm_Load(object sender, EventArgs e)
        {
            textBoxSubject.Text = textBoxDefaultSubject.Text;

            if (File.Exists(m_email))
            {
                richTextBoxDefaultBody.Text = File.ReadAllText(m_email);
                richTextBoxBody.Text = richTextBoxDefaultBody.Text;
            }
        }

        private void textBoxDefaultSubject_TextChanged(object sender, EventArgs e)
        {
            textBoxSubject.Text = textBoxDefaultSubject.Text;
        }

        private void richTextBoxDefaultBody_TextChanged(object sender, EventArgs e)
        {
            if (!String.IsNullOrEmpty(richTextBoxDefaultBody.Text))
            {
                richTextBoxBody.Text = richTextBoxDefaultBody.Text;
                File.WriteAllText(m_email, richTextBoxDefaultBody.Text);
            }
        }

        private Regex m_regex = new Regex(@"\w+([-+.]\w+)*@\w+([-.]\w+)*\.\w+([-.]\w+)*");

        private bool IsEmailAddress(String email)
        {
            return m_regex.IsMatch(email);
        }

        private MatchCollection GetEmailAddress(string input)
        {
            return m_regex.Matches(input);
        }

        private void buttonSend_Click(object sender, EventArgs e)
        {
            if (String.IsNullOrEmpty(textBoxUsername.Text))
            {
                ShowWarning("username can not be empty!");
                return;
            }
            else if (!IsEmailAddress(textBoxUsername.Text))
            {
                ShowWarning("username is not a valid email address!");
                return;
            }
            else if (String.IsNullOrEmpty(textBoxPassword.Text))
            {
                ShowWarning("password can not be empty!");
                return;
            }
            else if (String.IsNullOrEmpty(textBoxServer.Text))
            {
                ShowWarning("server can not be empty!");
                return;
            }
            else if (String.IsNullOrEmpty(textBoxSubject.Text))
            {
                ShowWarning("email subject can not be empty!");
                return;
            }
            else if (String.IsNullOrEmpty(richTextBoxBody.Text))
            {
                ShowWarning("email body can not be empty!");
                return;
            }
            else if (listBoxAddresses.Items.Count <= 0)
            {
                ShowWarning("recipients can not be empty!");
                return;
            }

            SmtpClient smtpClient = new SmtpClient();
            smtpClient.DeliveryMethod = SmtpDeliveryMethod.Network;
            smtpClient.EnableSsl = true;
            smtpClient.Credentials = new NetworkCredential(textBoxUsername.Text, textBoxPassword.Text);
            smtpClient.Host = textBoxServer.Text;
            smtpClient.Port = (int)numericUpDownPort.Value;

            MailMessage mailMessage = new MailMessage();
            mailMessage.From = new MailAddress(textBoxUsername.Text, textBoxNickname.Text);
            mailMessage.SubjectEncoding = Encoding.GetEncoding("GB2312");
            mailMessage.BodyEncoding = Encoding.GetEncoding("GB2312");
            mailMessage.Subject = textBoxSubject.Text;
            mailMessage.Body = richTextBoxBody.Text;

            int total = listBoxAddresses.Items.Count;
            int done = 0;
            int fail = 0;

            string formText = this.Text;

            string buttonText = this.buttonSend.Text;

            buttonSend.Text = "Sending...";

            StringBuilder report = new StringBuilder();
            DateTime begin = DateTime.Now;
            report.AppendFormat("send mail begin at {0}, total = {1}\n", begin.ToString("yyyy-MM-dd HH:mm:ss"), total);

            foreach (object item in listBoxAddresses.Items)
            {
                string emailAddress = item.ToString();
                if (IsEmailAddress(emailAddress))
                {
                    mailMessage.To.Clear();
                    mailMessage.To.Add(emailAddress);
                    this.Text = String.Format("{0} - {1}/{2} done, {3} fail. sending to {4}...", formText, done, total, fail, emailAddress);
                    this.Update();
                    try
                    {
                        smtpClient.Send(mailMessage);
                        report.AppendFormat("{0:30} OK.\n", emailAddress);
                    }
                    catch (System.Exception ex)
                    {
                        ++fail;
                        report.AppendFormat("{0:30} fail, msg: {1}.\n", emailAddress, ex.ToString());
                    }
                    finally
                    {
                        ++done;
                    }
                }
            }

            this.Text = String.Format("{0} - {1}/{2} done, {3} fail.", formText, done, total, fail);

            DateTime end = DateTime.Now;
            TimeSpan span = end.Subtract(begin);
            StringBuilder sum = new StringBuilder();
            sum.AppendFormat("send mail finished, total = {0}, fail = {1}\n", total, fail);
            sum.AppendFormat("begin at {0}\n", begin.ToString("yyyy-MM-dd HH:mm:ss"));
            sum.AppendFormat("end   at {0}\n", end.ToString("yyyy-MM-dd HH:mm:ss"));
            sum.AppendFormat("average {0}/{1} = {2} seconds\n", span.TotalSeconds, total, span.TotalSeconds / total);

            report.AppendLine(sum.ToString());
            File.AppendAllText("report.txt", report.ToString());
            ShowInfomation(sum.ToString());
            Text = formText;
            buttonSend.Text = buttonText;
        }

        private void buttonQuit_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            Settings.Default.Username = textBoxUsername.Text;
            Settings.Default.Password = textBoxPassword.Text;
            Settings.Default.Nickname = textBoxNickname.Text;
            Settings.Default.Server = textBoxServer.Text;
            Settings.Default.Port = numericUpDownPort.Value;
            Settings.Default.DefaultSubject = textBoxDefaultSubject.Text;
            Settings.Default.Save();
        }

        private void clearAllAddressToolStripMenuItem_Click(object sender, EventArgs e)
        {
            listBoxAddresses.Items.Clear();
        }

        private void clearAllAttachmentToolStripMenuItem_Click(object sender, EventArgs e)
        {
            listBoxAttachments.Items.Clear();
        }

        private void importToolStripMenuItem_Click(object sender, EventArgs e)
        {
            DialogResult r = openFileDialog1.ShowDialog();
            if (r == DialogResult.OK)
            {
                if (File.Exists(openFileDialog1.FileName))
                {
                    string text = File.ReadAllText(openFileDialog1.FileName);

                    MatchCollection emails = GetEmailAddress(text);
                    listBoxAddresses.Items.Clear();
                    foreach (Match m in emails)
                    {
                        if (!listBoxAddresses.Items.Contains(m.Value))
                        {
                            listBoxAddresses.Items.Add(m.Value);
                        }
                    }
                }
            }
        }

        private void exportToolStripMenuItem_Click(object sender, EventArgs e)
        {
            DialogResult r = saveFileDialog1.ShowDialog();
            if (r == DialogResult.OK)
            {
                using (StreamWriter sw = File.CreateText(saveFileDialog1.FileName))
                {
                    foreach (object item in listBoxAddresses.Items)
                    {
                        sw.WriteLine(item.ToString());
                    }
                }
            }
        }

        private void deleteToolStripMenuItem_Click(object sender, EventArgs e)
        {
            foreach (int index in listBoxAddresses.SelectedIndices)
            {
                listBoxAddresses.Items.RemoveAt(index);
            }
        }
    }
}