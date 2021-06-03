function print_size(f,strct,field,suffix,mult)
if ~exist('suffix','var')
	suffix = '';
end
if ~exist('mult','var')
	mult = 1;
end
if(isfield(strct,field))
	v = strct.(field)*mult;
	if(isscalar(v))
		if v>0
			if(v<10)
				fprintf(f,'$%2.1f$MB',v);
			elseif (v<512)
				fprintf(f,'$%3.0f$MB',v);
			elseif (v<10*1024)
				fprintf(f,'$%2.1f$GB',v/1024);
			elseif (v<512*1024)
				fprintf(f,'$%3.0f$GB',v/1024);
			else
				fprintf(f,'$%2.1f$TB',v/1024/1024);
			end
		end
	else
		fprintf(f,'%s',v);
	end
else
end
fprintf(f,suffix);
end
