import subprocess, os, argparse
from lxml import etree

# Python has many, many bencode decoder libraries, and they are of generally
# low quality as of 2018.  The module 'better_bencode' is the only one of
# several tried that actually decoded Mettle's wire protocol without crashing.

import better_bencode

class JUnitEncoder:
    """Receive a sequence of decoded bencode events and build an lxml.etree
    model of JUnit XML from them.  The specific event names and test data are
    defined by the Mettle wire protocol.  The JUnit XML attributes and text
    content are widely used by continuous integration systems, but
    documentation for the format is scarse.  The summary of the XML format at
    llg.cubic.org/docs/junit/ was useful for writing this class"""

    def __call__(self, event):
        """Dispatch a specific event handler given the event name provided by
        one of the elements of 'event'."""

        callback = getattr(self, event[b'event'].decode('utf-8'))
        callback(event)

    def started_run(self, event):
        self.suites = etree.Element('testsuites')
        for attr in ['tests', 'failures', 'errors', 'time']:
            self.suites.attrib[attr] = '0'

    def started_suite(self, event):
        self.suite = etree.SubElement(self.suites, "testsuite")
        self.suite.attrib["name"] = b'.'.join(event[b'suites'])
        for attr in ['tests', 'failures', 'errors', 'disabled', 'skipped', 'time']:
            self.suite.attrib[attr] = '0'

    def started_test(self, event):
        self.test = etree.SubElement(self.suite, "testcase")
        self._increment_attribute(self.suite, 'tests')
        self._increment_attribute(self.suites, 'tests')
        self.test.attrib["name"] = event[b'test'][b'test']

    def passed_test(self, event):
        self.test.attrib["id"] = str(event[b'test'][b'id'])
        self.test.attrib["time"] = str(event[b'duration'])
        self._accumulate_time(self.suite, event)
        self._accumulate_time(self.suites, event)
        self._copy_pipes(self.test, event)

    def failed_test(self, event):
        node = etree.SubElement(self.test, "failure")
        node.attrib['message'] = event[b'message']
        self.test.attrib["time"] = str(event[b'duration'])
        self._accumulate_time(self.suite, event)
        self._accumulate_time(self.suites, event)
        self.test.attrib["id"] = str(event[b'test'][b'id'])
        self._increment_attribute(self.suite, 'failures')
        self._increment_attribute(self.suites, 'failures')
        self._copy_pipes(self.test, event)

    def skipped_test(self, event):
        node = etree.SubElement(self.test, "skipped")
        node.attrib['message'] = event[b'message']
        self._increment_attribute(self.suite, 'disabled')
        self._increment_attribute(self.suites, 'disabled')

    def ended_suite(self, event):
        pass

    def ended_run(self, event):
        pass

    def _increment_attribute(self, tree, key):
        tree.attrib[key] = str(int(tree.attrib[key]) + 1)

    def _accumulate_time(self, tree, event):
        tree.attrib['time'] = str(float(tree.attrib['time']) + float(event[b'duration']))

    def _copy_pipes(self, node, event):
        output = event[b'output']
        if output[b'stdout_log']:
            stdout = etree.SubElement(node, 'system-out')
            stdout.text = output[b'stdout_log'].decode('ascii').strip()
        if output[b'stderr_log']:
            stderr = etree.SubElement(node, 'system-err')
            stderr.text = output[b'stderr_log'].decode('ascii').strip()

def run_test_file(test_file, xml_output):
    encode = JUnitEncoder()
    read_mettle_fd, write_mettle_fd = os.pipe()
    mettle_args = [test_file, '--output-fd={}'.format(write_mettle_fd)]
    read_mettle_wireprotocol = os.fdopen(read_mettle_fd, 'rb')

    with subprocess.Popen(mettle_args, pass_fds=(write_mettle_fd,)) as proc:
        event = { b'event': None }
        while event[b'event'] != b'ended_run':
            event = better_bencode.load(read_mettle_wireprotocol)
            encode(event)
        proc.wait()
               
    os.close(read_mettle_fd)
    os.close(write_mettle_fd)
    
    xml = etree.tostring(encode.suites, pretty_print=True, xml_declaration=True, encoding='UTF-8')
    open(xml_output, 'w').write(xml.decode('utf-8'))
    print('Wrote {}'.format(xml_output))
    
if __name__ == '__main__':
    parser = argparse.ArgumentParser(
            description='Mettle test runner for writing JUnit XML output')
    parser.add_argument('file', type=str, nargs='+', help='Mettle unittest executable')
    parser.add_argument('--output', type=str, help='output XML directory', 
            default='reports')
    args = parser.parse_args()
    
    for test_file in args.file:
        xml_output = '{}/{}.xml'.format(args.output, os.path.basename(test_file))
        run_test_file(test_file, xml_output)


