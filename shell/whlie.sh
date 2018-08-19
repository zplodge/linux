#! /bin/bash

echo "please enter a number between 1 and 10. enter 0 to exit."
read var

while [[ "$var" != 0 ]]
do
	if [ "$var" -lt 5 ]
	then
		echo "too small, try again."
		read var
	elif [ "$var" -gt 5 ]
	then
		echo "too big, try again."
		read var;
	else
		echo "congratulation! you are right."
		exit 0;
	fi
done
