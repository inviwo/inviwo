
import logging
import ivwpy.regression.logprinter


logging.basicConfig(format='%(message)s', level=getattr(logging, "warn".upper()))

logging.getLogger().setLevel(logging.DEBUG)

with ivwpy.regression.logprinter.LogPrinter(logging.getLogger()) as l: 
    l.info("message") 
    l.good("bla")
    l.success = True
