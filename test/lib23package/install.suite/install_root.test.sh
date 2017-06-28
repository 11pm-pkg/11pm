#!/bin/sh

set -e

DEST='dest/example.com/foo'

mkdir -pv \
  bin lib doc \
  "$DEST$PWD/bin" \
  "$DEST$PWD/lib" \
  "$DEST$PWD/doc"

out="$DEST$PWD/bin/foo"
echo "$out <<EOF"
echo "#!/bin/sh
source $PWD/lib/libfoo.sh
echo $greeting $name" | tee "$out"
echo EOF

out="$DEST$PWD/lib/libfoo.sh"
echo "$out <<EOF"
echo 'greeting=Hello
name=World' | tee "$out"
echo EOF

out="$DEST$PWD/doc/foo_usage.txt"
echo "$out <<EOF"
echo 'usage: foo
Prints Hello World' | tee "$out"
echo EOF

t -?0
## include "23package.h"
#= xipm_symlink(args[0], "/");
t eval @@ "$DEST" : assert -r0 -e0

t assert [ -L bin/foo ]
t assert [ "$(readlink bin/foo)" = "../$DEST$PWD/bin/foo" ] \
  -ax '$(readlink bin/foo)'
t assert [ -L lib/libfoo.sh ]
t assert [ "$(readlink lib/libfoo.sh)" = "../$DEST$PWD/lib/libfoo.sh" ] \
  -ax '$(readlink lib/libfoo.sh)'
t assert [ -L doc/foo_usage.txt ]
t assert [ "$(readlink doc/foo_usage.txt)" = "../$DEST$PWD/doc/foo_usage.txt" ] \
  -ax '$(readlink doc/foo_usage.txt)'

t assert [ -x bin/foo ] -?- \
  && t assert -?0 -- bin/foo | t assert [- = 'Hello World' ]
