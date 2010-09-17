#!/usr/bin/python

import os, sys
import subprocess
import optparse

def superwalk(folder):
    for dirpath, _dirnames, filenames in os.walk(folder):
        for filename in filenames:
            yield os.path.join(dirpath, filename)

def absolute(relative_path):
    return os.path.abspath(os.path.join(os.path.dirname(__file__), relative_path))

parser = optparse.OptionParser()
parser.add_option('-f', '--failfast', dest='failfast', help="Stop on first failed test", metavar="FAILFAST", action="store_true")
options, args = parser.parse_args()

print("Loading test suite...")

tests = {}
for f in superwalk('tests/formal'):
    if f.endswith('.p.expected'):
        test_name = f[:-len('.p.expected')]
        ext = '.p.expected'
    elif f.endswith('.p'):
        test_name = f[:-len('.p')]
        ext = '.p'
    else:
        continue

    if not tests.has_key(test_name):
        tests[test_name] = {}

    if ext == '.p.expected':
        tests[test_name]['expected'] = open(absolute(f), 'r').read()
    else:
        tests[test_name]['source'] = open(absolute(f), 'r').read()

fails = []
compiler_exe = absolute('opc')
passed = 0
for test_name, test in tests.iteritems():
    if not test.has_key('source'):
        print("%s missing source" % test_name)
        continue
    if not test.has_key('expected'):
        print("%s missing expected value" % test_name)
        continue

    compiler = subprocess.Popen([compiler_exe], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = compiler.communicate(test['source'])
    if stderr == test['expected']:
        sys.stdout.write('.')
        passed += 1
    else:
        sys.stdout.write('F')
        fails.append({
            'expected': test['expected'],
            'output': stderr,
            'name': test_name,
        })
        if options.failfast:
            break
    sys.stdout.flush()

if len(fails) > 0:
    print("\n=========================================")
    for fail in fails:
        print("""Test name: %(name)s
---- Output: ----
%(output)s
---- Expected output: ----
%(expected)s
--------
""" % fail)
        print("=========================================")
    print("\n%i tests passed, %i tests failed." % (passed, len(fails)))
else:
    print("\nAll %i tests passed." % len(tests))
