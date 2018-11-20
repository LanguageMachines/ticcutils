#! /bin/sh
valgrind ./.libs/runtest -ffalse +t true --test=test -d $topsrcdir/tests/  -q --raar blaat arg1 arg2

#./testlogstream
