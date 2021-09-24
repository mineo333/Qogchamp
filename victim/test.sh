#!/bin/bash

echo "Hello World" >> test_file
sudo insmod ../ghost.ko
sudo rmmod ghost
