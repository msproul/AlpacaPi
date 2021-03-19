namespace DotNetExample
{
    partial class Form1
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
            this.FindButton = new System.Windows.Forms.Button();
            this.CameraComboBox = new System.Windows.Forms.ComboBox();
            this.Connect = new System.Windows.Forms.Button();
            this.StartExposureButton = new System.Windows.Forms.Button();
            this.StartCoolingButton = new System.Windows.Forms.Button();
            this.PictureBox = new System.Windows.Forms.PictureBox();
            this.StopExposure = new System.Windows.Forms.Button();
            this.Disconnect = new System.Windows.Forms.Button();
            this.StopCooling = new System.Windows.Forms.Button();
            this.ConnectedLabel = new System.Windows.Forms.Label();
            this.ExposureStatus = new System.Windows.Forms.Label();
            this.Temperature = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.CMOSOptionsBox = new System.Windows.Forms.GroupBox();
            this.OffsetBox = new System.Windows.Forms.GroupBox();
            this.OffsetUpDown = new System.Windows.Forms.NumericUpDown();
            this.GainBox = new System.Windows.Forms.GroupBox();
            this.GainUpDown = new System.Windows.Forms.NumericUpDown();
            this.GOModeComboBox = new System.Windows.Forms.ComboBox();
            this.EvenIlluminationCheckBox = new System.Windows.Forms.CheckBox();
            this.PadDataCheckBox = new System.Windows.Forms.CheckBox();
            this.FastModeBox = new System.Windows.Forms.GroupBox();
            this.StopFastModeButton = new System.Windows.Forms.Button();
            this.StartFastModeButton = new System.Windows.Forms.Button();
            this.BitSendModeBox = new System.Windows.Forms.ComboBox();
            this.ExposureSpeedBox = new System.Windows.Forms.ComboBox();
            this.fpsLabel = new System.Windows.Forms.Label();
            this.fpsValueLabel = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.PictureBox)).BeginInit();
            this.CMOSOptionsBox.SuspendLayout();
            this.OffsetBox.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.OffsetUpDown)).BeginInit();
            this.GainBox.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.GainUpDown)).BeginInit();
            this.FastModeBox.SuspendLayout();
            this.SuspendLayout();
            // 
            // FindButton
            // 
            this.FindButton.Location = new System.Drawing.Point(13, 13);
            this.FindButton.Name = "FindButton";
            this.FindButton.Size = new System.Drawing.Size(75, 23);
            this.FindButton.TabIndex = 0;
            this.FindButton.Text = "Find Cameras";
            this.FindButton.UseVisualStyleBackColor = true;
            this.FindButton.Click += new System.EventHandler(this.Find_Click);
            // 
            // CameraComboBox
            // 
            this.CameraComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.CameraComboBox.FormattingEnabled = true;
            this.CameraComboBox.Location = new System.Drawing.Point(94, 12);
            this.CameraComboBox.Name = "CameraComboBox";
            this.CameraComboBox.Size = new System.Drawing.Size(121, 21);
            this.CameraComboBox.TabIndex = 1;
            this.CameraComboBox.SelectedIndexChanged += new System.EventHandler(this.CameraComboBox_SelectedIndexChanged);
            // 
            // Connect
            // 
            this.Connect.Enabled = false;
            this.Connect.Location = new System.Drawing.Point(222, 13);
            this.Connect.Name = "Connect";
            this.Connect.Size = new System.Drawing.Size(75, 23);
            this.Connect.TabIndex = 2;
            this.Connect.Text = "Connect";
            this.Connect.UseVisualStyleBackColor = true;
            this.Connect.Click += new System.EventHandler(this.Connect_Click);
            // 
            // StartExposureButton
            // 
            this.StartExposureButton.Enabled = false;
            this.StartExposureButton.Location = new System.Drawing.Point(13, 50);
            this.StartExposureButton.Name = "StartExposureButton";
            this.StartExposureButton.Size = new System.Drawing.Size(75, 35);
            this.StartExposureButton.TabIndex = 3;
            this.StartExposureButton.Text = "Start Exposure";
            this.StartExposureButton.UseVisualStyleBackColor = true;
            this.StartExposureButton.Click += new System.EventHandler(this.StartExposureButton_Click);
            // 
            // StartCoolingButton
            // 
            this.StartCoolingButton.Enabled = false;
            this.StartCoolingButton.Location = new System.Drawing.Point(13, 171);
            this.StartCoolingButton.Name = "StartCoolingButton";
            this.StartCoolingButton.Size = new System.Drawing.Size(75, 23);
            this.StartCoolingButton.TabIndex = 4;
            this.StartCoolingButton.Text = "Start Cooling";
            this.StartCoolingButton.UseVisualStyleBackColor = true;
            this.StartCoolingButton.Click += new System.EventHandler(this.StartCoolingButton_Click);
            // 
            // PictureBox
            // 
            this.PictureBox.BackColor = System.Drawing.SystemColors.WindowText;
            this.PictureBox.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.PictureBox.Location = new System.Drawing.Point(131, 50);
            this.PictureBox.Name = "PictureBox";
            this.PictureBox.Size = new System.Drawing.Size(685, 636);
            this.PictureBox.TabIndex = 5;
            this.PictureBox.TabStop = false;
            // 
            // StopExposure
            // 
            this.StopExposure.Enabled = false;
            this.StopExposure.Location = new System.Drawing.Point(13, 92);
            this.StopExposure.Name = "StopExposure";
            this.StopExposure.Size = new System.Drawing.Size(75, 34);
            this.StopExposure.TabIndex = 6;
            this.StopExposure.Text = "Stop Exposure";
            this.StopExposure.UseVisualStyleBackColor = true;
            this.StopExposure.Click += new System.EventHandler(this.StopExposure_Click);
            // 
            // Disconnect
            // 
            this.Disconnect.Enabled = false;
            this.Disconnect.Location = new System.Drawing.Point(304, 13);
            this.Disconnect.Name = "Disconnect";
            this.Disconnect.Size = new System.Drawing.Size(75, 23);
            this.Disconnect.TabIndex = 7;
            this.Disconnect.Text = "Disconnect";
            this.Disconnect.UseVisualStyleBackColor = true;
            this.Disconnect.Click += new System.EventHandler(this.Disconnect_Click);
            // 
            // StopCooling
            // 
            this.StopCooling.Enabled = false;
            this.StopCooling.Location = new System.Drawing.Point(13, 200);
            this.StopCooling.Name = "StopCooling";
            this.StopCooling.Size = new System.Drawing.Size(75, 23);
            this.StopCooling.TabIndex = 8;
            this.StopCooling.Text = "Stop Cooling";
            this.StopCooling.UseVisualStyleBackColor = true;
            this.StopCooling.Click += new System.EventHandler(this.StopCooling_Click);
            // 
            // ConnectedLabel
            // 
            this.ConnectedLabel.AutoSize = true;
            this.ConnectedLabel.Location = new System.Drawing.Point(385, 18);
            this.ConnectedLabel.Name = "ConnectedLabel";
            this.ConnectedLabel.Size = new System.Drawing.Size(79, 13);
            this.ConnectedLabel.TabIndex = 9;
            this.ConnectedLabel.Text = "Not Connected";
            // 
            // ExposureStatus
            // 
            this.ExposureStatus.AutoSize = true;
            this.ExposureStatus.Location = new System.Drawing.Point(19, 146);
            this.ExposureStatus.Name = "ExposureStatus";
            this.ExposureStatus.Size = new System.Drawing.Size(24, 13);
            this.ExposureStatus.TabIndex = 10;
            this.ExposureStatus.Text = "Idle";
            // 
            // Temperature
            // 
            this.Temperature.AutoSize = true;
            this.Temperature.Location = new System.Drawing.Point(16, 239);
            this.Temperature.Name = "Temperature";
            this.Temperature.Size = new System.Drawing.Size(60, 13);
            this.Temperature.TabIndex = 11;
            this.Temperature.Text = "No Camera";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(13, 226);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(70, 13);
            this.label1.TabIndex = 12;
            this.label1.Text = "Temperature:";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(13, 133);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(54, 13);
            this.label2.TabIndex = 13;
            this.label2.Text = "Exposure:";
            // 
            // CMOSOptionsBox
            // 
            this.CMOSOptionsBox.Controls.Add(this.OffsetBox);
            this.CMOSOptionsBox.Controls.Add(this.GainBox);
            this.CMOSOptionsBox.Controls.Add(this.GOModeComboBox);
            this.CMOSOptionsBox.Controls.Add(this.EvenIlluminationCheckBox);
            this.CMOSOptionsBox.Controls.Add(this.PadDataCheckBox);
            this.CMOSOptionsBox.Location = new System.Drawing.Point(12, 264);
            this.CMOSOptionsBox.Name = "CMOSOptionsBox";
            this.CMOSOptionsBox.Size = new System.Drawing.Size(98, 242);
            this.CMOSOptionsBox.TabIndex = 14;
            this.CMOSOptionsBox.TabStop = false;
            this.CMOSOptionsBox.Text = "CMOS Options";
            this.CMOSOptionsBox.Visible = false;
            // 
            // OffsetBox
            // 
            this.OffsetBox.Controls.Add(this.OffsetUpDown);
            this.OffsetBox.Location = new System.Drawing.Point(6, 101);
            this.OffsetBox.Name = "OffsetBox";
            this.OffsetBox.Size = new System.Drawing.Size(85, 53);
            this.OffsetBox.TabIndex = 10;
            this.OffsetBox.TabStop = false;
            this.OffsetBox.Text = "Offset";
            this.OffsetBox.Visible = false;
            // 
            // OffsetUpDown
            // 
            this.OffsetUpDown.Location = new System.Drawing.Point(6, 19);
            this.OffsetUpDown.Name = "OffsetUpDown";
            this.OffsetUpDown.Size = new System.Drawing.Size(63, 20);
            this.OffsetUpDown.TabIndex = 3;
            this.OffsetUpDown.ValueChanged += new System.EventHandler(this.OffsetUpDown_ValueChanged);
            // 
            // GainBox
            // 
            this.GainBox.Controls.Add(this.GainUpDown);
            this.GainBox.Location = new System.Drawing.Point(6, 47);
            this.GainBox.Name = "GainBox";
            this.GainBox.Size = new System.Drawing.Size(85, 46);
            this.GainBox.TabIndex = 9;
            this.GainBox.TabStop = false;
            this.GainBox.Text = "Gain";
            this.GainBox.Visible = false;
            // 
            // GainUpDown
            // 
            this.GainUpDown.Location = new System.Drawing.Point(6, 19);
            this.GainUpDown.Name = "GainUpDown";
            this.GainUpDown.Size = new System.Drawing.Size(63, 20);
            this.GainUpDown.TabIndex = 1;
            this.GainUpDown.ValueChanged += new System.EventHandler(this.GainUpDown_ValueChanged);
            // 
            // GOModeComboBox
            // 
            this.GOModeComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.GOModeComboBox.FormattingEnabled = true;
            this.GOModeComboBox.Items.AddRange(new object[] {
            "Custom",
            "Low",
            "Medium",
            "High"});
            this.GOModeComboBox.Location = new System.Drawing.Point(7, 20);
            this.GOModeComboBox.Name = "GOModeComboBox";
            this.GOModeComboBox.Size = new System.Drawing.Size(85, 21);
            this.GOModeComboBox.TabIndex = 8;
            this.GOModeComboBox.SelectedIndexChanged += new System.EventHandler(this.GOModeComboBox_SelectedIndexChanged);
            // 
            // EvenIlluminationCheckBox
            // 
            this.EvenIlluminationCheckBox.AutoSize = true;
            this.EvenIlluminationCheckBox.CheckAlign = System.Drawing.ContentAlignment.BottomCenter;
            this.EvenIlluminationCheckBox.Location = new System.Drawing.Point(4, 205);
            this.EvenIlluminationCheckBox.Name = "EvenIlluminationCheckBox";
            this.EvenIlluminationCheckBox.Size = new System.Drawing.Size(91, 31);
            this.EvenIlluminationCheckBox.TabIndex = 7;
            this.EvenIlluminationCheckBox.Text = "Even Illumination";
            this.EvenIlluminationCheckBox.UseVisualStyleBackColor = true;
            this.EvenIlluminationCheckBox.CheckedChanged += new System.EventHandler(this.EvenIlluminationCheckBox_CheckedChanged);
            // 
            // PadDataCheckBox
            // 
            this.PadDataCheckBox.AutoSize = true;
            this.PadDataCheckBox.CheckAlign = System.Drawing.ContentAlignment.BottomCenter;
            this.PadDataCheckBox.Location = new System.Drawing.Point(20, 160);
            this.PadDataCheckBox.Name = "PadDataCheckBox";
            this.PadDataCheckBox.Size = new System.Drawing.Size(56, 31);
            this.PadDataCheckBox.TabIndex = 6;
            this.PadDataCheckBox.Text = "Pad Data";
            this.PadDataCheckBox.UseVisualStyleBackColor = true;
            this.PadDataCheckBox.CheckedChanged += new System.EventHandler(this.PadDataCheckBox_CheckedChanged);
            // 
            // FastModeBox
            // 
            this.FastModeBox.Controls.Add(this.fpsValueLabel);
            this.FastModeBox.Controls.Add(this.fpsLabel);
            this.FastModeBox.Controls.Add(this.StopFastModeButton);
            this.FastModeBox.Controls.Add(this.StartFastModeButton);
            this.FastModeBox.Controls.Add(this.BitSendModeBox);
            this.FastModeBox.Controls.Add(this.ExposureSpeedBox);
            this.FastModeBox.Location = new System.Drawing.Point(12, 512);
            this.FastModeBox.Name = "FastModeBox";
            this.FastModeBox.Size = new System.Drawing.Size(98, 174);
            this.FastModeBox.TabIndex = 15;
            this.FastModeBox.TabStop = false;
            this.FastModeBox.Text = "Fast Mode";
            this.FastModeBox.Visible = false;
            // 
            // StopFastModeButton
            // 
            this.StopFastModeButton.Enabled = false;
            this.StopFastModeButton.Location = new System.Drawing.Point(7, 117);
            this.StopFastModeButton.Name = "StopFastModeButton";
            this.StopFastModeButton.Size = new System.Drawing.Size(75, 23);
            this.StopFastModeButton.TabIndex = 3;
            this.StopFastModeButton.Text = "Stop FM";
            this.StopFastModeButton.UseVisualStyleBackColor = true;
            this.StopFastModeButton.Click += new System.EventHandler(this.StopFastModeButton_Click);
            // 
            // StartFastModeButton
            // 
            this.StartFastModeButton.Location = new System.Drawing.Point(7, 87);
            this.StartFastModeButton.Name = "StartFastModeButton";
            this.StartFastModeButton.Size = new System.Drawing.Size(75, 23);
            this.StartFastModeButton.TabIndex = 2;
            this.StartFastModeButton.Text = "Start FM";
            this.StartFastModeButton.UseVisualStyleBackColor = true;
            this.StartFastModeButton.Click += new System.EventHandler(this.StartFastModeButton_Click);
            // 
            // BitSendModeBox
            // 
            this.BitSendModeBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.BitSendModeBox.FormattingEnabled = true;
            this.BitSendModeBox.Items.AddRange(new object[] {
            "16 Bit",
            "12 Bit"});
            this.BitSendModeBox.Location = new System.Drawing.Point(7, 59);
            this.BitSendModeBox.Name = "BitSendModeBox";
            this.BitSendModeBox.Size = new System.Drawing.Size(84, 21);
            this.BitSendModeBox.TabIndex = 1;
            this.BitSendModeBox.SelectedIndexChanged += new System.EventHandler(this.BitSendModeBox_SelectedIndexChanged);
            // 
            // ExposureSpeedBox
            // 
            this.ExposureSpeedBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.ExposureSpeedBox.FormattingEnabled = true;
            this.ExposureSpeedBox.Items.AddRange(new object[] {
            "Power Save",
            "Normal Mode",
            "Fast Mode"});
            this.ExposureSpeedBox.Location = new System.Drawing.Point(6, 17);
            this.ExposureSpeedBox.Name = "ExposureSpeedBox";
            this.ExposureSpeedBox.Size = new System.Drawing.Size(85, 21);
            this.ExposureSpeedBox.TabIndex = 0;
            this.ExposureSpeedBox.SelectedIndexChanged += new System.EventHandler(this.ExposureSpeedBox_SelectedIndexChanged);
            // 
            // fpsLabel
            // 
            this.fpsLabel.AutoSize = true;
            this.fpsLabel.Location = new System.Drawing.Point(7, 147);
            this.fpsLabel.Name = "fpsLabel";
            this.fpsLabel.Size = new System.Drawing.Size(30, 13);
            this.fpsLabel.TabIndex = 4;
            this.fpsLabel.Text = "FPS:";
            // 
            // fpsValueLabel
            // 
            this.fpsValueLabel.AutoSize = true;
            this.fpsValueLabel.Location = new System.Drawing.Point(44, 147);
            this.fpsValueLabel.Name = "fpsValueLabel";
            this.fpsValueLabel.Size = new System.Drawing.Size(22, 13);
            this.fpsValueLabel.TabIndex = 5;
            this.fpsValueLabel.Text = "0.0";
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.SystemColors.Control;
            this.ClientSize = new System.Drawing.Size(828, 698);
            this.Controls.Add(this.FastModeBox);
            this.Controls.Add(this.CMOSOptionsBox);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.Temperature);
            this.Controls.Add(this.ExposureStatus);
            this.Controls.Add(this.ConnectedLabel);
            this.Controls.Add(this.StopCooling);
            this.Controls.Add(this.Disconnect);
            this.Controls.Add(this.StopExposure);
            this.Controls.Add(this.PictureBox);
            this.Controls.Add(this.StartCoolingButton);
            this.Controls.Add(this.StartExposureButton);
            this.Controls.Add(this.Connect);
            this.Controls.Add(this.CameraComboBox);
            this.Controls.Add(this.FindButton);
            this.Name = "Form1";
            this.Text = "DotNet Example";
            ((System.ComponentModel.ISupportInitialize)(this.PictureBox)).EndInit();
            this.CMOSOptionsBox.ResumeLayout(false);
            this.CMOSOptionsBox.PerformLayout();
            this.OffsetBox.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.OffsetUpDown)).EndInit();
            this.GainBox.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.GainUpDown)).EndInit();
            this.FastModeBox.ResumeLayout(false);
            this.FastModeBox.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button FindButton;
        private System.Windows.Forms.ComboBox CameraComboBox;
        private System.Windows.Forms.Button Connect;
        private System.Windows.Forms.Button StartExposureButton;
        private System.Windows.Forms.Button StartCoolingButton;
        private System.Windows.Forms.PictureBox PictureBox;
        private System.Windows.Forms.Button StopExposure;
        private System.Windows.Forms.Button Disconnect;
        private System.Windows.Forms.Button StopCooling;
        private System.Windows.Forms.Label ConnectedLabel;
        private System.Windows.Forms.Label ExposureStatus;
        private System.Windows.Forms.Label Temperature;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.GroupBox CMOSOptionsBox;
        private System.Windows.Forms.NumericUpDown OffsetUpDown;
        private System.Windows.Forms.NumericUpDown GainUpDown;
        private System.Windows.Forms.CheckBox EvenIlluminationCheckBox;
        private System.Windows.Forms.CheckBox PadDataCheckBox;
        private System.Windows.Forms.ComboBox GOModeComboBox;
        private System.Windows.Forms.GroupBox GainBox;
        private System.Windows.Forms.GroupBox OffsetBox;
        private System.Windows.Forms.GroupBox FastModeBox;
        private System.Windows.Forms.ComboBox BitSendModeBox;
        private System.Windows.Forms.ComboBox ExposureSpeedBox;
        private System.Windows.Forms.Button StopFastModeButton;
        private System.Windows.Forms.Button StartFastModeButton;
        private System.Windows.Forms.Label fpsValueLabel;
        private System.Windows.Forms.Label fpsLabel;
    }
}

