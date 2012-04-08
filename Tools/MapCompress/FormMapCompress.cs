using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.IO.Compression;

namespace MapCompress
{
    public partial class FormMapCompress : Form
    {
        public FormMapCompress()
        {
            InitializeComponent();
        }

        private void BtnConvert_Click(object sender, EventArgs e)
        {
            if (DlgFileOpen.ShowDialog() == DialogResult.OK)
            {
                TxtBox.AppendText("Converting : " + DlgFileOpen.FileName);
                Convert(DlgFileOpen.FileName);
            }
        }

        private void Convert(string FileName)
        {
            ushort[,] ClientMap=new ushort[3072,3072];
            ushort[] TmpUnpack=new ushort[128*128];

            string ShortName = Path.GetFileNameWithoutExtension(FileName);

            FileStream MapFile = File.OpenRead(FileName);
            BinaryReader Br=new BinaryReader(MapFile);

            TxtBox.AppendText("Unpacking Map...");

            long Index1,Index2;
            int Num = 0;
            for (int NumY = 0; NumY < 24; NumY++)
            {
                for (int NumX = 0; NumX < 24; NumX++)
                {
                    Index1 = MapFile.Position + Num;                
                    MapFile.Seek(Index1, SeekOrigin.Begin);
                    Index2 = MapFile.Position + Br.ReadUInt32();
                    MapFile.Seek(Index2, SeekOrigin.Begin);
                    Decomp(Br, TmpUnpack);
                    Num++;

                    uint TmpIdx=0;
                    for (int y = 0; y < 128; y++)
                    {
                        for (int x = 0; x < 128; x++)
                        {
                            ClientMap[NumX<<7+x,NumY<<7+y]=TmpUnpack[TmpIdx];
                            TmpIdx++;
                        }
                    }
                }
            }

            ushort[] MapId=new ushort[8192];
            for (int i = 0; i < 8192; i++)
                MapId[i] = 0;

            for (int j = 0; j < 3072; j++)
                for (int i = 0; i < 3072; i++)
                    MapId[ClientMap[i, j]]++;

            int IdCount = 0;
            for (int i = 0; i < 8192; i++)
                if (MapId[i] > 0)
                    IdCount++;

            TxtBox.AppendText("There is " + IdCount + " Unique Id on that map.");

            TxtBox.AppendText("Computing .dat");
            ComputeMultiDat(ShortName, ClientMap);      

            TxtBox.AppendText("GZip Compress....");

            FileStream Fs = new FileStream(ShortName+".zlb", FileMode.Create, FileAccess.Write);
            using (GZipStream gzStream = new GZipStream(Fs, CompressionMode.Compress))
            {
                BinaryWriter Bw = new BinaryWriter(gzStream);
                for (int j = 0; j < 3072; j++)
                    for (int i = 0; i < 3072; i++)
                        Bw.Write(ClientMap[i, j]);
            }
            TxtBox.AppendText("Zlb Map Saved....");         
        }

        private void ComputeMultiDat(string ShortName,ushort [,] ClientMap)
        {
            FileStream MapFile = File.Create(ShortName+".dat");
            BinaryWriter Bw = new BinaryWriter(MapFile);

            MapIndexRecord [] IdIndex=new MapIndexRecord[24*24*2];

            //reserve size for the IdIndex
            for (int i = 0; i < 24 * 24 * 2; i++)
                Bw.Write((UInt64)0);


            int ZoneCount = 3072 / 128;
            int ActualIndex = 0;
            uint Position = 24 * 24 * 2 * 8;

            for (int j=0;j<ZoneCount;j++)
                for (int i = 0; i < ZoneCount; i++)
                {
                    //compute even zone
                    IdIndex[ActualIndex].Index = Position;
                    IdIndex[ActualIndex].IndexSize = MakeZone(i * 128, j * 128, ClientMap, Bw);
                    Position += IdIndex[ActualIndex].IndexSize * 2;
                    ActualIndex++;
                    //compute Odd zone
                    IdIndex[ActualIndex].Index = Position;
                    IdIndex[ActualIndex].IndexSize = MakeZone(i * 128, j * 128, ClientMap, Bw);
                    Position += IdIndex[ActualIndex].IndexSize * 2;
                    ActualIndex++;
                }

            Bw.BaseStream.Seek(0, SeekOrigin.Begin);

            for (int i = 0; i < IdIndex.Length; i++)
            {
                Bw.Write(IdIndex[i].Index);
                Bw.Write(IdIndex[i].IndexSize);
            }
        }

        private uint MakeZone(int PosX, int PosY, ushort[,] ClientMap, BinaryWriter Bw)
        {
            int XMin = Math.Max(0, PosX);
            int XMax = Math.Min(3071, PosX + 127);
            int YMin = Math.Max(0, PosY);
            int YMax = Math.Min(3071, PosY + 127);

            ushort [] IdArray=new ushort[8192];
            for (int i = 0; i < 8192; i++)
                IdArray[i] = 0;

            for (int j = YMin; j <= YMax; j++)
                for (int i = XMin; i <= XMax; i++)
                    IdArray[ClientMap[i, j]]++;

            uint IdCount = 0;
            for (int i = 0; i < 8192; i++)
            {
                if (IdArray[i] > 0)
                {
                    IdCount++;
                    Bw.Write((ushort)i);
                }
            }
            return IdCount;
        }

        private void Decomp(BinaryReader Br, ushort[] TmpUnpack)
        {
            
            ushort Val, Val2;
            int Decoded=0;
            int TmpIndex=0;
            do
            {
                Val = Br.ReadUInt16();
                if (Val < 0x2000)
                {
                    TmpUnpack[TmpIndex] = Val;
                    TmpIndex++;
                    Decoded++;
                }
                else
                {
                    Val2 = Br.ReadUInt16();
                    for (int i = 0; i < (Val - 0x2000); i++)
                    {
                        TmpUnpack[TmpIndex] = Val2;
                        TmpIndex++;
                        Decoded++;
                    }
                }
            } while (Decoded >= 0x4000);
        }
    }

    class MapIndexRecord
    {
        public uint Index;
        public uint IndexSize;

        public MapIndexRecord()
        {
        }
    }
}
