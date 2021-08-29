import LOG
import pyTestLog

LOG_PY = "python"

LOG.MILE( LOG_PY, "Starting the Python test...")

class TestDebug:
   def testLevels (self):
      LOG.LOG( LOG_PY, "A log message")
      LOG.TRACE( LOG_PY, "A trace message")
      LOG.WARN( LOG_PY, "A warn message")
      LOG.ERROR( LOG_PY, "An error message")

# Local
object = TestDebug()
object.testLevels()

# Call SWIG
for i in range(0,4):
   pyTestLog.testLog( i );


