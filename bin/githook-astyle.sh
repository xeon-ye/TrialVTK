#!/bin/bash
# Installation:
#   cd my_gitproject
#   wget -O pre-commit.sh http://tinyurl.com/mkovs45
#   ln -s ../../pre-commit.sh .git/hooks/pre-commit
#   chmod +x pre-commit.sh

# -A2 braces are attached
# -A4 stroustrup, attached except functions, else on new line
# -A3 Kernighan & Ritchie linux braces, attached except functions, else on same line
# -A14 Google style
OPTIONS="-A14 -s2 -c -xG --lineend=linux"

RETURN=0
ASTYLE=$(which astyle)
if [ $? -ne 0 ]; then
    echo "[!] astyle not installed. Unable to check source file format policy." >&2
    exit 1
fi

# Support old version of astyle 2.04
less -f `which astyle` | grep 2.04
BUG=$?

FILES=`git diff --cached --name-only --diff-filter=ACMR | grep -E "\.(c|cpp|h|hpp)$"`
for FILE in $FILES; do
    if [ $BUG -ne 0 ]; then
	$ASTYLE $OPTIONS < $FILE | cmp -s $FILE -
    else
	$ASTYLE $OPTIONS < $FILE | head -c -1 | cmp -s $FILE -
    fi
    if [ $? -ne 0 ]; then
	echo "$?" >&2
	echo "[!] $FILE does not respect the agreed coding style." >&2
	RETURN=1
    fi
done

if [ $RETURN -eq 1 ]; then
    echo "" >&2
    echo "Make sure you have run astyle with the following options: $OPTIONS" >&2
    echo "astyle -n $OPTIONS" >&2
fi

exit $RETURN

