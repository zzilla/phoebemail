namespace PhoebeMail
{
    partial class MainForm
    {
        /// <summary>
        /// 必需的设计器变量。
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 清理所有正在使用的资源。
        /// </summary>
        /// <param name="disposing">如果应释放托管资源，为 true；否则为 false。</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows 窗体设计器生成的代码

        /// <summary>
        /// 设计器支持所需的方法 - 不要
        /// 使用代码编辑器修改此方法的内容。
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.splitContainer1 = new System.Windows.Forms.SplitContainer();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.listBoxAddresses = new System.Windows.Forms.ListBox();
            this.contextMenuStripAddresses = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.importToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.exportToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.addToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.deleteToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.editToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.clearAllToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.splitContainer2 = new System.Windows.Forms.SplitContainer();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.listBoxAttachments = new System.Windows.Forms.ListBox();
            this.contextMenuStripAttachments = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.addToolStripMenuItem1 = new System.Windows.Forms.ToolStripMenuItem();
            this.deleteToolStripMenuItem1 = new System.Windows.Forms.ToolStripMenuItem();
            this.editToolStripMenuItem1 = new System.Windows.Forms.ToolStripMenuItem();
            this.clearAllToolStripMenuItem1 = new System.Windows.Forms.ToolStripMenuItem();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            this.richTextBoxBody = new System.Windows.Forms.RichTextBox();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.textBoxSubject = new System.Windows.Forms.TextBox();
            this.groupBox5 = new System.Windows.Forms.GroupBox();
            this.comboBoxAccounts = new System.Windows.Forms.ComboBox();
            this.panel2 = new System.Windows.Forms.Panel();
            this.dateTimePickerTimedSend = new System.Windows.Forms.DateTimePicker();
            this.checkBoxTimedSend = new System.Windows.Forms.CheckBox();
            this.buttonQuit = new System.Windows.Forms.Button();
            this.buttonSend = new System.Windows.Forms.Button();
            this.tabPage3 = new System.Windows.Forms.TabPage();
            this.listViewAccounts = new System.Windows.Forms.ListView();
            this.columnHeader1 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader2 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader3 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader4 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader5 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader6 = new System.Windows.Forms.ColumnHeader();
            this.contextMenuStripAccounts = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.addAccountToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.deleteAccountToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.editAccountToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.clearAccountToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
            this.saveFileDialog1 = new System.Windows.Forms.SaveFileDialog();
            this.backgroundWorkerMailSender = new System.ComponentModel.BackgroundWorker();
            this.tabControl1.SuspendLayout();
            this.tabPage1.SuspendLayout();
            this.splitContainer1.Panel1.SuspendLayout();
            this.splitContainer1.Panel2.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.contextMenuStripAddresses.SuspendLayout();
            this.splitContainer2.Panel1.SuspendLayout();
            this.splitContainer2.Panel2.SuspendLayout();
            this.splitContainer2.SuspendLayout();
            this.groupBox3.SuspendLayout();
            this.contextMenuStripAttachments.SuspendLayout();
            this.groupBox4.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.groupBox5.SuspendLayout();
            this.panel2.SuspendLayout();
            this.tabPage3.SuspendLayout();
            this.contextMenuStripAccounts.SuspendLayout();
            this.SuspendLayout();
            // 
            // tabControl1
            // 
            this.tabControl1.Controls.Add(this.tabPage1);
            this.tabControl1.Controls.Add(this.tabPage3);
            this.tabControl1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tabControl1.Location = new System.Drawing.Point(0, 0);
            this.tabControl1.Margin = new System.Windows.Forms.Padding(4);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(792, 566);
            this.tabControl1.TabIndex = 0;
            // 
            // tabPage1
            // 
            this.tabPage1.Controls.Add(this.splitContainer1);
            this.tabPage1.Location = new System.Drawing.Point(4, 25);
            this.tabPage1.Margin = new System.Windows.Forms.Padding(4);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Padding = new System.Windows.Forms.Padding(4);
            this.tabPage1.Size = new System.Drawing.Size(784, 537);
            this.tabPage1.TabIndex = 0;
            this.tabPage1.Text = "Mail";
            this.tabPage1.UseVisualStyleBackColor = true;
            // 
            // splitContainer1
            // 
            this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer1.Location = new System.Drawing.Point(4, 4);
            this.splitContainer1.Margin = new System.Windows.Forms.Padding(4);
            this.splitContainer1.Name = "splitContainer1";
            // 
            // splitContainer1.Panel1
            // 
            this.splitContainer1.Panel1.Controls.Add(this.groupBox1);
            // 
            // splitContainer1.Panel2
            // 
            this.splitContainer1.Panel2.Controls.Add(this.splitContainer2);
            this.splitContainer1.Panel2.Controls.Add(this.groupBox2);
            this.splitContainer1.Panel2.Controls.Add(this.groupBox5);
            this.splitContainer1.Panel2.Controls.Add(this.panel2);
            this.splitContainer1.Size = new System.Drawing.Size(776, 529);
            this.splitContainer1.SplitterDistance = 258;
            this.splitContainer1.SplitterWidth = 5;
            this.splitContainer1.TabIndex = 0;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.listBoxAddresses);
            this.groupBox1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.groupBox1.Location = new System.Drawing.Point(0, 0);
            this.groupBox1.Margin = new System.Windows.Forms.Padding(4);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Padding = new System.Windows.Forms.Padding(4);
            this.groupBox1.Size = new System.Drawing.Size(258, 529);
            this.groupBox1.TabIndex = 1;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Addresses";
            // 
            // listBoxAddresses
            // 
            this.listBoxAddresses.ContextMenuStrip = this.contextMenuStripAddresses;
            this.listBoxAddresses.Dock = System.Windows.Forms.DockStyle.Fill;
            this.listBoxAddresses.FormattingEnabled = true;
            this.listBoxAddresses.ItemHeight = 16;
            this.listBoxAddresses.Location = new System.Drawing.Point(4, 19);
            this.listBoxAddresses.Margin = new System.Windows.Forms.Padding(4);
            this.listBoxAddresses.Name = "listBoxAddresses";
            this.listBoxAddresses.Size = new System.Drawing.Size(250, 500);
            this.listBoxAddresses.TabIndex = 0;
            this.listBoxAddresses.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.listBoxAddresses_MouseDoubleClick);
            // 
            // contextMenuStripAddresses
            // 
            this.contextMenuStripAddresses.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.importToolStripMenuItem,
            this.exportToolStripMenuItem,
            this.addToolStripMenuItem,
            this.deleteToolStripMenuItem,
            this.editToolStripMenuItem,
            this.clearAllToolStripMenuItem});
            this.contextMenuStripAddresses.Name = "contextMenuStripAddresses";
            this.contextMenuStripAddresses.Size = new System.Drawing.Size(115, 136);
            // 
            // importToolStripMenuItem
            // 
            this.importToolStripMenuItem.Name = "importToolStripMenuItem";
            this.importToolStripMenuItem.Size = new System.Drawing.Size(114, 22);
            this.importToolStripMenuItem.Text = "Import...";
            this.importToolStripMenuItem.Click += new System.EventHandler(this.importToolStripMenuItem_Click);
            // 
            // exportToolStripMenuItem
            // 
            this.exportToolStripMenuItem.Name = "exportToolStripMenuItem";
            this.exportToolStripMenuItem.Size = new System.Drawing.Size(114, 22);
            this.exportToolStripMenuItem.Text = "Export...";
            this.exportToolStripMenuItem.Click += new System.EventHandler(this.exportToolStripMenuItem_Click);
            // 
            // addToolStripMenuItem
            // 
            this.addToolStripMenuItem.Name = "addToolStripMenuItem";
            this.addToolStripMenuItem.Size = new System.Drawing.Size(114, 22);
            this.addToolStripMenuItem.Text = "Add...";
            this.addToolStripMenuItem.Click += new System.EventHandler(this.addToolStripMenuItem_Click);
            // 
            // deleteToolStripMenuItem
            // 
            this.deleteToolStripMenuItem.Name = "deleteToolStripMenuItem";
            this.deleteToolStripMenuItem.Size = new System.Drawing.Size(114, 22);
            this.deleteToolStripMenuItem.Text = "Delete";
            this.deleteToolStripMenuItem.Click += new System.EventHandler(this.deleteToolStripMenuItem_Click);
            // 
            // editToolStripMenuItem
            // 
            this.editToolStripMenuItem.Name = "editToolStripMenuItem";
            this.editToolStripMenuItem.Size = new System.Drawing.Size(114, 22);
            this.editToolStripMenuItem.Text = "Edit...";
            this.editToolStripMenuItem.Click += new System.EventHandler(this.editToolStripMenuItem_Click);
            // 
            // clearAllToolStripMenuItem
            // 
            this.clearAllToolStripMenuItem.Name = "clearAllToolStripMenuItem";
            this.clearAllToolStripMenuItem.Size = new System.Drawing.Size(114, 22);
            this.clearAllToolStripMenuItem.Text = "Clear";
            this.clearAllToolStripMenuItem.Click += new System.EventHandler(this.clearAllAddressToolStripMenuItem_Click);
            // 
            // splitContainer2
            // 
            this.splitContainer2.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer2.Location = new System.Drawing.Point(0, 104);
            this.splitContainer2.Margin = new System.Windows.Forms.Padding(4);
            this.splitContainer2.Name = "splitContainer2";
            this.splitContainer2.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // splitContainer2.Panel1
            // 
            this.splitContainer2.Panel1.Controls.Add(this.groupBox3);
            this.splitContainer2.Panel1Collapsed = true;
            // 
            // splitContainer2.Panel2
            // 
            this.splitContainer2.Panel2.Controls.Add(this.groupBox4);
            this.splitContainer2.Size = new System.Drawing.Size(513, 385);
            this.splitContainer2.SplitterDistance = 74;
            this.splitContainer2.SplitterWidth = 5;
            this.splitContainer2.TabIndex = 2;
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.listBoxAttachments);
            this.groupBox3.Dock = System.Windows.Forms.DockStyle.Fill;
            this.groupBox3.Location = new System.Drawing.Point(0, 0);
            this.groupBox3.Margin = new System.Windows.Forms.Padding(4);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Padding = new System.Windows.Forms.Padding(4);
            this.groupBox3.Size = new System.Drawing.Size(150, 74);
            this.groupBox3.TabIndex = 4;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "Attachments";
            // 
            // listBoxAttachments
            // 
            this.listBoxAttachments.ContextMenuStrip = this.contextMenuStripAttachments;
            this.listBoxAttachments.Dock = System.Windows.Forms.DockStyle.Fill;
            this.listBoxAttachments.FormattingEnabled = true;
            this.listBoxAttachments.ItemHeight = 16;
            this.listBoxAttachments.Items.AddRange(new object[] {
            "a.txt",
            "b.doc",
            "c.pdf"});
            this.listBoxAttachments.Location = new System.Drawing.Point(4, 19);
            this.listBoxAttachments.Margin = new System.Windows.Forms.Padding(4);
            this.listBoxAttachments.Name = "listBoxAttachments";
            this.listBoxAttachments.Size = new System.Drawing.Size(142, 36);
            this.listBoxAttachments.TabIndex = 0;
            // 
            // contextMenuStripAttachments
            // 
            this.contextMenuStripAttachments.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.addToolStripMenuItem1,
            this.deleteToolStripMenuItem1,
            this.editToolStripMenuItem1,
            this.clearAllToolStripMenuItem1});
            this.contextMenuStripAttachments.Name = "contextMenuStripAttachments";
            this.contextMenuStripAttachments.Size = new System.Drawing.Size(118, 92);
            // 
            // addToolStripMenuItem1
            // 
            this.addToolStripMenuItem1.Name = "addToolStripMenuItem1";
            this.addToolStripMenuItem1.Size = new System.Drawing.Size(117, 22);
            this.addToolStripMenuItem1.Text = "Browse...";
            // 
            // deleteToolStripMenuItem1
            // 
            this.deleteToolStripMenuItem1.Name = "deleteToolStripMenuItem1";
            this.deleteToolStripMenuItem1.Size = new System.Drawing.Size(117, 22);
            this.deleteToolStripMenuItem1.Text = "Delete";
            // 
            // editToolStripMenuItem1
            // 
            this.editToolStripMenuItem1.Name = "editToolStripMenuItem1";
            this.editToolStripMenuItem1.Size = new System.Drawing.Size(117, 22);
            this.editToolStripMenuItem1.Text = "Edit";
            // 
            // clearAllToolStripMenuItem1
            // 
            this.clearAllToolStripMenuItem1.Name = "clearAllToolStripMenuItem1";
            this.clearAllToolStripMenuItem1.Size = new System.Drawing.Size(117, 22);
            this.clearAllToolStripMenuItem1.Text = "Clear";
            this.clearAllToolStripMenuItem1.Click += new System.EventHandler(this.clearAllAttachmentToolStripMenuItem_Click);
            // 
            // groupBox4
            // 
            this.groupBox4.Controls.Add(this.richTextBoxBody);
            this.groupBox4.Dock = System.Windows.Forms.DockStyle.Fill;
            this.groupBox4.Location = new System.Drawing.Point(0, 0);
            this.groupBox4.Margin = new System.Windows.Forms.Padding(4);
            this.groupBox4.Name = "groupBox4";
            this.groupBox4.Padding = new System.Windows.Forms.Padding(4);
            this.groupBox4.Size = new System.Drawing.Size(513, 385);
            this.groupBox4.TabIndex = 5;
            this.groupBox4.TabStop = false;
            this.groupBox4.Text = "Body";
            // 
            // richTextBoxBody
            // 
            this.richTextBoxBody.DataBindings.Add(new System.Windows.Forms.Binding("Text", global::PhoebeMail.Properties.Settings.Default, "Body", true, System.Windows.Forms.DataSourceUpdateMode.OnPropertyChanged));
            this.richTextBoxBody.Dock = System.Windows.Forms.DockStyle.Fill;
            this.richTextBoxBody.Location = new System.Drawing.Point(4, 19);
            this.richTextBoxBody.Margin = new System.Windows.Forms.Padding(4);
            this.richTextBoxBody.Name = "richTextBoxBody";
            this.richTextBoxBody.Size = new System.Drawing.Size(505, 362);
            this.richTextBoxBody.TabIndex = 0;
            this.richTextBoxBody.Text = global::PhoebeMail.Properties.Settings.Default.Body;
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.textBoxSubject);
            this.groupBox2.Dock = System.Windows.Forms.DockStyle.Top;
            this.groupBox2.Location = new System.Drawing.Point(0, 52);
            this.groupBox2.Margin = new System.Windows.Forms.Padding(4);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Padding = new System.Windows.Forms.Padding(4);
            this.groupBox2.Size = new System.Drawing.Size(513, 52);
            this.groupBox2.TabIndex = 1;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Subject";
            // 
            // textBoxSubject
            // 
            this.textBoxSubject.DataBindings.Add(new System.Windows.Forms.Binding("Text", global::PhoebeMail.Properties.Settings.Default, "Subject", true, System.Windows.Forms.DataSourceUpdateMode.OnPropertyChanged));
            this.textBoxSubject.Dock = System.Windows.Forms.DockStyle.Fill;
            this.textBoxSubject.Location = new System.Drawing.Point(4, 19);
            this.textBoxSubject.Margin = new System.Windows.Forms.Padding(4);
            this.textBoxSubject.Name = "textBoxSubject";
            this.textBoxSubject.Size = new System.Drawing.Size(505, 22);
            this.textBoxSubject.TabIndex = 0;
            this.textBoxSubject.Text = global::PhoebeMail.Properties.Settings.Default.Subject;
            // 
            // groupBox5
            // 
            this.groupBox5.Controls.Add(this.comboBoxAccounts);
            this.groupBox5.Dock = System.Windows.Forms.DockStyle.Top;
            this.groupBox5.Location = new System.Drawing.Point(0, 0);
            this.groupBox5.Name = "groupBox5";
            this.groupBox5.Size = new System.Drawing.Size(513, 52);
            this.groupBox5.TabIndex = 3;
            this.groupBox5.TabStop = false;
            this.groupBox5.Text = "Account";
            // 
            // comboBoxAccounts
            // 
            this.comboBoxAccounts.Dock = System.Windows.Forms.DockStyle.Fill;
            this.comboBoxAccounts.FormattingEnabled = true;
            this.comboBoxAccounts.Location = new System.Drawing.Point(3, 18);
            this.comboBoxAccounts.Name = "comboBoxAccounts";
            this.comboBoxAccounts.Size = new System.Drawing.Size(507, 24);
            this.comboBoxAccounts.TabIndex = 0;
            this.comboBoxAccounts.DropDown += new System.EventHandler(this.comboBoxAccounts_DropDown);
            // 
            // panel2
            // 
            this.panel2.Controls.Add(this.dateTimePickerTimedSend);
            this.panel2.Controls.Add(this.checkBoxTimedSend);
            this.panel2.Controls.Add(this.buttonQuit);
            this.panel2.Controls.Add(this.buttonSend);
            this.panel2.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.panel2.Location = new System.Drawing.Point(0, 489);
            this.panel2.Margin = new System.Windows.Forms.Padding(4);
            this.panel2.Name = "panel2";
            this.panel2.Size = new System.Drawing.Size(513, 40);
            this.panel2.TabIndex = 0;
            // 
            // dateTimePickerTimedSend
            // 
            this.dateTimePickerTimedSend.CustomFormat = "yyyy-MM-dd HH:mm:ss";
            this.dateTimePickerTimedSend.Enabled = false;
            this.dateTimePickerTimedSend.Format = System.Windows.Forms.DateTimePickerFormat.Custom;
            this.dateTimePickerTimedSend.Location = new System.Drawing.Point(82, 10);
            this.dateTimePickerTimedSend.Name = "dateTimePickerTimedSend";
            this.dateTimePickerTimedSend.Size = new System.Drawing.Size(153, 22);
            this.dateTimePickerTimedSend.TabIndex = 3;
            // 
            // checkBoxTimedSend
            // 
            this.checkBoxTimedSend.AutoSize = true;
            this.checkBoxTimedSend.Location = new System.Drawing.Point(4, 12);
            this.checkBoxTimedSend.Name = "checkBoxTimedSend";
            this.checkBoxTimedSend.Size = new System.Drawing.Size(71, 20);
            this.checkBoxTimedSend.TabIndex = 2;
            this.checkBoxTimedSend.Text = "send at";
            this.checkBoxTimedSend.UseVisualStyleBackColor = true;
            this.checkBoxTimedSend.CheckedChanged += new System.EventHandler(this.checkBoxTimedSend_CheckedChanged);
            // 
            // buttonQuit
            // 
            this.buttonQuit.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonQuit.Location = new System.Drawing.Point(408, 7);
            this.buttonQuit.Margin = new System.Windows.Forms.Padding(4);
            this.buttonQuit.Name = "buttonQuit";
            this.buttonQuit.Size = new System.Drawing.Size(100, 28);
            this.buttonQuit.TabIndex = 1;
            this.buttonQuit.Text = "Quit";
            this.buttonQuit.UseVisualStyleBackColor = true;
            this.buttonQuit.Click += new System.EventHandler(this.buttonQuit_Click);
            // 
            // buttonSend
            // 
            this.buttonSend.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonSend.Location = new System.Drawing.Point(290, 7);
            this.buttonSend.Margin = new System.Windows.Forms.Padding(4);
            this.buttonSend.Name = "buttonSend";
            this.buttonSend.Size = new System.Drawing.Size(100, 28);
            this.buttonSend.TabIndex = 0;
            this.buttonSend.Text = "Send";
            this.buttonSend.UseVisualStyleBackColor = true;
            this.buttonSend.Click += new System.EventHandler(this.buttonSend_Click);
            // 
            // tabPage3
            // 
            this.tabPage3.Controls.Add(this.listViewAccounts);
            this.tabPage3.Location = new System.Drawing.Point(4, 25);
            this.tabPage3.Name = "tabPage3";
            this.tabPage3.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage3.Size = new System.Drawing.Size(784, 537);
            this.tabPage3.TabIndex = 2;
            this.tabPage3.Text = "Accounts";
            this.tabPage3.UseVisualStyleBackColor = true;
            // 
            // listViewAccounts
            // 
            this.listViewAccounts.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader2,
            this.columnHeader3,
            this.columnHeader4,
            this.columnHeader5,
            this.columnHeader6});
            this.listViewAccounts.ContextMenuStrip = this.contextMenuStripAccounts;
            this.listViewAccounts.Dock = System.Windows.Forms.DockStyle.Fill;
            this.listViewAccounts.FullRowSelect = true;
            this.listViewAccounts.GridLines = true;
            this.listViewAccounts.Location = new System.Drawing.Point(3, 3);
            this.listViewAccounts.Name = "listViewAccounts";
            this.listViewAccounts.Size = new System.Drawing.Size(778, 531);
            this.listViewAccounts.TabIndex = 1;
            this.listViewAccounts.UseCompatibleStateImageBehavior = false;
            this.listViewAccounts.View = System.Windows.Forms.View.Details;
            this.listViewAccounts.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.listViewAccounts_MouseDoubleClick);
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "username";
            this.columnHeader1.Width = 150;
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "password";
            this.columnHeader2.Width = 100;
            // 
            // columnHeader3
            // 
            this.columnHeader3.Text = "nickname";
            this.columnHeader3.Width = 100;
            // 
            // columnHeader4
            // 
            this.columnHeader4.Text = "server";
            this.columnHeader4.Width = 150;
            // 
            // columnHeader5
            // 
            this.columnHeader5.Text = "port";
            // 
            // columnHeader6
            // 
            this.columnHeader6.Text = "ssl";
            // 
            // contextMenuStripAccounts
            // 
            this.contextMenuStripAccounts.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.addAccountToolStripMenuItem,
            this.deleteAccountToolStripMenuItem,
            this.editAccountToolStripMenuItem,
            this.clearAccountToolStripMenuItem});
            this.contextMenuStripAccounts.Name = "contextMenuStripAccounts";
            this.contextMenuStripAccounts.Size = new System.Drawing.Size(104, 92);
            // 
            // addAccountToolStripMenuItem
            // 
            this.addAccountToolStripMenuItem.Name = "addAccountToolStripMenuItem";
            this.addAccountToolStripMenuItem.Size = new System.Drawing.Size(103, 22);
            this.addAccountToolStripMenuItem.Text = "Add...";
            this.addAccountToolStripMenuItem.Click += new System.EventHandler(this.addAccountToolStripMenuItem_Click);
            // 
            // deleteAccountToolStripMenuItem
            // 
            this.deleteAccountToolStripMenuItem.Name = "deleteAccountToolStripMenuItem";
            this.deleteAccountToolStripMenuItem.Size = new System.Drawing.Size(103, 22);
            this.deleteAccountToolStripMenuItem.Text = "Delete";
            this.deleteAccountToolStripMenuItem.Click += new System.EventHandler(this.deleteAccountToolStripMenuItem_Click);
            // 
            // editAccountToolStripMenuItem
            // 
            this.editAccountToolStripMenuItem.Name = "editAccountToolStripMenuItem";
            this.editAccountToolStripMenuItem.Size = new System.Drawing.Size(103, 22);
            this.editAccountToolStripMenuItem.Text = "Edit";
            this.editAccountToolStripMenuItem.Click += new System.EventHandler(this.editAccountToolStripMenuItem_Click);
            // 
            // clearAccountToolStripMenuItem
            // 
            this.clearAccountToolStripMenuItem.Name = "clearAccountToolStripMenuItem";
            this.clearAccountToolStripMenuItem.Size = new System.Drawing.Size(103, 22);
            this.clearAccountToolStripMenuItem.Text = "Clear";
            this.clearAccountToolStripMenuItem.Click += new System.EventHandler(this.clearAccountToolStripMenuItem_Click);
            // 
            // openFileDialog1
            // 
            this.openFileDialog1.FileName = "import.txt";
            this.openFileDialog1.Filter = "txt file|*.txt";
            this.openFileDialog1.ReadOnlyChecked = true;
            this.openFileDialog1.RestoreDirectory = true;
            this.openFileDialog1.ShowReadOnly = true;
            // 
            // saveFileDialog1
            // 
            this.saveFileDialog1.FileName = "export.txt";
            this.saveFileDialog1.Filter = "txt file|*.txt";
            // 
            // backgroundWorkerMailSender
            // 
            this.backgroundWorkerMailSender.WorkerReportsProgress = true;
            this.backgroundWorkerMailSender.WorkerSupportsCancellation = true;
            this.backgroundWorkerMailSender.DoWork += new System.ComponentModel.DoWorkEventHandler(this.backgroundWorkerMailSender_DoWork);
            this.backgroundWorkerMailSender.RunWorkerCompleted += new System.ComponentModel.RunWorkerCompletedEventHandler(this.backgroundWorkerMailSender_RunWorkerCompleted);
            this.backgroundWorkerMailSender.ProgressChanged += new System.ComponentModel.ProgressChangedEventHandler(this.backgroundWorkerMailSender_ProgressChanged);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(792, 566);
            this.Controls.Add(this.tabControl1);
            this.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Margin = new System.Windows.Forms.Padding(4);
            this.Name = "MainForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "PhoebeMail";
            this.Load += new System.EventHandler(this.MainForm_Load);
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MainForm_FormClosing);
            this.tabControl1.ResumeLayout(false);
            this.tabPage1.ResumeLayout(false);
            this.splitContainer1.Panel1.ResumeLayout(false);
            this.splitContainer1.Panel2.ResumeLayout(false);
            this.splitContainer1.ResumeLayout(false);
            this.groupBox1.ResumeLayout(false);
            this.contextMenuStripAddresses.ResumeLayout(false);
            this.splitContainer2.Panel1.ResumeLayout(false);
            this.splitContainer2.Panel2.ResumeLayout(false);
            this.splitContainer2.ResumeLayout(false);
            this.groupBox3.ResumeLayout(false);
            this.contextMenuStripAttachments.ResumeLayout(false);
            this.groupBox4.ResumeLayout(false);
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.groupBox5.ResumeLayout(false);
            this.panel2.ResumeLayout(false);
            this.panel2.PerformLayout();
            this.tabPage3.ResumeLayout(false);
            this.contextMenuStripAccounts.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TabControl tabControl1;
        private System.Windows.Forms.TabPage tabPage1;
        private System.Windows.Forms.SplitContainer splitContainer1;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Panel panel2;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.TextBox textBoxSubject;
        private System.Windows.Forms.ListBox listBoxAddresses;
        private System.Windows.Forms.Button buttonQuit;
        private System.Windows.Forms.Button buttonSend;
        private System.Windows.Forms.SplitContainer splitContainer2;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.ListBox listBoxAttachments;
        private System.Windows.Forms.GroupBox groupBox4;
        private System.Windows.Forms.RichTextBox richTextBoxBody;
        private System.Windows.Forms.ContextMenuStrip contextMenuStripAddresses;
        private System.Windows.Forms.ToolStripMenuItem addToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem importToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem deleteToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem editToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem clearAllToolStripMenuItem;
        private System.Windows.Forms.ContextMenuStrip contextMenuStripAttachments;
        private System.Windows.Forms.ToolStripMenuItem addToolStripMenuItem1;
        private System.Windows.Forms.ToolStripMenuItem deleteToolStripMenuItem1;
        private System.Windows.Forms.ToolStripMenuItem editToolStripMenuItem1;
        private System.Windows.Forms.ToolStripMenuItem clearAllToolStripMenuItem1;
        private System.Windows.Forms.ToolStripMenuItem exportToolStripMenuItem;
        private System.Windows.Forms.OpenFileDialog openFileDialog1;
        private System.Windows.Forms.SaveFileDialog saveFileDialog1;
        private System.ComponentModel.BackgroundWorker backgroundWorkerMailSender;
        private System.Windows.Forms.CheckBox checkBoxTimedSend;
        private System.Windows.Forms.DateTimePicker dateTimePickerTimedSend;
        private System.Windows.Forms.TabPage tabPage3;
        private System.Windows.Forms.ListView listViewAccounts;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.ColumnHeader columnHeader3;
        private System.Windows.Forms.ColumnHeader columnHeader4;
        private System.Windows.Forms.ColumnHeader columnHeader5;
        private System.Windows.Forms.ColumnHeader columnHeader6;
        private System.Windows.Forms.ContextMenuStrip contextMenuStripAccounts;
        private System.Windows.Forms.ToolStripMenuItem addAccountToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem deleteAccountToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem editAccountToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem clearAccountToolStripMenuItem;
        private System.Windows.Forms.GroupBox groupBox5;
        private System.Windows.Forms.ComboBox comboBoxAccounts;
    }
}

