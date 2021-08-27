#!/usr/bin/python

import sys
import time
import LOG


def printConfig():
   level = LOG.GETLEVEL()
   print("Level:%d" % level)

   maskPair = LOG.GETMASKS()
   print("Masks:", maskPair)

   domainLevels = LOG.GETDOMAINLEVELS()
   print("Domain levels:", domainLevels)

print("--> Current configuration")
printConfig()

# Reset all settings to guarantee our log to go out
LOG.MASK(None)
LOG.NOTMASK(None)
LOG.SETDOMAINLEVEL(None)
LOG.SETLEVEL(LOG.LEVEL_DEBUG)

print("--> Altered configuration")
printConfig()

LOG.ERROR("TST", "Check full option")
LOG.ERROR("TST", "Now indent is full")
