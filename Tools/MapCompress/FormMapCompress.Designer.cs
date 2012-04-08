namespace MapCompress
{
    partial class FormMapCompress
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.BtnConvert = new System.Windows.Forms.Button();
            this.TxtBox = new System.Windows.Forms.RichTextBox();
            this.DlgFileOpen = new System.Windows.Forms.OpenFileDialog();
            this.SuspendLayout();
            // 
            // BtnConvert
            // 
            this.BtnConvert.Location = new System.Drawing.Point(12, 12);
            this.BtnConvert.Name = "BtnConvert";
            this.BtnConvert.Size = new System.Drawing.Size(75, 23);
            this.BtnConvert.TabIndex = 0;
            this.BtnConvert.Text = "Convert";
            this.BtnConvert.UseVisualStyleBackColor = true;
            this.BtnConvert.Click += new System.EventHandler(this.BtnConvert_Click);
            // 
            // TxtBox
            // 
            this.TxtBox.Location = new System.Drawing.Point(12, 41);
            this.TxtBox.Name = "TxtBox";
            this.TxtBox.Size = new System.Drawing.Size(451, 311);
            this.TxtBox.TabIndex = 1;
            this.TxtBox.Text = "";
            // 
            // FormMapCompress
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(475, 364);
            this.Controls.Add(this.TxtBox);
            this.Controls.Add(this.BtnConvert);
            this.Name = "FormMapCompress";
            this.Text = "Map Compressor/Converter";
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button BtnConvert;
        private System.Windows.Forms.RichTextBox TxtBox;
        private System.Windows.Forms.OpenFileDialog DlgFileOpen;
    }
}

