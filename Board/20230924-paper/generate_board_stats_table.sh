echo "
\begin{table*}[!htb]
\small
\begin{center}
%\resizebox{1.0\textwidth}{!}{
\begin{tabular}{|l|rr|rr|rrr|}
\hline
Board & Size X & Size Y & \#~Test~Points & \#~Nets & \#~Tests [small] & \#~Tests [medium] & \#~Tests [large] \\\\
\hline"
for folder in $(ls); do
	if [ -d $folder ]; then
		folder_translated=$(echo $folder | tr '_' "+")
		folder_translated="${folder_translated/+/\\_}"
		dimensions_line=$(head -n1 $folder/stat.txt)
		dimensions="${dimensions_line/Board\ size\ \[mm\]/ }"
		dimensions="${dimensions/x/ & }"
		# test points
		n_points=$(grep "Points" $folder/stat.txt | tail -n1)
		n_points="${n_points/\#Points\ \[total\]/ }"
		n_nets=$(grep "Nets" $folder/stat.txt | tail -n1)
		n_nets="${n_nets/\#Nets/ }"
		n_tests_s=$(grep "Test Small" $folder/stat.txt)
		n_tests_s="${n_tests_s/\#Test\ Small/ }"
		n_tests_m=$(grep "Test Medium" $folder/stat.txt)
		n_tests_m="${n_tests_m/\#Test\ Medium/ }"
		n_tests_l=$(grep "Test Large" $folder/stat.txt)
		n_tests_l="${n_tests_l/\#Test\ Large/ }"
		line="$folder_translated & $dimensions & $n_points & $n_nets & $n_tests_s & $n_tests_m & $n_tests_l \\\\"
		echo $line
	fi
done
echo "\hline
\end{tabular}
%}
\caption{
 \label{tab:benchmark}
 Our automatically generated benckmark suite: The table reports the main features for each board, i.e., size (in millimiters), number of test points, and number of nets.
}
\end{center}
\end{table*}
"
