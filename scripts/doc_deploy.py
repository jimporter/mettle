#!/usr/bin/env python

import argparse
import json
import re
import subprocess
from packaging.version import Version

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="Deploy mettle's documentation to the gh-pages branch"
    )
    parser.add_argument('version', help='the current mettle version number')

    args = parser.parse_args()

    v = Version(args.version)
    alias = 'dev' if v.is_devrelease else 'latest'
    title = '{} ({})'.format(v.base_version, alias)
    short_version = '{}.{}'.format(*v.release[:2])

    info = json.loads(subprocess.check_output(
        ['mike', 'list', '-j', alias],
        universal_newlines=True
    ))
    if info['version'] != short_version:
        t = re.sub(r' \({}\)$'.format(re.escape(alias)), '',
                   info['title'])
        subprocess.check_call(['mike', 'retitle', info['version'], t])

    subprocess.check_call(['mike', 'deploy', '-ut', title,
                           short_version, alias])
