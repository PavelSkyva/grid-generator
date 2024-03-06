#!/bin/bash

make clean
make

echo "Starting generating..."
ROWS_CAP=10
COLS_CAP=10
ROWS_START=5
COLS_START=5
ROWS=$ROWS_START
COLS=$COLS_START
SAMPLES=1
TIMEOUT=30
SLIPPERY_PROB=0.2
IMPASS_PROB=0.2


while [ $ROWS -le $ROWS_CAP ]; do
	while [ $COLS -le $COLS_CAP ]; do
		./cassandra_generator -rows $ROWS -cols $COLS -samples $SAMPLES --slippery $SLIPPERY_PROB
		./cassandra_generator -rows $ROWS -cols $COLS -samples $SAMPLES --impass $IMPASS_PROB
		((COLS++))
	done
	((ROWS++))
	COLS=$COLS_START
done

echo "Generating complete."
	
cd ..
cd ..
cd ..
cd ..
cd sarsop
cd src

ROWS=$ROWS_START
COLS=$COLS_START


echo "----------------------------------------------------------------------------------------"
echo ""
echo ""
echo ""
echo ""
echo ""
echo "STARTING SARSOP ANALYSIS"
echo ""
echo ""
echo ""
echo ""
echo ""
echo "----------------------------------------------------------------------------------------"


while [ $ROWS -le $ROWS_CAP ]; do
    while [ $COLS -le $COLS_CAP ]; do
        i=1 
        while [ $i -le $SAMPLES ]; do 
            timeout ${TIMEOUT} ./pomdpsol ../../synthesis/models/cassandra/grid-generator/outputs${ROWS}x${COLS}_impassable/output${i}/absorbing.pomdp > ../../synthesis/models/cassandra/grid-generator/outputs${ROWS}x${COLS}_impassable/output${i}/sarsop_output_absorbing.txt
            if [ $? -eq 124 ]; then
			    echo "Analysis of outputs${ROWS}x${COLS}_impassable/output${i}/absorbing.pomdp timed out."
			else
			    echo "Analysis of outputs${ROWS}x${COLS}_impassable/output${i}/absorbing.pomdp completed."
			fi
            timeout ${TIMEOUT} ./pomdpsol ../../synthesis/models/cassandra/grid-generator/outputs${ROWS}x${COLS}_impassable/output${i}/not_absorbing.pomdp > ../..//synthesis/models/cassandra/grid-generator/outputs${ROWS}x${COLS}_impassable/output${i}/sarsop_output_not_absorbing.txt
            if [ $? -eq 124 ]; then
			    echo "Analysis of outputs${ROWS}x${COLS}_impassable/output${i}/not_absorbing.pomdp timed out."
			else
			    echo "Analysis of outputs${ROWS}x${COLS}_impassable/output${i}/not_absorbing.pomdp completed."
			fi
			timeout ${TIMEOUT} ./pomdpsol ../../synthesis/models/cassandra/grid-generator/outputs${ROWS}x${COLS}_slippery/output${i}/absorbing.pomdp > ../..//synthesis/models/cassandra/grid-generator/outputs${ROWS}x${COLS}_slippery/output${i}/sarsop_output_absorbing.txt
			if [ $? -eq 124 ]; then
			    echo "Analysis of outputs${ROWS}x${COLS}_slippery/output${i}/absorbing.pomdp timed out."
			else
			    echo "Analysis of outputs${ROWS}x${COLS}_slippery/output${i}/absorbing.pomdp completed."
			fi
			timeout ${TIMEOUT} ./pomdpsol ../../synthesis/models/cassandra/grid-generator/outputs${ROWS}x${COLS}_slippery/output${i}/not_absorbing.pomdp > ../..//synthesis/models/cassandra/grid-generator/outputs${ROWS}x${COLS}_slippery/output${i}/sarsop_output_not_absorbing.txt
			if [ $? -eq 124 ]; then
			    echo "Analysis of outputs${ROWS}x${COLS}_slippery/output${i}/not_absorbing.pomdp timed out."
			else
			    echo "Analysis of outputs${ROWS}x${COLS}_slippery/output${i}/not_absorbing.pomdp completed."
			fi
            ((i++))
        done
        ((COLS++))
    done
    ((ROWS++))
    COLS=$COLS_START
done 

cd ..
cd ..
cd synthesis
source prerequisites/venv/bin/activate
ROWS=$ROWS_START
COLS=$COLS_START

echo "----------------------------------------------------------------------------------------"
echo ""
echo ""
echo ""
echo ""
echo ""
echo "STARTING PAYNT ANALYSIS"
echo ""
echo ""
echo ""
echo ""
echo ""
echo "----------------------------------------------------------------------------------------"

while [ $ROWS -le $ROWS_CAP ]; do
    while [ $COLS -le $COLS_CAP ]; do
        i=1 
        while [ $i -le $SAMPLES ]; do 
            timeout ${TIMEOUT} python3 paynt.py --project models/cassandra/ --props max_discounted_reward.props --sketch grid-generator/outputs${ROWS}x${COLS}_impassable/output${i}/absorbing.pomdp > ./models/cassandra/grid-generator/outputs${ROWS}x${COLS}_impassable/output${i}/paynt_output_absorbing.txt
            if [ $? -eq 124 ]; then
			    echo "Analysis of outputs${ROWS}x${COLS}_impassable/output${i}/absorbing.pomdp timed out."
			else
			    echo "Analysis of outputs${ROWS}x${COLS}_impassable/output${i}/absorbing.pomdp completed."
			fi
            timeout ${TIMEOUT} python3 paynt.py --project models/cassandra/ --props max_discounted_reward.props --sketch grid-generator/outputs${ROWS}x${COLS}_impassable/output${i}/not_absorbing.pomdp > models/cassandra/grid-generator/outputs${ROWS}x${COLS}_impassable/output${i}/paynt_output_not_absorbing.txt
            if [ $? -eq 124 ]; then
			    echo "Analysis of outputs${ROWS}x${COLS}_impassable/output${i}/not_absorbing.pomdp timed out."
			else
			    echo "Analysis of outputs${ROWS}x${COLS}_impassable/output${i}/not_absorbing.pomdp completed."
			fi
			timeout ${TIMEOUT} python3 paynt.py --project models/cassandra/ --props max_discounted_reward.props --sketch grid-generator/outputs${ROWS}x${COLS}_slippery/output${i}/absorbing.pomdp > models/cassandra/grid-generator/outputs${ROWS}x${COLS}_slippery/output${i}/paynt_output_absorbing.txt
			if [ $? -eq 124 ]; then
			    echo "Analysis of outputs${ROWS}x${COLS}_slippery/output${i}/absorbing.pomdp timed out."
			else
			    echo "Analysis of outputs${ROWS}x${COLS}_slippery/output${i}/absorbing.pomdp completed."
			fi
			timeout ${TIMEOUT} python3 paynt.py --project models/cassandra/ --props max_discounted_reward.props --sketch grid-generator/outputs${ROWS}x${COLS}_slippery/output${i}/not_absorbing.pomdp > models/cassandra/grid-generator/outputs${ROWS}x${COLS}_slippery/output${i}/paynt_output_not_absorbing.txt
			if [ $? -eq 124 ]; then
			    echo "Analysis of outputs${ROWS}x${COLS}_slippery/output${i}/not_absorbing.pomdp timed out."
			else
			    echo "Analysis of outputs${ROWS}x${COLS}_slippery/output${i}/not_absorbing.pomdp completed."
			fi
            ((i++))
        done
        ((COLS++))
    done
    ((ROWS++))
    COLS=$COLS_START
done


#python3 paynt.py --project models/cassandra/ --props max_discounted_reward.props --sketch grid-generator/outputs5x5_impassable/output1/absorbing_1.pomdp


echo ""
echo ""
echo ""
echo ""
echo ""
echo "----------------------------------------------------------------------------------------"
echo "Analysis complete."