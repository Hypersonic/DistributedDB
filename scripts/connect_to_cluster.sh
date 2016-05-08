#!/bin/sh
cluster_ip=$1
cluster_port=$2

node_ip=$3
node_port=$4

printf "MGMT\nCONNECT\n$cluster_ip:$cluster_port\nDONE\n" | nc $node_ip $node_port
