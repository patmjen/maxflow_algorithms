commandwindow;
solvers={
	'BK',
	'GT0',
	'GT05',
	'HPR',
	'S-ARD1',
	'S-PRD'
	};

solvers_legend={
	'BK',
	'HIPR0',
	'HIPR0.5',
	'HPR',
	'S-ARD',
	'S-PRD'
	};

collect_reports;

%% printing
%f = fopen('vision_results.tex','wt');
f = fopen('../../../tex/mixed_maxflow/TR/tex/streaming_results.tex','wt');
fprintf(f,'\\begin{longtable}{|lcc|c|c|c|c|ccc|ccc|}\n');
%
%fprintf(f,'\\input{tex/vision_results_caption.tex}\n');
%fprintf(f,'%s\n','\caption[Problems from~\url{http://vision.csd.uwo.ca/maxflow-data/}]{Problems from~\url{http://vision.csd.uwo.ca/maxflow-data/}} \label{table1} \\');
fprintf(f,'%s\n','\multicolumn{12}{p{\linewidth}}{{\small\bfseries\tablename\ \thetable{}.} \small Sequential Competition. CPU -- the time spent purely for computation, excluding the time for parsing, construction and disk I/O. The total time to solve the problem is not shown. $K$ -- number of regions. RAM -- memory taken by the solver; for BK in the case it exceeds 2GB limit, the expected required memory; for streaming solvers the sum of shared and region memory. I/O -- total bytes read or written to disk. \vspace{1mm}} \label{table1}\\*');
fprintf(f,'\\hline\n');
fprintf(f,'\\multicolumn{3}{|c|}{problem} & BK & HIPR0 & HIPR0.5 & HPR & \\multicolumn{3}{|c|}{S-ARD} & \\multicolumn{3}{|c|}{S-PRD}\\\\* \n');
fprintf(f,'\\hline\n');
fprintf(f,'\\rowcolor{blue!10}\n');
fprintf(f,' name & n($10^6)$ & m/n & CPU & CPU & CPU & CPU & CPU & \\hskip-1mm sweeps\\hskip-1mm &  $K$  & CPU & \\hskip-1mm sweeps\\hskip-1mm &   $K$ \\\\* \n');
fprintf(f,'\\rowcolor{blue!10}\n');
fprintf(f,' size &         &     & RAM & RAM & RAM & RAM & RAM &    & I/O & RAM &    & I/O\\\\* \n');
fprintf(f,'%s\n','\endfirsthead');
fprintf(f,'%s\n','\multicolumn{13}{c}%');
fprintf(f,'%s\n','{{\bfseries \tablename\ \thetable{} -- continued from previous page}} \\');
fprintf(f,'%s\n','\hline');
%fprintf(f,'%s\n','\hline \multicolumn{1}{|c|}{\textbf{Time (s)}} &');
%fprintf(f,'%s\n','\multicolumn{1}{c|}{\textbf{Triple chosen}} &');
%fprintf(f,'%s\n','\multicolumn{1}{c|}{\textbf{Other feasible triples}} \\ \hline ');
fprintf(f,'%s\n','\endhead');
fprintf(f,'%s\n','\hline \multicolumn{13}{|r|}{{Continued on next page}} \\ \hline');
fprintf(f,'%s\n','\endfoot');
fprintf(f,'%s\n','\hline \hline');
fprintf(f,'%s\n','\endlastfoot');
%
%{
f2 = fopen('../../../tex/mixed_maxflow/TR/tex/vision_preprocessing.tex','wt');
fprintf(f2,'%s\n','\begin{tabular}{|lc|lc|lc|lc|}');
fprintf(f2,'%s\n','\hline');
f2c=1;
f2h = ceil(size(lines,1)/4);
for k = 1:f2h
    for j=1:4
        l = (j-1)*f2h+k;
        if(l<=size(lines,1))
            line = lines(l,:);
            fprintf(f2,'%s & ',strrep(line{7}.name,'_','\_'));
            fprintf(f2,'%3.1f%s',line{5}.('preprocess'),'\%');
        else
            fprintf(f2,' & ');
        end
        if j<4
            fprintf(f2,' & ');
        else
            fprintf(f2,'%s\n','\\');
        end       
    end
end
%
% while f2c~=1
%     if f2c<4
%         fprintf(f2,' & ');
%         f2c = f2c+1;
%     else
%         fprintf(f2,'%s\n','\\');
%         f2c = 1;
%     end
% end
fprintf(f2,'%s\n','\hline');
fprintf(f2,'%s\n','\end{tabular}');
fclose(f2);
%}

for l=1:size(lines,1)
	line = lines(l,:);
	name = line{7}.name;
	vert = line{7}.vert;
	edges = line{7}.edges;
	if (l>1 && ~strcmp(lines{l,end}.type,lines{l-1,end}.type)) || l==1
		fprintf(f,'\\hline\n');
		fprintf(f,'\\multicolumn{13}{|c|}{\\bf %s}\\\\* \n',lines{l,end}.type(3:end));
		fprintf(f,'\\hline \n');
	end
	if(mod(l,2)==1)
		fprintf(f,'\\showrowcolors\n');
		fprintf(f,'\\rowcolor{blue!10}\n');
	else
		fprintf(f,'\\hiderowcolors\n');
	end
	fprintf(f,'%s & ',strrep(name,'_','\_'));
 	if(vert>0 && edges>0)
		fprintf(f,'$%3.1f$ & $%2.1f$ & ',vert/(10^6),edges/vert*2);
	else
		fprintf(f,' &  & ');
	end
	
	for g=1:4
		print_time(f,line{g},'cpu',' & ');
	end
	print_time(f,line{5},'cpu',' & ');
	print_i(f,line{5},'sweeps',' & ');
	print_i(f,line{5},'Regions',' & ');
	print_time(f,line{6},'cpu',' & ');
	print_i(f,line{6},'sweeps',' & ');
	print_i(f,line{5},'Regions',' ');
	fprintf(f,'\\\\* \n');
	
	if(mod(l,2)==1)
		fprintf(f,'\\showrowcolors\n');
		fprintf(f,'\\rowcolor{blue!10}\n');
	else
		fprintf(f,'\\hiderowcolors\n');
	end
 	if(mod(l,2)==1)
		fprintf(f,'%s','\multicolumn{3}{|>{\columncolor[rgb]{0.9,0.9,1}}l|}{');		
	else
		fprintf(f,'%s','\multicolumn{3}{|l|}{');
	end
	for d = 1:length(line{7}.regulargrid)
		fprintf(f,'%i',line{7}.regulargrid(d));
		if d<length(line{7}.regulargrid)
			fprintf(f,'%s','$\times$');
		end
	end
	if length(line{7}.regulargrid)>0
		fprintf(f,'%s',', \ \ ');
	end
	print_size(f,line{7},'source_size','} & ',1/1024/1024);
	%print_sizeb(f,line{7},'source_size',' & & & ');
	
	for g=1:4
		print_size(f,line{g},'mem_shared',' & ');
	end
	
	print_size2(f,line{5},'mem_shared','mem_region',' & ');
	fprintf(f,'& ');
	print_size(f,line{5},'disk',' & ');
	print_size2(f,line{6},'mem_shared','mem_region',' & ');
	fprintf(f,'& ');
	print_size(f,line{6},'disk','');
	fprintf(f,'\\\\ \n');

end
fprintf(f,'\\hline \n');
fprintf(f,'\\end{longtable} \n');
fclose(f);