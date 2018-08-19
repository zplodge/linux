#! /bin/bash

################################################
echo "###########function 1 start##############"
say_hello()
{
	echo "hello, the time is:"
}

get_current_time()
{
	say_hello
	current_time=`date`
	echo "$current_time"
}

get_current_time

echo "###########function 1 end##############"

################################################
echo "###########function 2 start##############"
sum()
{
	let "z = $1 + $2"
	return "$z"
}
sum 5 6
#echo the function return value
echo "$?"

echo "###########function 2 end##############"

################################################
echo "###########function 3 start##############"
length()
{
	str=$1
	result=0
	if [ "$str" != " " ];
	then
		result=${#str}
	fi
	echo "$result"
}
len=$(length "abc123")
echo "the string length is $len"

echo "###########function 3 end##############"

################################################
echo "###########function 4 start##############"
func()
{
	echo "the function  has $# parameters."
	echo "all parameters are: $*."
	echo "the script's name is $0."
	echo "the first parameters is: $1."
	echo "the second parameters is: $2."
}
func 1 2 3 4 5 a b c d e
func "hello world" hello world
func 

echo "###########function 4 end##############"

################################################
echo "###########function 5 start##############"
