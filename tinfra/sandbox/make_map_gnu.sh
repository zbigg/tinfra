#
# Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
# licensed to the public under the terms of the GNU GPL (>= 2)
# see the file COPYING for details
# I.e., do what you like, but keep copyright and there's NO WARRANTY.
#

#!/bin/sh

victim=$1;

nm -f b -n -l -C ${victim} | grep -i " t " | grep -v ".text" > ${victim}.map
