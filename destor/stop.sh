#!/bin/bash

ps -ef | grep "./proxy" | cut -c 9-15 | xargs kill -9
ps -ef | grep "./destor" | cut -c 9-15| xargs kill -9