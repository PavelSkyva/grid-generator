#!/bin/bash

make clean
make

echo "Starting generating..."
ROWS=5
COLS=5
SAMPLES=1
TIMEOUT=20


while [ $ROWS -le 10 ]; do
	while [ $COLS -le 10 ]; do
		./cassandra_generator -rows $ROWS -cols $COLS -samples $SAMPLES --slippery 0.2
		./cassandra_generator -rows $ROWS -cols $COLS -samples $SAMPLES --impass 0.2
		((COLS++))
	done
	((ROWS++))
	COLS=5
done

echo "Generating complete."
	
cd ..
cd ..
cd ..
cd ..
cd sarsop
cd src

ROWS=5
COLS=5


ROWS=5
COLS=5

while [ $ROWS -le 10 ]; do
    while [ $COLS -le 10 ]; do
        i=1  # Initialize i before entering the innermost loop
        while [ $i -le $SAMPLES ]; do 
            timeout ${TIMEOUT} ./pomdpsol ../../synthesis/models/cassandra/grid-generator/outputs${ROWS}x${COLS}_impassable/output${i}/absorbing_${i}.pomdp > ../..//synthesis/models/cassandra/grid-generator/outputs${ROWS}x${COLS}_impassable/output${i}/sarsop_output${i}_absorbing.txt
            if [ $? -eq 124 ]; then
			    echo "Analysis of outputs${ROWS}x${COLS}_impassable/output${i}/absorbing_${i}.pomdp timed out."
			else
			    echo "Analysis of outputs${ROWS}x${COLS}_impassable/output${i}/absorbing_${i}.pomdp completed."
			fi
            timeout ${TIMEOUT} ./pomdpsol ../../synthesis/models/cassandra/grid-generator/outputs${ROWS}x${COLS}_impassable/output${i}/not_absorbing_${i}.pomdp > ../..//synthesis/models/cassandra/grid-generator/outputs${ROWS}x${COLS}_impassable/output${i}/sarsop_output${i}_not_absorbing.txt
            if [ $? -eq 124 ]; then
			    echo "Analysis of outputs${ROWS}x${COLS}_impassable/output${i}/not_absorbing_${i}.pomdp timed out."
			else
			    echo "Analysis of outputs${ROWS}x${COLS}_impassable/output${i}/not_absorbing_${i}.pomdp completed."
			fi
            ((i++))  # Update i within the loop
        done
        ((COLS++))
    done
    ((ROWS++))
    COLS=5
done 







#source prerequisites/venv/bin/activate
#python3 paynt.py --project models/cassandra/ --props max_discounted_reward.props --sketch grid-generator/outputs5x5_impassable/output1/absorbing_1.pomdp



echo "Analysis complete."