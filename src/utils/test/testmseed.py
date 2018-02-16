"""Test for the mseedlite library."""
import unittest
import os
from seiscomp.mseedlite import Input, Record
from math import log


class MSeedLiteTests(unittest.TestCase):
    """Test the functionality of mseedlite.py"""

    def testReading(self):
        """Read an MSEED file"""

        with open('waveform.mseed', 'rb') as fin:
            inp = Input(fin)
            for rec in inp:
                self.assertEqual(rec.net, 'GE', 'Wrong network code!')
                self.assertEqual(rec.sta, 'APE', 'Wrong station code!')
                self.assertEqual(rec.loc, '', 'Wrong location code!')
                self.assertEqual(rec.begin_time.year, 2008, 'Wrong year!')
                self.assertEqual(rec.begin_time.month, 1, 'Wrong month!')
                self.assertEqual(rec.begin_time.day, 1, 'Wrong day!')
                self.assertEqual(rec.begin_time.hour, 0, 'Wrong hour!')


    def testWriting(self):
        """Write an MSEED file"""

        with open('waveform.mseed', 'rb') as fin:
            with open('delete.me', 'wb') as fout:
                inp = Input(fin)
                for rec in inp:
                    rec.write(fout, int(log(rec.size, 2)))

        with open('waveform.mseed', 'rb') as orig:
            with open('delete.me', 'rb') as copy:
                msg = 'The copy of the waveform differs from original!'
                self.assertEqual(orig.read(), copy.read(), msg)

        os.remove('delete.me')


    def testRecord(self):
        """Read MSEED record from string"""

        with open('waveform.mseed', 'rb') as fin:
            with open('delete.me', 'wb') as fout:
                data = fin.read(512)
                while data:
                    rec = Record(data)
                    self.assertEqual(rec.net, 'GE', 'Wrong network code!')
                    self.assertEqual(rec.sta, 'APE', 'Wrong station code!')
                    self.assertEqual(rec.loc, '', 'Wrong location code!')
                    self.assertEqual(rec.begin_time.year, 2008, 'Wrong year!')
                    self.assertEqual(rec.begin_time.month, 1, 'Wrong month!')
                    self.assertEqual(rec.begin_time.day, 1, 'Wrong day!')
                    self.assertEqual(rec.begin_time.hour, 0, 'Wrong hour!')
                    rec.write(fout, int(log(rec.size, 2)))
                    data = fin.read(512)

        with open('waveform.mseed', 'rb') as orig:
            with open('delete.me', 'rb') as copy:
                msg = 'The copy of the waveform differs from original!'
                self.assertEqual(orig.read(), copy.read(), msg)

        os.remove('delete.me')


    def testModifying(self):
        """Modify and write an MSEED file"""

        with open('waveform.mseed', 'rb') as fin:
            with open('delete.me', 'wb') as fout:
                inp = Input(fin)
                for rec in inp:
                    rec.net = 'GX'
                    rec.sta = rec.sta[:3] + 'X'
                    rec.loc = '01'
                    rec.cha = 'X' + rec.cha[1:]
                    rec.write(fout, int(log(rec.size, 2)))

        with open('outputref.mseed', 'rb') as refer:
            with open('delete.me', 'rb') as modif:
               msg = 'The copy of the waveform differs from original!'
               self.assertEqual(refer.read(), modif.read(), msg)

        os.remove('delete.me')


    def testMerge(self):
        """Merge MSEED records"""

        with open('waveform.mseed', 'rb') as fin:
            with open('delete.me', 'wb') as fout:
                inp = Input(fin)
                out_rec = None
                for rec in inp:
                    if out_rec is None:
                        out_rec = rec

                    else:
                        out_rec.merge(rec)

                    if out_rec.size >= 4096:
                        out_rec.write(fout, int(log(4096, 2)))
                        out_rec = None

                if out_rec is not None:
                    out_rec.write(fout, int(log(4096, 2)))
                    out_rec = None

        with open('mergeref.mseed', 'rb') as refer:
            with open('delete.me', 'rb') as modif:
               msg = 'The copy of the waveform differs from original!'
               self.assertEqual(refer.read(), modif.read(), msg)

        os.remove('delete.me')


def main():
    unittest.main()


if __name__ == '__main__':
    main()
