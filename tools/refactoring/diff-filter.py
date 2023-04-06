import sys
import rich
import rich.console
import rich.panel
import rich.text
import rich.syntax
import argparse
import re

parser = argparse.ArgumentParser(
        description="Run regression tests",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
)
parser.add_argument('-p', '--pattern', type=str, action="store",
                        help='search regex')
args = parser.parse_args()

patternre = re.compile("{0}".format(args.pattern))

linere = re.compile(r"@@ -(\d+),\d+ \+(\d+),\d+ @@.*")

console = rich.console.Console()
lastAtLine = None

panel = None
group = None
subLine = 0
addLine = 0

for line in sys.stdin.readlines():
    if line.startswith("+++"):
        if panel:
            console.print(panel)

        group = rich.console.Group()
        panel = rich.panel.Panel(group, title=rich.text.Text(line, style="bold red"), title_align="left")

    elif patternre.search(line):
        if lastAtLine:
            #group.renderables.append(lastAtLine.rstrip())
            lastAtLine = None

        if line.startswith('+') and addLine:
            lineno = addLine
            addLine += 1
        elif line.startswith('-') and subLine:
            lineno = subLine
            subLine += 1
        else:
            subLine += 1
            addLine += 1
            lineno = "?"

        group.renderables.append(rich.syntax.Syntax(f"{lineno:<4} {line.rstrip()}", "cpp"))
    elif line.startswith("@@"):
        lastAtLine = line
        if m := linere.match(line):
            subLine = int(m.group(1))
            addLine = int(m.group(2))
        else:
            subLine = 0
            addLine = 0

    else:
        if line.startswith('+'):
            lineno = addLine
            addLine += 1
        elif line.startswith('-'):
            lineno = subLine
            subLine += 1
        else:
            subLine += 1
            addLine += 1


if panel:
    console.print(panel)