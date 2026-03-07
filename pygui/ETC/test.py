from muchospec_etc.main import main
from muchospec_etc.arguments import parser, check_inputs_add_units

# Minimal valid argument set
cmd = """
G 500 510 SNR 10
-slit SET 0.5
-seeing 1 500
-airmass 1
-skymag 21.4
-mag 18
-magsystem AB
-magfilter match
"""

args = parser.parse_args(cmd.split())
check_inputs_add_units(args)

result = main(args, quiet=True)

print(result)
