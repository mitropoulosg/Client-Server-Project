#!/bin/bash

# check for input arguments/numbers
if [ $# -ne 2 ]; then
    echo "Usage: ./create_input.sh politicalParties.txt numLines"
    exit 1
fi

# take input arguments
politicalParties_file="$1"
numLines="$2"

#check if politicalParties.txt exists
if [ ! -f "$politicalParties_file" ]; then
    echo "Error: File $politicalParties_file does not exist."
    exit 1
fi

# this function stores to parties array every party
mapfile -t parties < "$politicalParties_file"


# remove previous file so as new file not to append to previous
rm -f "inputFile"

for (( i=0; i<numLines; i++ )); do
    # generate random first and last names
    #tr -dc to take only characters from a-z and A-Z
    firstName=$(cat /dev/urandom | tr -dc 'a-zA-Z' | head -c $((RANDOM % 10 + 3)))
    lastName=$(cat /dev/urandom | tr -dc 'a-zA-Z' | head -c $((RANDOM % 10 + 3)))


    # Select a random party name
    # Select a random party name
    Index=$((RANDOM % ${#parties[@]}))
    partyName="${parties[$Index]}"

    # Append the line to the inputFile
    echo "${firstName} ${lastName} ${partyName}" >> "inputFile"
done

