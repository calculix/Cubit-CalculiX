# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

from OpenPisco.QtApp.QtImplementation import (
    QRegularExpression,
    QColor,
    QTextCharFormat,
    QFont,
    QSyntaxHighlighter,
)

# Inspired by https://wiki.python.org/moin/PyQt/Python%20syntax%20highlighting
def format(color, style=''):
    """Return a QTextCharFormat with the given attributes.
    """
    _color = QColor()
    _color.setNamedColor(color)

    _format = QTextCharFormat()
    _format.setForeground(_color)
    if 'bold' in style:
        _format.setFontWeight(QFont.Bold)
    if 'italic' in style:
        _format.setFontItalic(True)

    return _format


# Syntax styles that can be shared by all languages
STYLES = {
    'keyword': format('blue'),
    'attributes': format('black', 'bold'),
    'attributes_value': format('brown'),

    'id=': format('red'),
    'brace': format('darkGray'),

    'string': format('magenta'),
    'string2': format('darkMagenta'),
    'comment': format('darkGreen', 'italic'),
    'self': format('black', 'italic'),
}


class PythonHighlighter (QSyntaxHighlighter):
    """Syntax highlighter for the Python language.
    """
    # Python keywords
    keywords = [
        'data', 'Zones', 'Grids', 'LevelSets', 'PhysicalProblems', 'OptimProblems',
        'TopologicalOptimizations', 'Outputs', 'Actions'
    ]

    # Python operators
    operators = [
        '=',
        # Comparison
        '==', '!=', '<', '<=', '>', '>=',
        # Arithmetic
        r'\+', '-', r'\*', '/', '//', r'\%', r'\*\*',
        # In-place
        r'\+=', '-=', r'\*=', '/=', r'\%=',
        # Bitwise
        r'\^', r'\|', r'\&', r'\~', '>>', '<<',
    ]

    # Python braces
    braces = [
        r'\{', r'\}', r'\(', r'\)', r'\[', r'\]',
    ]
    def __init__(self, document):
        QSyntaxHighlighter.__init__(self, document)

        # Multi-line strings (expression, flag, style)
        # FIXME: The triple-quotes in these two lines will mess up the
        # syntax highlighting from this point onward
        #self.tri_single = (QRegExp("'''"), 1, STYLES['string2'])
        #self.tri_double = (QRegExp('"""'), 2, STYLES['string2'])

        rules = []


        #rules += [(r'<[?\\s]*[/]?[\\s]*([^\\n][^>]*)(?=[\\s/>])', 0, STYLES['keyword'])]
        rules += [(r'\w+(?=\=)', 0, STYLES['attributes'])]
        rules += [(r'"[^\n"]+"',0,STYLES['attributes_value'])]
        rules += [(r'\sid="[0-9]*"',0,STYLES['id='])]
#(?=[?\s/>])
        # Keyword, operator, and brace rules


        rules += [(r'<\w+ *>' , 0, STYLES['string']) ]
        rules += [(r'<\w+' , 0, STYLES['string']) ]
        rules += [(r'<\/\w+>' , 0, STYLES['string']) ]


        rules += [(r'\b%s\b' % w, 0, STYLES['keyword'])
            for w in PythonHighlighter.keywords]


        rules += [(r'<!--[^\n]*-->', 0, STYLES['comment'])]

        rules += [(r'\{\w*\}', 0, STYLES['string2'])]
#        rules += [(r'%s' % o, 0, STYLES['operator'])
#            for o in PythonHighlighter.operators]
#        rules += [(r'%s' % b, 0, STYLES['brace'])
#            for b in PythonHighlighter.braces]
#
#        # All other rules
        #rules += [
        #    # Numeric literals
        #    (r'\b[+-]?[0-9]+[lL]?\b', 0, STYLES['string']),
        #    (r'\b[+-]?0[xX][0-9A-Fa-f]+[lL]?\b', 0, STYLES['string']),
        #    (r'\b[+-]?[0-9]+(?:\.[0-9]+)?(?:[eE][+-]?[0-9]+)?\b', 0, STYLES['string']),
        #]


        # Build a QRegularExpression for each pattern. QRegExp was removed in
        # Qt 6, and its indexIn/pos/cap API is not available on PySide6.
        self.rules = [(QRegularExpression(pat), index, fmt)
            for (pat, index, fmt) in rules]


    def highlightBlock(self, text):
        """Apply syntax highlighting to the given block of text.
        """
        # Do other syntax formatting
        for expression, nth, format in self.rules:
            match = expression.match(text, 0)

            while match.hasMatch():
                # We actually want the index of the nth match
                index = match.capturedStart(nth)
                length = match.capturedLength(nth)
                self.setFormat(index, length, format)
                match = expression.match(text, index + length)

        self.setCurrentBlockState(0)

        # Do multi-line strings
        self.match_multiline(
            text,
            QRegularExpression("<!--"),
            QRegularExpression("-->"),
            1,
            STYLES['comment'],
        )

    def match_multiline(self, text, delimiterStart,delimiterEnd,in_state,style):
#        """Do highlighting of multi-line strings. ``delimiter`` should be a
#        ``QRegExp`` for triple-single-quotes or triple-double-quotes, and
#        ``in_state`` should be a unique integer to represent the corresponding
#        state changes when inside those strings. Returns True if we're still
#        inside a multi-line string when this function is finished.
#        """
#        # If inside triple-single quotes, start at 0
        if self.previousBlockState() == in_state:
            start = 0
            add = 0
        # Otherwise, look for the delimiter on this line
        else:
            start_match = delimiterStart.match(text)
            start = start_match.capturedStart() if start_match.hasMatch() else -1
            #Move past this match
            add = start_match.capturedLength() if start >= 0 else 0

         # As long as there's a delimiter match on this line...
        while start >= 0:
            # Look for the ending delimiter
            end_match = delimiterEnd.match(text, start + add)
            end = end_match.capturedStart() if end_match.hasMatch() else -1
            # Ending delimiter on this line?
            if end >= add:
                length = end - start + add + end_match.capturedLength()
                self.setCurrentBlockState(0)
            # No; multi-line string
            else:
                self.setCurrentBlockState(in_state)
                length = len(text) - start + add
            # Apply formatting
            self.setFormat(start, length, style)
            # Look for the next match
            start_match = delimiterStart.match(text, start + length)
            start = start_match.capturedStart() if start_match.hasMatch() else -1

        # Return True if still inside a multi-line string, False otherwise
        if self.currentBlockState() == in_state:
            return True
        else:
            return False
