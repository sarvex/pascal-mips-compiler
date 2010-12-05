#!/usr/bin/python

import os, sys
import subprocess
import optparse
import tempfile

def which(executable, path=None):
    """Try to find 'executable' in the directories listed in 'path' (a
    string listing directories separated by 'os.pathsep'; defaults to
    os.environ['PATH']).  Returns the complete filename or None if not
    found
    """
    if path is None:
        path = os.environ['PATH']
    paths = path.split(os.pathsep)
    extlist = ['']
    if os.name == 'os2':
        (base, ext) = os.path.splitext(executable)
        # executable files on OS/2 can have an arbitrary extension, but
        # .exe is automatically appended if no dot is present in the name
        if not ext:
            executable = executable + ".exe"
    elif sys.platform == 'win32':
        pathext = os.environ['PATHEXT'].lower().split(os.pathsep)
        (base, ext) = os.path.splitext(executable)
        if ext.lower() not in pathext:
            extlist = pathext
    for ext in extlist:
        execname = executable + ext
        if os.path.isfile(execname):
            return execname
        else:
            for p in paths:
                f = os.path.join(p, execname)
                if os.path.isfile(f):
                    return f
    else:
        return None


def superwalk(folder):
    for dirpath, _dirnames, filenames in os.walk(folder):
        for filename in filenames:
            yield os.path.join(dirpath, filename)

def absolute(relative_path):
    return os.path.abspath(os.path.join(os.path.dirname(__file__), relative_path))

def execute_spim_code(asm_code):
    "execute asm_code and return stdout"
    exe = which('spim')

    # save asm_code to a file
    handle = tempfile.NamedTemporaryFile(suffix=".asm", delete=False)
    handle.write(asm_code)
    handle.close()
    spim = subprocess.Popen([exe, '-file', handle.name], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = spim.communicate()
    try:
        os.remove(handle.name)
    except:
        pass

    # chop off the 5 useless lines
    clean_out = "\n".join(stdout.split('\n')[5:])

    return clean_out

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
        if f.endswith('.p.errors'):
            test_name = f[:-len('.p.errors')]
            ext = '.p.errors'
        elif f.endswith('.p'):
            test_name = f[:-len('.p')]
            ext = '.p'
        elif f.endswith('.p.out'):
            test_name = f[:-len('.p.out')]
            ext = '.p.out'
        else:
            continue

        if not tests.has_key(test_name):
            tests[test_name] = {}

        if ext == '.p.errors':
            compiler_output = open(absolute(f), 'r').read()
            tests[test_name]['errors'] = compiler_output.replace('Errors detected. Exiting.\n', '')
        elif ext == '.p.out':
            expected_output = open(absolute(f), 'r').read()
            tests[test_name]['out'] = expected_output
        else: # ext == '.p'
            tests[test_name]['source'] = open(absolute(f), 'r').read()

    fails = []
    compiler_exe = absolute('opc')
    interpret_command = execute_spim_code
    passed = 0
    test_list = sorted(tests.iteritems())
    if options.backwards:
        test_list.reverse()
    for test_name, test in test_list:
        if not test.has_key('source'):
            print("%s missing source" % test_name)
            continue
        if not test.has_key('errors'):
            print("%s missing .errors file for expected compiler output" % test_name)
            continue
        if not test.has_key('out'):
            print("%s missing .out file for what the program should output" % test_name)
            continue

        # try compiling the test file
        compiler = subprocess.Popen([compiler_exe], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stdout, stderr = compiler.communicate(test['source'])
        if compiler.returncode not in [0, 1]:
            sys.stdout.write('E')
            fails.append({
                'errors': test['errors'],
                'output': stderr,
                'stdout': stdout,
                'name': test_name,
                'crash': True,
            })
            if options.failfast:
                break
        elif stderr != test['errors']:
            sys.stdout.write('F')
            fails.append({
                'errors': test['errors'],
                'output': stderr,
                'stdout': stdout,
                'name': test_name,
                'crash': False,
            })
            if options.failfast:
                break
        elif compiler.returncode != 1:
            # compiler output correct, now test the generated code output
            asm_output = interpret_command(stdout)
            if asm_output != test['out']:
                sys.stdout.write('W')
                fails.append({
                    'expected_runout': test['out'],
                    'runout': asm_output,
                    'name': test_name,
                    'crash': False,
                })
                if options.failfast:
                    break
            else:
                sys.stdout.write('.')
                passed += 1

        sys.stdout.flush()

    if len(fails) > 0:
        if not options.quiet:
            print("\n=========================================")
            for fail in fails:
                print("Test name: %(name)s" % fail)
                if 'expected_runout' in fail:
                    print("""\
---- Program Output: ----
%(runout)s\
---- Expected Program Output: ----
%(expected_runout)s\
--------""" % fail)
                else:
                    print("""\
---- Compiler Output: ----
%(output)s\
---- Expected Compiler output: ----
%(errors)s\
--------""" % fail)
                    if fail['crash']:
                        print("The program crashed.")
                    if fail['stdout'].strip() != '':
                        print("---- Compiler stdout ----")
                        print(fail['stdout'])
                        print("----------------")
                print("=========================================")
        print("\n%i tests passed, %i tests failed." % (passed, len(fails)))
    else:
        print("\nAll %i tests passed." % passed)

if __name__ == "__main__":
    main()

