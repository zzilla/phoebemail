namespace LazyAccountant
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            this.splitContainer1 = new System.Windows.Forms.SplitContainer();
            this.listView1 = new System.Windows.Forms.ListView();
            this.columnHeader1 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader2 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader3 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader4 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader6 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader7 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader8 = new System.Windows.Forms.ColumnHeader();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            this.labelStat = new System.Windows.Forms.Label();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.buttonCalcTax = new System.Windows.Forms.Button();
            this.numericUpDownTax = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownTaxAmount = new System.Windows.Forms.NumericUpDown();
            this.label11 = new System.Windows.Forms.Label();
            this.numericUpDownTaxStartPoint = new System.Windows.Forms.NumericUpDown();
            this.label8 = new System.Windows.Forms.Label();
            this.groupBoxReceipt = new System.Windows.Forms.GroupBox();
            this.richTextBoxReceipt = new System.Windows.Forms.RichTextBox();
            this.panel1 = new System.Windows.Forms.Panel();
            this.buttonExport = new System.Windows.Forms.Button();
            this.buttonSaveResult = new System.Windows.Forms.Button();
            this.checkBoxSelectedOnly = new System.Windows.Forms.CheckBox();
            this.buttonSendMail = new System.Windows.Forms.Button();
            this.buttonCalcSalary = new System.Windows.Forms.Button();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.numericUpDownPreviousTaxCut = new System.Windows.Forms.NumericUpDown();
            this.label10 = new System.Windows.Forms.Label();
            this.numericUpDownOtherCut = new System.Windows.Forms.NumericUpDown();
            this.label9 = new System.Windows.Forms.Label();
            this.numericUpDownAbsent = new System.Windows.Forms.NumericUpDown();
            this.label7 = new System.Windows.Forms.Label();
            this.numericUpDownLate = new System.Windows.Forms.NumericUpDown();
            this.label6 = new System.Windows.Forms.Label();
            this.numericUpDownAllowance = new System.Windows.Forms.NumericUpDown();
            this.label5 = new System.Windows.Forms.Label();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.numericUpDownTargetYear = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownTargetMonth = new System.Windows.Forms.NumericUpDown();
            this.label4 = new System.Windows.Forms.Label();
            this.numericUpDownWorkDayCount = new System.Windows.Forms.NumericUpDown();
            this.label3 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.labelCurrentYear = new System.Windows.Forms.Label();
            this.labelProgress = new System.Windows.Forms.Label();
            this.splitContainer1.Panel1.SuspendLayout();
            this.splitContainer1.Panel2.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            this.groupBox4.SuspendLayout();
            this.groupBox3.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownTax)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownTaxAmount)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownTaxStartPoint)).BeginInit();
            this.groupBoxReceipt.SuspendLayout();
            this.panel1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownPreviousTaxCut)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownOtherCut)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownAbsent)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownLate)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownAllowance)).BeginInit();
            this.groupBox1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownTargetYear)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownTargetMonth)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownWorkDayCount)).BeginInit();
            this.SuspendLayout();
            // 
            // splitContainer1
            // 
            this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer1.Location = new System.Drawing.Point(0, 0);
            this.splitContainer1.Name = "splitContainer1";
            // 
            // splitContainer1.Panel1
            // 
            this.splitContainer1.Panel1.Controls.Add(this.listView1);
            this.splitContainer1.Panel1.Controls.Add(this.groupBox4);
            this.splitContainer1.Panel1.Controls.Add(this.groupBox3);
            // 
            // splitContainer1.Panel2
            // 
            this.splitContainer1.Panel2.Controls.Add(this.groupBoxReceipt);
            this.splitContainer1.Panel2.Controls.Add(this.panel1);
            this.splitContainer1.Panel2.Controls.Add(this.groupBox2);
            this.splitContainer1.Panel2.Controls.Add(this.groupBox1);
            this.splitContainer1.Size = new System.Drawing.Size(792, 566);
            this.splitContainer1.SplitterDistance = 475;
            this.splitContainer1.TabIndex = 0;
            // 
            // listView1
            // 
            this.listView1.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader2,
            this.columnHeader3,
            this.columnHeader4,
            this.columnHeader6,
            this.columnHeader7,
            this.columnHeader8});
            this.listView1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.listView1.FullRowSelect = true;
            this.listView1.GridLines = true;
            this.listView1.HideSelection = false;
            this.listView1.Location = new System.Drawing.Point(0, 49);
            this.listView1.MultiSelect = false;
            this.listView1.Name = "listView1";
            this.listView1.Size = new System.Drawing.Size(475, 433);
            this.listView1.TabIndex = 0;
            this.listView1.UseCompatibleStateImageBehavior = false;
            this.listView1.View = System.Windows.Forms.View.Details;
            this.listView1.SelectedIndexChanged += new System.EventHandler(this.listView1_SelectedIndexChanged);
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "姓名";
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "奖金补贴";
            this.columnHeader2.Width = 70;
            // 
            // columnHeader3
            // 
            this.columnHeader3.Text = "迟到次数";
            this.columnHeader3.Width = 70;
            // 
            // columnHeader4
            // 
            this.columnHeader4.Text = "缺勤天数";
            this.columnHeader4.Width = 70;
            // 
            // columnHeader6
            // 
            this.columnHeader6.Text = "个税扣款";
            this.columnHeader6.Width = 70;
            // 
            // columnHeader7
            // 
            this.columnHeader7.Text = "其他扣款";
            this.columnHeader7.Width = 70;
            // 
            // columnHeader8
            // 
            this.columnHeader8.Text = "工资单";
            this.columnHeader8.Width = 400;
            // 
            // groupBox4
            // 
            this.groupBox4.Controls.Add(this.labelStat);
            this.groupBox4.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.groupBox4.Location = new System.Drawing.Point(0, 482);
            this.groupBox4.Name = "groupBox4";
            this.groupBox4.Size = new System.Drawing.Size(475, 84);
            this.groupBox4.TabIndex = 2;
            this.groupBox4.TabStop = false;
            this.groupBox4.Text = "统计信息";
            // 
            // labelStat
            // 
            this.labelStat.AutoSize = true;
            this.labelStat.Location = new System.Drawing.Point(6, 24);
            this.labelStat.Name = "labelStat";
            this.labelStat.Size = new System.Drawing.Size(0, 13);
            this.labelStat.TabIndex = 0;
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.buttonCalcTax);
            this.groupBox3.Controls.Add(this.numericUpDownTax);
            this.groupBox3.Controls.Add(this.numericUpDownTaxAmount);
            this.groupBox3.Controls.Add(this.label11);
            this.groupBox3.Controls.Add(this.numericUpDownTaxStartPoint);
            this.groupBox3.Controls.Add(this.label8);
            this.groupBox3.Dock = System.Windows.Forms.DockStyle.Top;
            this.groupBox3.Location = new System.Drawing.Point(0, 0);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(475, 49);
            this.groupBox3.TabIndex = 1;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "个税计算小工具";
            // 
            // buttonCalcTax
            // 
            this.buttonCalcTax.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonCalcTax.Location = new System.Drawing.Point(305, 16);
            this.buttonCalcTax.Name = "buttonCalcTax";
            this.buttonCalcTax.Size = new System.Drawing.Size(49, 23);
            this.buttonCalcTax.TabIndex = 13;
            this.buttonCalcTax.Text = "个税:";
            this.buttonCalcTax.UseVisualStyleBackColor = true;
            this.buttonCalcTax.Click += new System.EventHandler(this.buttonCalcTax_Click);
            // 
            // numericUpDownTax
            // 
            this.numericUpDownTax.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.numericUpDownTax.DecimalPlaces = 2;
            this.numericUpDownTax.Enabled = false;
            this.numericUpDownTax.Increment = new decimal(new int[] {
            100,
            0,
            0,
            0});
            this.numericUpDownTax.Location = new System.Drawing.Point(360, 17);
            this.numericUpDownTax.Maximum = new decimal(new int[] {
            100000,
            0,
            0,
            0});
            this.numericUpDownTax.Name = "numericUpDownTax";
            this.numericUpDownTax.ReadOnly = true;
            this.numericUpDownTax.Size = new System.Drawing.Size(96, 20);
            this.numericUpDownTax.TabIndex = 12;
            this.numericUpDownTax.Value = new decimal(new int[] {
            325,
            0,
            0,
            0});
            // 
            // numericUpDownTaxAmount
            // 
            this.numericUpDownTaxAmount.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.numericUpDownTaxAmount.DecimalPlaces = 2;
            this.numericUpDownTaxAmount.Increment = new decimal(new int[] {
            100,
            0,
            0,
            0});
            this.numericUpDownTaxAmount.Location = new System.Drawing.Point(175, 17);
            this.numericUpDownTaxAmount.Maximum = new decimal(new int[] {
            1000000,
            0,
            0,
            0});
            this.numericUpDownTaxAmount.Name = "numericUpDownTaxAmount";
            this.numericUpDownTaxAmount.Size = new System.Drawing.Size(114, 20);
            this.numericUpDownTaxAmount.TabIndex = 10;
            this.numericUpDownTaxAmount.Value = new decimal(new int[] {
            5000,
            0,
            0,
            0});
            // 
            // label11
            // 
            this.label11.AutoSize = true;
            this.label11.Location = new System.Drawing.Point(138, 21);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(31, 13);
            this.label11.TabIndex = 9;
            this.label11.Text = "金额";
            // 
            // numericUpDownTaxStartPoint
            // 
            this.numericUpDownTaxStartPoint.DecimalPlaces = 2;
            this.numericUpDownTaxStartPoint.Increment = new decimal(new int[] {
            100,
            0,
            0,
            0});
            this.numericUpDownTaxStartPoint.Location = new System.Drawing.Point(53, 17);
            this.numericUpDownTaxStartPoint.Maximum = new decimal(new int[] {
            100000,
            0,
            0,
            0});
            this.numericUpDownTaxStartPoint.Name = "numericUpDownTaxStartPoint";
            this.numericUpDownTaxStartPoint.Size = new System.Drawing.Size(77, 20);
            this.numericUpDownTaxStartPoint.TabIndex = 8;
            this.numericUpDownTaxStartPoint.Value = new decimal(new int[] {
            2000,
            0,
            0,
            0});
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(6, 19);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(43, 13);
            this.label8.TabIndex = 7;
            this.label8.Text = "起征点";
            // 
            // groupBoxReceipt
            // 
            this.groupBoxReceipt.Controls.Add(this.richTextBoxReceipt);
            this.groupBoxReceipt.Dock = System.Windows.Forms.DockStyle.Fill;
            this.groupBoxReceipt.Location = new System.Drawing.Point(0, 172);
            this.groupBoxReceipt.Name = "groupBoxReceipt";
            this.groupBoxReceipt.Size = new System.Drawing.Size(313, 325);
            this.groupBoxReceipt.TabIndex = 3;
            this.groupBoxReceipt.TabStop = false;
            this.groupBoxReceipt.Text = "工资单";
            // 
            // richTextBoxReceipt
            // 
            this.richTextBoxReceipt.Dock = System.Windows.Forms.DockStyle.Fill;
            this.richTextBoxReceipt.Font = new System.Drawing.Font("Microsoft Sans Serif", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.richTextBoxReceipt.Location = new System.Drawing.Point(3, 16);
            this.richTextBoxReceipt.Name = "richTextBoxReceipt";
            this.richTextBoxReceipt.ReadOnly = true;
            this.richTextBoxReceipt.Size = new System.Drawing.Size(307, 306);
            this.richTextBoxReceipt.TabIndex = 0;
            this.richTextBoxReceipt.Text = "";
            // 
            // panel1
            // 
            this.panel1.Controls.Add(this.labelProgress);
            this.panel1.Controls.Add(this.buttonExport);
            this.panel1.Controls.Add(this.buttonSaveResult);
            this.panel1.Controls.Add(this.checkBoxSelectedOnly);
            this.panel1.Controls.Add(this.buttonSendMail);
            this.panel1.Controls.Add(this.buttonCalcSalary);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.panel1.Location = new System.Drawing.Point(0, 497);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(313, 69);
            this.panel1.TabIndex = 2;
            // 
            // buttonExport
            // 
            this.buttonExport.Enabled = false;
            this.buttonExport.Location = new System.Drawing.Point(98, 39);
            this.buttonExport.Name = "buttonExport";
            this.buttonExport.Size = new System.Drawing.Size(75, 23);
            this.buttonExport.TabIndex = 4;
            this.buttonExport.Text = "导出工资单";
            this.buttonExport.UseVisualStyleBackColor = true;
            this.buttonExport.Click += new System.EventHandler(this.buttonExport_Click);
            // 
            // buttonSaveResult
            // 
            this.buttonSaveResult.Enabled = false;
            this.buttonSaveResult.Location = new System.Drawing.Point(11, 39);
            this.buttonSaveResult.Name = "buttonSaveResult";
            this.buttonSaveResult.Size = new System.Drawing.Size(75, 23);
            this.buttonSaveResult.TabIndex = 3;
            this.buttonSaveResult.Text = "保存结果";
            this.buttonSaveResult.UseVisualStyleBackColor = true;
            this.buttonSaveResult.Click += new System.EventHandler(this.buttonSaveResult_Click);
            // 
            // checkBoxSelectedOnly
            // 
            this.checkBoxSelectedOnly.AutoSize = true;
            this.checkBoxSelectedOnly.Location = new System.Drawing.Point(183, 14);
            this.checkBoxSelectedOnly.Name = "checkBoxSelectedOnly";
            this.checkBoxSelectedOnly.Size = new System.Drawing.Size(122, 17);
            this.checkBoxSelectedOnly.TabIndex = 2;
            this.checkBoxSelectedOnly.Text = "仅对选中员工操作";
            this.checkBoxSelectedOnly.UseVisualStyleBackColor = true;
            // 
            // buttonSendMail
            // 
            this.buttonSendMail.Enabled = false;
            this.buttonSendMail.Location = new System.Drawing.Point(98, 10);
            this.buttonSendMail.Name = "buttonSendMail";
            this.buttonSendMail.Size = new System.Drawing.Size(75, 23);
            this.buttonSendMail.TabIndex = 1;
            this.buttonSendMail.Text = "发送邮件";
            this.buttonSendMail.UseVisualStyleBackColor = true;
            this.buttonSendMail.Click += new System.EventHandler(this.buttonSendMail_Click);
            // 
            // buttonCalcSalary
            // 
            this.buttonCalcSalary.Location = new System.Drawing.Point(11, 10);
            this.buttonCalcSalary.Name = "buttonCalcSalary";
            this.buttonCalcSalary.Size = new System.Drawing.Size(75, 23);
            this.buttonCalcSalary.TabIndex = 0;
            this.buttonCalcSalary.Text = "开始计算";
            this.buttonCalcSalary.UseVisualStyleBackColor = true;
            this.buttonCalcSalary.Click += new System.EventHandler(this.buttonCalcSalary_Click);
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.numericUpDownPreviousTaxCut);
            this.groupBox2.Controls.Add(this.label10);
            this.groupBox2.Controls.Add(this.numericUpDownOtherCut);
            this.groupBox2.Controls.Add(this.label9);
            this.groupBox2.Controls.Add(this.numericUpDownAbsent);
            this.groupBox2.Controls.Add(this.label7);
            this.groupBox2.Controls.Add(this.numericUpDownLate);
            this.groupBox2.Controls.Add(this.label6);
            this.groupBox2.Controls.Add(this.numericUpDownAllowance);
            this.groupBox2.Controls.Add(this.label5);
            this.groupBox2.Dock = System.Windows.Forms.DockStyle.Top;
            this.groupBox2.Location = new System.Drawing.Point(0, 49);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(313, 123);
            this.groupBox2.TabIndex = 1;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "计薪信息";
            // 
            // numericUpDownPreviousTaxCut
            // 
            this.numericUpDownPreviousTaxCut.DecimalPlaces = 2;
            this.numericUpDownPreviousTaxCut.Increment = new decimal(new int[] {
            100,
            0,
            0,
            0});
            this.numericUpDownPreviousTaxCut.Location = new System.Drawing.Point(73, 79);
            this.numericUpDownPreviousTaxCut.Maximum = new decimal(new int[] {
            10000,
            0,
            0,
            0});
            this.numericUpDownPreviousTaxCut.Name = "numericUpDownPreviousTaxCut";
            this.numericUpDownPreviousTaxCut.Size = new System.Drawing.Size(77, 20);
            this.numericUpDownPreviousTaxCut.TabIndex = 16;
            this.numericUpDownPreviousTaxCut.ValueChanged += new System.EventHandler(this.numericUpDownPreviousTaxCut_ValueChanged);
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Location = new System.Drawing.Point(14, 82);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(55, 13);
            this.label10.TabIndex = 15;
            this.label10.Text = "个税扣款";
            // 
            // numericUpDownOtherCut
            // 
            this.numericUpDownOtherCut.DecimalPlaces = 2;
            this.numericUpDownOtherCut.Increment = new decimal(new int[] {
            100,
            0,
            0,
            0});
            this.numericUpDownOtherCut.Location = new System.Drawing.Point(220, 79);
            this.numericUpDownOtherCut.Maximum = new decimal(new int[] {
            10000,
            0,
            0,
            0});
            this.numericUpDownOtherCut.Name = "numericUpDownOtherCut";
            this.numericUpDownOtherCut.Size = new System.Drawing.Size(77, 20);
            this.numericUpDownOtherCut.TabIndex = 14;
            this.numericUpDownOtherCut.ValueChanged += new System.EventHandler(this.numericUpDownOtherCut_ValueChanged);
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(162, 82);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(55, 13);
            this.label9.TabIndex = 13;
            this.label9.Text = "其他扣款";
            // 
            // numericUpDownAbsent
            // 
            this.numericUpDownAbsent.DecimalPlaces = 2;
            this.numericUpDownAbsent.Increment = new decimal(new int[] {
            5,
            0,
            0,
            65536});
            this.numericUpDownAbsent.Location = new System.Drawing.Point(221, 49);
            this.numericUpDownAbsent.Name = "numericUpDownAbsent";
            this.numericUpDownAbsent.Size = new System.Drawing.Size(77, 20);
            this.numericUpDownAbsent.TabIndex = 10;
            this.numericUpDownAbsent.ValueChanged += new System.EventHandler(this.numericUpDownAbsent_ValueChanged);
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(162, 53);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(55, 13);
            this.label7.TabIndex = 9;
            this.label7.Text = "缺勤天数";
            // 
            // numericUpDownLate
            // 
            this.numericUpDownLate.Location = new System.Drawing.Point(72, 49);
            this.numericUpDownLate.Name = "numericUpDownLate";
            this.numericUpDownLate.Size = new System.Drawing.Size(77, 20);
            this.numericUpDownLate.TabIndex = 8;
            this.numericUpDownLate.ValueChanged += new System.EventHandler(this.numericUpDownLate_ValueChanged);
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(13, 53);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(55, 13);
            this.label6.TabIndex = 7;
            this.label6.Text = "迟到次数";
            // 
            // numericUpDownAllowance
            // 
            this.numericUpDownAllowance.DecimalPlaces = 2;
            this.numericUpDownAllowance.Increment = new decimal(new int[] {
            100,
            0,
            0,
            0});
            this.numericUpDownAllowance.Location = new System.Drawing.Point(72, 19);
            this.numericUpDownAllowance.Maximum = new decimal(new int[] {
            100000,
            0,
            0,
            0});
            this.numericUpDownAllowance.Name = "numericUpDownAllowance";
            this.numericUpDownAllowance.Size = new System.Drawing.Size(77, 20);
            this.numericUpDownAllowance.TabIndex = 6;
            this.numericUpDownAllowance.ValueChanged += new System.EventHandler(this.numericUpDownAllowance_ValueChanged);
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(13, 23);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(55, 13);
            this.label5.TabIndex = 5;
            this.label5.Text = "奖金补贴";
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.numericUpDownTargetYear);
            this.groupBox1.Controls.Add(this.numericUpDownTargetMonth);
            this.groupBox1.Controls.Add(this.label4);
            this.groupBox1.Controls.Add(this.numericUpDownWorkDayCount);
            this.groupBox1.Controls.Add(this.label3);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.labelCurrentYear);
            this.groupBox1.Dock = System.Windows.Forms.DockStyle.Top;
            this.groupBox1.Location = new System.Drawing.Point(0, 0);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(313, 49);
            this.groupBox1.TabIndex = 0;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "考勤区间";
            // 
            // numericUpDownTargetYear
            // 
            this.numericUpDownTargetYear.Location = new System.Drawing.Point(7, 21);
            this.numericUpDownTargetYear.Maximum = new decimal(new int[] {
            2050,
            0,
            0,
            0});
            this.numericUpDownTargetYear.Minimum = new decimal(new int[] {
            2010,
            0,
            0,
            0});
            this.numericUpDownTargetYear.Name = "numericUpDownTargetYear";
            this.numericUpDownTargetYear.Size = new System.Drawing.Size(51, 20);
            this.numericUpDownTargetYear.TabIndex = 8;
            this.numericUpDownTargetYear.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.numericUpDownTargetYear.Value = new decimal(new int[] {
            2010,
            0,
            0,
            0});
            this.numericUpDownTargetYear.ValueChanged += new System.EventHandler(this.numericUpDownTargetYear_ValueChanged);
            // 
            // numericUpDownTargetMonth
            // 
            this.numericUpDownTargetMonth.Location = new System.Drawing.Point(86, 21);
            this.numericUpDownTargetMonth.Maximum = new decimal(new int[] {
            12,
            0,
            0,
            0});
            this.numericUpDownTargetMonth.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.numericUpDownTargetMonth.Name = "numericUpDownTargetMonth";
            this.numericUpDownTargetMonth.Size = new System.Drawing.Size(35, 20);
            this.numericUpDownTargetMonth.TabIndex = 7;
            this.numericUpDownTargetMonth.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.numericUpDownTargetMonth.Value = new decimal(new int[] {
            3,
            0,
            0,
            0});
            this.numericUpDownTargetMonth.ValueChanged += new System.EventHandler(this.numericUpDownTargetMonth_ValueChanged);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(160, 24);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(19, 13);
            this.label4.TabIndex = 6;
            this.label4.Text = "共";
            // 
            // numericUpDownWorkDayCount
            // 
            this.numericUpDownWorkDayCount.Location = new System.Drawing.Point(182, 21);
            this.numericUpDownWorkDayCount.Name = "numericUpDownWorkDayCount";
            this.numericUpDownWorkDayCount.Size = new System.Drawing.Size(35, 20);
            this.numericUpDownWorkDayCount.TabIndex = 5;
            this.numericUpDownWorkDayCount.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.numericUpDownWorkDayCount.Value = new decimal(new int[] {
            22,
            0,
            0,
            0});
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(225, 24);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(55, 13);
            this.label3.TabIndex = 4;
            this.label3.Text = "个工作日";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(126, 24);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(19, 13);
            this.label2.TabIndex = 2;
            this.label2.Text = "月";
            // 
            // labelCurrentYear
            // 
            this.labelCurrentYear.AutoSize = true;
            this.labelCurrentYear.Location = new System.Drawing.Point(64, 24);
            this.labelCurrentYear.Name = "labelCurrentYear";
            this.labelCurrentYear.Size = new System.Drawing.Size(19, 13);
            this.labelCurrentYear.TabIndex = 0;
            this.labelCurrentYear.Text = "年";
            // 
            // labelProgress
            // 
            this.labelProgress.AutoSize = true;
            this.labelProgress.ForeColor = System.Drawing.Color.Red;
            this.labelProgress.Location = new System.Drawing.Point(183, 44);
            this.labelProgress.Name = "labelProgress";
            this.labelProgress.Size = new System.Drawing.Size(0, 13);
            this.labelProgress.TabIndex = 5;
            this.labelProgress.Visible = false;
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(792, 566);
            this.Controls.Add(this.splitContainer1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "MainForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "LazyAccountant";
            this.WindowState = System.Windows.Forms.FormWindowState.Maximized;
            this.Load += new System.EventHandler(this.MainForm_Load);
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MainForm_FormClosing);
            this.splitContainer1.Panel1.ResumeLayout(false);
            this.splitContainer1.Panel2.ResumeLayout(false);
            this.splitContainer1.ResumeLayout(false);
            this.groupBox4.ResumeLayout(false);
            this.groupBox4.PerformLayout();
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownTax)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownTaxAmount)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownTaxStartPoint)).EndInit();
            this.groupBoxReceipt.ResumeLayout(false);
            this.panel1.ResumeLayout(false);
            this.panel1.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownPreviousTaxCut)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownOtherCut)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownAbsent)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownLate)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownAllowance)).EndInit();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownTargetYear)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownTargetMonth)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownWorkDayCount)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.SplitContainer splitContainer1;
        private System.Windows.Forms.ListView listView1;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label labelCurrentYear;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.NumericUpDown numericUpDownWorkDayCount;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.GroupBox groupBoxReceipt;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.Button buttonSendMail;
        private System.Windows.Forms.Button buttonCalcSalary;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.ColumnHeader columnHeader3;
        private System.Windows.Forms.ColumnHeader columnHeader4;
        private System.Windows.Forms.ColumnHeader columnHeader6;
        private System.Windows.Forms.ColumnHeader columnHeader7;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.NumericUpDown numericUpDownOtherCut;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.NumericUpDown numericUpDownAbsent;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.NumericUpDown numericUpDownLate;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.NumericUpDown numericUpDownAllowance;
        private System.Windows.Forms.RichTextBox richTextBoxReceipt;
        private System.Windows.Forms.ColumnHeader columnHeader8;
        private System.Windows.Forms.NumericUpDown numericUpDownPreviousTaxCut;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.CheckBox checkBoxSelectedOnly;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.NumericUpDown numericUpDownTaxAmount;
        private System.Windows.Forms.Label label11;
        private System.Windows.Forms.NumericUpDown numericUpDownTaxStartPoint;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.NumericUpDown numericUpDownTax;
        private System.Windows.Forms.Button buttonCalcTax;
        private System.Windows.Forms.NumericUpDown numericUpDownTargetMonth;
        private System.Windows.Forms.Button buttonSaveResult;
        private System.Windows.Forms.NumericUpDown numericUpDownTargetYear;
        private System.Windows.Forms.GroupBox groupBox4;
        private System.Windows.Forms.Label labelStat;
        private System.Windows.Forms.Button buttonExport;
        private System.Windows.Forms.Label labelProgress;
    }
}

