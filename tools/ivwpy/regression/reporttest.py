class ReportTest:
	def __init__(self, key, testfun, message):
		self.key = key
		self.testfun = testfun
		self.message = message

	def test(self, report):
		return self.testfun(report[self.key])

	def failures(self):
		return {self.key : [self.message]}

class ReportImageTest(ReportTest):
	def __init__(self, key):
		self.key = key
		self.message = []

	def test(self, report):
		imgs = report[self.key]
		for img in imgs:
			if img["difference"] != 0.0:
				self.message.append(
					"Image {image} has non-zero ({difference}%) difference".format(**img))

		return len(self.message) == 0

	def failures(self):
		return {self.key : self.message}

class ReportLogTest(ReportTest):
	def __init__(self):
		self.key = 'log'
		self.message = []

	def test(self, report):
		with open(toPath(report['outputdir'], report['log']), 'r') as f:
			lines = f.readlines()
			for line in lines:
				if "Error:" in line:
					self.message.append(line)
			
		return len(self.message) == 0

	def failures(self):
		return {"log" : self.message}


class ReportTestSuite:
	def __init__(self):
		self.tests = [
			ReportTest('returncode', lambda x : x == 0, "Non zero retuncode"),
			ReportTest('timeout', lambda x : x == False, "Inviwo ran out of time"),
			ReportTest('missing_refs', lambda x : len(x) == 0, "Missing refecence image"),
			ReportTest('missing_imgs', lambda x : len(x) == 0, "Missing test image"),
			ReportImageTest('image_tests'),
			ReportLogTest()
		]

	def checkReport(self, report):
		failures = {}
		successes = []
		for t in self.tests:
			if not t.test(report):
				failures.update(t.failures())
			else:
				successes.append(t.key)
		report['failures'] = failures
		report['successes'] = successes
		return report