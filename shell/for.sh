#! /bin/bash

echo "please enter a num for 1 to 9."
read var

#for ((i=1; i<=$var; i++))
for i in {1..9}
do
	for((j=1; j<=i; j++))
	do
		let "product=i*j"
		printf "$i * $j=$product"
		if [[ "$product" -gt 9 ]]
		then
			printf "   "
		else
			printf "     "
		fi
		if [[ "$j" -eq "$var" ]]
		then
			break
		fi
	done
	echo
done
