using System;
using System.Collections.Generic;
using System.Drawing;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Windows.Forms;
using System.Drawing.Imaging;
using System.Diagnostics;

namespace DotNetExample
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
            ComboBoxCameras = new List<ComboBoxCamera>();
            fmm = new FastModeManager();
            fastCallback = OnFastImage;
            temp = new Thread(UpdateTemperature);
            temp.Start();
        }

        protected override void OnFormClosed(FormClosedEventArgs e)
        {
            checkTemperature = false;
            appRunning = false;
            base.OnFormClosed(e);
            AtikPInvoke.ArtemisShutdown();
        }

        void UpdateTemperature()
        {
            while (appRunning)
            {
                while (checkTemperature)
                {
                    int t = 0;
                    AtikPInvoke.ArtemisTemperatureSensorInfo(handle, 1, ref t);
                    double tD = t * 0.01;
                    Temperature.BeginInvoke(new Action(() => Temperature.Text = tD.ToString()));
                    Thread.Sleep(1000);
                }
            }
        }

        void OnFastImage(IntPtr handle, int x, int y, int w, int h, int binx, int biny, IntPtr imageBuffer)
        {
            if (this.handle != handle)
                return;

            if (fastModeNmrImage == 0)
                fastModeSW = Stopwatch.StartNew();

            ++fastModeNmrImage;

            if (fastModeNmrImage > 1)
            {
                double fps = (fastModeNmrImage - 1) / (0.001 * fastModeSW.ElapsedMilliseconds);
                fpsValueLabel.BeginInvoke(new Action(() =>
                    fpsValueLabel.Text = Math.Round(fps, 2).ToString()));
            }

            fmw = w;
            fmh = h;
            fmm.Update(x, y, w, h, binx, biny, imageBuffer);
        }

        void DisplayFastModeImages()
        {
            while (appRunning)
            {
                if(fastMode)
                {
                    byte[] rawImg = fmm.GetImage();

                    if (rawImg != null)
                    {
                        // create enough space for a 24 bit per pixel bitmap of the image
                        byte[] bmpBytes = new byte[fmw * fmh * 3];
                        for (int i = 0, j = 0; i < fmw * fmh; ++i, j += 2)
                        {
                            if (stopProcessing)
                            {
                                stopProcessing = false;
                                return;
                            }

                            var val = rawImg[j];
                            // we have a mono image so we place the same value into each bitmap byte
                            bmpBytes[i] = val;
                            bmpBytes[++i] = val;
                            bmpBytes[++i] = val;
                        }

                        var pic = new Bitmap(fmw, fmh);
                        var rect = new Rectangle(0, 0, fmw, fmh);
                        var picData = pic.LockBits(rect, ImageLockMode.WriteOnly, PixelFormat.Format24bppRgb);
                        Marshal.Copy(bmpBytes, 0, picData.Scan0, fmw * 3 * fmh);
                        pic.UnlockBits(picData);

                        PictureBox.BeginInvoke(new Action(() => PictureBox.Image = pic));
                    }
                }
            }
        }

        private void Find_Click(object sender, EventArgs e)
        {
            ComboBoxCameras.Clear();

            AtikPInvoke.ArtemisRefreshDevicesCount();
            for (var i = 0; i < 16; ++i)
            {
                if (AtikPInvoke.ArtemisDevicePresent(i))
                {
                    StringBuilder sbName = new StringBuilder();
                    StringBuilder sbSerial = new StringBuilder();
                    AtikPInvoke.ArtemisDeviceName(i, sbName);
                    AtikPInvoke.ArtemisDeviceSerial(i, sbSerial);

                    ComboBoxCameras.Add(new ComboBoxCamera(i, $"{sbName} {sbSerial}"));
                }
            }

            CameraComboBox.Items.Clear();
            foreach (var camera in ComboBoxCameras)
                CameraComboBox.Items.Add(camera);
        }

        private void Connect_Click(object sender, EventArgs e)
        {
            var selected = (ComboBoxCamera)CameraComboBox.SelectedItem;
            if (selected != null)
            {
                handle = AtikPInvoke.ArtemisConnect(selected.id);
                if (handle.ToInt32() != 0)
                {
                    ConnectedLabel.Text = "Connected";
                    Disconnect.Enabled = true;
                    Connect.Enabled = false;
                    StartExposureButton.Enabled = true;

                    // Using 0 for the sensor checks the number of sensors
                    // rather than checking for a temperature
                    int num = 0;
                    AtikPInvoke.ArtemisTemperatureSensorInfo(handle, 0, ref num);
                    if (num != 0)
                    {
                        StartCoolingButton.Enabled = true;
                        checkTemperature = true;
                    }
                    else
                    {
                        Temperature.Text = "No Cooling";
                    }
                }

                if(AtikPInvoke.ArtemisHasCameraSpecificOption(handle, (ushort) AtikCameraSpecificOptions.ID_GOCustomGain) &&
                   AtikPInvoke.ArtemisHasCameraSpecificOption(handle, (ushort)AtikCameraSpecificOptions.ID_GOCustomOffset) &&
                   AtikPInvoke.ArtemisHasCameraSpecificOption(handle, (ushort)AtikCameraSpecificOptions.ID_PadData) &&
                   AtikPInvoke.ArtemisHasCameraSpecificOption(handle, (ushort)AtikCameraSpecificOptions.ID_EvenIllumination))
                {
                    CMOSOptionsBox.Visible = true;

                    int length = 0;

                    byte[] go = new byte[2];
                    AtikPInvoke.ArtemisCameraSpecificOptionGetData(handle, (ushort)AtikCameraSpecificOptions.ID_GOPresetMode, go, 2, ref length);
                    GOModeComboBox.SelectedIndex = go[0];

                    if (go[0] == 0)
                    {
                        GainBox.Visible = true;
                        byte[] gain = new byte[6];
                        AtikPInvoke.ArtemisCameraSpecificOptionGetData(handle, (ushort)AtikCameraSpecificOptions.ID_GOCustomGain, gain, 6, ref length);
                        var gainMax = (gain[3] << 8) + gain[2];
                        var gainVal = (gain[5] << 8) + gain[4];
                        GainUpDown.Minimum   = 0;
                        GainUpDown.Maximum   = gainMax;
                        GainUpDown.Value     = gainVal;

                        OffsetBox.Visible = true;
                        byte[] offset = new byte[6];
                        AtikPInvoke.ArtemisCameraSpecificOptionGetData(handle, (ushort)AtikCameraSpecificOptions.ID_GOCustomOffset, offset, 6, ref length);
                        var offsetMax = (offset[3] << 8) + offset[2];
                        var offsetVal = (offset[5] << 8) + offset[4];
                        OffsetUpDown.Minimum = 0;
                        OffsetUpDown.Maximum = offsetMax;
                        OffsetUpDown.Value   = offsetVal;
                    }

                    byte[] padData = new byte[1];
                    AtikPInvoke.ArtemisCameraSpecificOptionGetData(handle, (ushort)AtikCameraSpecificOptions.ID_PadData, padData, 1, ref length);
                    PadDataCheckBox.Checked = Convert.ToBoolean(padData[0]);

                    byte[] evenIllu = new byte[1];
                    AtikPInvoke.ArtemisCameraSpecificOptionGetData(handle, (ushort)AtikCameraSpecificOptions.ID_EvenIllumination, evenIllu, 1, ref length);
                    EvenIlluminationCheckBox.Checked = Convert.ToBoolean(evenIllu[0]);
                }

                if(AtikPInvoke.ArtemisHasFastMode(handle))
                {
                    int length = 0;
                    FastModeBox.Visible = true;

                    byte[] expSpeed = new byte[2];
                    AtikPInvoke.ArtemisCameraSpecificOptionGetData(handle, (ushort)AtikCameraSpecificOptions.ID_ExposureSpeed, expSpeed, 2, ref length);
                    ExposureSpeedBox.SelectedIndex = expSpeed[0];

                    byte[] bitSend = new byte[2];
                    AtikPInvoke.ArtemisCameraSpecificOptionGetData(handle, (ushort)AtikCameraSpecificOptions.ID_BitSendMode, bitSend, 2, ref length);
                    BitSendModeBox.SelectedIndex = bitSend[0];

                    AtikPInvoke.ArtemisSetFastCallback(handle, fastCallback);
                    fastMode = true;
                    Thread fastModeReceive = new Thread(DisplayFastModeImages);
                    fastModeReceive.Start();
                }
            }
        }

        private void Disconnect_Click(object sender, EventArgs e)
        {
            checkTemperature = false;
            // Stop cooling otherwise camera will continue 
            // to cool when disconnected
            if (StopCooling.Enabled)
            {
                AtikPInvoke.ArtemisCoolerWarmUp(handle);
                StopCooling.Enabled = false;
            }

            AtikPInvoke.ArtemisStopExposure(handle);
            AtikPInvoke.ArtemisDisconnect(handle);
            ConnectedLabel.Text = "Not Connected";
            Temperature.Text = "No Camera";

            Disconnect.Enabled = false;
            Connect.Enabled = true;
            StartCoolingButton.Enabled = false;
            StartExposureButton.Enabled = false;
            CMOSOptionsBox.Visible = false;
            FastModeBox.Visible = false;
            fastModeNmrImage = 0;
        }

        private void StartExposureButton_Click(object sender, EventArgs e)
        {
            if (handle.ToInt32() != 0)
            {
                if (AtikPInvoke.ArtemisStartExposure(handle, 2) == (int)ArtemisError.OK)
                {
                    ExposureStatus.Text = "Starting Exposure";
                    Thread t1 = new Thread(WaitForImage);
                    t1.Start();

                    StopExposure.Enabled = true;
                    StartExposureButton.Enabled = false;
                }
                else
                {
                    ExposureStatus.Text = "Failed To Start Exposure";
                }
            }
        }

        private void WaitForImage()
        {
            ExposureStatus.BeginInvoke(new Action(() => ExposureStatus.Text = "Waiting For Image"));
            while (!AtikPInvoke.ArtemisImageReady(handle))
            {
                if (stopProcessing)
                {
                    stopProcessing = false;
                    return;
                }

                Thread.Sleep(100);
            }

            int x = 0, y = 0, w = 0, h = 0, binX = 0, binY = 0;
            AtikPInvoke.ArtemisGetImageData(handle, ref x, ref y,
                                                    ref w, ref h,
                                                    ref binX, ref binY);

            // Create memory to copy pixels into
            byte[] pix = new byte[w * h * 2];

            var intPtr = AtikPInvoke.ArtemisImageBuffer(handle);
            ExposureStatus.BeginInvoke(new Action(() => ExposureStatus.Text = "Converting"));

            Marshal.Copy(intPtr, pix, 0, w * h * 2);

            // create enough space for a 24 bit per pixel bitmap of the image
            byte[] bmpBytes = new byte[w * h * 3];
            for (int i = 0, j = 0; i < w * h; ++i, j += 2)
            {
                if (stopProcessing)
                {
                    stopProcessing = false;
                    return;
                }

                var val = pix[j];
                // we have a mono image so we place the same value into each bitmap byte
                bmpBytes[i]   = val;
                bmpBytes[++i] = val;
                bmpBytes[++i] = val;
            }

            var pic = new Bitmap(w, h);
            var rect = new Rectangle(0, 0, w, h);
            var picData = pic.LockBits(rect, ImageLockMode.WriteOnly, PixelFormat.Format24bppRgb);
            Marshal.Copy(bmpBytes, 0, picData.Scan0, w * 3 * h);
            pic.UnlockBits(picData);

            PictureBox.BeginInvoke(new Action(() => PictureBox.Image = pic));
            ExposureStatus.BeginInvoke(new Action(() => ExposureStatus.Text = "Idle"));

            StartExposureButton.BeginInvoke(new Action(() => StartExposureButton.Enabled = true));
            StopExposure.BeginInvoke(new Action(() => StopExposure.Enabled = false));
        }

        private void StopExposure_Click(object sender, EventArgs e)
        {
            AtikPInvoke.ArtemisStopExposure(handle);
            stopProcessing = true;

            StopExposure.Enabled = false;
            StartExposureButton.Enabled = true;
        }

        private void StartCoolingButton_Click(object sender, EventArgs e)
        {
            if (handle != null)
            {
                AtikPInvoke.ArtemisSetCooling(handle, -10);

                StopCooling.Enabled = true;
                StartCoolingButton.Enabled = false;
            }
        }

        private void StopCooling_Click(object sender, EventArgs e)
        {
            AtikPInvoke.ArtemisCoolerWarmUp(handle);

            StopCooling.Enabled = false;
            StartCoolingButton.Enabled = true;
        }

        private void CameraComboBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            Connect.Enabled = true;
        }

        private void GOModeComboBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            byte[] go = new byte[2];
            go[0] = (byte)GOModeComboBox.SelectedIndex;
            AtikPInvoke.ArtemisCameraSpecificOptionSetData(handle, (ushort)AtikCameraSpecificOptions.ID_GOPresetMode, go, 2);

            if(GOModeComboBox.SelectedIndex == 0)
            {
                GainBox.Visible = true;
                OffsetBox.Visible = true;
            }
            else
            {
                GainBox.Visible = false;
                OffsetBox.Visible = false;
            }
        }

        private void GainUpDown_ValueChanged(object sender, EventArgs e)
        {
            byte[] gain = new byte[2];
            gain[0] = (byte)GainUpDown.Value;
            AtikPInvoke.ArtemisCameraSpecificOptionSetData(handle, (ushort)AtikCameraSpecificOptions.ID_GOCustomGain, gain, 2);
        }

        private void OffsetUpDown_ValueChanged(object sender, EventArgs e)
        {
            byte[] offset = new byte[2];
            offset[0] = (byte)((int)OffsetUpDown.Value & 0x00FF);
            offset[1] = (byte)((int)OffsetUpDown.Value >> 8);
            AtikPInvoke.ArtemisCameraSpecificOptionSetData(handle, (ushort)AtikCameraSpecificOptions.ID_GOCustomOffset, offset, 2);
        }

        private void PadDataCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            byte[] padData = new byte[1];
            padData[0] = Convert.ToByte(PadDataCheckBox.Checked);
            AtikPInvoke.ArtemisCameraSpecificOptionSetData(handle, (ushort)AtikCameraSpecificOptions.ID_PadData, padData, 1);
        }

        private void EvenIlluminationCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            byte[] evenIllu = new byte[1];
            evenIllu[0] = Convert.ToByte(EvenIlluminationCheckBox.Checked);
            AtikPInvoke.ArtemisCameraSpecificOptionSetData(handle, (ushort)AtikCameraSpecificOptions.ID_EvenIllumination, evenIllu, 1);
        }

        private void ExposureSpeedBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            byte[] expSpeed = new byte[2];
            expSpeed[0] = (byte)ExposureSpeedBox.SelectedIndex;
            AtikPInvoke.ArtemisCameraSpecificOptionSetData(handle, (ushort)AtikCameraSpecificOptions.ID_ExposureSpeed, expSpeed, 2);
        }

        private void BitSendModeBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            byte[] bitSend = new byte[2];
            bitSend[0] = (byte)BitSendModeBox.SelectedIndex;
            AtikPInvoke.ArtemisCameraSpecificOptionSetData(handle, (ushort)AtikCameraSpecificOptions.ID_BitSendMode, bitSend, 2);
        }

        private void StartFastModeButton_Click(object sender, EventArgs e)
        {
            AtikPInvoke.ArtemisStartFastExposure(handle, 1);
            fastMode = true;
            StartFastModeButton.Enabled = false;
            StopFastModeButton.Enabled = true;
        }

        private void StopFastModeButton_Click(object sender, EventArgs e)
        {
            AtikPInvoke.ArtemisStopExposure(handle);
            StartFastModeButton.Enabled = true;
            StopFastModeButton.Enabled = false;
            fastMode = false;
            fastModeNmrImage = 0;
        }

        IntPtr handle;

        List<ComboBoxCamera> ComboBoxCameras;

        bool stopProcessing;
        bool checkTemperature;
        bool appRunning = true;
        Thread temp;

        ArtemisSetFastCallback fastCallback;
        bool fastMode;
        FastModeManager fmm;
        int fmw, fmh;
        int fastModeNmrImage;
        Stopwatch fastModeSW;
    }

    public class ComboBoxCamera
    {
        public ComboBoxCamera(int id, string name)
        {
            this.id = id;
            this.name = name;
        }

        public override string ToString()
        {
            return name;
        }

        public int id;
        public string name;
    }
}
