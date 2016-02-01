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
import sys
import datetime

from sqlalchemy import Column, ForeignKey, Integer, String, DateTime, Float
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy import create_engine, select
from sqlalchemy.orm import sessionmaker, relationship, backref 
from sqlalchemy import func
from sqlalchemy import desc

# Table declarations

SqlBase = declarative_base()

class Module(SqlBase):
	__tablename__ = 'module'
	id 			= Column(Integer, primary_key=True)
	created 	= Column(DateTime, nullable=False, default=datetime.datetime.now)

	name 		= Column(String, nullable=False, unique=True)

class Quantity(SqlBase):
	__tablename__ = "quantity"
	id 			= Column(Integer, primary_key=True)
	created 	= Column(DateTime(), nullable=False, default=datetime.datetime.now)
	
	name 		= Column(String(), nullable=False, unique=True)
	unit 		= Column(String(), nullable=False)

class Test(SqlBase):
	__tablename__ = 'test'
	id          = Column(Integer, primary_key=True)
	created     = Column(DateTime, nullable=False, default=datetime.datetime.now)

	module_id    = Column(Integer, ForeignKey('module.id'))
	module       = relationship(Module, backref=backref('tests', uselist=True))

	name        = Column(String, nullable=False)

class Series(SqlBase):
	__tablename__ = "series"
	id          = Column(Integer, primary_key=True)
	created     = Column(DateTime, nullable=False, default=datetime.datetime.now)

	test_id     = Column(Integer, ForeignKey('test.id'))
	test        = relationship(Test, backref=backref('serieses', uselist=True))

	quantity_id = Column(Integer, ForeignKey('quantity.id'))
	quantity    = relationship(Quantity, backref=backref('serieses', uselist=True))

	name        = Column(String(), nullable=False)

class Commit(SqlBase):
	__tablename__ = "commit"
	id 		    = Column(Integer, primary_key=True)
	created     = Column(DateTime, nullable=False, default=datetime.datetime.now)

	hash 	    = Column(String, nullable=False, unique=True)
	date 	    = Column(DateTime, nullable=False)
	author 	    = Column(String)
	message     = Column(String)
	server 	    = Column(String)

class TestRun(SqlBase):
	__tablename__ = "testrun"
	id 		    = Column(Integer, primary_key=True)
	created     = Column(DateTime, nullable=False, default=datetime.datetime.now)

	test_id     = Column(Integer, ForeignKey('test.id'))
	test  	    = relationship(Test, backref=backref('testruns', uselist=True))

	commit_id   = Column(Integer, ForeignKey('commit.id'))
	commit      = relationship(Commit, backref=backref('testruns', uselist=True))

class TestFailure(SqlBase):
	__tablename__ = "testfailure"
	id          = Column(Integer, primary_key=True)
	created     = Column(DateTime, nullable=False, default=datetime.datetime.now)

	testrun_id  = Column(Integer, ForeignKey('testrun.id'))
	testrun     = relationship(TestRun, backref=backref('testfailures', uselist=True))

	key         = Column(String, nullable=False)
	message     = Column(String)

class Measurement(SqlBase):
	__tablename__ = "measurement"
	id          = Column(Integer, primary_key=True)
	created     = Column(DateTime, nullable=False, default=datetime.datetime.now)

	series_id   = Column(Integer, ForeignKey('series.id'))
	series      = relationship(Series, backref=backref('measurements', uselist=True))

	testrun_id  = Column(Integer, ForeignKey('testrun.id'))
	testrun     = relationship(TestRun, backref=backref('measurements', uselist=True))

	value       = Column(Float, nullable=False)


class Database():
	def __init__(self, dbfile):
		self.dbfile = dbfile
		if not os.path.exists(self.dbfile):  # open 
			self.engine = create_engine('sqlite:///' + dbfile)
			SqlBase.metadata.create_all(self.engine)
		else:                            # create db 
			self.engine = create_engine('sqlite:///' + dbfile)
			SqlBase.metadata.bind = self.engine

		self.session = sessionmaker(bind=self.engine)()

	def getOrAddModule(self, name):
		module = self.session.query(Module).filter(Module.name == name).one_or_none()
		if module == None:
			module = Module(name = name)
			self.session.add(module)
			self.session.commit()
		return module

	def getOrAddTest(self, module, name):
		if isinstance(module, str): module = self.getOrAddModule(module)
		test = self.session.query(Test).filter(Test.name == name, 
											   Test.module == module).one_or_none()
		if test == None:
			test = Test(name = name, module = module)
			self.session.add(test)
			self.session.commit()
		return test

	def getOrAddQuantity(self, name, unit):
		quantity = self.session.query(Quantity).filter(Quantity.name == name).one_or_none()
		if quantity == None:
			quantity = Quantity(name = name, unit = unit)
			self.session.add(quantity)
			self.session.commit()
		return quantity

	def getOrAddSeries(self, test, quantity, name):
		series = self.session.query(Series).filter(Series.name == name, 
			 									   Series.test == test, 
			 									   Series.quantity == quantity).one_or_none()
		if series == None:
			series = Series(name = name, test = test, quantity = quantity)
			self.session.add(series)
			self.session.commit()
		return series


	def addEntry(self, Type, **kvargs):
		entry = Type(**kvargs)
		self.session.add(entry)
		self.session.commit()
		return entry;

	def getCommit(self, hash):
		commit = self.session.query(Commit).filter(Commit.hash == hash).one_or_none()
		return commit

	def addCommit(self, hash, date, author, message, server):
		return self.addEntry(Commit, 
							 hash = hash, 
							 date = date, 
							 author = author, 
							 message = message, 
							 server = server)

	def getOrAddCommit(self, hash, date, author, message, server):
		commit = self.getCommit(hash)
		if commit is None:
			commit = self.addCommit(hash, date, author, message, server)
		return commit


	def addTestRun(self, test, commit):
		return self.addEntry(TestRun, test = test, commit = commit)

	def addTestFailure(self, testrun, key, message):
		return self.addEntry(TestFailure, testrun = testrun, key = key, message = message)

	def addMeasurement(self, series, testrun, value):
		return self.addEntry(Measurement, series = series, testrun = testrun, value = value)

	def getModules(self):
		return self.session.query(Module).all()

	def getSeries(self, modulename, testname, seriesname):
		return self.session.query(Series).join(Test).join(Module).filter(Test.name == testname, 
																		 Module.name == modulename, 
																		 Series.name == seriesname).one()

	def getSerieses(self, modulename, testname):
		return self.session.query(Series).join(Test).join(Module).filter(Test.name == testname, 
																		 Module.name == modulename).all()


	def getLastTestRun(self, modulename, testname):
		return self.session.query(TestRun).\
							join(Test).\
							join(Module).\
							filter(Test.name == testname, 
		 						   Module.name == modulename).\
							order_by(desc(TestRun.created)).\
							first()


	def getLastSuccessFirstFailure(self, modulename, testname):
		testruns = self.session.query(TestRun).\
								join(Test).\
								join(Module).\
								filter(Test.name == testname, 
		 							   Module.name == modulename).\
								order_by(desc(TestRun.created)).\
								all()

		lastSuccess = None
		firstFailure = None
		for testrun in testruns:
			if len(testrun.testfailures) == 0:
				lastSuccess = testrun
				break
			else:
				firstFailure = testrun

		return lastSuccess, firstFailure


if __name__ == '__main__':
	db = Database(sys.argv[1])









