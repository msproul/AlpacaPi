using System;
using System.Runtime.InteropServices;

namespace DotNetExample
{
    public class FastImage
    {
        public void Set(int x, int y, int w, int h, int binx, int biny, IntPtr imageBuffer)
        {
            this.x = x;
            this.y = y;
            this.w = w;
            this.h = h;
            this.binx = binx;
            this.biny = biny;
            pixels = new byte[w * h * 2];

            Marshal.Copy(imageBuffer, pixels, 0, w * h * 2);
        }

        int x;
        int y;
        int w;
        int h;
        int binx;
        int biny;
        public byte[] pixels;
    }

    public class FastModeManager
    {
        public FastModeManager()
        {
            one = new FastImage();
            two = new FastImage();
            current = new FastImage();
            padlock = new Object();
        }

        public void Update(int x, int y, int w, int h, int binx, int biny, IntPtr imageBuffer)
        {
            lock (padlock)
            {
                current = update ? one : two;
                current.Set(x, y, w, h, binx, biny, imageBuffer);
                update = !update;
                updated = true;
            }
        }

        public byte[] GetImage()
        {
            lock (padlock)
            {
                if (updated)
                {
                    updated = false;
                    return current.pixels;
                }
                else
                    return null;
            }
        }

        bool update, updated;
        FastImage one;
        FastImage two;
        FastImage current;
        Object padlock;
    }
}
