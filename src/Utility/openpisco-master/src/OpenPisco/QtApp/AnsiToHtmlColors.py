# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

import re


COLOR_DICT = {
    '31': [(255, 0, 0), (128, 0, 0)],
    '32': [(0, 255, 0), (0, 128, 0)],
    '33': [(255, 255, 0), (128, 128, 0)],
    '34': [(0, 0, 255), (0, 0, 128)],
    '35': [(255, 0, 255), (128, 0, 128)],
    '36': [(0, 255, 255), (0, 128, 128)],
}

COLOR_REGEX = re.compile(r'\x1b\[(?P<arg_1>\d+)(;(?P<arg_2>\d+)(;(?P<arg_3>\d+))?)?m')

BOLD_TEMPLATE = '<span style="color:rgb{}; font-weight: bolder">'
LIGHT_TEMPLATE = '<span style="color:rgb{}">'


def ansi_to_html(text):
    """Change the color of a text string from ansi to html."""

    text = text.replace('\n', '<BR>')
    text = text.replace('\x1b[0m', '</span>')

    def single_sub(match):
        argsdict = match.groupdict()
        if argsdict['arg_3'] is None:
            if argsdict['arg_2'] is None:
                color, bold = argsdict['arg_1'], 0# pragma: no cover
            else:
                color, bold = argsdict['arg_1'], int(argsdict['arg_2'])
        else:
            color, bold = argsdict['arg_2'], int(argsdict['arg_3'])# pragma: no cover

        if bold:
            return BOLD_TEMPLATE.format(COLOR_DICT[color][1])
        return LIGHT_TEMPLATE.format(COLOR_DICT[color][0])# pragma: no cover

    return COLOR_REGEX.sub(single_sub, text)# pragma: no cover

#print ansi_to_html('[06-10-13 21:28:23] [INFO] [0;31;1mUsage: /kick [reason ...][m')
#print ansi_to_html('[06-10-13 21:28:23] [INFO] [31;0mUsage: /kick [reason ...][m')
#print '[06-10-13 21:28:23] [INFO] [31mUsage: /kick [reason ...][m'
#print ansi_to_html('[06-10-13 21:28:23] [INFO] [31mUsage: /kick [reason ...][m')

def CheckIntegrity():
    from Muscat.Helpers.TextFormatHelper import TFormat as TF
    print(ansi_to_html(TF.InRed("hola") ))
    print(ansi_to_html(TF.InRed("hola\n") ))
    return "ok"


if __name__ == '__main__':
    CheckIntegrity() # pragma: no cover
