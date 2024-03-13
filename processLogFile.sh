#!/bin/bash

pollLog="$1"

# check if the pollLog exists and has appropriate usage rights
if [[ ! -f "$pollLog" || ! -r "$pollLog" ]]; then
  echo "Error: Either input file '$pollLog' does not exist or cannot be read."
  exit 1
fi

# declare an array to store the votes for each party
declare -A partyVotes

while read -r line || [[ -n "$line" ]]; do
   # skip if the line is empty
  if [[ -z "$line" ]]; then
    continue
  fi
   # Split the line into an array of words
  words=($line)

  party="${words[2]}"
    # Increment the vote count for the party
  ((partyVotes[$party]++))
done < "$pollLog"

#prin in pollResustsFile
for party in "${!partyVotes[@]}"; do
  votes="${partyVotes[$party]}"
  echo "$party $votes"
done | sort > "pollResultsFile"