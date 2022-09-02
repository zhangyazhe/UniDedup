#!/bin/bash

redis-cli flushall

sudo service redis_6379 restart

./OECAgent