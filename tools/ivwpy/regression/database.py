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

# Table declarations

SqlBase = declarative_base()

class Group(SqlBase):
	__tablename__ = 'group'
	id = Column(Integer, primary_key=True)
	created = Column(DateTime, nullable=False, default=datetime.datetime.now)
	name = Column(String, nullable=False, unique=True)

class Quantity(SqlBase):
	__tablename__ = "quantity"
	id = Column(Integer, primary_key=True)
	created = Column(DateTime(), nullable=False, default=datetime.datetime.now)
	name = Column(String(), nullable=False, unique=True)
	unit = Column(String(), nullable=False)

class Test(SqlBase):
	__tablename__ = 'test'
	id = Column(Integer, primary_key=True)
	created = Column(DateTime, nullable=False, default=datetime.datetime.now)
	group_id = Column(Integer, ForeignKey('group.id'))
	group = relationship(Group, backref=backref('tests', uselist=True))
	name = Column(String, nullable=False)

class Series(SqlBase):
	__tablename__ = "series"
	id = Column(Integer, primary_key=True)
	created = Column(DateTime, nullable=False, default=datetime.datetime.now)

	test_id = Column(Integer, ForeignKey('test.id'))
	test = relationship(Test, backref=backref('serieses', uselist=True))

	quantity_id = Column(Integer, ForeignKey('quantity.id'))
	quantity = relationship(Quantity, backref=backref('serieses', uselist=True))

	name = Column(String(), nullable=False)

class Measurement(SqlBase):
	__tablename__ = "measurement"
	id = Column(Integer, primary_key=True)
	created = Column(DateTime, nullable=False, default=datetime.datetime.now)
	series_id = Column(Integer, ForeignKey('series.id'))
	series = relationship(Series, backref=backref('measurements', uselist=True))
	value = Column(Float, nullable=False)
	

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

	def getOrAddGroup(self, name):
		group = self.session.query(Group).filter(Group.name == name).one_or_none();
		if group == None:
			group = Group(name = name)
			self.session.add(group)
			self.session.commit()
		return group

	def getOrAddTest(self, group, name):
		if isinstance(group, str): group = self.getOrAddGroup(group)
		test = self.session.query(Test).filter(Test.name == name, Test.group == group).one_or_none();
		if test == None:
			test = Test(name = name, group = group)
			self.session.add(test)
			self.session.commit()
		return test

	def getOrAddQuantity(self, name, unit):
		quantity = self.session.query(Quantity).filter(Quantity.name == name).one_or_none();
		if quantity == None:
			quantity = Quantity(name = name, unit = unit)
			self.session.add(quantity)
			self.session.commit()
		return quantity

	def getOrAddSeries(self, test, quantity, name):
		series = self.session.query(Series).filter(Series.name == name, Series.test == test, Series.quantity == quantity).one_or_none();
		if series == None:
			series = Series(name = name, test = test, quantity = quantity)
			self.session.add(series)
			self.session.commit()
		return series

	def addMeasurement(self, series, value):
		m = Measurement(series = series, value = value)
		self.session.add(m)
		self.session.commit()

	def getGroups(self):
		return self.session.query(Group).all()

	def getSeries(self, groupname, testname, seriesname):
		return self.session.query(Series).join(Test).join(Group).filter(Test.name == testname, Group.name == groupname, Series.name == seriesname).one()


	def getSerieses(self, groupname, testname):
		return self.session.query(Series).join(Test).join(Group).filter(Test.name == testname, Group.name == groupname).all()


if __name__ == '__main__':
	print("Start")
	#db = DataBase("/Users/petst/Work/Projects/Inviwo-Developent/Private/regress/regress-test.sqlite")
	db = Database("")


	grp1 = Group(name="base")

	test1 = Test(name = "test1", group = grp1)
	test2 = Test(name = "test2", group = grp1)
	
	qtime = Quantity(name = "time", unit = "s")
	qfreq = Quantity(name = "frequency", unit = "Hz")

	s1 = Series(name = "runtime", test = test1, quantity = qtime)
	s2 = Series(name = "framerate", test = test1, quantity = qfreq)

	s3 = Series(name = "runtime", test = test2, quantity = qtime)
	s4 = Series(name = "framerate", test = test2, quantity = qfreq)

	m1 = [Measurement(series = s1, value = x) for x in range(0,10)]
	m2 = [Measurement(series = s2, value = 2*x) for x in range(0,10)]

	m3 = [Measurement(series = s3, value = x/10) for x in range(0,10)]
	m4 = [Measurement(series = s4, value = 100*x) for x in range(0,10)]

	s = db.session
	s.add(test1)
	s.add(test2)
	s.add(qtime)
	s.add(qfreq)

	s.add(s1)
	s.add(s2)
	s.add(s3)
	s.add(s4)

	s.add_all(m1)
	s.add_all(m2)
	s.add_all(m3)
	s.add_all(m4)
	s.commit()

	for t in s.query(Test):
		print(t.name)

	for s in db.getSerieses("base", "test1"):
		print(s.name)
		for m in s.measurements:
   			print(m.value, s.quantity.unit)

	print("End")