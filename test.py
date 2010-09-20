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


def main():
    parser = optparse.OptionParser()
    parser.add_option('-f', '--failfast', help="Stop on first failed test", action="store_true")
    parser.add_option("-q", "--quiet", help="only print dots and summary", action="store_true")
    parser.add_option("-b", "--backwards", help="run tests in reverse order", action="store_true")
    options, args = parser.parse_args()

    if not options.quiet:
        print("Loading test suite...")

    tests = {}
    for f in superwalk("tests/"):
        if f.find("/_") != -1:
            continue
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
            expected_output = open(absolute(f), 'r').read()
            tests[test_name]['expected'] = expected_output.replace('Errors detected. Exiting.\n', '')
        else:
            tests[test_name]['source'] = open(absolute(f), 'r').read()

    fails = []
    compiler_exe = absolute('opc')
    passed = 0
    test_list = sorted(tests.iteritems())
    if options.backwards:
        test_list.reverse()
    for test_name, test in test_list:
        if not test.has_key('source'):
            print("%s missing source" % test_name)
            continue
        if not test.has_key('expected'):
            print("%s missing expected value" % test_name)
            continue

        compiler = subprocess.Popen([compiler_exe], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stdout, stderr = compiler.communicate(test['source'])
        if compiler.returncode not in [0, 1]:
            sys.stdout.write('E')
            fails.append({
                'expected': test['expected'],
                'output': stderr,
                'stdout': stdout,
                'name': test_name,
                'crash': True,
            })
        if stderr == test['expected']:
            sys.stdout.write('.')
            passed += 1
        else:
            sys.stdout.write('F')
            fails.append({
                'expected': test['expected'],
                'output': stderr,
                'stdout': stdout,
                'name': test_name,
                'crash': False,
            })
            if options.failfast:
                break
        sys.stdout.flush()



    if len(fails) > 0:
        if not options.quiet:
            print("\n=========================================")
            for fail in fails:
                print("""Test name: %(name)s
---- Output: ----
%(output)s
---- Expected output: ----
%(expected)s
--------""" % fail)
                if fail['crash']:
                    print("The program crashed.")
                if fail['stdout'].strip() != '':
                    print("---- stdout ----")
                    print(fail['stdout'])
                    print("----------------")
                print("=========================================")
        print("\n%i tests passed, %i tests failed." % (passed, len(fails)))
    else:
        print("\nAll %i tests passed." % len(tests))

if __name__ == "__main__":
    main()

