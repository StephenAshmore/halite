#!/bin/bash

cd src
./runGame.sh
cd ..
mv src/*.hlt replays/.
mv src/*.log logs/.
