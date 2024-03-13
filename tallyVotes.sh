#!/bin/bash

inputFile="$1"

# check if the inputFile exists and has appropriate usage rights
if [[ ! -f "$inputFile" || ! -r "$inputFile" ]]; then
  echo "Error: Either input file '$inputFile' does not exist or cannot be read."
  exit 1
fi

# declare an array to store the first two words
names=()
# declare an array to store the votes for each party
declare -A partyVotes

while read -r line || [[ -n "$line" ]]; do
   # skip if the line is empty
  if [[ -z "$line" ]]; then
    continue
  fi
   # Split the line into an array of words
  words=($line)
  # fullname has the full name
  fullname="${words[0]} ${words[1]}"
  # check if the combined word (full name) is already present in the array
  if [[ " ${names[*]} " =~ " $fullname " ]]; then
    continue
  fi
  names+=("$fullname")
  party="${words[2]}"
    # Increment the vote count for the party
  ((partyVotes[$party]++))
done < "$inputFile"

for party in "${!partyVotes[@]}"; do
  votes="${partyVotes[$party]}"
  echo "$party $votes"
done | sort > "tallyResultsFile"


