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
using System.Collections;

namespace PhoebeMail
{
    public partial class MainForm : Form
    {
        private string m_formText;
        private string m_buttonText;

        private System.Timers.Timer m_timer = new System.Timers.Timer();

        public MainForm()
        {
            InitializeComponent();
            m_formText = Text;
            m_buttonText = buttonSend.Text;

            m_timer.AutoReset = false;//只发一次
            m_timer.Enabled = false;
            m_timer.Elapsed += new System.Timers.ElapsedEventHandler(Timeout);
        }

        private void MainForm_Load(object sender, EventArgs e)
        {
            DateTime now = DateTime.Now;
            dateTimePickerTimedSend.Value = new DateTime(now.Year, now.Month, now.Day, 21, 0, 0);
            //加载已有账号
            DataCenter.Instance.LoadAccounts();
            foreach (EmailAccount a in DataCenter.Instance.Accounts)
            {
                listViewAccounts.Items.Add(BuildListViewItem(a));
            }
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            DialogResult r = QMessageBox.ShowQuestion("Are you sure to exit?");

            if (r == DialogResult.No)
            {
                e.Cancel = true;
                return;
            }

            Settings.Default.Subject = textBoxSubject.Text;
            Settings.Default.Body = richTextBoxBody.Text;
            Settings.Default.Save();

            DataCenter.Instance.SaveAccounts();
        }

        private void buttonQuit_Click(object sender, EventArgs e)
        {
            Close();
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
            OnImportAddress();
        }

        private void OnImportAddress()
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

        private Regex m_regex = new Regex(@"\w+([-+.]\w+)*@\w+([-.]\w+)*\.\w+([-.]\w+)*");

        private bool IsEmailAddress(String email)
        {
            return m_regex.IsMatch(email);
        }

        private MatchCollection GetEmailAddress(string input)
        {
            return m_regex.Matches(input);
        }

        private MailJob job = null;

        private void buttonSend_Click(object sender, EventArgs e)
        {
            if (m_timer.Enabled)//表示timer正在工作
            {
                m_timer.Stop();
                m_timer.Enabled = false;
                OnDone();
                return;
            }
            else if (backgroundWorkerMailSender.IsBusy)
            {
                backgroundWorkerMailSender.CancelAsync();
                OnDone();
                return;
            }

            if (comboBoxAccounts.Items.Count < 1)
            {
                QMessageBox.ShowWarning("account can not be empty!");
                return;
            }
            else if (comboBoxAccounts.SelectedIndex < 0)
            {
                QMessageBox.ShowWarning("you must select an account!");
                return;
            }
            else if (String.IsNullOrEmpty(textBoxSubject.Text))
            {
                QMessageBox.ShowWarning("email subject can not be empty!");
                return;
            }
            else if (String.IsNullOrEmpty(richTextBoxBody.Text))
            {
                QMessageBox.ShowWarning("email body can not be empty!");
                return;
            }
            else if (listBoxAddresses.Items.Count <= 0)
            {
                QMessageBox.ShowWarning("recipients can not be empty!");
                return;
            }
            else
            {
                if (checkBoxTimedSend.Checked)
                {
                    if (dateTimePickerTimedSend.Value <= DateTime.Now)
                    {
                        QMessageBox.ShowError("the time is in the past!");
                        return;
                    }
                    m_timer.Interval = dateTimePickerTimedSend.Value.Subtract(DateTime.Now).TotalSeconds * 1000;
                }
                else
                {
                    m_timer.Interval = 1;
                }
            }

            WrappedItem item = (WrappedItem)comboBoxAccounts.SelectedItem;
            EmailAccount a = item.Account;
            a.Init(textBoxSubject.Text, richTextBoxBody.Text);
            object[] addresses = new object[listBoxAddresses.Items.Count];
            listBoxAddresses.Items.CopyTo(addresses, 0);
            job = new MailJob(a, addresses);
            m_timer.Start();
            buttonSend.Text = "Cancel";
            buttonQuit.Enabled = false;

            this.Text = String.Format("{0} - waiting to send...", m_formText);
        }

        private void backgroundWorkerMailSender_DoWork(object sender, DoWorkEventArgs e)
        {
            job.Send(backgroundWorkerMailSender);
        }

        private void backgroundWorkerMailSender_ProgressChanged(object sender, ProgressChangedEventArgs e)
        {
            this.Invoke(new ProgressChangedEventHandler(UpdateProgress));
        }

        private delegate void ProgressChangedEventHandler();

        private void UpdateProgress()
        {
            this.Text = String.Format("{0} - {1}/{2} done, {3} fail. sending to {4}...", m_formText, job.m_done, job.m_addresses.Length, job.m_fail, job.m_current);
            this.Update();
        }

        private void backgroundWorkerMailSender_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            this.Invoke(new ProgressChangedEventHandler(OnDone));
        }

        private void OnDone()
        {
            Text = m_formText;
            buttonSend.Text = m_buttonText;
            buttonQuit.Enabled = true;

            StringBuilder sum = new StringBuilder();
            sum.AppendFormat("send mail finished, total = {0}, done = {1}, fail = {2}\n", job.m_addresses.Length, job.m_done, job.m_fail);
            sum.AppendFormat("begin at {0}\n", job.m_begin.ToString("yyyy-MM-dd HH:mm:ss"));
            sum.AppendFormat("end   at {0}\n", job.m_end.ToString("yyyy-MM-dd HH:mm:ss"));

            if (job.m_done > 0)
            {
                TimeSpan span = job.m_end.Subtract(job.m_begin);
                int seconds = (int)(span.TotalSeconds + 0.5);
                sum.AppendFormat("average {0}/{1} = {2} seconds\n", seconds, job.m_done, seconds / job.m_done);
            }

            QMessageBox.ShowInfomation(sum.ToString());
        }

        private void Timeout(Object sender, System.Timers.ElapsedEventArgs e)
        {
            backgroundWorkerMailSender.RunWorkerAsync();
            m_timer.Stop();
        }

        private void checkBoxTimedSend_CheckedChanged(object sender, EventArgs e)
        {
            dateTimePickerTimedSend.Enabled = checkBoxTimedSend.Checked;
        }

        private void addToolStripMenuItem_Click(object sender, EventArgs e)
        {
            EmailForm form = new EmailForm();
            DialogResult r = form.ShowDialog();
            if (r == DialogResult.OK)
            {
                string address = form.GetAddress();

                if (!IsEmailAddress(address))
                {
                    QMessageBox.ShowWarning("invalid email address!");
                }
                else if (listBoxAddresses.Items.Contains(address))
                {
                    QMessageBox.ShowWarning("email address existed!");
                }
                else
                {
                    listBoxAddresses.Items.Add(form.GetAddress());
                }
            }
        }

        private void editToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OnEditAddress();
        }

        private void listBoxAddresses_MouseDoubleClick(object sender, MouseEventArgs e)
        {

            if (listBoxAddresses.SelectedIndices.Count > 0)
            {
                OnEditAddress();
            }
            else
            {
                OnImportAddress();
            }
        }

        private void OnEditAddress()
        {
            if (listBoxAddresses.SelectedIndices.Count > 0)
            {
                EmailForm form = new EmailForm();
                string old = listBoxAddresses.SelectedItems[0].ToString();
                form.SetAddress(old);
                DialogResult r = form.ShowDialog();
                if (r == DialogResult.OK)
                {
                    string newAddress = form.GetAddress();

                    if (!IsEmailAddress(newAddress) && (newAddress != old))
                    {
                        QMessageBox.ShowWarning("invalid email address!");
                    }
                    else if (listBoxAddresses.Items.Contains(newAddress))
                    {
                        QMessageBox.ShowWarning("email address existed!");
                    }
                    else if (newAddress == old)
                    {
                        //Do Nothing
                    }
                    else
                    {
                        listBoxAddresses.Items.RemoveAt(listBoxAddresses.SelectedIndices[0]);
                        listBoxAddresses.Items.Add(newAddress);
                    }
                }
            }
        }

        private ListViewItem BuildListViewItem(EmailAccount a)
        {
            ListViewItem item = new ListViewItem();
            item.Tag = a;
            ShowAccount(item);
            return item;
        }

        private void addAccountToolStripMenuItem_Click(object sender, EventArgs e)
        {
            AccountForm f = new AccountForm();
            while (f.ShowDialog() == DialogResult.OK)
            {
                string username = f.GetUsername();

                if (string.IsNullOrEmpty(username))
                {
                    QMessageBox.ShowWarning("username can't be empty!");
                }
                else if (!IsEmailAddress(username))
                {
                    QMessageBox.ShowWarning("email address is not valid!");
                }
                else if (string.IsNullOrEmpty(f.GetPassword()))
                {
                    QMessageBox.ShowWarning("password can't be empty!");
                }
                else if (string.IsNullOrEmpty(f.GetNickname()))
                {
                    QMessageBox.ShowWarning("nickname can't be empty!");
                }
                else if (string.IsNullOrEmpty(f.GetServer()))
                {
                    QMessageBox.ShowWarning("server can't be empty!");
                }
                else
                {
                    EmailAccount a = f.GetAccount();
                    listViewAccounts.Items.Add(BuildListViewItem(a));
                    DataCenter.Instance.Add(a);
                    BuildAccountsList();
                    break;
                }
            }
        }

        private void deleteAccountToolStripMenuItem_Click(object sender, EventArgs e)
        {
            foreach (int index in listViewAccounts.SelectedIndices)
            {
                listViewAccounts.Items.RemoveAt(index);
                DataCenter.Instance.RemoveAt(index);
            }
            BuildAccountsList();
        }

        private void clearAccountToolStripMenuItem_Click(object sender, EventArgs e)
        {
            listViewAccounts.Items.Clear();
            comboBoxAccounts.Items.Clear();
            comboBoxAccounts.Update();
            DataCenter.Instance.Clear();
        }

        private void editAccountToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OnEditAccount();
        }

        private void listViewAccounts_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            OnEditAccount();
        }

        private void ShowAccount(ListViewItem item)
        { 
            EmailAccount a = (EmailAccount)item.Tag;
            item.SubItems.Clear();
            item.Text = a.Username;
            item.SubItems.Add(a.DisplayPassword());
            item.SubItems.Add(a.Nickname);
            item.SubItems.Add(a.Server);
            item.SubItems.Add(a.Port.ToString());
            item.SubItems.Add(a.EnableSsl.ToString());
        }

        private void OnEditAccount()
        {
            if (listViewAccounts.SelectedItems.Count > 0)
            {
                ListViewItem item = listViewAccounts.SelectedItems[0];
                EmailAccount a = (EmailAccount)item.Tag;

                AccountForm f = new AccountForm(a);

                while (f.ShowDialog() == DialogResult.OK)
                {
                    string username = f.GetUsername();

                    if (string.IsNullOrEmpty(username))
                    {
                        QMessageBox.ShowWarning("username can't be empty!");
                    }
                    else if (!IsEmailAddress(username))
                    {
                        QMessageBox.ShowWarning("email address is not valid!");
                    }
                    else if (string.IsNullOrEmpty(f.GetPassword()))
                    {
                        QMessageBox.ShowWarning("password can't be empty!");
                    }
                    else if (string.IsNullOrEmpty(f.GetNickname()))
                    {
                        QMessageBox.ShowWarning("nickname can't be empty!");
                    }
                    else if (string.IsNullOrEmpty(f.GetServer()))
                    {
                        QMessageBox.ShowWarning("server can't be empty!");
                    }
                    else
                    {
                        a.Username = f.GetUsername();
                        a.Password = f.GetPassword();
                        a.Nickname = f.GetNickname();
                        a.Server = f.GetServer();
                        a.Port = f.GetPort();
                        a.EnableSsl = f.EnabledSsl();

                        ShowAccount(item);
                        listViewAccounts.Update();
                        BuildAccountsList();
                        break;
                    }
                }
            }
        }

        private void comboBoxAccounts_DropDown(object sender, EventArgs e)
        {
            BuildAccountsList();
        }

        private void BuildAccountsList()
        {
            comboBoxAccounts.Items.Clear();
            foreach (EmailAccount a in DataCenter.Instance.Accounts)
            {
                comboBoxAccounts.Items.Add(new WrappedItem(a));
            }
        }
    }
}