#!/usr/bin/env python

# A simple script for copying bencode.hpp to mettle's include directory. This
# eliminates the need for users to manually download bencode.hpp if they don't
# want to.

import os
import shutil
import tarfile

try:
    from StringIO import StringIO
except ImportError:
    from io import BytesIO as StringIO

try:
    from urllib.request import urlopen
except ImportError:
    from urllib import urlopen

srcdir = os.path.realpath(os.path.join(__file__, '..',  '..'))

bencode_version = '0.1'
bencode_name = 'bencode_hpp-{}'.format(bencode_version)
bencode_url = ('https://github.com/jimporter/bencode.hpp/releases/download/' +
               'v{version}/{name}.tar.gz').format(version=bencode_version,
                                                  name=bencode_name)

if __name__ == '__main__':
    f = urlopen(bencode_url)
    with tarfile.open(mode='r:gz', fileobj=StringIO(f.read())) as tar:
        src = tar.extractfile('{}/include/bencode.hpp'.format(bencode_name))
        with open(os.path.join(srcdir, 'include', 'bencode.hpp'), 'wb') as dst:
            shutil.copyfileobj(src, dst)
