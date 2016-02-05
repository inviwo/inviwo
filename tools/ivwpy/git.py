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

import os
import re
import configparser
import subprocess
import datetime

from . import util
from . import colorprint as cp
from . import ivwpaths

def findGit(pyconfsearchpath = ""):
	pyconfig = util.find_pyconfig(pyconfsearchpath)
	config = configparser.ConfigParser()
	config.read([
		util.toPath(ivwpaths.find_inv_path(), "pyconfig.ini"),
		pyconfig if pyconfig is not None else ""
		])
	if config.has_option("Git", "path"):
		git = config.get("Git", "path")
	elif os.name == 'posix': 
		git='git'
	else: 
		git='git.exe'

	return git


class Git:
	def __init__(self, pyconfsearchpath = ""):
		self.gitexe = findGit(pyconfsearchpath)

	def run(self, path, command):
		try:
			with subprocess.Popen(
				[self.gitexe] + command, 
		  		cwd=path,
				stdout=subprocess.PIPE, 
		  		stderr=subprocess.STDOUT,
				universal_newlines=True) as proc:
					out, err = proc.communicate(timeout=60)
	
		except FileNotFoundError as e:
			cp.print_error("Could not find " + self.gitexe + " in the path")
			raise e
	
		except subprocess.TimeoutExpired as e:
			proc.kill()
			cp.print_error("Git command timeout: " + " ".join(command))
			raise e
	
		return out, err

	def foundGit(self):
		try:
			self.gitversion()
		except:
			return False
		return True

	def gitversion(self):
		out, err = self.run(".", ["--version"])
		return out

	def hash(self, path):
		out, err = self.run(path, ["log", "-n1", "--pretty=format:%H"])
		return out

	def date(self, path):
		out, err = self.run(path, ["log", "-n1", "--pretty=format:%cI"])
		return datetime.datetime.strptime(out[:-6], "%Y-%m-%dT%H:%M:%S" )

	def author(self, path):
		out, err = self.run(path, ["log", "-n1", "--pretty=format:%cn"])
		return out

	def message(self, path):
		out, err = self.run(path, ["log", "-n1", "--pretty=format:%B"])
		return out

	def server(self, path):
		out, err = self.run(path, ["config", "--local", "remote.origin.url"])
		m = re.match(r"(?P<proto>https?:\/\/)(\w+@)?(?P<url>[_\w.\d/-]+)\.git", out)
		if m:
			return m.group("proto") + m.group("url")
		else:
			return ""

	def info(self, path):
		return {
			'hash'   : self.hash(path),
			'date'   : util.dateToString(self.date(path)),
			'author' : self.author(path),
			'message': self.message(path),
			'server' : self.server(path)
		}