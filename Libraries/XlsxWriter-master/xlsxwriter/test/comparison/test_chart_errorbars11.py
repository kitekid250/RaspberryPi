###############################################################################
#
# Tests for XlsxWriter.
#
# Copyright (c), 2013-2014, John McNamara, jmcnamara@cpan.org
#

import unittest
import os
from ...workbook import Workbook
from ..helperfunctions import _compare_xlsx_files


class TestCompareXLSXFiles(unittest.TestCase):
    """
    Test file created by XlsxWriter against a file created by Excel.

    """

    def setUp(self):
        self.maxDiff = None

        filename = 'chart_errorbars10.xlsx'

        test_dir = 'xlsxwriter/test/comparison/'
        self.got_filename = test_dir + '_test_chart_errorbars11.xlsx'
        self.exp_filename = test_dir + 'xlsx_files/' + filename

        self.ignore_files = []

        # Test for issue #115. We don't add plus_data and minus_data, like in
        # test_chart_errorbars10.py, as would be done from user API. Instead
        # we ignore the point data in the comparison test.
        self.ignore_elements = {'xl/charts/chart1.xml':
                                ['<c:ptCount', '<c:pt', '<c:v', '</c:pt>']}

    def test_create_file(self):
        """Test the creation of an XlsxWriter file with error bars."""
        filename = self.got_filename

        ####################################################

        workbook = Workbook(filename)

        worksheet = workbook.add_worksheet()
        chart = workbook.add_chart({'type': 'line'})

        chart.axis_ids = [69198976, 69200896]

        data = [
            [1, 2, 3, 4, 5],
            [2, 4, 6, 8, 10],
            [3, 6, 9, 12, 15],

        ]

        worksheet.write_column('A1', data[0])
        worksheet.write_column('B1', data[1])
        worksheet.write_column('C1', data[2])

        chart.add_series({
            'categories': '=Sheet1!$A$1:$A$5',
            'values': '=Sheet1!$B$1:$B$5',
            'y_error_bars': {
                'type': 'custom',
                'plus_values': '=Sheet1!$A$1',
                'minus_values': '=Sheet1!$B$1:$B$3',
            },
        })

        chart.add_series({
            'categories': '=Sheet1!$A$1:$A$5',
            'values': '=Sheet1!$C$1:$C$5',
        })

        worksheet.insert_chart('E9', chart)

        workbook.close()

        ####################################################

        got, exp = _compare_xlsx_files(self.got_filename,
                                       self.exp_filename,
                                       self.ignore_files,
                                       self.ignore_elements)

        self.assertEqual(got, exp)

    def tearDown(self):
        # Cleanup.
        if os.path.exists(self.got_filename):
            os.remove(self.got_filename)


if __name__ == '__main__':
    unittest.main()
