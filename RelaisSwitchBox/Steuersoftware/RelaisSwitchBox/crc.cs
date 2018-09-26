using System;

namespace Crc
{
    /// <summary>
    /// Checksummen Berechnung für "8 Bit CCITT"
    /// </summary>
    public  class Crc8_CCITT
    {
        public  byte cmdCrc8CCITTUpdate(byte inCrc, byte poly , byte inData)
        {
            byte i = 0;
            byte data = 0;

            data = (byte)(inCrc ^ inData);

            for (i = 0; i < 8; i++)
            {
                if ((data & 0x80) != 0)
                {
                    data <<= 1;
                    data ^= poly;
                }
                else
                {
                    data <<= 1;
                }
            }

            return data;
        }

        public  byte cmdCrc8StrCCITT(string str , byte poly , byte beginnCrc )
        {
            byte crc = beginnCrc;
            byte x = 0;

            for (x = 0; x < str.Length; x++)
            {
                crc = cmdCrc8CCITTUpdate( crc , poly , (byte)str[x] );
            }

            return crc;
        }
    }
}



