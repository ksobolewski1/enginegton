import sys
import subprocess
import routines

if len(sys.argv) < 2:
    print("Test name not given. Exit.")
    exit(1)

subprocess.run(["make", "clean-test"])
subprocess.run(["make", "test"])
# make clean-test
# make test

# check if the test exists in available

routines.available[sys.argv[1]]()
