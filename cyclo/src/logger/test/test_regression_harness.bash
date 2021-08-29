#!/usr/bin/env bash

#
# This harness tests the tracing library for C, C++ and Python applications
# It only tests outputs to stdout, stderr and file.
# It checks for leaks too
#
# Please run after any change to the tracing library.
# Option : -g Regenerate the reference file
#

readonly refFile="test_reference_results.txt"
readonly destFile="test_results_raw.txt"
readonly test_results="test_results.txt"

readonly otherRefFile="other_reference_results.txt"
readonly otherDestFile="other_test_results.txt"

readonly test_config=test_config.py

# Activate extended gloh
shopt -s extglob

function print () {
   echo "!! $*" >> $destFile
}

if [[ "$1" == "--help" ]]; then
   echo "Usage : $0 [-g] [path]"
   echo "Where -g regenerates the reference file"
   echo "      path  Points to BUILDDIR to test a particular build"
   exit
fi

if [[ "$1" == "-g" ]]; then
   regen="yes"
   shift
fi

# Reset destFile
date
echo "!! Start the regression test" > $destFile
echo "Starting regression"

#
# Check argument passing
#
if [[ -f runpy ]]; then
   # Checking output control
   print "Python: Checking settings with no args on stdout"
   export LOG="stdout"
   ./runpy $test_config >> $destFile

   print "Python: Checking settings with no args on stderr"
   export LOG="stderr"
   ./runpy $test_config 1>/dev/null >> $destFile

   print "Python: Checking settings with no args in a file"
   export LOG="file=log_test.tmp"
   ./runpy $test_config >> $destFile
   cat log_test.tmp >> $destFile
   rm log_test.tmp

   # Checking level control
   print "Python: Checking all level settings"
   for level in error warn mile info trace debug; do
      print "Checking level $level"
      export LOG="stdout $level"
      ./runpy $test_config >> $destFile
   done

   # Checking full mode
   print "Python: Checking non-indented output"
   export LOG="stdout full"
   ./runpy $test_config >> $destFile

   # Checking domains and levels
   print "Python: Checking domain settings"
   export LOG="error dom[k:ka:ouette]=debug dom[coco]=warn stdout domain=da:dou:di exclude=ni:na:no"
   ./runpy $test_config >> $destFile

   # Testing internal changes
   export LOG="stdout"
   ./runpy test_python.py >> $destFile
fi

#
# Testing C/C++ - take the release 1st
#
if [[ -e $1 ]]; then
   harness=$(find $1 -name testLog | head -1)
else
   harness=$(find *_*{release,debug} -name testLog 2> /dev/null | head -1)
fi

if [[ -z "$harness" ]]; then
   echo "You must build to C/C++ test harness. Use make test"
   exit
else
   echo "Using $harness as C/C++ harness"
fi

# Append to the path for tests in release mode
buildFullTarget=$(dirname ${harness})
buildTarget=${buildFullTarget/%_@(release|debug)/}

# Append to the path for tests in release mode
[[ ${buildFullTarget} == *_release ]] && export LD_LIBRARY_PATH="../export/lib/${buildTarget}/"

# Run!
for level in error warn mile info trace debug; do
   print "C/C++: Checking level $level"
   export LOG="stdout $level"
   ./$harness $otherDestFile >> $destFile
done

#
# ignore all time/date fields and blank lines
# convert thread id 0x..... to 0xXXXX
# Example - 2007/11/27 12:30:29  into - yyyy/mm/dd hh:mm:ss
#
cat $destFile | \
   sed '/^- 20[0-9][0-9]\/[0-9][0-9]\/[0-9][0-9] [0-9][0-9]:[0-9][0-9]:[0-9][0-9]/d' | \
   sed '/^$/d' | \
   sed -r 's/(0x[0-9A-Fa-f]+)/(0xXXXX)/' | \
   sed 's/0x..../0xXXXX/' > $test_results

if [[ "$regen" == "yes" || ! -e $refFile || ! -e $otherRefFile ]]; then
   echo "New reference file created. Don't forget to commit"
   mv $test_results $refFile
   mv $otherDestFile $otherRefFile
else

#
# Diff against reference files
#
diff $test_results $refFile
res1=$?
diff $otherRefFile $otherDestFile
res2=$?

if [ $res1 == 0 -a $res2 == 0 ]; then
      echo "PASS!!"
   else
      echo "FAILED!!"
   fi
fi

#
#  Test for memory leaks
#
if which valgrind &> /dev/null; then
   echo "Valgrind detected : Testing for memory leaks"
   unset LOG
   if valgrind --leak-check=yes --show-reachable=yes ./$harness 2>&1 | grep "LEAK SUMMARY"; then
      echo "Leak found. FAILED!!"
   else
      echo "PASS!!"
   fi
fi
