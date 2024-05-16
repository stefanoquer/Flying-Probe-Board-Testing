#/bin/bash

CURRENT_FOLDER=$(pwd)

echo "\begin{table*}[!htbp]
\begin{center}
\resizebox{1.0\textwidth}{!}{
\begin{tabular}{|c|c|cccc|cc|}
\hline
Board & \multicolumn{1}{|c|}{Test Joiner} & \multicolumn{4}{c|}{Distance Crossed [units]} & \multicolumn{2}{c|}{Elapsed time [s]} \\\\
& Reduced Tests [\%] & Total & Avg. between tests & Min. between tests & Max. between tests  & Join & Path finding \\\\
\hline" > result.tex


for folder in $(ls .) ; do
	folder_name="$CURRENT_FOLDER/$folder"
	if [ -d $folder_name ] ; then
		board_name=$(echo "board\\_${folder: -2}")
		perc_red_string=($(cat "$folder_name/Results_s/statsJoin.txt" | grep "Reduction percentage"))
		time_elapsed_str=($(cat "$folder_name/Results_s/statsJoin.txt" | grep "Process"))
		total_path_str=($(cat "$folder_name/Results_s/statsHLRouter.txt" | grep "Total distance"))
		avg_path_str=($(cat "$folder_name/Results_s/statsHLRouter.txt" | grep "Average"))
		min_path_str=($(cat "$folder_name/Results_s/statsHLRouter.txt" | grep "Min"))
		max_path_str=($(cat "$folder_name/Results_s/statsHLRouter.txt" | grep "Max"))
		time_path_str=($(cat "$folder_name/Results_s/statsHLRouter.txt" | grep "duration"))
		perc_red=${perc_red_string[2]::5}
		time_elapsed=${time_elapsed_str[2]}
		total_path=${total_path_str[2]}
		avg_path=${avg_path_str[4]}
		min_path=${min_path_str[2]}
		max_path=${max_path_str[2]}
		time_path=${time_path_str[2]}
		echo "${board_name} & ${perc_red} & ${total_path} & ${avg_path} & ${min_path} & ${max_path} & ${time_elapsed} & ${time_path} \\\\" >> result.tex
	fi
done

echo "\hline
\end{tabular}
}
\caption{
  \label{tab:exp_board_large_tests}
  Experimental results on a large number of tests for each board.
}
\end{center}
\end{table*}

" >> result.tex

echo "\begin{table*}[!htbp]
\begin{center}
\resizebox{1.0\textwidth}{!}{
\begin{tabular}{|c|c|cccc|cc|}
\hline
Board & \multicolumn{1}{|c|}{Test Joiner} & \multicolumn{4}{c|}{Distance Crossed [units]} & \multicolumn{2}{c|}{Elapsed time [s]} \\\\
& Reduced Tests [\%] & Total & Avg. between tests & Min. between tests & Max. between tests  & Join & Path finding \\\\
\hline" >> result.tex


for folder in $(ls .) ; do
	folder_name="$CURRENT_FOLDER/$folder"
	if [ -d $folder_name ] ; then
		board_name=$(echo "board\\_${folder: -2}")
		perc_red_string=($(cat "$folder_name/Results_m/statsJoin.txt" | grep "Reduction percentage"))
		time_elapsed_str=($(cat "$folder_name/Results_m/statsJoin.txt" | grep "Process"))
		total_path_str=($(cat "$folder_name/Results_m/statsHLRouter.txt" | grep "Total distance"))
		avg_path_str=($(cat "$folder_name/Results_m/statsHLRouter.txt" | grep "Average"))
		min_path_str=($(cat "$folder_name/Results_m/statsHLRouter.txt" | grep "Min"))
		max_path_str=($(cat "$folder_name/Results_m/statsHLRouter.txt" | grep "Max"))
		time_path_str=($(cat "$folder_name/Results_m/statsHLRouter.txt" | grep "duration"))
		perc_red=${perc_red_string[2]::5}
		time_elapsed=${time_elapsed_str[2]}
		total_path=${total_path_str[2]}
		avg_path=${avg_path_str[4]}
		min_path=${min_path_str[2]}
		max_path=${max_path_str[2]}
		time_path=${time_path_str[2]}
		echo "${board_name} & ${perc_red} & ${total_path} & ${avg_path} & ${min_path} & ${max_path} & ${time_elapsed} & ${time_path} \\\\" >> result.tex
	fi
done

echo "\hline
\end{tabular}
}
\caption{
  \label{tab:exp_board_medium_tests}
  Experimental results on a medium number of tests for each board.
}
\end{center}
\end{table*}

" >> result.tex


echo "\begin{table*}[!htbp]
\begin{center}
\resizebox{1.0\textwidth}{!}{
\begin{tabular}{|c|c|cccc|cc|}
\hline
Board & \multicolumn{1}{|c|}{Test Joiner} & \multicolumn{4}{c|}{Distance Crossed [units]} & \multicolumn{2}{c|}{Elapsed time [s]} \\\\
& Reduced Tests [\%] & Total & Avg. between tests & Min. between tests & Max. between tests  & Join & Path finding \\\\
\hline" >> result.tex


for folder in $(ls .) ; do
	folder_name="$CURRENT_FOLDER/$folder"
	if [ -d $folder_name ] ; then
		board_name=$(echo "board\\_${folder: -2}")
		perc_red_string=($(cat "$folder_name/Results_l/statsJoin.txt" | grep "Reduction percentage"))
		time_elapsed_str=($(cat "$folder_name/Results_l/statsJoin.txt" | grep "Process"))
		total_path_str=($(cat "$folder_name/Results_l/statsHLRouter.txt" | grep "Total distance"))
		avg_path_str=($(cat "$folder_name/Results_l/statsHLRouter.txt" | grep "Average"))
		min_path_str=($(cat "$folder_name/Results_l/statsHLRouter.txt" | grep "Min"))
		max_path_str=($(cat "$folder_name/Results_l/statsHLRouter.txt" | grep "Max"))
		time_path_str=($(cat "$folder_name/Results_l/statsHLRouter.txt" | grep "duration"))
		perc_red=${perc_red_string[2]::5}
		time_elapsed=${time_elapsed_str[2]}
		total_path=${total_path_str[2]}
		avg_path=${avg_path_str[4]}
		min_path=${min_path_str[2]}
		max_path=${max_path_str[2]}
		time_path=${time_path_str[2]}
		echo "${board_name} & ${perc_red} & ${total_path} & ${avg_path} & ${min_path} & ${max_path} & ${time_elapsed} & ${time_path} \\\\" >> result.tex
	fi
done

echo "\hline
\end{tabular}
}
\caption{
  \label{tab:exp_board_large_tests}
  Experimental results on a large number of tests for each board.
}
\end{center}
\end{table*}

" >> result.tex
