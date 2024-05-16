#!/bin/bash

echo "%"
echo "\begin{table*}[!htbp]"
echo "\begin{center}"
echo "\resizebox{0.99\textwidth}{!}{"
echo "\begin{tabular}{|c|r|rrrrrr|rrrrrr|rrr|}"
echo "\hline"
echo " & & \multicolumn{6}{c|}{Top} & \multicolumn{6}{c|}{Bottom} & & & \\\\"
echo " \multicolumn{1}{|c|}{Board} & \multicolumn{1}{c|}{Size} & \#Nofly & \#Notouch & \#GND & \#VDD & \#CLOCK & \#Point & \#Nofly & \#Notouch & \#GND & \#VDD & \#CLOCK & \#Point & \#Test\_1 & \#Test\_2 & \#Test\_3 \\\\"
echo " & \multicolumn{1}{c|}{[mm]} & & & & & & & & & & & & & & & \\\\"
echo "\hline"

for dir in $(ls $1) ; do
    cd $1$dir
    #sed -e "s/board_/BOARD_{/" < "$tmp"
    echo -n "${dir/board_/board\\_}"
    n=0
    while read line ; do
	let n=n+1
	if [ $n -eq 1 -o $n -eq 6 -o $n -eq 7 -o $n -eq 9 -o $n -eq 10 -o $n -eq 11 -o $n -eq 14 -o $n -eq 19 -o $n -eq 20 -o $n -eq 22 -o $n -eq 23 -o $n -eq 24 -o $n -eq 27 -o $n -eq 43 -o $n -eq 44 -o $n -eq 45 ] ; then
	    for word in $line ; do
		:
	    done
	    echo -n " &" $word
	fi
    done < "stat.txt"
    echo " \\\\"
done


echo "\hline"
echo "\end{tabular}"
echo "}"
echo "\caption{"
echo "  \label{tab:benchmark}"
echo "  bla bla bla"
echo "}"
echo "\end{center}"
echo "\end{table*}"

exit 1
