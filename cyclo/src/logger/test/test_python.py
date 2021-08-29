#!/usr/bin/python

import sys
import time
import LOG


#-----------------------------------------------------------------------------
# Output test cases
#-----------------------------------------------------------------------------
class A:
    def __init__(self):
        LOG.DEBUG("A", "This is a test", "Test")
        LOG.WARN("B", "This is a warn test", 3, "moretext")
        LOG.MILE("C", "A mile test")


def testLevelOutput():
   LOG.ERROR("A", "ERROR level")
   LOG.WARN("A", "WARN level")
   LOG.MILE("A", "MILE level")
   LOG.TRACE("A", "TRACE level")
   LOG.DEBUG("A", "LOG level")


def testDomainOutput():
   LOG.ERROR("A", "A domain")
   LOG.ERROR("B", "B domain")
   LOG.ERROR("C", "C domain")
   LOG.ERROR("D", "D domain")
   LOG.ERROR("E", "E domain")


def testCombinedOutput():
   LOG.ERROR("A", "A domain ERROR")
   LOG.ERROR("B", "B domain ERROR")
   LOG.ERROR("C", "C domain ERROR")
   LOG.ERROR("D", "D domain ERROR")
   LOG.ERROR("E", "E domain ERROR")

   LOG.TRACE("A", "A domain TRACE")
   LOG.TRACE("B", "B domain TRACE")
   LOG.TRACE("C", "C domain TRACE")
   LOG.TRACE("D", "D domain TRACE")
   LOG.TRACE("E", "E domain TRACE")
   LOG.TRACE("F", "E domain TRACE")

   LOG.DEBUG("LONGNAME", "LONGNAME domain DEBUG")

#-----------------------------------------------------------------------------


class ItemFailed(Exception):
   pass

# Basic function test
print("--> Checking mapping")
print("  - Error level is ", LOG.LEVEL_ERROR)
if LOG.LEVEL_ERROR != 0:
   raise ItemFailed()
print("  - Warn level is ",  LOG.LEVEL_WARN)
if LOG.LEVEL_WARN != 1:
   raise ItemFailed()
print("  - Mile level is ",  LOG.LEVEL_MILE)
if LOG.LEVEL_MILE != 2:
   raise ItemFailed()
print("  - Info level is ",  LOG.LEVEL_INFO)
if LOG.LEVEL_INFO != 3:
   raise ItemFailed()
print("  - Trace level is ", LOG.LEVEL_TRACE)
if LOG.LEVEL_TRACE != 4:
   raise ItemFailed()
print("  - Debug level is ",   LOG.LEVEL_DEBUG)
if LOG.LEVEL_DEBUG != 5:
   raise ItemFailed()
print("<-- Pass")

print("--> Checking level settings")
level = LOG.GETLEVEL()
print("  -Level is : %d" % level)

print("@START=================================================================")
testLevelOutput()
print("@END===================================================================")

print("  +Setting level to ERROR")
LOG.SETLEVEL(LOG.LEVEL_ERROR)
if LOG.GETLEVEL() != LOG.LEVEL_ERROR:
   raise ItemFailed()
print("@START=================================================================")
testLevelOutput()
print("@END===================================================================")

print("  +Setting level to WARN")
LOG.SETLEVEL(LOG.LEVEL_WARN)
if LOG.GETLEVEL() != LOG.LEVEL_WARN:
   raise ItemFailed()
print("@START=================================================================")
testLevelOutput()
print("@END===================================================================")

print("  +Setting level to MILE")
LOG.SETLEVEL(LOG.LEVEL_MILE)
if LOG.GETLEVEL() != LOG.LEVEL_MILE:
   raise ItemFailed()
print("@START=================================================================")
testLevelOutput()
print("@END===================================================================")

print("  +Setting level to INFO")
LOG.SETLEVEL(LOG.LEVEL_INFO)
if LOG.GETLEVEL() != LOG.LEVEL_INFO:
   raise ItemFailed()
print("@START=================================================================")
testLevelOutput()
print("@END===================================================================")

print("  +Setting level to TRACE")
LOG.SETLEVEL(LOG.LEVEL_TRACE)
if LOG.GETLEVEL() != LOG.LEVEL_TRACE:
   raise ItemFailed()
print("@START=================================================================")
testLevelOutput()
print("@END===================================================================")

print("  +Setting level to DEBUG")
LOG.SETLEVEL(LOG.LEVEL_DEBUG)
if LOG.GETLEVEL() != LOG.LEVEL_DEBUG:
   raise ItemFailed()
print("@START=================================================================")
testLevelOutput()
print("@END===================================================================")

print("  +Restoring level")
LOG.SETLEVEL(level)
if LOG.GETLEVEL() != level:
   raise ItemFailed()
print("@START=================================================================")
testLevelOutput()
print("@END===================================================================")
print("<-- Pass")

print("--> Checking mask (domain filtering) setting")
maskPair = LOG.GETMASKS()
print("  - Current mask setting is", maskPair)
print("@START=================================================================")
testDomainOutput()
print("@END===================================================================")

print("  + Setting mask to A B C")
LOG.MASK("A:B:C")
if LOG.GETMASKS()[0] != "A:B:C":
   raise ItemFailed()
print("@START=================================================================")
testDomainOutput()
print("@END===================================================================")

print("  + Setting not mask to D E F")
LOG.NOTMASK("D:E:F")
if LOG.GETMASKS()[1] != "D:E:F":
   raise ItemFailed()
print("@START=================================================================")
testDomainOutput()
print("@END===================================================================")

print("  + Restoring previous mask settings")
LOG.MASK(maskPair[0])
LOG.NOTMASK(maskPair[1])
if maskPair != LOG.GETMASKS():
   raise ItemFailed()
print("@START=================================================================")
testDomainOutput()
print("@END===================================================================")
print("<-- Pass")

print("--> Checking domain level filtering setting")
domainLevels = LOG.GETDOMAINLEVELS()
print("  - Current domain level filtering setting is", domainLevels)
print("@START=================================================================")
testCombinedOutput()
print("@END===================================================================")

print("  - Resetting levels")
LOG.SETDOMAINLEVEL(None)
if len(LOG.GETDOMAINLEVELS()) > 0:
   raise ItemFailed()
print("@START=================================================================")
testCombinedOutput()
print("@END===================================================================")

print("  - Adding 7 level filtering + check")
testDoms = [
   ("A", LOG.LEVEL_TRACE),
   ("B", LOG.LEVEL_ERROR),
   ("C", LOG.LEVEL_ERROR),
   ("D", LOG.LEVEL_WARN),
   ("E", LOG.LEVEL_MILE),
   ("F", LOG.LEVEL_TRACE),
   ("LONGNAME", LOG.LEVEL_DEBUG),  # A maximal length name
]
for (domain, level) in testDoms:
   LOG.SETDOMAINLEVEL(domain, level)

newDomainLevels = LOG.GETDOMAINLEVELS()

if newDomainLevels != testDoms:
   raise ItemFailed()

print("  - Current domain level filtering setting is", newDomainLevels)
print("@START=================================================================")
testLevelOutput()
testCombinedOutput()
print("@END===================================================================")
print("<-- Pass")

print("--> Checking basic messages")
testLevelOutput()
testDomainOutput()
testCombinedOutput()
a = A()
print("<-- Manual check required")
