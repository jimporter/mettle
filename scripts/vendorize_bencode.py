#!/usr/bin/env python

# A simple script for copying bencode.hpp to mettle's include directory. This
# eliminates the need for users to manually download bencode.hpp if they don't
# want to.

import os
import shutil
import tarfile
from io import BytesIO

try:
    from urllib.request import urlopen
except ImportError:
    from urllib import urlopen

srcdir = os.path.normpath(os.path.join(__file__, '..',  '..'))

commit = '2312b988924ce178e6b5283cbb831068925fc2eb'
bencode_name = 'bencode.hpp-{}'.format(commit)
bencode_url = ('https://github.com/jimporter/bencode.hpp/archive/{}.tar.gz'
               .format(commit))

if __name__ == '__main__':
    print('Fetching {}'.format(bencode_url))
    f = urlopen(bencode_url)

    with tarfile.open(mode='r:gz', fileobj=BytesIO(f.read())) as tar:
        src_name = '{}/include/bencode.hpp'.format(bencode_name)
        print('Extracting {}'.format(src_name))
        src = tar.extractfile(src_name)

        dst_name = os.path.join(srcdir, 'include', 'bencode.hpp')
        with open(dst_name, 'wb') as dst:
            print('Copying to {}'.format(dst_name))
            shutil.copyfileobj(src, dst)
