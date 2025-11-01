import sys
import subprocess
import routines

if len(sys.argv) < 2:
    print("Test name not given. Exit.")
    exit(1)

subprocess.run(["make", "clean-test"])
subprocess.run(["make", "test"])

test_name = sys.argv[1]

if test_name not in routines.available:
    print("Test name not recognised. Exit.")
    exit(1)

routines.available[test_name]()
