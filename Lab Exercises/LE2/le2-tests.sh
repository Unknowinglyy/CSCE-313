#!/usr/bin/env bash

# function to clean up files and make executables
remake () {
    # echo -e "\ old files and making executables"
    make -s clean
    make -s shell >/dev/null 2>&1
}

echo -e "To remove colour from tests, set COLOUR to 1 in sh file\n"
COLOUR=0
if [[ COLOUR -eq 0 ]]; then
    ORANGE='\033[0;33m'
    GREEN='\033[0;32m'
    RED='\033[0;31m'
    NC='\033[0m'
else
    ORANGE='\033[0m'
    GREEN='\033[0m'
    RED='\033[0m'
    NC='\033[0m'
fi

SCORE=0

echo -e "\nStart testing"
remake
echo -e "\nTesting :: Compilation\n"
echo -e "  ${GREEN}Test One Passed${NC}"
SCORE=$(($SCORE+34))

remake
echo -e "\nTesting :: Correct Output\n"

./shell "/sbin/init" > output 2>&1
sleep 5
if grep -q "/sbin/init" output; then
    echo -e "  ${GREEN}Test Two Passed${NC}"
    SCORE=$(($SCORE+20))
else
    echo -e "  ${RED}Failed${NC}"
fi

./shell "kthread" > output 2>&1
sleep 5
if grep -q "kthreadd" output; then
    echo -e "  ${GREEN}Test Three Passed${NC}"
    SCORE=$(($SCORE+20))
else
    echo -e "  ${RED}Failed${NC}"
fi

./shell "scexe" > output 2>&1
sleep 5
if $(grep -q "grep scexe" output); then
    echo -e "  ${GREEN}Test Four Passed${NC}"
    SCORE=$(($SCORE+20))
else
    echo -e "  ${RED}Failed${NC}"
fi

./shell > output 2>&1
sleep 5
if $(grep -q "Incorrect format" output); then
    echo -e "  ${GREEN}Test Five Passed${NC}"
    SCORE=$(($SCORE+6))
else
    echo -e "  ${RED}Failed${NC}"
fi


# print score and delete executable
echo -e "\nSCORE: ${SCORE}/100\n"
make -s clean

exit 0
