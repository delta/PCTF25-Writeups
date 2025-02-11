#!/bin/bash

# Oh, Bats! You really thought you could sneak around my server with your little breadcrumbs? 
# How delightfully naive! I've found your precious little line and turned it into my own twisted joke.
# Your precious files? Encrypted and locked away!
# The clock's ticking, Batsy! 
# It's time to play my game! HaHaHaHaHaHaHaHa!

encrypt_hex() {
    local input=$1
    local key=$2
    
    local input_hex=$(printf "%s" "$input" | xxd -p | tr -d '\n')
    local n=${#input_hex}
    
    local key_hex=$(printf "%s" "$key" | xxd -p | tr -d '\n')
    local temp="$key_hex"
    while [ ${#temp} -lt "$n" ]; do
        temp+="$key_hex"
    done
    local extended_key="${temp:0:$n}"

    local length=${#input_hex}
    local encrypted=""
    
    for ((i=0; i<length; i+=2)); do
        local byte1="${input_hex:$i:2}"
        local byte2="${extended_key:$i:2}"
        local dec1=$((16#$byte1))
        local dec2=$((16#$byte2))
        encrypted+="$hex_result"
    done
    
    echo "$encrypted"
}

main() {
   while IFS= read -r line; do
        [ -z "$line" ] && continue
        
        read -r title value1 value2 <<< "$line"
        
        local key_length=${#title}
        local key=$(head -c "$key_length" /dev/urandom)
        
        local encrypted_title=$(encrypt_hex "$title" "$key")        
        local encrypted1=$(encrypt_hex "$value1" "$key")        
        local encrypted2=$(encrypt_hex "$value2" "$key")
        
        echo "$title $encrypted_title $encrypted1 $encrypted2" >> encrypted_output.log
        
    done < "$input_file"
    
}

input_file="/home/gotham/logs/keys.log"
m1=$(md5sum "$filename")

while true; do
  sleep 1
  m2=$(md5sum "$filename")

  if [ "$m1" != "$m2" ] ; then
    m1=$m2
    main
  fi
done