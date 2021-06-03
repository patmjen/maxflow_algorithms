commandwindow;

solvers={
	'BK',
	'P-DDx2',
	'P-DDx4',
	'P-ARD1',
	'P-PRD',
	'RPR'
	};

solvers_legend={
	'BK',
	'DDx2',
	'DDx4',
	'P-ARD',
	'P-PRD',
	'RPR'
	};

collect_reports;

%% printing
%f = fopen('vision_results.tex','wt');
f = fopen('../../../tex/mixed_maxflow/TR/tex/parallel_results.tex','wt');
fprintf(f,'\\begin{longtable}{|l|c|lr|lr|lr|lr|lr|}\n');
%
%fprintf(f,'\\input{tex/vision_results_caption.tex}\n');
fprintf(f,'%s\n','\caption[Parallel Competition]{Parallel Competition} \label{table_parallel} \\');
fprintf(f,'\\hline\n');
fprintf(f,'%s\n','\multicolumn{1}{|c|}{problem} & BK~\cite{BK04} & \multicolumn{2}{|c|}{DDx2~\cite{Strandmark10}} & \multicolumn{2}{|c|}{DDx4~\cite{Strandmark10}} & \multicolumn{2}{|c|}{P-ARD} & \multicolumn{2}{|c|}{P-PRD} & \multicolumn{2}{|c|}{RPR~\cite{Delong08}}\\*');
fprintf(f,'\\hline\n');
fprintf(f,'\\rowcolor{blue!10}\n');
fprintf(f,'%s\n','  & time & \multicolumn{10}{|>{\columncolor[rgb]{0.9,0.9,1}}c|}{time\ \ \ sweeps}\\*');
%& sw & CPU & sw & CPU & sw & CPU & sw & CPU & sw\\\\* \n');
%fprintf(f,'\\rowcolor{blue!10}\n');
%fprintf(f,'   &   & sweeps & sweeps & sweeps & sweeps \\\\* \n');
fprintf(f,'%s\n','\endfirsthead');
fprintf(f,'%s\n','\multicolumn{12}{c}%');
fprintf(f,'%s\n','{{\bfseries \tablename\ \thetable{} -- continued from previous page}} \\');
fprintf(f,'%s\n','\hline');
%fprintf(f,'%s\n','\hline \multicolumn{1}{|c|}{\textbf{Time (s)}} &');
%fprintf(f,'%s\n','\multicolumn{1}{c|}{\textbf{Triple chosen}} &');
%fprintf(f,'%s\n','\multicolumn{1}{c|}{\textbf{Other feasible triples}} \\ \hline ');
fprintf(f,'%s\n','\endhead');
fprintf(f,'%s\n','\hline \multicolumn{12}{|r|}{{Continued on next page}} \\ \hline');
fprintf(f,'%s\n','\endfoot');
fprintf(f,'%s\n','\hline \hline');
fprintf(f,'%s\n','\endlastfoot');
%
color = 0;
for l=1:size(lines,1)
	line = lines(l,:);
	name = line{7}.name;
	vert = line{7}.vert;
	edges = line{7}.edges;
	if (l>1 && ~strcmp(lines{l,end}.type,lines{l-1,end}.type)) || l==1
		fprintf(f,'\\hline\n');
		fprintf(f,'\\multicolumn{12}{|c|}{\\bf %s}\\\\* \n',lines{l,end}.type(3:end));
		fprintf(f,'\\hline \n');
	end
	if(line{7}.('source_size')>1024^3*2)
		continue;
	end
	if(~isfield(line{4},'cpu'))
		continue;
	end
	color = ~color;
	
	if(color)
		fprintf(f,'\\showrowcolors\n');
		fprintf(f,'\\rowcolor{blue!10}\n');
	else
		fprintf(f,'\\hiderowcolors\n');
	end
	fprintf(f,'%s & ',strrep(name,'_','\_'));
 	%if(vert>0 && edges>0)
	%	fprintf(f,'$%3.1f$ & $%2.1f$ & ',vert/(10^6),edges/vert*2);
	%else
	%	fprintf(f,' &  & ');
	%end
	
	for g=1:length(solvers)
		if(isfield(line{g},'sweeps') && line{g}.sweeps>=1000 && ~isempty(strfind(line{g}.solver,'P-DD')))
			fprintf(f,'%s','{\red\bfseries\sffamily X }');
			line{g}.sweeps = 1000;
		end
		print_time(f,line{g},'cpu',' & ');
		if(g>1)
			print_i(f,line{g},'sweeps');
		end
		if(g>1 && g<length(solvers))
			fprintf(f,' & ');
		end
	end
	fprintf(f,'\\\\* \n');	
% 	if(color)
% 		fprintf(f,'\\showrowcolors\n');
% 		fprintf(f,'\\rowcolor{blue!10}\n');
% 	else
% 		fprintf(f,'\\hiderowcolors\n');
% 	end
	% print problem size
%  	if(color)
% 		fprintf(f,'%s','\multicolumn{3}{|>{\columncolor[rgb]{0.9,0.9,1}}l|}{');		
% 	else
% 		fprintf(f,'%s','\multicolumn{3}{|l|}{');
% 	end
% 	for d = 1:length(line{7}.regulargrid)
% 		fprintf(f,'%i',line{7}.regulargrid(d));
% 		if d<length(line{7}.regulargrid)
% 			fprintf(f,'%s','$\times$');
% 		end
% 	end
% 	if length(line{7}.regulargrid)>0
% 		fprintf(f,'%s',', \ \ ');
% 	end
% 	print_size(f,line{7},'source_size','} & ',1/1024/1024);
	%
	%print sweeps	
% 	fprintf(f,' &  & ');
% 	for g=2:length(solvers)
% 		print_i(f,line{g},'sweeps');
% 		if g<length(solvers)
% 			fprintf(f,' & ');
% 		end
% 	end
% 	fprintf(f,'\\\\ \n');
end
fprintf(f,'\\hline \n');
fprintf(f,'\\end{longtable} \n');
fclose(f);