#*********************************************************************************
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2013-2015 Inviwo Foundation
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
#*********************************************************************************

from enum import Enum, unique

@unique
class Color(Enum):
	black = 0
	blue = 1
	cyan = 2
	green = 3
	magenta = 4
	red = 5
	white = 6
	yellow = 7
	light_black = 8
	light_blue = 9
	light_cyan = 10
	light_green = 11
	light_magenta = 12
	light_red = 13
	light_white = 14
	light_yellow = 15

try:
	import colorama
	colorama.init()	

	def cprint(color, mess, **kwargs):
		colors = {
			Color.black : colorama.Fore.BLACK,
			Color.blue : colorama.Fore.BLUE,
			Color.cyan : colorama.Fore.CYAN,
			Color.green : colorama.Fore.GREEN,
			Color.magenta : colorama.Fore.MAGENTA,
			Color.red : colorama.Fore.RED,
			Color.white : colorama.Fore.WHITE,
			Color.yellow : colorama.Fore.YELLOW,
			Color.light_black : colorama.Fore.LIGHTBLACK_EX,
			Color.light_blue : colorama.Fore.LIGHTBLUE_EX,
			Color.light_cyan : colorama.Fore.LIGHTCYAN_EX,
			Color.light_green : colorama.Fore.LIGHTGREEN_EX,
			Color.light_magenta : colorama.Fore.LIGHTMAGENTA_EX,
			Color.light_red : colorama.Fore.LIGHTRED_EX,
			Color.light_white : colorama.Fore.LIGHTWHITE_EX,
			Color.light_yellow : colorama.Fore.LIGHTYELLOW_EX
		}
		print(colors[color] + colorama.Style.BRIGHT + str(mess) + colorama.Style.RESET_ALL, **kwargs)
	
except ImportError:
	def cprint(color, mess, **kwargs):
		print(str(mess), **kwargs)

def print_text(mess, **kwargs):
		print(mess, **kwargs)

def print_error(mess, **kwargs):
		cprint(Color.red, mess, **kwargs)

def print_warn(mess, **kwargs):
		cprint(Color.yellow, mess, **kwargs)

def print_good(mess, **kwargs):
		cprint(Color.green, mess, **kwargs)

def print_info(mess, **kwargs):
		cprint(Color.cyan, mess, **kwargs)

def print_pair(a,b, width=15):
	print_info("{:>{width}} : ".format(a, width=width), end="")
	print("{:<}".format(b))