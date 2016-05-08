#!/bin/sh

printf "MGMT\nCONNECT\n0.0.0.0:3004\nDONE\n" | nc localhost 3005
printf "MGMT\nCONNECT\n0.0.0.0:3005\nDONE\n" | nc localhost 3006
printf "MGMT\nCONNECT\n0.0.0.0:3004\nDONE\n" | nc localhost 3007
