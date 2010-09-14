namespace PhoebeMail
{
    partial class AccountForm
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(AccountForm));
            this.checkBoxSsl = new System.Windows.Forms.CheckBox();
            this.numericUpDownPort = new System.Windows.Forms.NumericUpDown();
            this.label4 = new System.Windows.Forms.Label();
            this.textBoxServer = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.textBoxPassword = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.textBoxNickname = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.textBoxUsername = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.buttonOK = new System.Windows.Forms.Button();
            this.buttonCancel = new System.Windows.Forms.Button();
            this.label8 = new System.Windows.Forms.Label();
            this.label9 = new System.Windows.Forms.Label();
            this.label10 = new System.Windows.Forms.Label();
            this.label11 = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownPort)).BeginInit();
            this.SuspendLayout();
            // 
            // checkBoxSsl
            // 
            this.checkBoxSsl.AutoSize = true;
            this.checkBoxSsl.Checked = true;
            this.checkBoxSsl.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBoxSsl.Location = new System.Drawing.Point(85, 284);
            this.checkBoxSsl.Name = "checkBoxSsl";
            this.checkBoxSsl.Size = new System.Drawing.Size(81, 16);
            this.checkBoxSsl.TabIndex = 5;
            this.checkBoxSsl.Text = "Enable SSL";
            this.checkBoxSsl.UseVisualStyleBackColor = true;
            // 
            // numericUpDownPort
            // 
            this.numericUpDownPort.Location = new System.Drawing.Point(85, 226);
            this.numericUpDownPort.Margin = new System.Windows.Forms.Padding(4);
            this.numericUpDownPort.Maximum = new decimal(new int[] {
            65535,
            0,
            0,
            0});
            this.numericUpDownPort.Name = "numericUpDownPort";
            this.numericUpDownPort.Size = new System.Drawing.Size(264, 21);
            this.numericUpDownPort.TabIndex = 4;
            this.numericUpDownPort.Value = new decimal(new int[] {
            587,
            0,
            0,
            0});
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(53, 229);
            this.label4.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(27, 12);
            this.label4.TabIndex = 24;
            this.label4.Text = "Port";
            // 
            // textBoxServer
            // 
            this.textBoxServer.Location = new System.Drawing.Point(86, 177);
            this.textBoxServer.Margin = new System.Windows.Forms.Padding(4);
            this.textBoxServer.Name = "textBoxServer";
            this.textBoxServer.Size = new System.Drawing.Size(264, 21);
            this.textBoxServer.TabIndex = 3;
            this.textBoxServer.Text = "smtp.gmail.com";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(43, 180);
            this.label5.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(37, 12);
            this.label5.TabIndex = 22;
            this.label5.Text = "Server";
            // 
            // textBoxPassword
            // 
            this.textBoxPassword.Location = new System.Drawing.Point(86, 79);
            this.textBoxPassword.Margin = new System.Windows.Forms.Padding(4);
            this.textBoxPassword.Name = "textBoxPassword";
            this.textBoxPassword.PasswordChar = '*';
            this.textBoxPassword.Size = new System.Drawing.Size(264, 21);
            this.textBoxPassword.TabIndex = 1;
            this.textBoxPassword.Text = "123456";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(25, 82);
            this.label3.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(55, 12);
            this.label3.TabIndex = 20;
            this.label3.Text = "Password";
            // 
            // textBoxNickname
            // 
            this.textBoxNickname.Location = new System.Drawing.Point(86, 128);
            this.textBoxNickname.Margin = new System.Windows.Forms.Padding(4);
            this.textBoxNickname.Name = "textBoxNickname";
            this.textBoxNickname.Size = new System.Drawing.Size(264, 21);
            this.textBoxNickname.TabIndex = 2;
            this.textBoxNickname.Text = "Phoebe";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(22, 131);
            this.label2.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(58, 12);
            this.label2.TabIndex = 18;
            this.label2.Text = "Nickname";
            // 
            // textBoxUsername
            // 
            this.textBoxUsername.Location = new System.Drawing.Point(86, 30);
            this.textBoxUsername.Margin = new System.Windows.Forms.Padding(4);
            this.textBoxUsername.Name = "textBoxUsername";
            this.textBoxUsername.Size = new System.Drawing.Size(264, 21);
            this.textBoxUsername.TabIndex = 0;
            this.textBoxUsername.Text = "phoebe@gmail.com";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(23, 33);
            this.label1.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(57, 12);
            this.label1.TabIndex = 16;
            this.label1.Text = "Username";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.ForeColor = System.Drawing.Color.Red;
            this.label6.Location = new System.Drawing.Point(85, 312);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(174, 12);
            this.label6.TabIndex = 27;
            this.label6.Text = "*gmail, hotmail must check this";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.ForeColor = System.Drawing.Color.Red;
            this.label7.Location = new System.Drawing.Point(85, 258);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(121, 12);
            this.label7.TabIndex = 28;
            this.label7.Text = "*gmail 587, default 25";
            // 
            // buttonOK
            // 
            this.buttonOK.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.buttonOK.Location = new System.Drawing.Point(113, 340);
            this.buttonOK.Name = "buttonOK";
            this.buttonOK.Size = new System.Drawing.Size(75, 23);
            this.buttonOK.TabIndex = 6;
            this.buttonOK.Text = "OK";
            this.buttonOK.UseVisualStyleBackColor = true;
            // 
            // buttonCancel
            // 
            this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.buttonCancel.Location = new System.Drawing.Point(245, 340);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.Size = new System.Drawing.Size(75, 23);
            this.buttonCancel.TabIndex = 7;
            this.buttonCancel.Text = "Cancel";
            this.buttonCancel.UseVisualStyleBackColor = true;
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.ForeColor = System.Drawing.Color.Red;
            this.label8.Location = new System.Drawing.Point(85, 207);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(120, 12);
            this.label8.TabIndex = 31;
            this.label8.Text = "*e.g., smtp.gmail.com";
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.ForeColor = System.Drawing.Color.Red;
            this.label9.Location = new System.Drawing.Point(89, 108);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(124, 12);
            this.label9.TabIndex = 32;
            this.label9.Text = "*make sure it\'s correct";
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.ForeColor = System.Drawing.Color.Red;
            this.label10.Location = new System.Drawing.Point(89, 59);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(142, 12);
            this.label10.TabIndex = 33;
            this.label10.Text = "*e.g., phoebe@gmail.com";
            // 
            // label11
            // 
            this.label11.AutoSize = true;
            this.label11.ForeColor = System.Drawing.Color.Red;
            this.label11.Location = new System.Drawing.Point(89, 157);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(76, 12);
            this.label11.TabIndex = 34;
            this.label11.Text = "*e.g., Phoebe";
            // 
            // AccountForm
            // 
            this.AcceptButton = this.buttonOK;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.buttonCancel;
            this.ClientSize = new System.Drawing.Size(403, 377);
            this.Controls.Add(this.label11);
            this.Controls.Add(this.label10);
            this.Controls.Add(this.label9);
            this.Controls.Add(this.label8);
            this.Controls.Add(this.buttonCancel);
            this.Controls.Add(this.buttonOK);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.checkBoxSsl);
            this.Controls.Add(this.numericUpDownPort);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.textBoxServer);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.textBoxPassword);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.textBoxNickname);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.textBoxUsername);
            this.Controls.Add(this.label1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "AccountForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Account";
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownPort)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.CheckBox checkBoxSsl;
        private System.Windows.Forms.NumericUpDown numericUpDownPort;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.TextBox textBoxServer;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.TextBox textBoxPassword;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox textBoxNickname;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox textBoxUsername;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Button buttonOK;
        private System.Windows.Forms.Button buttonCancel;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.Label label11;
    }
}